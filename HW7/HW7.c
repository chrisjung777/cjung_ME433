#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_BAUDRATE 400000

void drawChar(unsigned char x, unsigned char y, char c);
void drawString(unsigned char x, unsigned char y, const char *str);

int main() {
    stdio_init_all();

    // Initialize I2C0 on GP0 (SDA), GP1 (SCL)
    i2c_init(i2c0, I2C_BAUDRATE);
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    ssd1306_setup();
    ssd1306_clear();

    drawString(0, 0, "ME433 HW7");
    drawString(0, 10, "Pico 2 + OLED");
    ssd1306_update();

    while (1) {
        tight_loop_contents();
    }
}
