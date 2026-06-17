#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "simples.h"

#define STACK_SIZE 1024
#define PRIORITY 5

volatile int saldo_vitrine_1 = 0;
volatile int saldo_vitrine_2 = 0;
volatile int saldo_vitrine_3 = 0;

// Mesma prioridade

void thread_padeiro_1(void *arg1, void *arg2, void *arg3) {
    while (1) {
        printk("Padeiro - pão produzido\n");
        saldo_vitrine_1++;
        printk("Saldo vitrine 1: %d\n",saldo_vitrine_1);
        k_msleep(1000);
    }
}

void thread_cliente_1(void *arg1, void *arg2, void *arg3) {
    while (1) {
        printk("Cliente - pão retirado\n");
        saldo_vitrine_1--;
        printk("Saldo vitrine 1: %d\n",saldo_vitrine_1);
        k_msleep(1500);
    }
}

K_THREAD_DEFINE(thread_padeiro_1_id, STACK_SIZE, thread_padeiro_1,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(thread_cliente_1_id, STACK_SIZE, thread_cliente_1,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

// Mesma prioridade, tempos alterados

void thread_padeiro_2(void *arg1, void *arg2, void *arg3) {
    while (1) {
        printk("Padeiro - pão produzido\n");
        saldo_vitrine_2++;
        printk("Saldo vitrine 2: %d\n",saldo_vitrine_2);
        k_msleep(1500);
    }
}

void thread_cliente_2(void *arg1, void *arg2, void *arg3) {
    while (1) {
        printk("Cliente - pão retirado\n");
        saldo_vitrine_2--;
        printk("Saldo vitrine 2: %d\n",saldo_vitrine_2);
        k_msleep(1000);
    }
}

K_THREAD_DEFINE(thread_padeiro_2_id, STACK_SIZE, thread_padeiro_2,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(thread_cliente_2_id, STACK_SIZE, thread_cliente_2,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

// Padeiro com prioridade maior e cliente com prioridade menor

void thread_padeiro_3(void *arg1, void *arg2, void *arg3) {
    while (1) {
        printk("Padeiro - pão produzido\n");
        saldo_vitrine_3++;
        printk("Saldo vitrine 3: %d\n",saldo_vitrine_3);
        k_msleep(1000);
    }
}

void thread_cliente_3(void *arg1, void *arg2, void *arg3) {
    while (1) {
        printk("Cliente - pão retirado\n");
        saldo_vitrine_3--;
        printk("Saldo vitrine 3: %d\n",saldo_vitrine_3);
        k_msleep(1500);
    }
}

K_THREAD_DEFINE(thread_padeiro_3_id, STACK_SIZE, thread_padeiro_3,
                NULL, NULL, NULL,
                PRIORITY+1, 0, 0);

K_THREAD_DEFINE(thread_cliente_3_id, STACK_SIZE, thread_cliente_3,
                NULL, NULL, NULL,
                PRIORITY-1, 0, 0);