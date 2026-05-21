#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/console/console.h>
#include <stdlib.h>
#include "pwm_z42.h"

#define TPM_MODULE 1000 

int main(void)
{
    // inicializa o TPM2 com clock da placa e prescaler de 128
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    
    // canal 0 (LED vermelho)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_L, GPIOB, 18);
    // canal 1 (LED verde)
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_L, GPIOB, 19);

    pwm_tpm_CnV(TPM2, 0, 0);
    pwm_tpm_CnV(TPM2, 1, 0);

    // terminal do Zephyr
    console_getline_init();
    printk("=== Controle de Cor: Laranja (Hibrido) ===\n");

    while (1) {
        printk("\nDigite a intensidade total (0 a 100%%): ");
        
        char *input_str = console_getline();
        int intensidade = atoi(input_str);

        // limite de intensidade
        if (intensidade < 0) intensidade = 0;
        if (intensidade > 100) intensidade = 100;

        printk("Ajustando brilho global para: %d%%\n", intensidade);

        // cálculo de Duty Cycle para a cor laranja
        // vermelho em 100% e verde em 30%
        
        uint16_t pulse_red = (TPM_MODULE * intensidade) / 100;
        
        uint16_t pulse_green = (TPM_MODULE * intensidade * 30) / (100 * 100);

        // envia os valores para os pinos
        pwm_tpm_CnV(TPM2, 0, pulse_red);
        pwm_tpm_CnV(TPM2, 1, pulse_green);
    }

    return 0;
}