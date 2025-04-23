#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MCP23008_ADDR 0x20

#define IODIR   0x00  
#define GPIO    0x09  
#define OLAT    0x0A  
#define GPPU    0x06 

void i2c_write_register(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(i2c_default, MCP23008_ADDR, buf, 2, false);
}

uint8_t i2c_read_register(uint8_t reg) {
    uint8_t val;
    i2c_write_blocking(i2c_default, MCP23008_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, MCP23008_ADDR, &val, 1, false);
    return val;
}

int main() {
    stdio_init_all();

    i2c_init(i2c_default, 100 * 1000);  
    gpio_set_function(4, GPIO_FUNC_I2C); 
    gpio_set_function(5, GPIO_FUNC_I2C); 
    gpio_pull_up(4);
    gpio_pull_up(5);

    sleep_ms(500);

    i2c_write_register(IODIR, 0b00000001); 
    i2c_write_register(GPPU, 0b00000001);  
    i2c_write_register(OLAT, 0b00000000);  

    while (1) {
        uint8_t gpio_val = i2c_read_register(GPIO);
        if ((gpio_val & 0b00000001) == 0) { 
            i2c_write_register(OLAT, 0b10000000); 
        } else {
            i2c_write_register(OLAT, 0b00000000); 
        }
        sleep_ms(100);
    }
}
