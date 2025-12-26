/**
 * touch.h - Capacitive Touch Support for T-Display-S3-Pro
 */

#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include <lvgl.h>

/**
 * Initialize touch controller (CST226SE)
 * @return true if successful
 */
bool touchInit();

/**
 * Read touch input - call this in loop
 */
void touchLoop();

/**
 * Check if screen is currently being touched
 */
bool touchIsPressed();

/**
 * Get last touch X coordinate
 */
int16_t touchGetX();

/**
 * Get last touch Y coordinate
 */
int16_t touchGetY();

/**
 * Check if touch is within a rectangle
 */
bool touchInArea(int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * LVGL touch read callback
 */
void touchReadCallback(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

/**
 * Register touch input device with LVGL
 */
void touchRegisterLvgl();

/**
 * Set callback function for home button press
 */
typedef void (*touch_home_callback_t)(void);
void touchSetHomeCallback(touch_home_callback_t callback);

#endif // TOUCH_H
