#include "adc_helper.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>

/* ADC0_SE12 = PTB2 = pino A0 do header Arduino da FRDM-K64F */
#define ADC_CHANNEL_ID  12
#define ADC_RESOLUTION  12
#define ADC_VREF_MV     3300

static const struct device *adc_dev;
static int16_t adc_buf;

static const struct adc_channel_cfg ch0_cfg = {
    .gain             = ADC_GAIN_1,
    .reference        = ADC_REF_VDD_1,   /* VDDIO = 3,3 V na K64F */
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = ADC_CHANNEL_ID,
};

static struct adc_sequence sequence = {
    .channels    = BIT(ADC_CHANNEL_ID),
    .buffer      = &adc_buf,
    .buffer_size = sizeof(adc_buf),
    .resolution  = ADC_RESOLUTION,
};

int adc_helper_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

    if (!device_is_ready(adc_dev)) {
        printk("Erro: ADC0 nao esta pronto\n");
        return -ENODEV;
    }

    int ret = adc_channel_setup(adc_dev, &ch0_cfg);
    if (ret < 0) {
        printk("Erro ao configurar canal ADC: %d\n", ret);
        return ret;
    }

    printk("ADC0 canal %d inicializado (12-bit, 3.3V ref)\n", ADC_CHANNEL_ID);
    return 0;
}

float adc_helper_read_voltage(void)
{
    int ret = adc_read(adc_dev, &sequence);
    if (ret < 0) {
        return 0.0f;
    }

    int32_t raw = adc_buf;
    adc_raw_to_millivolts(ADC_VREF_MV, ADC_GAIN_1, ADC_RESOLUTION, &raw);
    return (float)raw / 1000.0f;
}
