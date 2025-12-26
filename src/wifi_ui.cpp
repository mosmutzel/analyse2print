#include "wifi_ui.h"
#include "wifi_manager.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include <Arduino.h>

// Selected network for connection
static char selectedSSID[WIFI_SSID_MAX_LENGTH] = {0};
static bool selectedEncrypted = false;

// Forward declarations
static void wifiNetworkClickHandler(lv_event_t *e);
static void wifiConnectClickHandler(lv_event_t *e);
static void wifiBackClickHandler(lv_event_t *e);
static void showPasswordInput(void);
static void showNetworkList(void);

// Initialize WiFi UI callbacks
void wifiUIInit(void) {
    // Connect button callback
    if (objects.wifi_connect_btn) {
        lv_obj_add_event_cb(objects.wifi_connect_btn, wifiConnectClickHandler, LV_EVENT_CLICKED, NULL);
    }

    // Back button callback
    if (objects.wifi_back_btn) {
        lv_obj_add_event_cb(objects.wifi_back_btn, wifiBackClickHandler, LV_EVENT_CLICKED, NULL);
    }
}

// Update WiFi list with scan results
void wifiUIUpdateList(void) {
    if (objects.wifi_list == NULL) return;

    // Clear existing list
    lv_obj_clean(objects.wifi_list);

    wifi_network_t networks[15];
    int count = wifiManagerGetScanResults(networks, 15);

    if (count < 0) {
        // Still scanning
        lv_label_set_text(objects.wifi_status, "Suche Netzwerke...");
        return;
    }

    if (count == 0) {
        lv_label_set_text(objects.wifi_status, "Keine Netzwerke gefunden");
        return;
    }

    char statusBuf[32];
    snprintf(statusBuf, sizeof(statusBuf), "%d Netzwerke gefunden", count);
    lv_label_set_text(objects.wifi_status, statusBuf);

    // Add networks to list
    for (int i = 0; i < count; i++) {
        char itemText[64];
        const char* lockIcon = networks[i].encrypted ? LV_SYMBOL_EYE_CLOSE : "";
        const char* savedIcon = networks[i].saved ? LV_SYMBOL_OK : "";

        // Signal strength indicator
        const char* signalIcon;
        if (networks[i].rssi > -50) {
            signalIcon = LV_SYMBOL_WIFI;
        } else if (networks[i].rssi > -70) {
            signalIcon = LV_SYMBOL_WIFI;
        } else {
            signalIcon = LV_SYMBOL_WIFI;
        }

        snprintf(itemText, sizeof(itemText), "%s %s%s %s",
                 signalIcon, networks[i].ssid, lockIcon, savedIcon);

        lv_obj_t *btn = lv_list_add_btn(objects.wifi_list, NULL, itemText);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2a2a2a), LV_PART_MAIN);
        lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);

        // Store network info in button user data
        // We use a simple approach: store index as user data
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, wifiNetworkClickHandler, LV_EVENT_CLICKED, &networks[i]);

        // Copy SSID to button's user data (we need to store it differently)
        // For simplicity, we'll use the button's child label text
    }
}

// Called periodically to check scan status
void wifiUITick(void) {
    static unsigned long lastCheck = 0;
    unsigned long now = millis();

    // Check every 500ms
    if (now - lastCheck < 500) return;
    lastCheck = now;

    wifi_state_t state = wifiManagerGetState();

    if (state == WIFI_STATE_SCANNING) {
        wifi_network_t networks[15];
        int count = wifiManagerGetScanResults(networks, 15);
        if (count >= 0) {
            // Scan complete, update list
            wifiUIUpdateList();
        }
    }
}

