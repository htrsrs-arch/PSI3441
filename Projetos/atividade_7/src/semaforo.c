#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "semaforo.h"

#define STACK_SIZE 1024
#define PRIORITY 5

volatile int saldo_vitrine_sem = 0;

// contar o espaço livre na vitrine; máximo 10, começa com 10 livres.
K_SEM_DEFINE(sem_vagas, 10, 10);

// contar pães prontos para o cliente; máximo 10, começa com 0 pães.
K_SEM_DEFINE(sem_paes, 0, 10);

// semáforo binário atuando como Mutex para proteger a região crítica (printk e saldo)
K_SEM_DEFINE(mutex_print, 1, 1);


void thread_padeiro_sem(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_sem_take(&sem_vagas, K_FOREVER);
        k_sem_take(&mutex_print, K_FOREVER);
        printk("Padeiro - pão produzido\n");
        saldo_vitrine_sem++;
        printk("Saldo vitrine: %d\n", saldo_vitrine_sem);
        k_sem_give(&mutex_print); 
        k_sem_give(&sem_paes);
        k_msleep(1000);
    }
}

void thread_cliente_sem(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_sem_take(&sem_paes, K_FOREVER);
        k_sem_take(&mutex_print, K_FOREVER);
        printk("Cliente - pão retirado\n");
        saldo_vitrine_sem--;
        printk("Saldo vitrine: %d\n", saldo_vitrine_sem);
        k_sem_give(&mutex_print); 
        k_sem_give(&sem_vagas);
        k_msleep(1500);
    }
}

K_THREAD_DEFINE(thread_padeiro_sem_id, STACK_SIZE, thread_padeiro_sem,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(thread_cliente_sem_id, STACK_SIZE, thread_cliente_sem,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);