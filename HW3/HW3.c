#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define LED_PIN 15
#define BUTTON_PIN 14
#define ADC_PIN 26     
#define ADC_INPUT 0    

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1); 

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); 

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_INPUT); 

    while (true) {
        printf("Waiting for button press...\n");
        while (gpio_get(BUTTON_PIN)) {
            tight_loop_contents();
        }

        gpio_put(LED_PIN, 0);

        int num_samples = 0;
        printf("Enter number of ADC samples (1–100): ");
        scanf("%d", &num_samples);

        if (num_samples < 1) num_samples = 1;
        if (num_samples > 100) num_samples = 100;

        printf("Sampling ADC %d times at 100 Hz:\n", num_samples);

        for (int i = 0; i < num_samples; i++) {
            uint16_t raw = adc_read();
            float voltage = raw * 3.3f / 4095.0f;
            printf("Sample %d: %.3f V\n", i + 1, voltage);
            sleep_ms(10); 
        }

        gpio_put(LED_PIN, 1);
        printf("Done.\n\n");
        sleep_ms(500); 
    }

    return 0;
}
