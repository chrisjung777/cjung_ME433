#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/stdio_usb.h" 

// SPI config
#define SPI_PORT spi0
#define PIN_MISO 16  // Not used
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

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

// DAC write - for MCP4912 (10-bit DAC)
void writeDac(int channel, float voltage) {
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 1.0f) voltage = 1.0f;

    // Convert to 10-bit value (0-1023) for MCP4912
    uint16_t val10bit = (uint16_t)(voltage * 1023.0f);

    uint16_t packet = 0;
    packet |= (channel & 0x1) << 15;   // Channel A or B
    packet |= (0b1 << 14);             // Buffer input
    packet |= (0b1 << 13);             // Gain = 1x
    packet |= (0b1 << 12);             // Active mode (SHDN)
    packet |= (val10bit & 0x03FF);     // 10-bit data (mask with 0x03FF)

    uint8_t data[2] = {
        (packet >> 8) & 0xFF,
        packet & 0xFF
    };

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();  // Initialize USB serial
    
    // Wait for USB serial connection to establish
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(1000); // Give terminal time to connect
    
    printf("Initializing SPI for MCP4912 DAC...\n");

    // SPI setup
    spi_init(SPI_PORT, 1000 * 1000);  // 1 MHz
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);  // Not used but configure anyway
    
    // CS pin must be manually controlled
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);  // Set CS high initially (inactive)
    
    printf("SPI initialization complete, starting waveform generation\n");

    // Set plotting range once at startup (optional)
    printf(">min:0,max:1\r\n");
    
    float t = 0.0;

    while (true) {
        sleep_ms(10);  // 100 Hz update rate (50× faster than 2Hz signal)
        t += 0.01f;

        // Generate sine wave: 2 Hz, range 0-1 (maps to 0-3.3V on DAC)
        float sine = 0.5f + 0.5f * sinf(2 * M_PI * 2 * t);
        
        // Generate triangle wave: 1 Hz, range 0-1 (maps to 0-3.3V on DAC)
        float tri = fmodf(t, 1.0f);
        if (tri > 0.5f) tri = 1.0f - tri;
        tri *= 2.0f;  

        // Send to DAC - these will output actual 0-3.3V signals
        writeDac(0, sine);   // Channel A (VOUTA) - sine wave
        writeDac(1, tri);    // Channel B (VOUTB) - triangle wave

        // Send to Serial Plotter in correct format
        // Must start with '>' and use name:value format with no spaces
        printf(">sine:%.3f,triangle:%.3f\r\n", sine*3.3f, tri*3.3f);
    }

    return 0;
}
