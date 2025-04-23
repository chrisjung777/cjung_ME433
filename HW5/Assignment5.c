#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/stdio_usb.h"

// SPI configuration
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_CS_SRAM 5
#define PIN_SCK 18
#define PIN_MOSI 19

// 23K256 SRAM commands
#define SRAM_READ  0x03
#define SRAM_WRITE 0x02
#define SRAM_WRSR  0x01
#define SRAM_MODE  0x40  // Sequential mode

#define NUM_SAMPLES 1000  // Must not exceed 8192
#define DELAY_MS 1

union FloatBytes {
    float f;
    uint8_t b[4];
};

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void sram_init() {
    uint8_t cmd[2] = {SRAM_WRSR, SRAM_MODE};
    cs_select(PIN_CS_SRAM);
    spi_write_blocking(SPI_PORT, cmd, 2);
    cs_deselect(PIN_CS_SRAM);
    sleep_ms(10);
}

void sram_write_float(uint16_t addr, float value) {
    if (addr > 0x7FFC) return; // Prevent overflow
    uint8_t cmd[3] = {
        SRAM_WRITE,
        (uint8_t)(addr >> 8),
        (uint8_t)(addr & 0xFF)
    };
    union FloatBytes data;
    data.f = value;
    cs_select(PIN_CS_SRAM);
    spi_write_blocking(SPI_PORT, cmd, 3);
    spi_write_blocking(SPI_PORT, data.b, 4);
    cs_deselect(PIN_CS_SRAM);
}

float sram_read_float(uint16_t addr) {
    if (addr > 0x7FFC) return 0.0f;
    uint8_t cmd[3] = {
        SRAM_READ,
        (uint8_t)(addr >> 8),
        (uint8_t)(addr & 0xFF)
    };
    union FloatBytes data;
    cs_select(PIN_CS_SRAM);
    spi_write_blocking(SPI_PORT, cmd, 3);
    spi_read_blocking(SPI_PORT, 0xFF, data.b, 4);
    cs_deselect(PIN_CS_SRAM);
    return data.f;
}

void write_dac(int channel, float voltage) {
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 1.0f) voltage = 1.0f;
    uint16_t val10bit = (uint16_t)(voltage * 1023.0f);
    uint16_t packet = 0;
    packet |= (channel & 0x1) << 15;
    packet |= (0b1 << 14);
    packet |= (0b1 << 13);
    packet |= (0b1 << 12);
    packet |= (val10bit & 0x03FF);
    uint8_t data[2] = {
        (packet >> 8) & 0xFF,
        packet & 0xFF
    };
    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) { sleep_ms(100); }
    sleep_ms(1000);

    // SPI and GPIO setup
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_SRAM, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_set_dir(PIN_CS_SRAM, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
    gpio_put(PIN_CS_SRAM, 1);

    sram_init();

    // Store one cycle of sine wave as floats in SRAM
    for (int i = 0; i < NUM_SAMPLES; i++) {
        float value = 3.3f * (0.5f + 0.5f * sinf(2.0f * M_PI * i / NUM_SAMPLES));
        sram_write_float(i * 4, value);
    }

    // Output 1Hz sine wave from SRAM to DAC and Serial Plotter
    while (true) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            float value = sram_read_float(i * 4);
            float dac_value = value / 3.3f;
            write_dac(0, dac_value);
            printf(">sine:%.3f\r\n", value);
            sleep_ms(DELAY_MS);
        }
    }
    return 0;
}
