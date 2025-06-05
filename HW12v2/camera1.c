#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "ov7670.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define VS 8
#define HS 9
#define MCLK 10
#define PCLK 11
#define RST 12

#define IMAGESIZEX 80
#define IMAGESIZEY 60

static volatile uint8_t saveImage = 0;
static volatile uint8_t startImage = 0;
static volatile uint8_t startCollect = 0;
static volatile uint32_t rawIndex = 0;
static volatile uint32_t hsCount = 0;
static volatile uint32_t vsCount = 0;
static volatile uint8_t cameraData[IMAGESIZEX*IMAGESIZEY*2];

typedef struct cameraImage{
    uint32_t index;
    uint8_t r[IMAGESIZEX*IMAGESIZEY];
    uint8_t g[IMAGESIZEX*IMAGESIZEY];
    uint8_t b[IMAGESIZEX*IMAGESIZEY];
} cameraImage_t;

static volatile struct cameraImage picture;

void init_camera_pins();
void init_camera();
void setSaveImage(uint32_t);
uint32_t getSaveImage();
uint32_t getHSCount();
uint32_t getPixelCount();
void convertImage();
void printImage();
void findLine();
void OV7670_write_register(uint8_t reg, uint8_t value);
uint8_t OV7670_read_register(uint8_t reg);

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == VS){
        if (saveImage==1){
            rawIndex = 0;
            hsCount = 0;
            vsCount = 0;
            startImage = 1;
            startCollect = 0;
        }
    }
    if (gpio == HS){
        if(saveImage){
            if (startImage){
                startCollect = 1;
                hsCount++;
                if (hsCount == IMAGESIZEY){
                    saveImage = 0;
                    startImage = 0;
                    startCollect = 0;
                    hsCount = 0;
                }
            }
        }
    }
    if (gpio == PCLK){
        if(saveImage){
            if(startImage){
                if(startCollect){
                    vsCount++;
                    uint32_t d = gpio_get_all();
                    cameraData[rawIndex] = d & 0xFF;
                    rawIndex++;
                    if (rawIndex == IMAGESIZEX*IMAGESIZEY*2){
                        saveImage = 0;
                        startImage = 0;
                        startCollect = 0;
                    }
                    if (vsCount == IMAGESIZEX*2){
                        startCollect = 0;
                        vsCount = 0;
                    }
                }
            }
        }
    }
}

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, camera!\n");

    init_camera_pins();
 
    while (true) {
        char m[10];
        scanf("%s",m);
        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        findLine();
        printImage();
    }
}