// Network item click handler
static void wifiNetworkClickHandler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);

    // Get the label text from the button
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    if (label == NULL) return;

    const char* text = lv_label_get_text(label);
    if (text == NULL) return;

    // Parse SSID from button text (format: "WIFI_SYMBOL SSID[LOCK]")
    // Skip the WiFi symbol and space
    const char* ssidStart = text;
    while (*ssidStart && (*ssidStart == ' ' || (unsigned char)*ssidStart > 127)) {
        ssidStart++;
    }
    // Skip leading spaces
    while (*ssidStart == ' ') ssidStart++;

    // Copy SSID until we hit lock symbol, checkmark, or end
    int i = 0;
    while (ssidStart[i] && ssidStart[i] != ' ' &&
           (unsigned char)ssidStart[i] < 127 && i < WIFI_SSID_MAX_LENGTH - 1) {
        selectedSSID[i] = ssidStart[i];
        i++;
    }
    // Handle SSIDs with spaces - look for lock/check symbols
    while (ssidStart[i] && i < WIFI_SSID_MAX_LENGTH - 1) {
        if ((unsigned char)ssidStart[i] > 127) break;  // Symbol found
        selectedSSID[i] = ssidStart[i];
        i++;
    }
    // Trim trailing spaces
    while (i > 0 && selectedSSID[i-1] == ' ') i--;
    selectedSSID[i] = '\0';

    // Check if encrypted (has lock symbol)
    selectedEncrypted = (strstr(text, LV_SYMBOL_EYE_CLOSE) != NULL);

    Serial.printf("[WiFi UI] Selected: %s (encrypted: %d)\n", selectedSSID, selectedEncrypted);

    if (selectedEncrypted) {
        // Show password input
        showPasswordInput();
    } else {
        // Connect directly (open network)
        lv_label_set_text(objects.wifi_status, "Verbinde...");
        if (wifiManagerConnect(selectedSSID, NULL)) {
            wifiManagerSaveNetwork(selectedSSID, "");
            wifiManagerUpdateUI();
            loadScreen(SCREEN_ID_SETTINGS);
        } else {
            lv_label_set_text(objects.wifi_status, "Verbindung fehlgeschlagen");
            showNetworkList();
        }
    }
}

// Connect button handler
static void wifiConnectClickHandler(lv_event_t *e) {
    const char* password = lv_textarea_get_text(objects.wifi_password_ta);

    lv_label_set_text(objects.wifi_status, "Verbinde...");

    // Hide keyboard during connection
    lv_obj_add_flag(objects.wifi_keyboard, LV_OBJ_FLAG_HIDDEN);

    if (wifiManagerConnect(selectedSSID, password)) {
        // Save network on successful connection
        wifiManagerSaveNetwork(selectedSSID, password);
        wifiManagerUpdateUI();
        loadScreen(SCREEN_ID_SETTINGS);
    } else {
        lv_label_set_text(objects.wifi_status, "Verbindung fehlgeschlagen");
        // Show keyboard again
        lv_obj_clear_flag(objects.wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// Back button handler
static void wifiBackClickHandler(lv_event_t *e) {
    // If showing password input, go back to list
    if (!lv_obj_has_flag(objects.wifi_password_ta, LV_OBJ_FLAG_HIDDEN)) {
        showNetworkList();
    } else {
        // Go back to settings
        loadScreen(SCREEN_ID_SETTINGS);
    }
}

// Show password input UI
static void showPasswordInput(void) {
    // Hide list
    lv_obj_add_flag(objects.wifi_list, LV_OBJ_FLAG_HIDDEN);

    // Update title to show selected network
    char title[64];
    snprintf(title, sizeof(title), LV_SYMBOL_WIFI " %s", selectedSSID);
    lv_label_set_text(objects.wifi_title, title);
    lv_label_set_text(objects.wifi_status, "Passwort eingeben:");

    // Show password input elements
    lv_obj_clear_flag(objects.wifi_password_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(objects.wifi_password_ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(objects.wifi_connect_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(objects.wifi_back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(objects.wifi_keyboard, LV_OBJ_FLAG_HIDDEN);

    // Clear previous password
    lv_textarea_set_text(objects.wifi_password_ta, "");

    // Connect keyboard to textarea
    lv_keyboard_set_textarea(objects.wifi_keyboard, objects.wifi_password_ta);
}

// Show network list UI
static void showNetworkList(void) {
    // Show list
    lv_obj_clear_flag(objects.wifi_list, LV_OBJ_FLAG_HIDDEN);

    // Reset title
    lv_label_set_text(objects.wifi_title, LV_SYMBOL_WIFI " WLAN");

    // Hide password input elements
    lv_obj_add_flag(objects.wifi_password_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.wifi_password_ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.wifi_connect_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.wifi_back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.wifi_keyboard, LV_OBJ_FLAG_HIDDEN);

    // Refresh scan
    wifiManagerStartScan();
}

// Show WiFi screen and start scan
void wifiUIShow(void) {
    showNetworkList();
    loadScreen(SCREEN_ID_WIFI);
    wifiManagerStartScan();
}
