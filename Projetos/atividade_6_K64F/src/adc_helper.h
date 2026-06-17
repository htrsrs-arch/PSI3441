#ifndef ADC_HELPER_H_
#define ADC_HELPER_H_

#include <zephyr/device.h>

int adc_helper_init(void);
void read_and_display_adc(void);

#endif