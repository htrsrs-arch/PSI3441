#include "MKL25Z4.h"
#define LED_VERDE_PIN   19  // PTB19
#define LED_AZUL_PIN    1   // PTD1

void GPIO_Init(void) {
    // ativa o clock para as portas PORTB e PORTD (OR)
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;

    // configura o pino PTB0 como entrada do ADC (MUX 0 - Pin Disabled / Analog; AND NOT)
    PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK; 

    // configura os pinos dos LEDs como GPIO (MUX 1)
    PORTB->PCR[LED_VERDE_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_AZUL_PIN]  = PORT_PCR_MUX(1);

    // onfigura a direção dos pinos dos LEDs como Saída
    PTB->PDDR |= (1UL << LED_VERDE_PIN);
    PTD->PDDR |= (1UL << LED_AZUL_PIN);

    // inicia com os dois LEDs desligados
    PTB->PSOR = (1UL << LED_VERDE_PIN);
    PTD->PSOR = (1UL << LED_AZUL_PIN);
}

void ADC0_Init(void) {
    // ativa o clock para o ADC0
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

    // software trigger (bit ADTRG = 0)
    ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;

    // fonte de clock (bus clock * 1/2) e resolução (10 bits, mode 0b10)
    ADC0->CFG1 = ADC_CFG1_MODE(2) | ADC_CFG1_ADICLK(1);
}

uint16_t ADC0_Read(uint8_t channel) {
    // seleciona o canal e inicia a conversão
    ADC0->SC1[0] = channel & ADC_SC1_ADCH_MASK;

    // flag de fim de conversão (COCO)
    while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK)) {
        // Aguarda a conversão terminar
    }

    // lê o resultado (zera o flag COCO)
    return (uint16_t)ADC0->R[0];
}

int main(void) {
    uint16_t adc_value = 0;

    GPIO_Init();
    ADC0_Init();

    while (1) {
        // leitura no canal 8 (PTB0)
        adc_value = ADC0_Read(8);

        // configuração dosLEDs (resolução de 0 a 1023)
        // próximo de 0V (~100)
        if (adc_value < 100) {
            PTB->PCOR = (1UL << LED_VERDE_PIN);  // Liga o LED Verde
            PTD->PSOR = (1UL << LED_AZUL_PIN);   // Desliga o LED Azul
        }
        // próximo de 3.3V (~923)
        else if (adc_value > 923) {
            PTD->PCOR = (1UL << LED_AZUL_PIN);   // Liga o LED Azul
            PTB->PSOR = (1UL << LED_VERDE_PIN);  // Desliga o LED Verde
        }
        // deadzone
        else {
            PTB->PSOR = (1UL << LED_VERDE_PIN);
            PTD->PSOR = (1UL << LED_AZUL_PIN);
        }
    }
}