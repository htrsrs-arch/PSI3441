#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "button_helper.h"

#define LED_C_NODE  DT_ALIAS(led1) // LED verde
#define BUTTON_NODE DT_ALIAS(sw0)  // botão SW3 da placa

static const struct gpio_dt_spec ledc = GPIO_DT_SPEC_GET(LED_C_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;

volatile modo_exibicao_t modo_atual = MODO_ADC;

static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    static int64_t ultimo_tempo_press = 0;
    int64_t tempo_atual = k_uptime_get();

    if ((tempo_atual - ultimo_tempo_press) > 200) {
        gpio_pin_toggle_dt(&ledc);

        if (modo_atual == MODO_ADC) {
            modo_atual = MODO_COMPLETO;
            printk("\n>>> MODO: COMPLETO (ADC + Acelerometro) <<<\n\n");
        } else {
            modo_atual = MODO_ADC;
            printk("\n>>> MODO: APENAS ADC <<<\n\n");
        }
        
        ultimo_tempo_press = tempo_atual;
    }
}

int button_helper_init(void)
{
    if (!device_is_ready(ledc.port) || !device_is_ready(button.port)) {
        return -ENODEV;
    }

    gpio_pin_configure_dt(&ledc, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    
    return 0;
}