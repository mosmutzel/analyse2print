/**
 * settings.h - Settings Manager
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <lvgl.h>

/**
 * Initialize settings screen button callbacks
 */
void settingsInit();

/**
 * Update battery button label based on current state
 */
void settingsUpdateBatteryLabel();

/**
 * Update WiFi button label based on current state
 */
void settingsUpdateWifiLabel();

/**
 * Toggle battery charging
 */
void settingsToggleBatteryCharging();

/**
 * Toggle WiFi/OTA
 */
void settingsToggleWifi();

/**
 * Get current battery charging state
 */
bool settingsIsBatteryChargingEnabled();

/**
 * Get current WiFi state
 */
bool settingsIsWifiEnabled();

#endif // SETTINGS_H
