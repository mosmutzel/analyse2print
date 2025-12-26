/**
 * display.cpp - TFT Display Functions for LilyGo T-Display-S3-Pro
 *
 * This module handles all TFT display operations using LVGL.
 * Uses TFT_eSPI library with ST7796 driver (222x480 pixels).
 */

#include "display.h"
#include "lvgl_hal.h"
#include "ui/ui.h"
#include "ui/vars.h"
#include "ui/screens.h"
#include "ui_functions.h"
#include "touch.h"

// Forward declaration of C function from ui.c
extern "C" {
  void loadScreen(enum ScreensEnum screenId);
}

// ============================================================================
// Global Variables
// ============================================================================

TFT_eSPI tft = TFT_eSPI();
String displayLines[20];
int displayLineIndex = 0;

// Gas sensor data
String lastVar1 = "00.0";  // O2 percentage
String lastVar2 = "00.0";   // He percentage
String lastVar5 = "";      // Date
String mod = "0";           // MOD (Maximum Operating Depth
String end = "0";           // END Equivalent Nacotic Depth
String info = "";          // Additional info
unsigned long lastDataTime = 0;
bool dataPending = false;

// LVGL initialized flag
static bool lvglInitialized = false;

// Screen switching - use definition from display.h
#define BUTTON_PIN SCREEN_SWITCH_PIN
static bool lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;

// Current screen: 0=main, 1=settings, 2=debug
static int currentScreen = 0;
static int previousScreen = 0;  // For home button toggle
#define SCREEN_MAIN     0
#define SCREEN_SETTINGS 1
#define SCREEN_DEBUG    2
#define SCREEN_COUNT    3


// ============================================================================
// Display Functions
// ============================================================================

void displayInit() {
  log_i("[DISPLAY] Starting displayInit()...");

  // Initialize LVGL and display hardware
  log_i("[DISPLAY] Calling lvgl_hal_init()...");
  lvgl_hal_init();
  Serial.println("[DISPLAY] lvgl_hal_init() completed");

  // Initialize UI
  Serial.println("[DISPLAY] Calling ui_init()...");
  ui_init();
  Serial.println("[DISPLAY] ui_init() completed");

  lvglInitialized = true;
  Serial.println("[DISPLAY] LVGL initialized flag set");

  // Initialize button for screen switching
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.print("[DISPLAY] Screen switch button initialized on GPIO");
  Serial.print(BUTTON_PIN);
  Serial.print(", current state: ");
  Serial.println(digitalRead(BUTTON_PIN) ? "HIGH" : "LOW");

  // Show initial status
  Serial.println("[DISPLAY] Setting initial UI values...");
  ui_set_printer_status("Scanning...");
  ui_set_analyzer_status("Waiting...");
  ui_set_date("--");
  ui_set_o2("0.0", 0.0f);
  ui_set_he("0.0", 0.0f);
  ui_set_end("0.0", 0.0f);
  Serial.println("[DISPLAY] displayInit() completed!");
}

void displayPrint(String text) {
  // Add to display buffer
  displayLines[displayLineIndex] = text;
  displayLineIndex = (displayLineIndex + 1) % 20;

  // Update debug screen with latest message
  Serial.println(text);  // Also output to serial
  if (lvglInitialized) {
    ui_set_debug(text.c_str());
  }
}

void displayDebug(String text) {
  displayPrint(text);
}

void displayStatus(String line1, String line2, String line3, String line4) {
  if (!lvglInitialized) return;

  // Update printer status if applicable
  if (line1.length() > 0) {
    ui_set_printer_status(line1.c_str());
  }
}

void displaySensorData() {
  if (!lvglInitialized) return;

  // Update O2 and He values
  float o2_val = lastVar1.toFloat();
  float he_val = lastVar2.toFloat();

  ui_set_o2(lastVar1.c_str(), o2_val);
  ui_set_he(lastVar2.c_str(), he_val);

  // Update date
  if (lastVar5.length() > 0) {
    ui_set_date(lastVar5.c_str());
  }
}

/**
 * Update printer connection status
 */
void displaySetPrinterStatus(const char* status) {
  if (lvglInitialized) {
    ui_set_printer_status(status);
  }
}



/**
 * Update analyzer connection status
 */
void displaySetAnalyzerStatus(const char* status) {
  if (lvglInitialized) {
    ui_set_analyzer_status(status);
  }
}

void displayUpdateBattery(int voltage_mv, bool charging) {
  if (lvglInitialized) {
    ui_set_battery(voltage_mv, charging);
  }
}

void displaySetWifiStatus(bool connected) {
  if (lvglInitialized) {
    ui_set_wifi_status(connected);
  }
}

void displaySetBluetoothStatus(bool connected, bool scanning) {
  if (lvglInitialized) {
    ui_set_bluetooth_status(connected, scanning);
  }
}

void displaySetUsbStatus(bool connected) {
  if (lvglInitialized) {
    ui_set_usb_status(connected);
  }
}

/**
 * Switch to a specific screen
 */
static void switchToScreen(int screen) {
  extern objects_t objects;

  previousScreen = currentScreen;
  currentScreen = screen;

  switch (currentScreen) {
    case SCREEN_MAIN:
      if (objects.main != NULL && lv_obj_is_valid(objects.main)) {
        loadScreen(SCREEN_ID_MAIN);
        log_i("Switched to MAIN screen");
      }
      break;
    case SCREEN_SETTINGS:
      if (objects.settings != NULL && lv_obj_is_valid(objects.settings)) {
        loadScreen(SCREEN_ID_SETTINGS);
        log_i("Switched to SETTINGS screen");
      }
      break;
    case SCREEN_DEBUG:
      if (objects.debug != NULL && lv_obj_is_valid(objects.debug)) {
        loadScreen(SCREEN_ID_DEBUG);
        log_i("Switched to DEBUG screen");
        ui_set_debug("Debug screen active");
      }
      break;
  }
}

/**
 * Switch to the next screen in rotation: main -> settings -> debug -> main
 */
static void switchToNextScreen() {
  int nextScreen = (currentScreen + 1) % SCREEN_COUNT;
  switchToScreen(nextScreen);
}

/**
 * Toggle settings screen (for touch home button)
 * If on settings: go back to previous screen
 * If not on settings: go to settings
 */
void displayToggleSettings() {
  if (currentScreen == SCREEN_SETTINGS) {
    // Go back to previous screen (main or debug)
    switchToScreen(previousScreen != SCREEN_SETTINGS ? previousScreen : SCREEN_MAIN);
  } else {
    // Open settings
    switchToScreen(SCREEN_SETTINGS);
  }
}

/**
 * Check button and switch screens if pressed
 */
static void checkButtonAndSwitchScreen() {
  bool reading = digitalRead(BUTTON_PIN);

  // If button state changed, reset debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If button state has been stable for debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Button pressed (LOW because of INPUT_PULLUP)
    static bool lastStableState = HIGH;

    if (reading != lastStableState) {
      lastStableState = reading;

      // Only act on button press (transition from HIGH to LOW)
      if (reading == LOW) {
        switchToNextScreen();
      }
    }
  }

  lastButtonState = reading;
}

/**
 * Call this from main loop to update LVGL
 */
void displayLoop() {
  if (lvglInitialized) {
    checkButtonAndSwitchScreen();
    ui_update_battery_blink();
    ui_update_bluetooth_blink();
    ui_tick();
    lvgl_hal_loop();
  }
}
