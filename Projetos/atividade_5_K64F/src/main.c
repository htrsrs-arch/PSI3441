#include <zephyr/kernel.h>
#include "ultrasonic_helper.h"

#define STACK_SIZE      1024
#define ULTRASONIC_PRIORITY 6

void ultrasonic_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    if (ultrasonic_helper_init() != 0) {
        printk("Thread do ultrassonico abortada\n");
        return;
    }
    while (1) {
        read_and_display_ultrasonic();
        k_sleep(K_MSEC(100));
    }
}

K_THREAD_DEFINE(us_thread_id,    STACK_SIZE, ultrasonic_thread_entry, NULL, NULL, NULL, ULTRASONIC_PRIORITY, 0, 0);

int main(void)
{
    printk("Sistema iniciado\n");
    
    while (1) {
        k_sleep(K_FOREVER); 
    }
    
    return 0;
}