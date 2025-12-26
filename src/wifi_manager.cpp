#include "wifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>
#include <Arduino.h>
#include "ui/screens.h"
#include "ui/ui.h"

// NVS namespace for WiFi credentials
#define WIFI_NVS_NAMESPACE "wifi_creds"

// Internal state
static wifi_state_t currentState = WIFI_STATE_DISCONNECTED;
static char connectedSSID[WIFI_SSID_MAX_LENGTH] = {0};
static char ipAddress[16] = {0};
static Preferences wifiPrefs;

// Saved networks storage
static char savedSSIDs[WIFI_MAX_SAVED_NETWORKS][WIFI_SSID_MAX_LENGTH];
static char savedPasswords[WIFI_MAX_SAVED_NETWORKS][WIFI_PASS_MAX_LENGTH];
static int savedNetworkCount = 0;

// Scan results
static wifi_network_t scanResults[20];
static int scanResultCount = 0;

// Forward declarations
static void loadSavedNetworks(void);
static void saveSavedNetworks(void);
static bool isNetworkSaved(const char* ssid);

void wifiManagerInit(void) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    currentState = WIFI_STATE_DISCONNECTED;

    // Load saved networks from NVS
    loadSavedNetworks();

    Serial.printf("[WiFi] Initialized, %d saved networks\n", savedNetworkCount);

    // Try to auto-connect to saved networks
    if (savedNetworkCount > 0) {
        Serial.println("[WiFi] Attempting auto-connect on startup...");
        if (wifiManagerAutoConnect()) {
            Serial.println("[WiFi] Auto-connect successful!");
        } else {
            Serial.println("[WiFi] Auto-connect failed, manual connection required");
        }
    }
}

wifi_state_t wifiManagerGetState(void) {
    // Update state based on WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        currentState = WIFI_STATE_CONNECTED;
    } else if (currentState == WIFI_STATE_CONNECTING) {
        // Keep connecting state
    } else if (currentState != WIFI_STATE_SCANNING) {
        currentState = WIFI_STATE_DISCONNECTED;
    }
    return currentState;
}

bool wifiManagerIsConnected(void) {
    return WiFi.status() == WL_CONNECTED;
}

const char* wifiManagerGetSSID(void) {
    if (wifiManagerIsConnected()) {
        strncpy(connectedSSID, WiFi.SSID().c_str(), WIFI_SSID_MAX_LENGTH - 1);
        return connectedSSID;
    }
    return "";
}

const char* wifiManagerGetIP(void) {
    if (wifiManagerIsConnected()) {
        snprintf(ipAddress, sizeof(ipAddress), "%s", WiFi.localIP().toString().c_str());
        return ipAddress;
    }
    return "0.0.0.0";
}

void wifiManagerStartScan(void) {
    currentState = WIFI_STATE_SCANNING;
    WiFi.scanDelete();
    WiFi.scanNetworks(true);  // Async scan
    Serial.println("[WiFi] Scan started");
}

int wifiManagerGetScanResults(wifi_network_t* results, int maxResults) {
    int16_t scanStatus = WiFi.scanComplete();

    if (scanStatus == WIFI_SCAN_RUNNING) {
        return -1;  // Still scanning
    }

    if (scanStatus == WIFI_SCAN_FAILED) {
        currentState = WIFI_STATE_DISCONNECTED;
        return 0;
    }

    scanResultCount = min((int)scanStatus, min(maxResults, 20));

    for (int i = 0; i < scanResultCount; i++) {
        strncpy(results[i].ssid, WiFi.SSID(i).c_str(), WIFI_SSID_MAX_LENGTH - 1);
        results[i].ssid[WIFI_SSID_MAX_LENGTH - 1] = '\0';
        results[i].rssi = WiFi.RSSI(i);
        results[i].encrypted = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        results[i].saved = isNetworkSaved(results[i].ssid);
    }

    currentState = WIFI_STATE_DISCONNECTED;
    Serial.printf("[WiFi] Scan complete, found %d networks\n", scanResultCount);

    return scanResultCount;
}

