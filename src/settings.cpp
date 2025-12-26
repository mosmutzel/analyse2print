/**
 * settings.cpp - Settings Manager
 *
 * Handles settings screen button events and state management
 * - Battery charging toggle (PMU OTG control)
 * - WiFi connection manager
 * - Firmware update check
 */

#include "settings.h"
#include "ui/screens.h"
#include "ui/vars.h"
#include <XPowersLib.h>
#include "wifi_manager.h"
#include "wifi_ui.h"
#include "ota_update.h"
#include "version.h"

// External PMU instance from main.cpp
extern PowersSY6970 PMU;

// Settings state
static bool batteryChargingEnabled = false;  // Default: OTG enabled (charging disabled)

// Button event callbacks
static void batteryBtnEventCb(lv_event_t *e);
static void wifiBtnEventCb(lv_event_t *e);
static void infoBtnEventCb(lv_event_t *e);

/**
 * Initialize settings screen button callbacks
 */
void settingsInit() {
    extern objects_t objects;

    // Wait for screens to be created
    if (objects.btn_settings_battery == NULL || objects.btn_settings_wifi == NULL) {
        return;
    }

    // Add event callbacks to buttons
    lv_obj_add_event_cb(objects.btn_settings_battery, batteryBtnEventCb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.btn_settings_wifi, wifiBtnEventCb, LV_EVENT_CLICKED, NULL);

    // Add info/update button callback
    if (objects.btn_settings_gear != NULL) {
        lv_obj_add_event_cb(objects.btn_settings_gear, infoBtnEventCb, LV_EVENT_CLICKED, NULL);
    }

    // Update button labels to reflect initial state
    settingsUpdateBatteryLabel();
    settingsUpdateWifiLabel();

    // Set version on settings screen
    ui_set_settings_version(FIRMWARE_VERSION);

    Serial.println("[SETTINGS] Initialized");
}

/**
 * Update battery button label based on current state
 */
void settingsUpdateBatteryLabel() {
    extern objects_t objects;

    if (objects.lbl_battery_status == NULL) return;

    if (batteryChargingEnabled) {
        lv_label_set_text(objects.lbl_battery_status, LV_SYMBOL_BATTERY_FULL "\nLaden: EIN");
        lv_obj_set_style_bg_color(objects.btn_settings_battery, lv_color_hex(0x2E7D32), LV_PART_MAIN);  // Green
    } else {
        lv_label_set_text(objects.lbl_battery_status, LV_SYMBOL_BATTERY_FULL "\nLaden: AUS");
        lv_obj_set_style_bg_color(objects.btn_settings_battery, lv_color_hex(0xC62828), LV_PART_MAIN);  // Red
    }
}

/**
 * Update WiFi button label based on current state
 */
void settingsUpdateWifiLabel() {
    wifiManagerUpdateUI();
}

/**
 * Toggle battery charging
 * When charging is enabled: OTG is disabled
 * When charging is disabled: OTG is enabled (for USB analyzer)
 */
void settingsToggleBatteryCharging() {
    batteryChargingEnabled = !batteryChargingEnabled;

    if (batteryChargingEnabled) {
        // Enable charging = disable OTG
        PMU.disableOTG();
        PMU.enableCharge();
        Serial.println("[SETTINGS] Battery charging ENABLED (OTG disabled)");
    } else {
        // Disable charging = enable OTG for USB analyzer
        //PMU.disableCharge();
        PMU.enableOTG();
        Serial.println("[SETTINGS] Battery charging DISABLED (OTG enabled)");
    }

    settingsUpdateBatteryLabel();
}

/**
 * Toggle WiFi - show WiFi screen or disconnect
 */
void settingsToggleWifi() {
    if (wifiManagerIsConnected()) {
        // Disconnect
        wifiManagerDisconnect();
        wifiManagerUpdateUI();
        Serial.println("[SETTINGS] WiFi disconnected");
    } else {
        // Show WiFi scan screen
        wifiUIShow();
        Serial.println("[SETTINGS] Opening WiFi screen");
    }
}

/**
 * Get current battery charging state
 */
bool settingsIsBatteryChargingEnabled() {
    return batteryChargingEnabled;
}

/**
 * Get current WiFi state
 */
bool settingsIsWifiEnabled() {
    return wifiManagerIsConnected();
}

/**
 * Battery button click callback
 */
static void batteryBtnEventCb(lv_event_t *e) {
    settingsToggleBatteryCharging();
}

/**
 * WiFi button click callback
 */
static void wifiBtnEventCb(lv_event_t *e) {
    settingsToggleWifi();
}

/**
 * Info button click callback - check for firmware update
 */
static void infoBtnEventCb(lv_event_t *e) {
    extern objects_t objects;

    Serial.println("[SETTINGS] Info button clicked - checking for update");

    // Check if WiFi is connected
    if (!wifiManagerIsConnected()) {
        // Show message that WiFi is required
        lv_obj_t* msgbox = lv_msgbox_create(NULL, "Update", "Bitte zuerst mit WLAN verbinden!", NULL, true);
        lv_obj_center(msgbox);
        return;
    }

    // Show checking message
    lv_obj_t* msgbox = lv_msgbox_create(NULL, "Update", "Pruefe auf Updates...", NULL, false);
    lv_obj_center(msgbox);
    lv_refr_now(NULL);

    // Check for update
    bool updateAvailable = otaCheckForUpdate();

    // Close checking message
    lv_msgbox_close(msgbox);

    if (updateAvailable) {
        // Show update available dialog
        char msg[256];
        snprintf(msg, sizeof(msg), "Neue Version verfuegbar!\n\nAktuell: %s\nNeu: %s\n\n%s",
                 FIRMWARE_VERSION, otaGetLatestVersion(), otaGetChangelog());

        static const char* btns[] = {"Update", "Abbrechen", ""};
        lv_obj_t* updateBox = lv_msgbox_create(NULL, "Update verfuegbar", msg, btns, false);
        lv_obj_center(updateBox);

        // Add callback for button clicks
        lv_obj_add_event_cb(updateBox, [](lv_event_t* e) {
            lv_obj_t* obj = lv_event_get_current_target(e);
            const char* btn_text = lv_msgbox_get_active_btn_text(obj);

            if (btn_text && strcmp(btn_text, "Update") == 0) {
                // Close the dialog
                lv_msgbox_close(obj);

                // Show progress
                lv_obj_t* progressBox = lv_msgbox_create(NULL, "Update", "Update wird installiert...\nBitte warten!", NULL, false);
                lv_obj_center(progressBox);
                lv_refr_now(NULL);

                // Start update
                otaStartUpdate();
                // If we get here, update failed
                lv_msgbox_close(progressBox);
                lv_obj_t* errorBox = lv_msgbox_create(NULL, "Fehler", "Update fehlgeschlagen!", NULL, true);
                lv_obj_center(errorBox);
            } else {
                lv_msgbox_close(obj);
            }
        }, LV_EVENT_VALUE_CHANGED, NULL);
    } else {
        // Show already up to date
        char msg[128];
        snprintf(msg, sizeof(msg), "Firmware ist aktuell!\n\nVersion: %s", FIRMWARE_VERSION);
        lv_obj_t* infoBox = lv_msgbox_create(NULL, "Info", msg, NULL, true);
        lv_obj_center(infoBox);
    }
}
