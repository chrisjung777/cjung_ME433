#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"

#define NUM_PIXELS 4
#define WS2812_PIN 2
#define SERVO_PIN 3
#define CYCLE_TIME_MS 5000
#define IS_RGBW false

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} wsColor;

wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (sat == 0.0) {
        red = brightness;
        green = brightness;
        blue = brightness;
    } else {
        if (hue == 360.0) {
            hue = 0;
        }

        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;

        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0:
                red = brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = brightness;
                break;
            case 5:
                red = brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    unsigned char ired = red * 255.0;
    unsigned char igreen = green * 255.0;
    unsigned char iblue = blue * 255.0;

    wsColor c;
    c.r = ired;
    c.g = igreen;
    c.b = iblue;
    return c;
}

void set_leds(PIO pio, uint sm, wsColor colors[]) {
    for(int i=0; i<NUM_PIXELS; i++) {
        uint32_t color = ((uint32_t)colors[i].g << 16) | 
                        ((uint32_t)colors[i].r << 8) | 
                        (uint32_t)colors[i].b;
        pio_sm_put_blocking(pio, sm, color << 8u);
    }
    sleep_ms(1); 
}
void servo_init() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);  
    pwm_config_set_wrap(&config, 20000);     
    pwm_init(slice, &config, true);
}

void set_servo_angle(uint angle) {
    uint16_t pulse = 500 + (angle * 11.11); 
    pwm_set_gpio_level(SERVO_PIN, pulse);
}

int main() {
    stdio_init_all();
    
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    servo_init();

    absolute_time_t cycle_start = get_absolute_time();
    float hue = 0.0f;
    uint angle = 0;

    while(1) {
        wsColor colors[NUM_PIXELS];
        
        uint32_t elapsed = absolute_time_diff_us(cycle_start, get_absolute_time()) / 1000;
        float progress = (float)(elapsed % CYCLE_TIME_MS) / CYCLE_TIME_MS;

        for(int i=0; i<NUM_PIXELS; i++) {
            colors[i] = HSBtoRGB(fmod(hue + i*90.0f, 360.0f), 1.0f, 0.5f);
        }
        set_leds(pio, sm, colors);

        angle = (uint)(progress * 180.0f);
        set_servo_angle(angle);

        hue = fmod(progress * 360.0f, 360.0f);

        sleep_until(delayed_by_us(cycle_start, (elapsed + 20) * 1000));
    }
}
