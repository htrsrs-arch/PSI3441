#ifndef BUTTON_HELPER_H_
#define BUTTON_HELPER_H_

typedef enum {
    MODO_ADC,       
    MODO_COMPLETO   
} modo_exibicao_t;

extern volatile modo_exibicao_t modo_atual;

int button_helper_init(void);

#endif /* BUTTON_HELPER_H_ */