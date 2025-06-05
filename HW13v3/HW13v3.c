#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "ssd1306.h"
#include "font.h"
#include "stdlib.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

#define MPU6050_ADDR 0x68

#define PWR_MGMT_1 0x6B
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG 0x1B
#define WHO_AM_I 0x75
#define ACCEL_XOUT_H 0x3B

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

void drawMessage(int x, int y, char *m);
void drawLetter(int x, int y, char c);
void drawLine(int x0, int y0, int x1, int y1, int color);
void mpu6050_init();
void mpu6050_read_data(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z);
int16_t combine_bytes(uint8_t msb, uint8_t lsb);

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    mpu6050_init();

    uint8_t whoami = 0;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, (uint8_t[]){WHO_AM_I}, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &whoami, 1, false);

    if (whoami != 0x68) {
        gpio_init(25);
        gpio_set_dir(25, true);
        while (1) {
            gpio_put(25, 1);
        }
    }

    while (true) {
        absolute_time_t t1 = get_absolute_time();

        int16_t ax, ay, az;
        mpu6050_read_data(&ax, &ay, &az);

        float ax_g = ax * 0.000061;
        float ay_g = ay * 0.000061;

        int x_len = (int)(ax_g * 40);
        int y_len = (int)(ay_g * 40);

        ssd1306_clear();
        drawLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                 SCREEN_WIDTH / 2 + x_len, SCREEN_HEIGHT / 2, 1);
        drawLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                 SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - y_len, 1);

        char msg[32];
        sprintf(msg, "ax: %.2fg", ax_g);
        drawMessage(0, 0, msg);

        sprintf(msg, "ay: %.2fg", ay_g);
        drawMessage(0, 8, msg);

        ssd1306_update();

        sleep_ms(10);
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

void drawLine(int x0, int y0, int x1, int y1, int color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        ssd1306_drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void mpu6050_init() {
    uint8_t buf[] = {PWR_MGMT_1, 0x00};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);

    buf[0] = ACCEL_CONFIG;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);

    buf[0] = GYRO_CONFIG;
    buf[1] = 0x18;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

void mpu6050_read_data(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[14];
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 14, false);

    *accel_x = combine_bytes(data[0], data[1]);
    *accel_y = combine_bytes(data[2], data[3]);
    *accel_z = combine_bytes(data[4], data[5]);
}

int16_t combine_bytes(uint8_t msb, uint8_t lsb) {
    return (int16_t)((msb << 8) | lsb);
}