bool wifiManagerConnect(const char* ssid, const char* password) {
    if (ssid == NULL || strlen(ssid) == 0) {
        return false;
    }

    Serial.printf("[WiFi] Connecting to: %s\n", ssid);
    currentState = WIFI_STATE_CONNECTING;

    WiFi.disconnect();
    delay(100);

    if (password && strlen(password) > 0) {
        WiFi.begin(ssid, password);
    } else {
        WiFi.begin(ssid);
    }

    // Wait for connection (with timeout)
    int timeout = 20;  // 10 seconds
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(500);
        Serial.print(".");
        timeout--;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        currentState = WIFI_STATE_CONNECTED;
        strncpy(connectedSSID, ssid, WIFI_SSID_MAX_LENGTH - 1);
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    currentState = WIFI_STATE_ERROR;
    Serial.println("[WiFi] Connection failed");
    return false;
}

void wifiManagerDisconnect(void) {
    WiFi.disconnect();
    currentState = WIFI_STATE_DISCONNECTED;
    connectedSSID[0] = '\0';
    Serial.println("[WiFi] Disconnected");
}

bool wifiManagerAutoConnect(void) {
    if (savedNetworkCount == 0) {
        Serial.println("[WiFi] No saved networks for auto-connect");
        return false;
    }

    Serial.println("[WiFi] Trying auto-connect to saved networks...");

    // Synchronous scan for available networks
    int scanCount = WiFi.scanNetworks(false, false, false, 300);  // Sync scan, 300ms per channel

    if (scanCount <= 0) {
        Serial.println("[WiFi] No networks found during scan");
        return false;
    }

    Serial.printf("[WiFi] Found %d networks, checking against %d saved\n", scanCount, savedNetworkCount);

    // Try each saved network
    for (int i = 0; i < savedNetworkCount; i++) {
        // Check if this saved network is available
        for (int j = 0; j < scanCount; j++) {
            if (strcmp(savedSSIDs[i], WiFi.SSID(j).c_str()) == 0) {
                Serial.printf("[WiFi] Found saved network: %s\n", savedSSIDs[i]);
                if (wifiManagerConnect(savedSSIDs[i], savedPasswords[i])) {
                    WiFi.scanDelete();  // Clean up scan results
                    return true;
                }
            }
        }
    }

    WiFi.scanDelete();  // Clean up scan results
    return false;
}

bool wifiManagerSaveNetwork(const char* ssid, const char* password) {
    if (ssid == NULL || strlen(ssid) == 0) {
        return false;
    }

    // Check if already saved, update password
    for (int i = 0; i < savedNetworkCount; i++) {
        if (strcmp(savedSSIDs[i], ssid) == 0) {
            strncpy(savedPasswords[i], password ? password : "", WIFI_PASS_MAX_LENGTH - 1);
            saveSavedNetworks();
            Serial.printf("[WiFi] Updated saved network: %s\n", ssid);
            return true;
        }
    }

    // Add new if space available
    if (savedNetworkCount < WIFI_MAX_SAVED_NETWORKS) {
        strncpy(savedSSIDs[savedNetworkCount], ssid, WIFI_SSID_MAX_LENGTH - 1);
        strncpy(savedPasswords[savedNetworkCount], password ? password : "", WIFI_PASS_MAX_LENGTH - 1);
        savedNetworkCount++;
        saveSavedNetworks();
        Serial.printf("[WiFi] Saved network: %s (total: %d)\n", ssid, savedNetworkCount);
        return true;
    }

    Serial.println("[WiFi] Cannot save, max networks reached");
    return false;
}

bool wifiManagerDeleteNetwork(const char* ssid) {
    for (int i = 0; i < savedNetworkCount; i++) {
        if (strcmp(savedSSIDs[i], ssid) == 0) {
            // Shift remaining networks
            for (int j = i; j < savedNetworkCount - 1; j++) {
                strcpy(savedSSIDs[j], savedSSIDs[j + 1]);
                strcpy(savedPasswords[j], savedPasswords[j + 1]);
            }
            savedNetworkCount--;
            saveSavedNetworks();
            Serial.printf("[WiFi] Deleted network: %s\n", ssid);
            return true;
        }
    }
    return false;
}

int wifiManagerGetSavedCount(void) {
    return savedNetworkCount;
}

const char* wifiManagerGetSavedSSID(int index) {
    if (index >= 0 && index < savedNetworkCount) {
        return savedSSIDs[index];
    }
    return "";
}

