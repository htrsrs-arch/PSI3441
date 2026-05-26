// registrador de clock
#define SIM_SCGC5      (*((volatile unsigned int*)0x40048038))

// PCR do pino 19 no PORTB (endereço base do PORTB 0x4004A000 + offset do pino 19 0x4C)
#define PORTB_PCR19    (*((volatile unsigned int*)0x4004A04C))

// registradores do módulo GPIO da porta B (endereço base 0x400FF040 + offset)
#define PTB_PDDR       (*((volatile unsigned int*)0x400FF054)) // direção de dados
#define PTB_PCOR       (*((volatile unsigned int*)0x400FF048)) // limpar pino (coloca em 0)
#define PTB_PSOR       (*((volatile unsigned int*)0x400FF044)) // setar pino (coloca em 1)

void delayMs(int n) {
    volatile int i;

    for (i = 0; i < n; i++) {
        for (volatile int j = 0; j < 7000; j++) {
        }
    }
}

int main(void)
{
    // habilitar clock da porta B (bit 10 do SIM_SCGC5 -> valor hexadecimal 0x400)
    SIM_SCGC5 |= 0x400;

    // configurar pino 19 como GPIO (mux em "Alternative 1" -> bit 8 em 1 -> valor 0x100)
    PORTB_PCR19 = (1 << 8);

    // direção do Pino 19 como saída (bit 19 em 1)
    PTB_PDDR |= (1 << 19);

    PTB_PSOR = (1 << 19); // começa apagado

    while (1) {
        // habilitar saída (ligar o LED) - lógica active-low, nível lógico 0
        PTB_PCOR = (1 << 19);

        delayMs(1000);

        // desabilitar saída (apagar o LED), nível lógico 1
        PTB_PSOR = (1 << 19);

        delayMs(1000);
    }

    return 0;
}