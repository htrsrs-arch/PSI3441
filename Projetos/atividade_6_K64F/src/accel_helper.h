#ifndef ACCEL_HELPER_H_
#define ACCEL_HELPER_H_

#include <stdint.h>

int accel_helper_init(void);
void read_and_display_accel(uint32_t *tempo_ms);

#endif