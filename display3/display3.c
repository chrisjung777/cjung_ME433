#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

void drawMessage(int x, int y, char *m);
void drawLetter(int x, int y, char c);

int main() {
    stdio_init_all();

    // I2C setup
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // OLED setup
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // ADC setup (GPIO26 → ADC0)
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while (true) {
        absolute_time_t t1 = get_absolute_time();

        // Read ADC0 and convert to volts
        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / 4095.0f;

        // Format voltage string
        char volts[32];
        sprintf(volts, "ADC0: %.2f V", voltage);

        // Clear display
        ssd1306_clear();
        drawMessage(0, 10, volts);

        // Time after rendering
        absolute_time_t t2 = get_absolute_time();
        int64_t t_diff_us = absolute_time_diff_us(t1, t2);

        // FPS display
        int fps_val = 1000000 / t_diff_us;
        char fps[32];
        sprintf(fps, "FPS: %d", fps_val);
        drawMessage(0, 24, fps);

        ssd1306_update();
        sleep_ms(1000);
    }
}

void drawMessage(int x, int y, char *m) {
    int i = 0;
    while (m[i] != 0) {
        drawLetter(x + i * 6, y, m[i]);
        i++;
    }
}

void drawLetter(int x, int y, char c) {
    if (c < 0x20 || c > 0x7F) return;
    int index = c - 0x20;
    for (int col = 0; col < 5; col++) {
        char byte = ASCII[index][col];
        for (int row = 0; row < 8; row++) {
            char pixel = (byte >> row) & 0x01;
            ssd1306_drawPixel(x + col, y + row, pixel);
        }
    }
}
