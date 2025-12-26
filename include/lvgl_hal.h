/**
 * lvgl_hal.h - LVGL Hardware Abstraction Layer for T-Display-S3-Pro
 *
 * This module provides the display driver and touch interface for LVGL v8.x
 */

#ifndef LVGL_HAL_H
#define LVGL_HAL_H

#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Display buffer size (partial buffer for memory efficiency)
#define LVGL_BUF_SIZE (222 * 40)

// Function prototypes
void lvgl_hal_init();
void lvgl_hal_loop();

#endif // LVGL_HAL_H
