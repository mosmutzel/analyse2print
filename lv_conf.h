/**
 * lv_conf.h - LVGL Configuration for T-Display-S3-Pro
 *
 * LVGL v9.x configuration for 222x480 ST7796 display
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

/*====================
   MEMORY SETTINGS
 *====================*/

/* Size of the memory available for `lv_malloc()` in bytes (>= 2kB) */
#define LV_MEM_SIZE (64 * 1024U)

/* Use the standard `malloc` and `free` from the C library */
#define LV_MEM_CUSTOM 0

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period in milliseconds */
#define LV_DEF_REFR_PERIOD 16

/* Default DPI. Used to initialize default sizes */
#define LV_DPI_DEF 130

/*====================
   DISPLAY SETTINGS
 *====================*/

/* Horizontal and vertical resolution */
#define LV_HOR_RES_MAX 222
#define LV_VER_RES_MAX 480

/*====================
   DRAWING SETTINGS
 *====================*/

/* Enable complex draw engine (gradients, masks, etc.) */
#define LV_DRAW_COMPLEX 1

/* Enable SW rendering */
#define LV_USE_DRAW_SW 1

/* Enable triangle drawing */
#define LV_DRAW_SW_COMPLEX 1

/*====================
   GPU SETTINGS
 *====================*/

/* No GPU acceleration */
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL 0

/*====================
   LOG SETTINGS
 *====================*/

/* Enable/disable the log module */
#define LV_USE_LOG 0

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

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable FreeType support */
#define LV_USE_FREETYPE 0

/*====================
   TEXT SETTINGS
 *====================*/

/* Select character encoding */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

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
#define LV_USE_THEME_SIMPLE 1

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
