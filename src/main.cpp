/**
 * main.cpp - Niimbot B1 Main Program
 *
 * Main entry point for the Niimbot B1 printer controller.
 * Handles BLE connection, sensor data processing, and automatic label printing.
 *
 * Hardware: LilyGo T-Display-S3-Pro (ESP32-S3 with BLE support)
 * Display: ST7796 222x480 TFT with LVGL
 * Printer: Niimbot B1 thermal label printer
 */

#include "print.h"
#include "display.h"
#include "analyzer.h"
#include "light_sensor.h"
#include <XPowersLib.h>
#include "utilities.h"
#include "esp_sleep.h"
#include "ui/vars.h"
#include "touch.h"
#include "settings.h"
#include "wifi_manager.h"
#include "wifi_ui.h"
#include "names.h"
#include "license.h"
#include "ble_printer.h"
#include "webserver.h"
#include "version.h"

// ============================================================================
// Configuration
// ============================================================================

PowersSY6970 PMU;

uint32_t cycleInterval;

// ============================================================================
// BLE Global Variables (managed by ble_printer module)
// ============================================================================

// Connection status is now managed by ble_printer.cpp
// Use blePrinterIsConnected() instead of 'connected' variable

// Deep sleep variables
static unsigned long btn1PressStart = 0;
static bool btn1WasPressed = false;
#define DEEP_SLEEP_HOLD_TIME 2000  // Hold BTN1 for 2 seconds to enter deep sleep

// ============================================================================
// Deep Sleep Functions
// ============================================================================

void enterDeepSleep() {
  displayDebug("Entering deep sleep...");
  delay(500);

  // Disable USB OTG power
  PMU.disableOTG();

  // Turn off display backlight
  pinMode(BOARD_TFT_BL, OUTPUT);
  digitalWrite(BOARD_TFT_BL, LOW);

  // Configure BTN1 (GPIO0) as wake-up source
  // GPIO0 is active LOW, so we wake on LOW level
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BOARD_BTN1, 0);

  // Enter deep sleep
  esp_deep_sleep_start();
}

void checkDeepSleepButton() {
  bool btn1State = digitalRead(BOARD_BTN1);

  if (btn1State == LOW) {  // Button pressed (active LOW)
    if (!btn1WasPressed) {
      // Button just pressed
      btn1WasPressed = true;
      btn1PressStart = millis();
    } else {
      // Button held - check if held long enough
      if (millis() - btn1PressStart >= DEEP_SLEEP_HOLD_TIME) {
        enterDeepSleep();
      }
    }
  } else {
    // Button released
    btn1WasPressed = false;
  }
}

// BLE callbacks are now in ble_printer.cpp

// Connection functions are now in ble_printer.cpp

// ============================================================================
// Helper Functions
// ============================================================================
static float ppO2max = 1.2f;             // Maximum ppO2 in bar
static float ppO2maxDeco = 1.6f;             // Maximum ppO2 in bar
void printDemo() {
  clearBitmap();
  drawRect(5, 5, labelWidth - 10, labelHeight - 10);
  drawText(20, 20, "NIIMBOT B1", 3);
  drawText(20, 70, "ESP32 BLE TEST", 2);
  fillRect(20, 120, 100, 50);
  drawText(20, 190, "HELLO WORLD", 2);
  printLabel();
}



/**
 * Calculate MOD (Maximum Operating Depth) for a given O2 percentage
 * MOD = ((ppO2max / FO2) - 1) * 10
 * Using ppO2max = 1.2 bar for technical diving
 */
static float calculate_mod(float o2_percent) {
         if (o2_percent <= 0.0f || o2_percent > 100.0f) {
        return 0.0f;
    }
    if (o2_percent > 0.0f && o2_percent < 40.0f){
    float fo2 = o2_percent / 100.0f;  // Fraction of O2

    float mod = ((ppO2max / fo2) - 1.0f) * 10.0f;  // Depth in meters
    return mod > 0 ? mod : 0;
    }
    else if (o2_percent >= 40.0f){
        float fo2 = o2_percent / 100.0f;  // Fraction of O2

    float mod = ((ppO2maxDeco / fo2) - 1.0f) * 10.0f;  // Depth in meters
    return mod > 0 ? mod : 0;
    }    
}