void init_camera_pins(){
    for(int i = 0; i < 8; i++){
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
    }

    gpio_init(RST);
    gpio_set_dir(RST, GPIO_OUT);
    gpio_put(RST, 1);

    gpio_set_function(MCLK, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MCLK);
    float div = 2;
    pwm_set_clkdiv(slice_num, div);
    uint16_t wrap = 3;
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    pwm_set_gpio_level(MCLK, wrap / 2);

    sleep_ms(1000);

    i2c_init(I2C_PORT, 100*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    printf("Start init camera\n");
    init_camera();
    printf("End init camera\n");

    gpio_init(VS);
    gpio_set_dir(VS, GPIO_IN);
    gpio_set_irq_enabled_with_callback(VS, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(HS);
    gpio_set_dir(HS, GPIO_IN);
    gpio_set_irq_enabled_with_callback(HS, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    gpio_init(PCLK);
    gpio_set_dir(PCLK, GPIO_IN);
    gpio_set_irq_enabled_with_callback(PCLK, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
}

void init_camera(){
    gpio_put(RST, 0);
    sleep_ms(1);
    gpio_put(RST, 1);
    sleep_ms(1000);

    OV7670_write_register(0x12, 0x80);
    sleep_ms(1000);

    OV7670_write_register(OV7670_REG_CLKRC, 1);
    OV7670_write_register(OV7670_REG_DBLV, 0);

    for(int i=0; i<3; i++){
        OV7670_write_register(OV7670_rgb[i][0], OV7670_rgb[i][1]);
    }

    for(int i=0; i<92; i++){
        OV7670_write_register(OV7670_init[i][0], OV7670_init[i][1]);
    }

    uint8_t size = OV7670_SIZE_DIV8;
    uint16_t vstart = 12;
    uint16_t hstart = 210;
    uint16_t edge_offset = 0;
    uint16_t pclk_delay = 2;

    OV7670_write_register(OV7670_REG_COM3, OV7670_COM3_DCWEN);
    OV7670_write_register(OV7670_REG_COM14, 0x1B);
    OV7670_write_register(OV7670_REG_SCALING_DCWCTR, size * 0x11);
    OV7670_write_register(OV7670_REG_SCALING_PCLK_DIV, 0xF3);

    uint16_t vstop = vstart + 480;
    uint16_t hstop = (hstart + 640) % 784;
    OV7670_write_register(OV7670_REG_HSTART, hstart >> 3);
    OV7670_write_register(OV7670_REG_HSTOP, hstop >> 3);
    OV7670_write_register(OV7670_REG_HREF,(edge_offset << 6) | ((hstop & 0b111) << 3) | (hstart & 0b111));
    OV7670_write_register(OV7670_REG_VSTART, vstart >> 2);
    OV7670_write_register(OV7670_REG_VSTOP, vstop >> 2);
    OV7670_write_register(OV7670_REG_VREF, ((vstop & 0b11) << 2) | (vstart & 0b11));
    OV7670_write_register(OV7670_REG_SCALING_PCLK_DELAY, pclk_delay);

    sleep_ms(300);

    uint8_t p = OV7670_read_register(OV7670_REG_PID);
    printf("pid = %d\n",p);
    uint8_t v = OV7670_read_register(OV7670_REG_VER);
    printf("ver = %d\n",v);
}

void convertImage(){
    picture.index = 0;
    for(int i=0; i<IMAGESIZEX*IMAGESIZEY*2; i+=2){
        picture.r[picture.index] = cameraData[i]>>3;
        picture.g[picture.index] = ((cameraData[i]&0b111)<<3) | cameraData[i+1]>>5;
        picture.b[picture.index] = cameraData[i+1]&0b11111;
        
        picture.r[picture.index] = (picture.r[picture.index] * 255) / 31;
        picture.g[picture.index] = (picture.g[picture.index] * 255) / 63;
        picture.b[picture.index] = (picture.b[picture.index] * 255) / 31;
        
        picture.index++;
    }
}

void findLine(){
    int center_x = 0, center_y = 0;
    int pixel_count = 0;
    
    for(int y = 0; y < IMAGESIZEY; y++){
        for(int x = 0; x < IMAGESIZEX; x++){
            int idx = y * IMAGESIZEX + x;
            
            int gray = (picture.r[idx] * 299 + picture.g[idx] * 587 + picture.b[idx] * 114) / 1000;
            
            bool is_line = false;
            
            if(gray < 80){
                is_line = true;
            }
            
            if(is_line){
                picture.r[idx] = 0;
                picture.g[idx] = 255;
                picture.b[idx] = 0;
                
                center_x += x;
                center_y += y;
                pixel_count++;
            }
        }
    }
    
    if(pixel_count > 0){
        center_x /= pixel_count;
        center_y /= pixel_count;
        
        int center_idx = center_y * IMAGESIZEX + center_x;
        if(center_idx >= 0 && center_idx < IMAGESIZEX * IMAGESIZEY){
            picture.r[center_idx] = 255;
            picture.g[center_idx] = 0;
            picture.b[center_idx] = 0;
        }
        
        printf("Line center: (%d, %d), Pixels: %d\n", center_x, center_y, pixel_count);
        
        int image_center_x = IMAGESIZEX / 2;
        int offset = center_x - image_center_x;
        printf("Line offset from center: %d\n", offset);
        
    } else {
        printf("No line detected\n");
    }
}

void printImage(){
    for(int i=0; i<IMAGESIZEX*IMAGESIZEY; i++){
        printf("%d %d %d %d\r\n", i, picture.r[i], picture.g[i], picture.b[i]);
    }
}

void OV7670_write_register(uint8_t reg, uint8_t value){
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, OV7670_ADDR, buf, 2, false);
    sleep_ms(1);
}

uint8_t OV7670_read_register(uint8_t reg){
    uint8_t buf;
    i2c_write_blocking(I2C_PORT, OV7670_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, OV7670_ADDR, &buf, 1, false);
    return buf;
}

void setSaveImage(uint32_t s){ saveImage = s; }
uint32_t getSaveImage(){ return saveImage; }
uint32_t getHSCount(){ return hsCount; }
uint32_t getPixelCount(){ return rawIndex; }
