/**
 * analyzer.h - Divesoft Analyzer USB Host Interface
 *
 * This module handles USB Host communication with the Divesoft Analyzer
 * using an FTDI FT232R chip via ESP32-S3 USB OTG.
 */

#ifndef ANALYZER_H
#define ANALYZER_H

#include <Arduino.h>
#include "usb/usb_host.h"
#include "esp_log.h"

// ============================================================================
// Data Structures
// ============================================================================

struct AnalyzerData {
  bool valid;
  float oxygen;     // O2 percentage
  float helium;     // He percentage
  float temperature;
  float pressure;
  String timestamp;
};

// ============================================================================
// External Variables
// ============================================================================

extern unsigned long analyzerLastDataTime;
extern bool analyzerConnected;

// ============================================================================
// Function Prototypes
// ============================================================================

void analyzerInit();
void analyzerLoop();
bool isAnalyzerConnected();
AnalyzerData getAnalyzerData();

#endif // ANALYZER_H
