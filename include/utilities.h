/**
 * @file      utilities.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-09-25
 *
 */
#pragma once
// https://www.lilygo.cc/products/t-display-s3-pro
// T-Display-Pro PinMaps


//! Using T-Display-Pro V1.0 , uncomment use V1.1 ,
//! The difference between V1.0 and V1.1 is the backlight driver.
// #define USING_DISPLAY_PRO_V1


// LTR553 , TOUCH , SY6970 , Camera share I2C Bus
#define BOARD_I2C_SDA       5
#define BOARD_I2C_SCL       6

// SD , TFT share SPI Bus
#define BOARD_SPI_MISO      8
#define BOARD_SPI_MOSI      17
#define BOARD_SPI_SCK       18
#define BOARD_TFT_CS        39
#define BOARD_TFT_RST       47
#define BOARD_TFT_DC        9
#define BOARD_TFT_BL        48
#define BOARD_SD_CS         14
#define BOARD_SENSOR_IRQ    21
#define BOARD_TOUCH_RST     13

#define BOARD_TFT_WIDTH     222
#define BOARD_TFT_HEIHT     480

// BUTTON PinMaps
#define BOARD_USER_BUTTON   {0 /*btn1*/,12/*btn2*/,16/*btn3*/}
#define BOARD_USER_BTN_NUM  3

// Individual button pins for easy access
#define BOARD_BTN1          0   // Boot button - also wake from deep sleep
#define BOARD_BTN2          12  // Screen switch button
#define BOARD_BTN3          16  // User button 3

//#define LEDC_TFT_CH                 1
//#define LEDC_WHITE_CH               2


#ifdef USING_DISPLAY_PRO_V1
#define BRIGHTNESS_MAX_LEVEL        255
#else
#define BRIGHTNESS_MAX_LEVEL        16
#endif
