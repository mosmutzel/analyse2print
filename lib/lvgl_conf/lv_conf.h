/**
 * lv_conf.h - LVGL Configuration for T-Display-S3-Pro
 *
 * LVGL v8.x configuration for 222x480 ST7796 display
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 16 (RGB565) */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color for SPI displays */
#define LV_COLOR_16_SWAP 1

/*Enable features to draw on transparent background.
 *It's required if opa, and transform_* style properties are used.
 *Can be also used if the UI is above another layer, e.g. an OSD menu or video player.*/
//#define LV_COLOR_SCREEN_TRANSP 1

/* Adjust color mix functions rounding. GPUs might calculate color mix (blending) differently.
 * 0: round down, 64: round up from x.75, 128: round up from half, 192: round up from x.25, 254: round up */
#define LV_COLOR_MIX_ROUND_OFS 0

/*Images pixels with this color will not be drawn if they are chroma keyed)*/
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)         /*pure green*/

/*====================
   MEMORY SETTINGS
 *====================*/

/* Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB) */
#define LV_MEM_SIZE (48 * 1024U)

/* Use the standard `malloc` and `free` from the C library */
#define LV_MEM_CUSTOM 0

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period in milliseconds */
#define LV_DISP_DEF_REFR_PERIOD 16

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 30

/* Default DPI. Used to initialize default sizes */
#define LV_DPI_DEF 130

/*====================
   DRAWING SETTINGS
 *====================*/

/* Enable complex draw engine (gradients, masks, etc.) */
#define LV_DRAW_COMPLEX 1

/* Allow buffering some shadow calculation for efficiency */
#define LV_SHADOW_CACHE_SIZE 0

/* Set the number of maximum cached circle data */
#define LV_CIRCLE_CACHE_SIZE 4

/*====================
   GPU SETTINGS
 *====================*/

/* No GPU acceleration for ESP32 */
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL 0

/*====================
   LOG SETTINGS
 *====================*/

/* Enable/disable the log module */
#define LV_USE_LOG 0

#if LV_USE_LOG
  #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
  #define LV_LOG_PRINTF 0
  #define LV_LOG_TRACE_MEM 1
  #define LV_LOG_TRACE_TIMER 1
  #define LV_LOG_TRACE_INDEV 1
  #define LV_LOG_TRACE_DISP_REFR 1
  #define LV_LOG_TRACE_EVENT 1
  #define LV_LOG_TRACE_OBJ_CREATE 1
  #define LV_LOG_TRACE_LAYOUT 1
  #define LV_LOG_TRACE_ANIM 1
#endif

/*====================
   ASSERT SETTINGS
 *====================*/

#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*====================
   FONT SETTINGS
 *====================*/

/* Montserrat fonts */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1

/* Unscii fonts */
#define LV_FONT_UNSCII_8 0
#define LV_FONT_UNSCII_16 0

/* Declare custom fonts */
#define LV_FONT_CUSTOM_DECLARE

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable FreeType support */
#define LV_USE_FONT_SUBPX 1
#define LV_FONT_SUBPX_BGR 0

/*====================
   TEXT SETTINGS
 *====================*/

/* Select character encoding */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/* Buffer size for sprintf */
#define LV_SPRINTF_CUSTOM 0
#define LV_SPRINTF_USE_FLOAT 0

/*====================
   WIDGET SETTINGS
 *====================*/

/* Base widgets */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1

/* Extra widgets */
#define LV_USE_ANIMIMG    0
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   1
#define LV_USE_LED        1
#define LV_USE_LIST       1
#define LV_USE_MENU       0
#define LV_USE_METER      1
#define LV_USE_MSGBOX     1
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    1
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0
#define LV_USE_SPAN       1

/*====================
   LAYOUT SETTINGS
 *====================*/

/* Enable flex layout */
#define LV_USE_FLEX 1

/* Enable grid layout */
#define LV_USE_GRID 1

/*====================
   THEME SETTINGS
 *====================*/

/* Enable default theme */
#define LV_USE_THEME_DEFAULT 1

/* Enable dark mode for default theme */
#define LV_THEME_DEFAULT_DARK 1

/* Enable simple theme (good for small displays) */
#define LV_USE_THEME_BASIC 1
#define LV_USE_THEME_MONO 0

/*====================
   OTHER SETTINGS
 *====================*/

/* Enable file system support */
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0

/* Enable PNG/BMP/SJPG/GIF decoder */
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_SJPG 0
#define LV_USE_GIF 0

/* Enable QR code */
#define LV_USE_QRCODE 0

/* Enable snapshot */
#define LV_USE_SNAPSHOT 0

/*====================
   TICK SETTINGS
 *====================*/

/* Use custom tick source (we'll provide lv_tick_inc) */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#endif /* LV_CONF_H */
