#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define FLAG_ADC_READ 0
#define FLAG_LED_ON   1
#define FLAG_LED_OFF  2
#define FLAG_DONE     3

volatile uint16_t adc_result = 0;

void core1_entry() {
    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 0);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0); 
    while (1) {
        uint32_t cmd = multicore_fifo_pop_blocking(); 

        if (cmd == FLAG_ADC_READ) {
            adc_result = adc_read(); 
        } else if (cmd == FLAG_LED_ON) {
            gpio_put(15, 1); 
        } else if (cmd == FLAG_LED_OFF) {
            gpio_put(15, 0);
        }

        multicore_fifo_push_blocking(FLAG_DONE); 
    }
}

int main() {
    stdio_init_all();
    sleep_ms(1000); 
    printf("=== Multicore HW9 Ready ===\n");

    multicore_launch_core1(core1_entry);

    char input;

    while (1) {
        printf("\nCommands:\n");
        printf(" 0 - Read ADC voltage (GPIO26)\n");
        printf(" 1 - Turn ON LED (GP15)\n");
        printf(" 2 - Turn OFF LED (GP15)\n");
        printf("Enter command: ");

        scanf(" %c", &input);

        if (input == '0') {
            multicore_fifo_push_blocking(FLAG_ADC_READ);
            multicore_fifo_pop_blocking(); 
            float voltage = adc_result * 3.3f / 4095.0f;
            printf("ADC Voltage on GPIO26: %.2f V\n", voltage);
        } else if (input == '1') {
            multicore_fifo_push_blocking(FLAG_LED_ON);
            multicore_fifo_pop_blocking();
            printf("LED turned ON\n");
        } else if (input == '2') {
            multicore_fifo_push_blocking(FLAG_LED_OFF);
            multicore_fifo_pop_blocking();
            printf("LED turned OFF\n");
        } else {
            printf("Invalid command. Try 0, 1, or 2.\n");
        }
    }
}
