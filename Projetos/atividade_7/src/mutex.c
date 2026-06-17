#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "mutex.h"

#define STACK_SIZE 1024
#define PRIORITY 5

volatile int saldo_vitrine = 0;

K_MUTEX_DEFINE(print_mutex);

void thread_padeiro_mutex(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_mutex_lock(&print_mutex, K_FOREVER);
        printk("Padeiro - pão produzido (mutex bloqueado)\n");
        saldo_vitrine++;
        printk("Saldo vitrine: %d (mutex liberado)\n",saldo_vitrine);
        k_mutex_unlock(&print_mutex);
        k_msleep(1000);
    }
}

void thread_cliente_mutex(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_mutex_lock(&print_mutex, K_FOREVER);
        printk("Cliente - pão retirado (mutex bloqueado)\n");
        saldo_vitrine--;
        printk("Saldo vitrine: %d (mutex liberado)\n",saldo_vitrine);
        k_mutex_unlock(&print_mutex);
        k_msleep(1500);
    }
}

K_THREAD_DEFINE(thread_padeiro_id, STACK_SIZE, thread_padeiro_mutex,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(thread_cliente_id, STACK_SIZE, thread_cliente_mutex,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);