#include "ultrasonic_helper.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/irq.h>

/* ── Trigger via PWM (Zephyr API) ────────────────────────────────────────── */

#define PWM_TRIGGER_NODE   DT_ALIAS(pwm_trigger)

#if !DT_NODE_HAS_STATUS(PWM_TRIGGER_NODE, okay)
#error "Alias pwm-trigger não encontrado ou desabilitado no app.overlay!"
#endif

static const struct pwm_dt_spec trigger = PWM_DT_SPEC_GET(PWM_TRIGGER_NODE);

/*
 * Período: 60 ms — espaçamento entre medições para evitar ecos residuais.
 * Pulso:   15 µs — satisfaz o requisito mínimo de 10 µs do HC-SR04.
 */
#define TRIGGER_PERIOD_NS  60000000U   /* 60 ms */
#define TRIGGER_PULSE_NS      15000U   /* 15 µs */

/* ── Echo via Input Capture (FTM3 CMSIS) ─────────────────────────────────── */

/*
 * Pino de echo: PTC8 = FTM3_CH4 (ALT3)
 *
 * Clock do FTM3: bus clock da K64F (~60 MHz), prescaler PS=128
 *   → tick ≈ 128 / 60 MHz ≈ 2,13 µs
 *   → conversão: tempo_us = (ticks * 32) / 15
 *
 * Se a conversão parecer errada, calibre medindo um pulso conhecido
 * (ex.: conecte PTC1 → PTC8 e verifique se o resultado é ~15 µs).
 */

static volatile uint32_t t_subida          = 0;
static volatile uint32_t t_descida         = 0;
static volatile uint32_t largura_pulso_ticks = 0;
static volatile bool     captura_pronta    = false;
static volatile uint8_t  borda_atual       = 0;

static void ftm3_isr(void *arg)
{
    ARG_UNUSED(arg);

    /* Limpa a flag de interrupção do canal 4 */
    FTM3->CONTROLS[4].CnSC &= ~FTM_CnSC_CHF_MASK;

    if (borda_atual == 0) {
        /* Borda de subida: registra timestamp e arma para borda de descida */
        t_subida = FTM3->CONTROLS[4].CnV;
        FTM3->CONTROLS[4].CnSC =
            (FTM3->CONTROLS[4].CnSC & ~(FTM_CnSC_ELSA_MASK | FTM_CnSC_ELSB_MASK))
            | FTM_CnSC_ELSB_MASK | FTM_CnSC_CHIE_MASK;
        borda_atual = 1;
    } else {
        /* Borda de descida: calcula largura do pulso (trata wraparound 16-bit) */
        t_descida = FTM3->CONTROLS[4].CnV;
        if (t_descida >= t_subida) {
            largura_pulso_ticks = t_descida - t_subida;
        } else {
            largura_pulso_ticks = (0xFFFFU - t_subida) + t_descida + 1U;
        }
        captura_pronta = true;

        /* Volta a armar para borda de subida */
        FTM3->CONTROLS[4].CnSC =
            (FTM3->CONTROLS[4].CnSC & ~(FTM_CnSC_ELSA_MASK | FTM_CnSC_ELSB_MASK))
            | FTM_CnSC_ELSA_MASK | FTM_CnSC_CHIE_MASK;
        borda_atual = 0;
    }
}

/* ── API pública ─────────────────────────────────────────────────────────── */

int ultrasonic_helper_init(void)
{
    /* --- Trigger via Zephyr PWM API --- */
    printk("Ultrassonico: iniciando trigger PWM (PTC1 / FTM0_CH0)...\n");

    if (!pwm_is_ready_dt(&trigger)) {
        printk("Ultrassonico: erro — dispositivo PWM nao esta pronto.\n");
        return -ENODEV;
    }

    int ret = pwm_set_dt(&trigger, TRIGGER_PERIOD_NS, TRIGGER_PULSE_NS);
    if (ret != 0) {
        printk("Ultrassonico: erro ao configurar PWM (ret=%d).\n", ret);
        return ret;
    }

    printk("Ultrassonico: trigger OK — periodo 60 ms, pulso 15 us.\n");

    /* --- Echo via FTM3 Input Capture (CMSIS) --- */
    printk("Ultrassonico: iniciando input capture (PTC8 / FTM3_CH4)...\n");

    /* Habilita clocks do FTM3 e PORTC */
    SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

    /* Multiplexa PTC8 para FTM3_CH4 (ALT3) */
    PORTC->PCR[8] = PORT_PCR_MUX(3);

    /* Registra e habilita a ISR do FTM3 */
    IRQ_CONNECT(FTM3_IRQn, 1, ftm3_isr, NULL, 0);
    irq_enable(FTM3_IRQn);

    /* Configura o canal 4 em Input Capture — borda de subida + interrupção */
    FTM3->MOD = 0xFFFFU;
    FTM3->CONTROLS[4].CnSC = FTM_CnSC_ELSA_MASK | FTM_CnSC_CHIE_MASK;

    /* Inicia o contador: clock do barramento, prescaler 128 */
    FTM3->SC |= FTM_SC_CLKS(1) | FTM_SC_PS(7);

    printk("Ultrassonico: input capture OK.\n");
    return 0;
}

void read_and_display_ultrasonic(void)
{
    if (!captura_pronta) {
        return;
    }

    captura_pronta = false;

    /*
     * Conversão de ticks para µs:
     *   bus_clock = 60 MHz, PS = 128
     *   tick = 128 / 60e6 s = 2,133 µs = 32/15 µs
     *   tempo_us = ticks × 32 / 15
     */
    uint32_t tempo_us    = (largura_pulso_ticks * 32U) / 15U;
    uint32_t distancia_cm = tempo_us / 58U;

    printk("Pulso eco: %u us | Distancia: %u cm\n", tempo_us, distancia_cm);
}
