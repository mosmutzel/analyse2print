/**
 * touch.cpp - Capacitive Touch Support for T-Display-S3-Pro
 *
 * Uses CST226SE touch controller via I2C
 */

#include "touch.h"
#include <Wire.h>
#include <SensorLib.h>
#include "touch/TouchClassCST226.h"
#include "utilities.h"
#include "lvgl_hal.h"

// Touch controller instance
static TouchClassCST226 touch;
static bool touchInitialized = false;

// Touch interrupt pin
#define BOARD_TOUCH_IRQ 21

// CST226SE I2C address
#define CST226SE_ADDR 0x5A

// Touch data
static int16_t touch_x[5] = {0};
static int16_t touch_y[5] = {0};
static bool touchPressed = false;
static int16_t lastTouchX = 0;
static int16_t lastTouchY = 0;

// External home button callback
static touch_home_callback_t externalHomeCallback = NULL;

// Debounce for home button
static unsigned long lastHomeButtonTime = 0;
#define HOME_BUTTON_DEBOUNCE_MS 500  // 500ms debounce

// Home button callback from touch controller
static void touchHomeKeyCallback(void *user_data) {
    unsigned long now = millis();

    // Debounce: ignore if pressed too recently
    if (now - lastHomeButtonTime < HOME_BUTTON_DEBOUNCE_MS) {
        return;
    }
    lastHomeButtonTime = now;

    Serial.println("Touch: Home key pressed");
    if (externalHomeCallback != NULL) {
        externalHomeCallback();
    }
}

/**
 * Initialize touch controller
 */
bool touchInit() {
    Serial.println("[TOUCH] Initializing CST226SE...");

    touch.setPins(BOARD_TOUCH_RST, BOARD_TOUCH_IRQ);

    if (!touch.begin(Wire, CST226SE_ADDR, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("[TOUCH] CST226SE not found!");
        return false;
    }

    touch.setHomeButtonCallback(touchHomeKeyCallback, NULL);

    touchInitialized = true;
    Serial.println("[TOUCH] CST226SE initialized successfully");
    Serial.print("[TOUCH] Max touch points: ");
    Serial.println(touch.getSupportTouchPoint());

    return true;
}

/**
 * Read touch input - call this in loop
 */
void touchLoop() {
    if (!touchInitialized) return;

    uint8_t touched = touch.getPoint(touch_x, touch_y, touch.getSupportTouchPoint());

    if (touched > 0) {
        touchPressed = true;
        lastTouchX = touch_x[0];
        lastTouchY = touch_y[0];
    } else {
        touchPressed = false;
    }
}

/**
 * Check if screen is currently being touched
 */
bool touchIsPressed() {
    return touchPressed;
}

/**
 * Get last touch X coordinate
 */
int16_t touchGetX() {
    return lastTouchX;
}

/**
 * Get last touch Y coordinate
 */
int16_t touchGetY() {
    return lastTouchY;
}

/**
 * Check if touch is within a rectangle
 */
bool touchInArea(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!touchPressed) return false;
    return (lastTouchX >= x && lastTouchX < x + w &&
            lastTouchY >= y && lastTouchY < y + h);
}

/**
 * LVGL touch read callback
 */
void touchReadCallback(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    if (!touchInitialized) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }

    uint8_t touched = touch.getPoint(touch_x, touch_y, 1);

    if (touched > 0) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touch_x[0];
        data->point.y = touch_y[0];
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/**
 * Register touch input device with LVGL
 */
void touchRegisterLvgl() {
    if (!touchInitialized) return;

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchReadCallback;
    lv_indev_drv_register(&indev_drv);

    Serial.println("[TOUCH] LVGL input device registered");
}

/**
 * Set callback function for home button press
 */
void touchSetHomeCallback(touch_home_callback_t callback) {
    externalHomeCallback = callback;
}
