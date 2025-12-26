#ifndef BLE_PRINTER_H
#define BLE_PRINTER_H

#include <NimBLEDevice.h>

#ifdef __cplusplus
extern "C" {
#endif

// BLE connection states
typedef enum {
    BLE_STATE_IDLE,
    BLE_STATE_SCANNING,
    BLE_STATE_FOUND,
    BLE_STATE_CONNECTING,
    BLE_STATE_CONNECTED,
    BLE_STATE_FAILED,
    BLE_STATE_DISCONNECTED
} ble_state_t;

// Initialize BLE printer module (creates task on Core 1)
void blePrinterInit(void);

// Start scanning for printer (non-blocking)
void blePrinterStartScan(void);

// Get current BLE state
ble_state_t blePrinterGetState(void);

// Check if printer is connected
bool blePrinterIsConnected(void);

// Get status message for display
const char* blePrinterGetStatusMessage(void);

// Called from loop to update UI based on BLE state
void blePrinterUpdateUI(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_PRINTER_H
