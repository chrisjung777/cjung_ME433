#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 15
#define BUTTON_PIN 14

volatile int button_press_count = 0;

// IRQ callback for button press
void button_callback(uint gpio, uint32_t events) {
    button_press_count++;
    gpio_xor_mask(1u << LED_PIN); // Toggle LED
    printf("Button pressed %d times\n", button_press_count);
}

int main() {
    stdio_init_all(); // Initialize USB serial output

    // Initialize LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // Initialize button pin
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // Enable internal pull-up resistor

    // Enable interrupt on falling edge (button press)
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Main loop just idles
    while (true) {
        tight_loop_contents(); // Prevent the CPU from sleeping too deep
    }

    return 0;
}
