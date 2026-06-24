#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include "adc_helper.h"

LOG_MODULE_REGISTER(adc_stress, LOG_LEVEL_INF);

#define STACK_SIZE 1024

// --- ENGENHARIA DE TEMPO REAL: PRIORIDADES ASSIMÉTRICAS ---
// A aquisição precisa ser mais prioritária (número menor) para esmagar a comunicação
#define ACQ_PRIORITY  5
#define COMM_PRIORITY 7

struct adc_combined_data {
    uint32_t timestamp;
    float ch0_raw;
    float ch0_filt;
};

// Fila propositalmente pequena (2 posições) para estourar facilmente sob estresse
K_MSGQ_DEFINE(msgq_combined, sizeof(struct adc_combined_data), 2, 4);

static volatile uint32_t dropped_samples = 0;
static volatile uint32_t successful_samples = 0;

// --- CONFIGURAÇÃO DO FILTRO DE ESTRESSE COMPUTACIONAL ---
// Uma janela imensa força o algoritmo de ordenação a consumir milissegundos da CPU
#define STRESS_TAPS 100
static float stress_history[STRESS_TAPS] = {0};
static float sort_buffer[STRESS_TAPS]    = {0};
static int   stress_idx                  = 0;

// Filtro não-linear pesado (Mediana) usando Insertion Sort O(N^2)
float apply_stress_filter(float in_val)
{
    stress_history[stress_idx] = in_val;
    stress_idx = (stress_idx + 1) % STRESS_TAPS;

    // Copia o histórico para o buffer que será ordenado
    for (int i = 0; i < STRESS_TAPS; i++) {
        sort_buffer[i] = stress_history[i];
    }

    // Algoritmo Insertion Sort pesado executado na CPU a cada amostragem
    for (int i = 1; i < STRESS_TAPS; i++) {
        float key = sort_buffer[i];
        int j = i - 1;
        while (j >= 0 && sort_buffer[j] > key) {
            sort_buffer[j + 1] = sort_buffer[j];
            j = j - 1;
        }
        sort_buffer[j + 1] = key;
    }

    // Retorna o valor central (mediana)
    return sort_buffer[STRESS_TAPS / 2];
}

// [x] THREAD DE AQUISIÇÃO (PRODUTOR DE ALTA PRIORIDADE)
void acq_thread_entry(void *p1, void *p2, void *p3)
{
    struct adc_combined_data data;
    LOG_INF("Thread de Aquisicao sob ESTRESSE inicializada.");

    while (1) {
        data.timestamp = k_uptime_get_32();
        data.ch0_raw   = adc_helper_read_voltage();

        // 1. O filtro roda AQUI e rouba tempo precioso da CPU
        data.ch0_filt  = apply_stress_filter(data.ch0_raw);

        // 2. Tenta colocar na fila. Se a comunicação estiver atrasada, estoura e incrementa perda
        int ret = k_msgq_put(&msgq_combined, &data, K_NO_WAIT);
        if (ret == -ENOMSG) {
            dropped_samples++;
        }

        // Sleep agressivo de 2ms para sufocar o escalonador do RTOS
        k_sleep(K_MSEC(5));
    }
}

// [x] THREAD DE COMUNICAÇÃO (CONSUMIDOR DE BAIXA PRIORIDADE)
void comm_thread_entry(void *p1, void *p2, void *p3)
{
    struct adc_combined_data data;
    uint32_t last_report_time = k_uptime_get_32();

    LOG_INF("Thread de Comunicacao inicializada.");

    while (1) {
        // Fica bloqueada aguardando dados da fila
        if (k_msgq_get(&msgq_combined, &data, K_FOREVER) == 0) {
            
            int r_int  = (int)data.ch0_raw;
            int r_frac = __builtin_abs((int)((data.ch0_raw  - r_int) * 1000000));
            int f_int  = (int)data.ch0_filt;
            int f_frac = __builtin_abs((int)((data.ch0_filt - f_int) * 1000000));

            // O printk via UART é lento (~4ms). Como esta thread tem menor prioridade, 
            // ela será interrompida constantemente no meio do print pela thread de aquisição.
            LOG_INF("T: %u, CH0R: %d.%06d, CH0F: %d.%06d",
                    data.timestamp, r_int, r_frac, f_int, f_frac);

            successful_samples++;

            // Emite o relatório de degradação a cada 1 segundo no terminal do VS Code
            uint32_t now = k_uptime_get_32();
            if (now - last_report_time >= 1000) {
                uint32_t elapsed        = now - last_report_time;
                uint32_t effective_rate = (successful_samples * 1000) / elapsed;

                LOG_INF("[SISTEMA SATURADO] Taxa Efetiva: %u Hz | MENSAGENS PERDIDAS: %u",
                        effective_rate, dropped_samples);

                successful_samples = 0;
                dropped_samples    = 0;
                last_report_time   = now;
            }
        }
    }
}

// Definição das Threads com as novas prioridades de estresse
K_THREAD_DEFINE(acq_thread_id, STACK_SIZE, acq_thread_entry, NULL, NULL, NULL, ACQ_PRIORITY, 0, 0);
K_THREAD_DEFINE(comm_thread_id, STACK_SIZE, comm_thread_entry, NULL, NULL, NULL, COMM_PRIORITY, 0, 0);

int main(void)
{
    if (adc_helper_init() != 0) {
        LOG_ERR("Erro ao inicializar ADC.");
        return -1;
    }
    return 0;
}