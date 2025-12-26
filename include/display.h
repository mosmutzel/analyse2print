/**
 * display.h - TFT Display Functions for LilyGo T-Display-S3-Pro
 *
 * This module handles all TFT display operations including
 * status messages, sensor data display, and general text output.
 * Uses TFT_eSPI library with ST7796 driver (222x480 pixels).
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "utilities.h"

// ============================================================================
// Display Configuration
// ============================================================================

// T-Display-S3-Pro has 222x480 TFT (ST7796)
#define DISPLAY_WIDTH  BOARD_TFT_WIDTH
#define DISPLAY_HEIGHT BOARD_TFT_HEIHT

// Screen switching button (BTN2 = GPIO12)
#define SCREEN_SWITCH_PIN BOARD_BTN2

// Colors (RGB565)
#define COLOR_BG TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_O2 TFT_CYAN
#define COLOR_HE TFT_YELLOW
#define COLOR_STATUS TFT_GREEN
#define COLOR_WARNING TFT_ORANGE

// ============================================================================
// External Variables (defined in display.cpp)
// ============================================================================
extern TFT_eSPI tft;
extern String displayLines[20];  // More lines for taller display
extern int displayLineIndex;

// Gas sensor data
extern String lastVar1;  // O2 percentage
extern String lastVar2;  // He percentage
extern String lastVar5;  // Date
extern unsigned long lastDataTime;
extern bool dataPending;

#define DATA_TIMEOUT 10000  // 10 seconds

// ============================================================================
// Function Prototypes
// ============================================================================

// Initialize display
void displayInit();

// Display functions
void displayPrint(String text);
void displayDebug(String text);
void displayStatus(String line1, String line2 = "", String line3 = "", String line4 = "");
void displaySensorData();

// Status update functions
void displaySetPrinterStatus(const char* status);
void displaySetAnalyzerStatus(const char* status);
void displayUpdateBattery(int voltage_mv, bool charging);

// Status symbol functions
void displaySetWifiStatus(bool connected);
void displaySetBluetoothStatus(bool connected, bool scanning);
void displaySetUsbStatus(bool connected);

// LVGL loop handler - call from main loop
void displayLoop();

// Toggle settings screen (for touch home button)
void displayToggleSettings();

#endif // DISPLAY_H