void wifiManagerToggle(void) {
    if (wifiManagerIsConnected()) {
        wifiManagerDisconnect();
    } else {
        // Try auto-connect first, if no saved networks or fails, show scan screen
        if (savedNetworkCount > 0 && wifiManagerAutoConnect()) {
            // Connected via auto-connect
        } else {
            // Show WiFi scan screen
            loadScreen(SCREEN_ID_WIFI);
            wifiManagerStartScan();
        }
    }
    wifiManagerUpdateUI();
}

void wifiManagerUpdateUI(void) {
    if (objects.lbl_wifi_status == NULL) return;

    if (wifiManagerIsConnected()) {
        char buf[64];
        // Show IP address instead of SSID
        snprintf(buf, sizeof(buf), LV_SYMBOL_WIFI "\n%s", wifiManagerGetIP());
        lv_label_set_text(objects.lbl_wifi_status, buf);
        lv_obj_set_style_bg_color(objects.btn_settings_wifi, lv_color_hex(0x2E7D32), LV_PART_MAIN);  // Green

        // Update main screen WiFi icon
        if (objects.v_wifi) {
            lv_obj_set_style_text_color(objects.v_wifi, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
        }
    } else {
        lv_label_set_text(objects.lbl_wifi_status, LV_SYMBOL_WIFI "\nWiFi: AUS");
        lv_obj_set_style_bg_color(objects.btn_settings_wifi, lv_color_hex(0x1565C0), LV_PART_MAIN);  // Blue

        // Update main screen WiFi icon
        if (objects.v_wifi) {
            lv_obj_set_style_text_color(objects.v_wifi, lv_color_hex(0x757575), LV_PART_MAIN);  // Gray
        }
    }
}

// Internal functions
static void loadSavedNetworks(void) {
    wifiPrefs.begin(WIFI_NVS_NAMESPACE, true);  // Read-only
    savedNetworkCount = wifiPrefs.getInt("count", 0);

    if (savedNetworkCount > WIFI_MAX_SAVED_NETWORKS) {
        savedNetworkCount = WIFI_MAX_SAVED_NETWORKS;
    }

    for (int i = 0; i < savedNetworkCount; i++) {
        char keySSID[16], keyPass[16];
        snprintf(keySSID, sizeof(keySSID), "ssid%d", i);
        snprintf(keyPass, sizeof(keyPass), "pass%d", i);

        String ssid = wifiPrefs.getString(keySSID, "");
        String pass = wifiPrefs.getString(keyPass, "");

        strncpy(savedSSIDs[i], ssid.c_str(), WIFI_SSID_MAX_LENGTH - 1);
        strncpy(savedPasswords[i], pass.c_str(), WIFI_PASS_MAX_LENGTH - 1);
    }

    wifiPrefs.end();
    Serial.printf("[WiFi] Loaded %d saved networks\n", savedNetworkCount);
}

static void saveSavedNetworks(void) {
    wifiPrefs.begin(WIFI_NVS_NAMESPACE, false);  // Read-write
    wifiPrefs.putInt("count", savedNetworkCount);

    for (int i = 0; i < savedNetworkCount; i++) {
        char keySSID[16], keyPass[16];
        snprintf(keySSID, sizeof(keySSID), "ssid%d", i);
        snprintf(keyPass, sizeof(keyPass), "pass%d", i);

        wifiPrefs.putString(keySSID, savedSSIDs[i]);
        wifiPrefs.putString(keyPass, savedPasswords[i]);
    }

    // Clear unused slots
    for (int i = savedNetworkCount; i < WIFI_MAX_SAVED_NETWORKS; i++) {
        char keySSID[16], keyPass[16];
        snprintf(keySSID, sizeof(keySSID), "ssid%d", i);
        snprintf(keyPass, sizeof(keyPass), "pass%d", i);

        wifiPrefs.remove(keySSID);
        wifiPrefs.remove(keyPass);
    }

    wifiPrefs.end();
}

static bool isNetworkSaved(const char* ssid) {
    for (int i = 0; i < savedNetworkCount; i++) {
        if (strcmp(savedSSIDs[i], ssid) == 0) {
            return true;
        }
    }
    return false;
}
