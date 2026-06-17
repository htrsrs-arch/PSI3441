#include "accel_helper.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdlib.h> 

// acelerômetro da K64F (fxos8700)
static const struct device *const accel = DEVICE_DT_GET(DT_NODELABEL(fxos8700));

int accel_helper_init(void)
{
    if (!device_is_ready(accel)) {
        printk("Erro: Acelerometro FXOS8700 nao esta pronto\n");
        return -ENODEV;
    }

    printk("Acelerometro FXOS8700 inicializado\n");
    return 0;
}

void read_and_display_accel(uint32_t *tempo_ms)
{
    struct sensor_value accel_x, accel_y, accel_z;
    
    if (!accel) {
        printk("Erro: Sensor nao inicializado\n");
        return;
    }

    int ret = sensor_sample_fetch(accel);
    if (ret) {
        printk("Erro ao ler sensor: %d\n", ret);
        return;
    }

    sensor_channel_get(accel, SENSOR_CHAN_ACCEL_X, &accel_x);
    sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Y, &accel_y);
    sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Z, &accel_z);

    printk("T: %u, X: %d.%06d, Y: %d.%06d, Z: %d.%06d\r\n", 
           *tempo_ms,
           accel_x.val1, abs(accel_x.val2),
           accel_y.val1, abs(accel_y.val2),
           accel_z.val1, abs(accel_z.val2));
}