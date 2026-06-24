#ifndef ADC_HELPER_H
#define ADC_HELPER_H

#include <zephyr/kernel.h>

int   adc_helper_init(void);
float adc_helper_read_voltage(void);

#endif /* ADC_HELPER_H */
