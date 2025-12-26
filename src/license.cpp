/**
 * license.cpp - eFuse-based License Manager
 *
 * Uses ESP32's unique chip ID (eFuse MAC) to generate device-specific licenses.
 * License keys are generated using HMAC-like hashing with a secret key.
 *
 * License Key Format: XXXX-XXXX-XXXX-XXXX (16 hex chars with dashes)
 */

#include "license.h"
#include <Preferences.h>
#include <Arduino.h>
#include "ui/screens.h"
#include "ui/ui.h"

// Secret key for license generation (change this for your deployment!)
// In production, this should be kept secret and not in public source code
#define LICENSE_SECRET "A2P_SECRET_2024_CHANGE_ME"

// NVS namespace and key
#define LICENSE_NVS_NAMESPACE "license"
#define LICENSE_NVS_KEY "key"

// Internal state
static Preferences licensePrefs;
static char deviceId[20] = {0};
static char storedLicense[20] = {0};
static bool isLicensed = false;

// Forward declarations
static void generateDeviceId(void);
static bool validateLicenseKey(const char* deviceId, const char* licenseKey);
static uint32_t simpleHash(const char* str);

void licenseInit(void) {
    // Generate device ID from eFuse
    generateDeviceId();

    // Load stored license
    licensePrefs.begin(LICENSE_NVS_NAMESPACE, true);
    String stored = licensePrefs.getString(LICENSE_NVS_KEY, "");
    strncpy(storedLicense, stored.c_str(), sizeof(storedLicense) - 1);
    licensePrefs.end();

    // Validate stored license
    if (strlen(storedLicense) > 0) {
        isLicensed = validateLicenseKey(deviceId, storedLicense);
    }

    Serial.printf("[LICENSE] Device ID: %s\n", deviceId);
    Serial.printf("[LICENSE] Status: %s\n", isLicensed ? "LICENSED" : "NOT LICENSED");
}

license_status_t licenseCheck(void) {
    if (strlen(storedLicense) == 0) {
        return LICENSE_NOT_FOUND;
    }

    if (validateLicenseKey(deviceId, storedLicense)) {
        return LICENSE_VALID;
    }

    return LICENSE_INVALID;
}

const char* licenseGetDeviceId(void) {
    return deviceId;
}

bool licenseActivate(const char* licenseKey) {
    if (licenseKey == NULL || strlen(licenseKey) < 10) {
        Serial.println("[LICENSE] Invalid key format");
        return false;
    }

    // Remove dashes for validation
    char cleanKey[20] = {0};
    int j = 0;
    for (int i = 0; licenseKey[i] && j < 16; i++) {
        if (licenseKey[i] != '-' && licenseKey[i] != ' ') {
            cleanKey[j++] = toupper(licenseKey[i]);
        }
    }
    cleanKey[j] = '\0';

    // Validate the key
    if (!validateLicenseKey(deviceId, cleanKey)) {
        Serial.printf("[LICENSE] Key validation failed: %s\n", cleanKey);
        return false;
    }

    // Store the license
    licensePrefs.begin(LICENSE_NVS_NAMESPACE, false);
    licensePrefs.putString(LICENSE_NVS_KEY, cleanKey);
    licensePrefs.end();

    strncpy(storedLicense, cleanKey, sizeof(storedLicense) - 1);
    isLicensed = true;

    Serial.println("[LICENSE] Activation successful!");
    return true;
}

void licenseClear(void) {
    licensePrefs.begin(LICENSE_NVS_NAMESPACE, false);
    licensePrefs.remove(LICENSE_NVS_KEY);
    licensePrefs.end();

    storedLicense[0] = '\0';
    isLicensed = false;

    Serial.println("[LICENSE] License cleared");
}

bool licenseIsValid(void) {
    return isLicensed;
}

// Generate device ID from ESP32 eFuse MAC
static void generateDeviceId(void) {
    uint64_t chipId = ESP.getEfuseMac();

    // Format as 12-char hex string (last 6 bytes of MAC)
    snprintf(deviceId, sizeof(deviceId), "%04X%08X",
             (uint16_t)(chipId >> 32),
             (uint32_t)chipId);
}

// Simple hash function for license validation
static uint32_t simpleHash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Validate license key against device ID
static bool validateLicenseKey(const char* devId, const char* licenseKey) {
    // Generate expected license from device ID + secret
    char combined[64];
    snprintf(combined, sizeof(combined), "%s%s", devId, LICENSE_SECRET);

    uint32_t hash = simpleHash(combined);

    // Generate expected key (16 hex chars)
    char expectedKey[20];
    snprintf(expectedKey, sizeof(expectedKey), "%08X%08X",
             hash, hash ^ 0xA2B1C2D3);

    // Compare (case-insensitive)
    for (int i = 0; i < 16 && licenseKey[i] && expectedKey[i]; i++) {
        if (toupper(licenseKey[i]) != toupper(expectedKey[i])) {
            return false;
        }
    }

    return strlen(licenseKey) >= 16;
}

