#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define PIN_UP     2
#define PIN_DOWN   3
#define PIN_LEFT   4
#define PIN_RIGHT  5
#define PIN_MODE   6
#define PIN_LED    15

enum {
    MODE_MANUAL = 0,
    MODE_CIRCLE = 1,
};

static uint8_t current_mode = MODE_MANUAL;
static uint32_t blink_interval_ms = 1000;

void board_init_after_tusb(void) {
    gpio_init(PIN_UP);    gpio_pull_up(PIN_UP);    gpio_set_dir(PIN_UP, GPIO_IN);
    gpio_init(PIN_DOWN);  gpio_pull_up(PIN_DOWN);  gpio_set_dir(PIN_DOWN, GPIO_IN);
    gpio_init(PIN_LEFT);  gpio_pull_up(PIN_LEFT);  gpio_set_dir(PIN_LEFT, GPIO_IN);
    gpio_init(PIN_RIGHT); gpio_pull_up(PIN_RIGHT); gpio_set_dir(PIN_RIGHT, GPIO_IN);
    gpio_init(PIN_MODE);  gpio_pull_up(PIN_MODE);  gpio_set_dir(PIN_MODE, GPIO_IN);
    gpio_init(PIN_LED);   gpio_set_dir(PIN_LED, GPIO_OUT);
}

void toggle_mode_if_requested(void) {
    static bool prev = true;
    bool curr = gpio_get(PIN_MODE);
    if (!curr && prev) {
        current_mode = !current_mode;
        gpio_put(PIN_LED, current_mode == MODE_CIRCLE);
        sleep_ms(200);
    }
    prev = curr;
}

void send_manual_mouse_report(void) {
    static absolute_time_t press_start[4] = {0};
    const uint PIN_DIRS[4] = {PIN_UP, PIN_DOWN, PIN_LEFT, PIN_RIGHT};
    const int8_t DX[4] = {0, 0, -1, 1};
    const int8_t DY[4] = {-1, 1, 0, 0};

    int16_t final_dx = 0, final_dy = 0;

    for (int i = 0; i < 4; i++) {
        bool pressed = !gpio_get(PIN_DIRS[i]);
        if (pressed) {
            if (is_nil_time(press_start[i])) {
                press_start[i] = get_absolute_time();
            }
            int held_ms = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(press_start[i]);
            int speed;
            if (held_ms > 2000)      speed = 8;
            else if (held_ms > 1000) speed = 5;
            else if (held_ms > 500)  speed = 4;
            else if (held_ms > 250)  speed = 2;
            else                     speed = 1;
            final_dx += speed * DX[i];
            final_dy += speed * DY[i];
        } else {
            press_start[i] = nil_time;
        }
    }

    if (tud_hid_ready()) {
        if (final_dx > 127) final_dx = 127;
        if (final_dx < -127) final_dx = -127;
        if (final_dy > 127) final_dy = 127;
        if (final_dy < -127) final_dy = -127;
        tud_hid_mouse_report(REPORT_ID_MOUSE, 0, (int8_t)final_dx, (int8_t)final_dy, 0, 0);
    }
}


void send_circle_mouse_report(void) {
    static float theta = 0.0f;
    theta += 0.05f;
    if (theta >= 2 * 3.14159f) theta -= 2 * 3.14159f;

    int8_t dx = (int8_t)(3.0f * cosf(theta));
    int8_t dy = (int8_t)(3.0f * sinf(theta));

    tud_hid_mouse_report(REPORT_ID_MOUSE, 0, dx, dy, 0, 0);
}

void hid_task(void) {
    const uint32_t interval_ms = 2;
    static uint32_t start_ms = 0;
    if (board_millis() - start_ms < interval_ms) return;
    start_ms += interval_ms;

    toggle_mode_if_requested();

    if (!tud_hid_ready()) return;

    if (current_mode == MODE_MANUAL) {
        send_manual_mouse_report();
    } else {
        send_circle_mouse_report();
    }
}

void led_blinking_task(void) {
    static uint32_t start_ms = 0;
    static bool led_state = false;
    if (!blink_interval_ms) return;
    if (board_millis() - start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;
}

void tud_mount_cb(void)    { blink_interval_ms = 1000; }
void tud_umount_cb(void)   { blink_interval_ms = 250; }
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    blink_interval_ms = 2500;
}
void tud_resume_cb(void) {
    blink_interval_ms = tud_mounted() ? 1000 : 250;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}

int main(void) {
    board_init();
    tud_init(0); 
    if (board_init_after_tusb) board_init_after_tusb();

    while (1) {
        tud_task();
        led_blinking_task();
        hid_task();
    }
}
