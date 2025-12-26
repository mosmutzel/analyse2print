/**
 * lvgl_hal.cpp - LVGL Hardware Abstraction Layer for T-Display-S3-Pro
 *
 * This module provides the display driver for LVGL v8.x using TFT_eSPI
 */

#include "lvgl_hal.h"

// Note: PMU initialization removed - causes I2C conflicts with USB Analyzer
// The display works without explicit PMU configuration when powered via USB-C

// TFT display instance
static TFT_eSPI tft_lvgl = TFT_eSPI();

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* buf1 = nullptr;
static lv_color_t* buf2 = nullptr;

// LVGL display driver
static lv_disp_drv_t disp_drv;

/**
 * Display flush callback for LVGL
 * Called when LVGL wants to update a portion of the screen
 */
static void disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft_lvgl.startWrite();
  tft_lvgl.setAddrWindow(area->x1, area->y1, w, h);
  // Note: LV_COLOR_16_SWAP=1 in lv_conf.h already swaps bytes for SPI
  // So we pass false here to avoid double-swapping
  tft_lvgl.pushColors((uint16_t*)&color_p->full, w * h, false);
  tft_lvgl.endWrite();

  lv_disp_flush_ready(disp);
}

/**
 * Initialize LVGL and the display hardware
 */
void lvgl_hal_init() {
  Serial.println("Starting display initialization...");

  // Enable backlight FIRST before TFT init
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  Serial.println("Backlight enabled");

  delay(50);  // Short delay for power stabilization

  // Initialize TFT
  Serial.println("Initializing TFT...");
  tft_lvgl.init();
  tft_lvgl.setRotation(0);  // Portrait mode (222x480)
  tft_lvgl.fillScreen(TFT_BLACK);

  Serial.println("TFT initialized");

  // Initialize LVGL
  lv_init();

  // Allocate display buffers in PSRAM if available
  #if defined(BOARD_HAS_PSRAM)
    buf1 = (lv_color_t*)ps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t));
    buf2 = (lv_color_t*)ps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t));
  #else
    buf1 = (lv_color_t*)malloc(LVGL_BUF_SIZE * sizeof(lv_color_t));
    buf2 = nullptr;  // Single buffer if no PSRAM
  #endif

  // Initialize draw buffer
  if (buf2) {
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_BUF_SIZE);
  } else {
    lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, LVGL_BUF_SIZE);
  }

  // Initialize display driver
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 222;
  disp_drv.ver_res = 480;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

/**
 * LVGL tick handler - call in main loop
 */
void lvgl_hal_loop() {
  lv_timer_handler();
}