// License activation screen UI
static lv_obj_t* activationScreen = NULL;
static lv_obj_t* licenseTextarea = NULL;
static lv_obj_t* licenseKeyboard = NULL;
static lv_obj_t* deviceIdLabel = NULL;
static lv_obj_t* statusLabel = NULL;

static void activateButtonCb(lv_event_t* e) {
    const char* key = lv_textarea_get_text(licenseTextarea);

    if (licenseActivate(key)) {
        lv_label_set_text(statusLabel, "Lizenz aktiviert!");
        lv_obj_set_style_text_color(statusLabel, lv_palette_main(LV_PALETTE_GREEN), 0);

        // Go to main screen after delay
        delay(1500);
        loadScreen(SCREEN_ID_MAIN);
    } else {
        lv_label_set_text(statusLabel, "Ungueltige Lizenz!");
        lv_obj_set_style_text_color(statusLabel, lv_palette_main(LV_PALETTE_RED), 0);
    }
}

void licenseShowActivation(void) {
    // Create activation screen
    activationScreen = lv_obj_create(NULL);
    lv_obj_set_size(activationScreen, 222, 480);
    lv_obj_set_style_bg_color(activationScreen, lv_color_black(), 0);
    lv_obj_clear_flag(activationScreen, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t* title = lv_label_create(activationScreen);
    lv_obj_set_pos(title, 0, 10);
    lv_obj_set_size(title, 222, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_label_set_text(title, LV_SYMBOL_WARNING " Lizenz erforderlich");

    // Device ID label
    deviceIdLabel = lv_label_create(activationScreen);
    lv_obj_set_pos(deviceIdLabel, 0, 40);
    lv_obj_set_size(deviceIdLabel, 222, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(deviceIdLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(deviceIdLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(deviceIdLabel, lv_color_hex(0x888888), 0);
    char idText[48];
    snprintf(idText, sizeof(idText), "Geraete-ID: %s", deviceId);
    lv_label_set_text(deviceIdLabel, idText);

    // Instructions
    lv_obj_t* instr = lv_label_create(activationScreen);
    lv_obj_set_pos(instr, 10, 60);
    lv_obj_set_size(instr, 202, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(instr, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(instr, lv_color_white(), 0);
    lv_label_set_text(instr, "Bitte Lizenzschluessel eingeben:");

    // License key textarea
    licenseTextarea = lv_textarea_create(activationScreen);
    lv_obj_set_pos(licenseTextarea, 10, 80);
    lv_obj_set_size(licenseTextarea, 202, 40);
    lv_textarea_set_one_line(licenseTextarea, true);
    lv_textarea_set_placeholder_text(licenseTextarea, "XXXX-XXXX-XXXX-XXXX");
    lv_textarea_set_max_length(licenseTextarea, 19);
    lv_obj_set_style_text_font(licenseTextarea, &lv_font_montserrat_14, 0);
    lv_obj_set_style_bg_color(licenseTextarea, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_text_color(licenseTextarea, lv_color_white(), 0);

    // Activate button
    lv_obj_t* btn = lv_btn_create(activationScreen);
    lv_obj_set_pos(btn, 10, 125);
    lv_obj_set_size(btn, 202, 40);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E7D32), 0);
    lv_obj_add_event_cb(btn, activateButtonCb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btnLabel = lv_label_create(btn);
    lv_obj_center(btnLabel);
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_16, 0);
    lv_label_set_text(btnLabel, "Aktivieren");

    // Status label
    statusLabel = lv_label_create(activationScreen);
    lv_obj_set_pos(statusLabel, 0, 170);
    lv_obj_set_size(statusLabel, 222, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(statusLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x888888), 0);
    lv_label_set_text(statusLabel, "");

    // Keyboard
    licenseKeyboard = lv_keyboard_create(activationScreen);
    lv_obj_set_size(licenseKeyboard, 222, 280);
    lv_obj_align(licenseKeyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(licenseKeyboard, licenseTextarea);

    // Load the screen
    lv_scr_load(activationScreen);
}

// Tool function to generate license key for a device ID
// Call this from a separate tool/script, NOT in the firmware!
#ifdef LICENSE_GENERATOR_TOOL
void licenseGenerateKey(const char* devId) {
    char combined[64];
    snprintf(combined, sizeof(combined), "%s%s", devId, LICENSE_SECRET);

    uint32_t hash = simpleHash(combined);

    printf("Device ID: %s\n", devId);
    printf("License Key: %04X-%04X-%04X-%04X\n",
           (hash >> 16) & 0xFFFF,
           hash & 0xFFFF,
           ((hash ^ 0xA2B1C2D3) >> 16) & 0xFFFF,
           (hash ^ 0xA2B1C2D3) & 0xFFFF);
}
#endif
