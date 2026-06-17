#include "adc_helper.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>

#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL_ID       0  
#define ADC_VREF_MV          3300

static const struct device *adc_dev;
static int16_t sample_buffer;
static struct adc_sequence sequence;

int adc_helper_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));
    if (!device_is_ready(adc_dev)) {
        printk("Erro: ADC0 nao esta pronto.\n");
        return -ENODEV;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain = ADC_GAIN,
        .reference = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id = ADC_CHANNEL_ID,
        .differential = 0,
    };

    int err = adc_channel_setup(adc_dev, &channel_cfg);
    if (err != 0) {
        printk("Erro ao configurar canal ADC: %d\n", err);
        return err;
    }

    sequence.channels    = BIT(ADC_CHANNEL_ID);
    sequence.buffer      = &sample_buffer;
    sequence.buffer_size = sizeof(sample_buffer);
    sequence.resolution  = ADC_RESOLUTION;

    return 0;
}

void read_and_display_adc(void)
{
    if (!adc_dev) return;

    int err = adc_read(adc_dev, &sequence);
    if (err != 0) {
        printk("Falha na leitura do ADC: %d\n", err);
    } else {
        int32_t mv = sample_buffer;
        adc_raw_to_millivolts(ADC_VREF_MV, ADC_GAIN, ADC_RESOLUTION, &mv);
        printk("ADC: %d (raw), %d mV\n", sample_buffer, mv);
    }
}