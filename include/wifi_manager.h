#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_MAX_SAVED_NETWORKS 5
#define WIFI_SSID_MAX_LENGTH 33
#define WIFI_PASS_MAX_LENGTH 65

// WiFi connection states
typedef enum {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_SCANNING,
    WIFI_STATE_ERROR
} wifi_state_t;

// Scanned network info
typedef struct {
    char ssid[WIFI_SSID_MAX_LENGTH];
    int32_t rssi;
    bool encrypted;
    bool saved;  // true if this network is in saved list
} wifi_network_t;

// Initialize WiFi manager
void wifiManagerInit(void);

// Get current state
wifi_state_t wifiManagerGetState(void);

// Check if connected
bool wifiManagerIsConnected(void);

// Get current SSID (if connected)
const char* wifiManagerGetSSID(void);

// Get IP address string (if connected)
const char* wifiManagerGetIP(void);

// Start WiFi scan
void wifiManagerStartScan(void);

// Get scan results (returns number of networks found)
int wifiManagerGetScanResults(wifi_network_t* results, int maxResults);

// Connect to a network
bool wifiManagerConnect(const char* ssid, const char* password);

// Disconnect from current network
void wifiManagerDisconnect(void);

// Try to auto-connect to saved networks
bool wifiManagerAutoConnect(void);

// Save network credentials
bool wifiManagerSaveNetwork(const char* ssid, const char* password);

// Delete saved network
bool wifiManagerDeleteNetwork(const char* ssid);

// Get number of saved networks
int wifiManagerGetSavedCount(void);

// Get saved network SSID by index
const char* wifiManagerGetSavedSSID(int index);

// Toggle WiFi (connect/disconnect)
void wifiManagerToggle(void);

// Update WiFi status label on settings screen
void wifiManagerUpdateUI(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