// ============================================================================
// Setup and Loop
// ============================================================================

void setup() {
  //Serial.begin(115200);
  //delay(1000);

  // Initialize BTN1 for deep sleep trigger
  pinMode(BOARD_BTN1, INPUT_PULLUP);

  // Initialize BTN3 for manual print trigger
  pinMode(BOARD_BTN3, INPUT_PULLUP);

  bool result = PMU.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, SY6970_SLAVE_ADDRESS);
  
  log_i("=== Analyzer Interface Starting ===");
  log_i("Initializing PMU...");
  // Set USB input current limit
  PMU.setInputCurrentLimit(1000);

  // The onboard battery is fully charged at 4.35V
  // Set the charging target voltage, Range:3840 ~ 4608mV ,step:16 mV
  PMU.setChargeTargetVoltage(4352);

  // Set the precharge current , Range: 64mA ~ 1024mA ,step:64mA
  PMU.setPrechargeCurr(64);

  // Set the charging current , Range:0~5056mA ,step:64mA
  PMU.setChargerConstantCurr(320);

  // When the battery is not connected and the USB is plugged in, the board's LED status indicator light will flash.
  // You can use `PMU.disableStatLed();` to turn off the indicator light,
  // but this means that if the battery is connected for charging, the LED light will also be disabled.
  // PMU.disableStatLed();
  // Enable charging led
  PMU.enableStatLed();

  // To obtain voltage data, the ADC must be enabled first
  PMU.enableADCMeasure();


  // For devices that have been connected to the battery, the charging function is turned on by default.
  PMU.enableCharge();

  // For boards without an external battery, the charging function should be turned off, otherwise the power supply current of the power chip will be limited.
  //PMU.disableCharge();
  log_i("PMU init completed");

  log_i("Initializing display...");

  displayInit();
  displayLoop();  // Initial display update to show UI

  log_i("Display init completed");

  // Initialize touch controller
  log_i("Initializing touch...");
  if (touchInit()) {
    touchRegisterLvgl();
    // Set home button callback to toggle settings screen
    touchSetHomeCallback(displayToggleSettings);
    log_i("Touch initialized");
  } else {
    log_i("Touch not found");
  }

  // Initialize settings (button callbacks)
  log_i("Initializing settings...");
  settingsInit();

  // Initialize names manager
  log_i("Initializing names...");
  namesInit();

  // Initialize WiFi manager
  log_i("Initializing WiFi manager...");
  wifiManagerInit();
  wifiUIInit();

  // Initialize webserver (handlers registered, started when WiFi connects)
  log_i("Initializing webserver...");
  webserverInit();

  // Initialize license manager
  log_i("Initializing license manager...");
  licenseInit();

  // Check license - show activation screen if not licensed
  if (!licenseIsValid()) {
    log_i("License not valid - showing activation screen");
    licenseShowActivation();
    // Keep running LVGL loop for activation screen
    while (!licenseIsValid()) {
      displayLoop();
      delay(10);
    }
    log_i("License activated - continuing startup");
  }

  // Initialize light sensor for auto-brightness
  // NOTE: Temporarily disabled - may cause I2C conflicts
  // log_i("Initializing light sensor...");
  // if (lightSensorInit()) {
  //   log_i("Light sensor ready - auto-brightness enabled");
  // } else {
  //   log_i("Light sensor not found - using fixed brightness");
  // }

  // Initialize USB Host for Analyzer
  displaySetAnalyzerStatus("Init USB...");
  displayLoop();  // Update display
  log_i("Initializing Analyzer USB Host...");
  analyzerInit();
  delay(1000);
  log_i("Analyzer init completed");

  // Initialize BLE Printer module (runs on Core 1)
  displaySetPrinterStatus("Init BLE...");
  displayLoop();  // Update display before BLE init
  log_i("Initializing BLE Printer module...");
  blePrinterInit();
  displayLoop();  // Update display after BLE init

  // Start scanning for printer (non-blocking)
  displaySetPrinterStatus("Scanning...");
  displaySetBluetoothStatus(false, true);  // Scanning - blink
  blePrinterStartScan();

  displaySetAnalyzerStatus("Waiting...");
  displayLoop();  // Update display

  // WiFi not used - show as inactive
  displaySetWifiStatus(false);

  // Set version on main screen
  ui_set_settings_version(FIRMWARE_VERSION);
}

