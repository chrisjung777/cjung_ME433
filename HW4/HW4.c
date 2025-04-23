#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/stdio_usb.h" 

// SPI config
#define SPI_PORT spi0
#define PIN_MISO 16 
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

// DAC write 
void writeDac(int channel, float voltage) {
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

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();  
    
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(1000); 
    
    printf("Initializing SPI for MCP4912 DAC...\n");

    // SPI setup
    spi_init(SPI_PORT, 1000 * 1000);  
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);  
    
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);  
    
    printf("SPI initialization complete, starting waveform generation\n");
    printf(">min:0,max:1\r\n");
    
    float t = 0.0;

    while (true) {
        sleep_ms(10);  
        t += 0.01f;

        float sine = 0.5f + 0.5f * sinf(2 * M_PI * 2 * t);

        float tri = fmodf(t, 1.0f);
        if (tri > 0.5f) tri = 1.0f - tri;
        tri *= 2.0f;  

        writeDac(0, sine);   
        writeDac(1, tri);    

        printf(">sine:%.3f,triangle:%.3f\r\n", sine*3.3f, tri*3.3f);
    }

    return 0;
}
