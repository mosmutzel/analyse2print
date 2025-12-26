/**
 * print.h - Niimbot B1 Printer Functions
 *
 * This module handles all Niimbot B1 printer operations including
 * BLE communication, label rendering, and print job management.
 * Uses multi-core architecture with print task on Core 1.
 */

#ifndef PRINT_H
#define PRINT_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// ============================================================================
// Label Configuration
// ============================================================================

#define LABEL_WIDTH 50    // mm
#define LABEL_HEIGHT 30   // mm
#define DPI 203           // dots per inch

// Calculate pixel dimensions
const int labelWidth = (LABEL_WIDTH * DPI) / 25.4;   // ~400 pixels
const int labelHeight = (LABEL_HEIGHT * DPI) / 25.4; // ~240 pixels

// Maximum dimensions for bitmap buffer
#define MAX_WIDTH 400
#define MAX_HEIGHT 240
#define MAX_WIDTH_BYTES ((MAX_WIDTH + 7) / 8)

// ============================================================================
// Print Job Structure
// ============================================================================

struct PrintJob {
  bool valid;
  uint16_t width;
  uint16_t height;
};

// ============================================================================
// External Variables
// ============================================================================

extern NimBLERemoteCharacteristic* pCharacteristic;
extern uint8_t bitmapBuffer[MAX_HEIGHT][MAX_WIDTH_BYTES];
extern uint8_t responseBuffer[256];
extern int responseLength;
extern volatile bool responseReceived;

// Multi-core task variables
extern TaskHandle_t printTaskHandle;
extern volatile bool printerBusy;

// ============================================================================
// Function Prototypes
// ============================================================================

// Initialization
void initPrintTask();

// Connection
bool connectToPrinter();
void sendHeartbeat();

// Bitmap operations
void clearBitmap();
void setPixel(int x, int y);
void drawLine(int x0, int y0, int x1, int y1);
void drawRect(int x, int y, int w, int h);
void fillRect(int x, int y, int w, int h);
void drawText(int x, int y, const char* text, int scale = 1);

// Printing
void printLabel();
bool queuePrintJob();
bool isPrinterBusy();

// Label templates
void printGasLabel(const char* o2, const char* he, const char* mod, const char* info, const char* date);
//void printGasLabelSimple(const char* o2, const char* he, const char* date);

#endif // PRINT_H
