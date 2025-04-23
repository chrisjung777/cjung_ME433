#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 15
#define BUTTON_PIN 14

volatile int button_press_count = 0;

void button_callback(uint gpio, uint32_t events) {
    button_press_count++;
    gpio_xor_mask(1u << LED_PIN); 
    printf("Button pressed %d times\n", button_press_count);
}

int main() {
    stdio_init_all(); 

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); 

    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    while (true) {
        tight_loop_contents(); 
    }

    return 0;
}
