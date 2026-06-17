#include <zephyr/kernel.h>
#include "adc_helper.h"
#include "accel_helper.h"
#include "button_helper.h"

#define STACK_SIZE 1024
#define ADC_PRIORITY 7
#define ACCEL_PRIORITY 5

void adc_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    if (adc_helper_init() != 0) {
        printk("Thread do ADC abortada\n");
        return;
    }
    while (1) {
        read_and_display_adc();
        k_sleep(K_MSEC(500));
    }
}

void accel_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    uint32_t tempo_ms = 0;

    if (accel_helper_init() != 0) {
        printk("Thread do Acelerometro abortada\n");
        return;
    }
    k_sleep(K_MSEC(1000));
    tempo_ms += 1000;

    while (1) {
        if (modo_atual == MODO_COMPLETO) {
            read_and_display_accel(&tempo_ms);
        }
        k_sleep(K_MSEC(1000));
        tempo_ms += 1000;
    }
}

K_THREAD_DEFINE(adc_thread_id, STACK_SIZE, adc_thread_entry, NULL, NULL, NULL, ADC_PRIORITY, 0, 0);
K_THREAD_DEFINE(accel_thread_id, STACK_SIZE, accel_thread_entry, NULL, NULL, NULL, ACCEL_PRIORITY, 0, 0);

int main(void)
{
    printk("Sistema iniciado\n");
    
    if (button_helper_init() != 0) {
        printk("Erro: Falha ao inicializar o botao e LED\n");
    } else {
        printk("Botao e LED configurados com sucesso\n");
    }

    printk("Thread do ADC rodando em background...\n");
    
    while (1) {
        k_sleep(K_FOREVER); 
    }
    
    return 0;
}