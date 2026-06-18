#ifndef ULTRASONIC_HELPER_H_
#define ULTRASONIC_HELPER_H_

#include <stdint.h>

/*
 * Inicializa o sensor HC-SR04:
 *   - Trigger: PWM contínuo via FTM0_CH0 (PTC1), período 60 ms, pulso 15 µs
 *   - Echo:    Input capture via FTM3_CH4 (PTC8), prescaler 128
 *
 * Retorna 0 em sucesso, negativo em erro.
 */
int ultrasonic_helper_init(void);

/*
 * Lê a última captura do eco e imprime a largura do pulso (µs) e a distância (cm)
 * via printk. Não faz medição nova — o FTM3 captura continuamente via ISR.
 */
void read_and_display_ultrasonic(void);

#endif /* ULTRASONIC_HELPER_H_ */
