#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c1
#define PIN_SDA 6
#define PIN_SCL 7

// Returns true if address is reserved (should not be probed)
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main() {
    stdio_init_all();

    // Initialize I2C1 at 100 kHz
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);

    sleep_ms(2000); // Give time for USB serial to connect

    printf("\nI2C Bus Scan (I2C1, SDA=GPIO6, SCL=GPIO7)\n");
    printf("    0 1 2 3 4 5 6 7 8 9 A B C D E F\n");

    for (int addr = 0; addr < 0x80; ++addr) {
        if (addr % 16 == 0) {
            printf("%02X: ", addr);
        }
        int ret;
        uint8_t dummy = 0;
        if (reserved_addr(addr)) {
            ret = -1;
        } else {
            // Try to write zero bytes to this address
            ret = i2c_write_blocking(I2C_PORT, addr, &dummy, 0, false);
        }
        printf(ret >= 0 ? "@" : ".");
        if (addr % 16 == 15) printf("\n");
    }
    printf("Done.\n");

    while (1) tight_loop_contents();
}