void loop() {
  static unsigned long lastHB = 0;
  static unsigned long lastDisplayUpdate = 0;

  extern String lastVar1;
  extern String lastVar2;
  extern String lastVar5;
  extern String mod;
  extern String info;
  extern unsigned long lastDataTime;
  extern bool dataPending;

  // Check BTN1 for deep sleep (hold 2 seconds)
  checkDeepSleepButton();

  // Check BTN3 for manual print trigger
  static bool btn3WasPressed = false;
  static unsigned long lastPrintTime = 0;
  static unsigned long printOverlayHideTime = 0;
  bool btn3State = digitalRead(BOARD_BTN3);

  if (btn3State == LOW && !btn3WasPressed) {  // Button just pressed (active LOW)
    btn3WasPressed = true;
    // Debounce: only allow print every 2 seconds
    if (blePrinterIsConnected() && (millis() - lastPrintTime) > 2000) {
      lastPrintTime = millis();
      displayDebug("BTN3: Manual print");
      displayDebug("O2=" + lastVar1 + " He=" + lastVar2);
      // Show print overlay
      ui_show_print_overlay();
      printOverlayHideTime = millis() + 3000;  // Hide after 3 seconds
      printGasLabel(lastVar1.c_str(), lastVar2.c_str(), mod.c_str(), info.c_str(), lastVar5.c_str());
      displayDebug("Print job sent");
    } else if (!blePrinterIsConnected()) {
      displayDebug("BTN3: Not connected!");
    }
  } else if (btn3State == HIGH) {
    btn3WasPressed = false;
  }

  // Hide print overlay after timeout
  if (printOverlayHideTime > 0 && millis() >= printOverlayHideTime) {
    ui_hide_print_overlay();
    printOverlayHideTime = 0;
  }

  // Update battery status every second
  static unsigned long lastBattUpdate = 0;
  static bool wasWifiConnected = false;
  static bool versionSet = false;
  if (millis() - lastBattUpdate > 1000) {
    lastBattUpdate = millis();
    int battVoltage = PMU.getBattVoltage();
    bool isCharging = (PMU.chargeStatus() != 0);  // 0 = not charging
    displayUpdateBattery(battVoltage, isCharging);

    // Update WiFi status icon based on connection state
    bool wifiConnected = wifiManagerIsConnected();
    displaySetWifiStatus(wifiConnected);

    // Start/stop webserver based on WiFi connection
    if (wifiConnected && !wasWifiConnected) {
      webserverStart();
    } else if (!wifiConnected && wasWifiConnected) {
      webserverStop();
    }
    wasWifiConnected = wifiConnected;

    // Update current name on main screen
    ui_set_name(namesGetCurrent());

    // Set version on main screen (once, after UI is ready)
    if (!versionSet) {
      ui_set_settings_version(FIRMWARE_VERSION);
      versionSet = true;
    }
  }

  // Handle webserver requests
  webserverLoop();

  // Update display brightness based on ambient light
  // NOTE: Temporarily disabled - may cause I2C conflicts
  // lightSensorUpdateBrightness();

  // Handle WiFi UI updates (scan results etc.)
  wifiUITick();

  // Update LVGL display
  displayLoop();

/*
  // PMU - SY6970 When VBUS is input, the battery voltage detection will not take effect
  if (millis() > cycleInterval) {

      
        log_i("Sats        VBUS    VBAT   SYS    VbusStatus      String   ChargeStatus     String      TargetVoltage       ChargeCurrent       Precharge       NTCStatus           String");
        log_i("            (mV)    (mV)   (mV)   (HEX)                         (HEX)                    (mV)                 (mA)                   (mA)           (HEX)           ");
        //log_i("--------------------------------------------------------------------------------------------------------------------------------");
        log_i(PMU.isVbusIn() ? "Connected" : "Disconnect");
        log_i(PMU.getVbusVoltage());
        log_i(PMU.getBattVoltage()); Serial.print("\t");
        Serial.print(PMU.getSystemVoltage()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(PMU.getBusStatus(), HEX); Serial.print("\t");
        Serial.print(PMU.getBusStatusString()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(PMU.chargeStatus(), HEX); Serial.print("\t");
        Serial.print(PMU.getChargeStatusString()); Serial.print("\t");

        Serial.print(PMU.getChargeTargetVoltage()); Serial.print("\t");
        Serial.print(PMU.getChargerConstantCurr()); Serial.print("\t");
        Serial.print(PMU.getPrechargeCurr()); Serial.print("\t");
        Serial.print(PMU.getNTCStatus()); Serial.print("\t");
        Serial.print(PMU.getNTCStatusString()); Serial.print("\t");

        Serial.println();
        Serial.println();

        cycleInterval = millis() + 1000;
    }
  */


  if (blePrinterIsConnected() && (millis() - lastHB) > 5000) {
    sendHeartbeat();
    lastHB = millis();
  }

  // Serial commands removed - no serial available in USB Host mode

  analyzerLoop();

  // Update analyzer status and USB symbol
  static bool wasAnalyzerConnected = false;
  bool analyzerConnected = isAnalyzerConnected();

  if (analyzerConnected != wasAnalyzerConnected) {
    log_i("Analyzer connection changed: %s", analyzerConnected ? "Connected" : "Waiting...");
    displaySetAnalyzerStatus(analyzerConnected ? "Connected" : "Waiting...");
    displaySetUsbStatus(analyzerConnected);  // USB symbol reflects analyzer connection
    displayDebug(analyzerConnected ? "Analyzer: Connected" : "Analyzer: Waiting...");
    wasAnalyzerConnected = analyzerConnected;
  }

  if (analyzerConnected) {
    static unsigned long lastProcessedTime = 0;
    AnalyzerData data = getAnalyzerData();
    if (data.valid && analyzerLastDataTime > lastProcessedTime) {
      lastProcessedTime = analyzerLastDataTime;

      lastVar1 = String(data.oxygen, 1);
      lastVar2 = String(data.helium, 1);

      if (data.timestamp.length() > 0) {
        int firstSlash = data.timestamp.indexOf("/");
        int secondSlash = data.timestamp.indexOf("/", firstSlash + 1);
        int space = data.timestamp.indexOf(" ");
        if (firstSlash > 0 && secondSlash > firstSlash && space > secondSlash) {
          String year = data.timestamp.substring(0, firstSlash);
          String month = data.timestamp.substring(firstSlash + 1, secondSlash);
          String day = data.timestamp.substring(secondSlash + 1, space);
          lastVar5 = day + "." + month + "." + year;
        }
      }

      mod = String(calculate_mod(String(lastVar1).toFloat()), 0);  // MOD in meters, no decimals
      info = String(v_name);  // Use v_name variable for printing


      lastDataTime = millis();
      dataPending = true;
      displaySensorData();
    }
  }

  if (dataPending && (millis() - lastDisplayUpdate) > 1000) {
    displaySensorData();
    lastDisplayUpdate = millis();
  }
/*
  if (dataPending && blePrinterIsConnected() && (millis() - lastDataTime) >= DATA_TIMEOUT) {
    displayDebug("Print: O2=" + lastVar1 + " He=" + lastVar2);
    displayDebug("Date: " + lastVar5);
    //printGasLabelSimple(lastVar1.c_str(), lastVar2.c_str(), lastVar5.c_str());
    printGasLabel(lastVar1.c_str(), lastVar2.c_str(), mod.c_str(), info.c_str(), lastVar5.c_str());
    dataPending = false;
    displayDebug("Print job sent");
  }
*/

  // Update BLE printer UI status (non-blocking, handles reconnection automatically)
  blePrinterUpdateUI();



  delay(5);
}
