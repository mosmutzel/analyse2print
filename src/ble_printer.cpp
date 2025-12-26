/**
 * ble_printer.cpp - Non-blocking BLE Printer Connection Module
 *
 * Handles BLE scanning and connection on Core 1 to prevent blocking the UI.
 * Uses FreeRTOS task and queue for asynchronous operation.
 */

#include "ble_printer.h"
#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "display.h"
#include "print.h"

// ============================================================================
// Configuration
// ============================================================================

#define PRINTER_NAME "B1-H119122559"
#define PRINTER_MAC "19:01:12:f0:2f:4c"

// BLE UUIDs
#define SERVICE_UUID "e7810a71-73ae-499d-8c15-faa9aef0c3f2"
#define CHAR_UUID "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f"

// Alternative UUIDs
#define SERVICE_UUID_ALT "49535343-FE7D-4AE5-8FA9-9FAFD205E455"
#define CHAR_TX_UUID "49535343-1E4D-4BD9-BA61-23C647249616"
#define CHAR_RX_UUID "49535343-8841-43F4-A8D4-ECBE34729BB3"

#define BLE_TASK_STACK_SIZE 8192
#define BLE_SCAN_TIMEOUT_MS 30000
#define BLE_CONNECT_TIMEOUT_S 10

// ============================================================================
// Command types for BLE task
// ============================================================================

typedef enum {
    BLE_CMD_SCAN,
    BLE_CMD_CONNECT,
    BLE_CMD_DISCONNECT,
    BLE_CMD_INIT_PRINTER
} ble_command_t;

typedef struct {
    ble_command_t command;
} BleCommand;

// ============================================================================
// Internal State
// ============================================================================

static TaskHandle_t bleTaskHandle = nullptr;
static QueueHandle_t bleCommandQueue = nullptr;
static SemaphoreHandle_t bleStateMutex = nullptr;

static volatile ble_state_t currentState = BLE_STATE_IDLE;
static char statusMessage[64] = "Idle";

// BLE objects
static NimBLEClient* pClient = nullptr;
static NimBLEScan* pBLEScan = nullptr;
static NimBLEAdvertisedDevice* targetDevice = nullptr;
static bool deviceFound = false;
static bool connected = false;

// External reference to characteristic from print.cpp
extern NimBLERemoteCharacteristic* pCharacteristic;
extern uint8_t responseBuffer[256];
extern int responseLength;
extern volatile bool responseReceived;

// ============================================================================
// BLE Callbacks
// ============================================================================

class BleScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        String name = advertisedDevice->getName().c_str();
        String address = advertisedDevice->getAddress().toString().c_str();

        Serial.print("[BLE] Found: ");
        Serial.print(name);
        Serial.print(" (");
        Serial.print(address);
        Serial.println(")");

        if (name == PRINTER_NAME || address == PRINTER_MAC) {
            Serial.println("[BLE] >>> Target printer found!");

            if (targetDevice) {
                delete targetDevice;
            }
            targetDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
            deviceFound = true;

            // Update state
            xSemaphoreTake(bleStateMutex, portMAX_DELAY);
            currentState = BLE_STATE_FOUND;
            strncpy(statusMessage, "Printer found!", sizeof(statusMessage) - 1);
            xSemaphoreGive(bleStateMutex);

            NimBLEDevice::getScan()->stop();
        }
    }
};

static void bleNotifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic,
                               uint8_t* pData, size_t length, bool isNotify) {
    static int notifyCount = 0;
    notifyCount++;

    if (notifyCount <= 3) {
        char dbg[40];
        snprintf(dbg, sizeof(dbg), "Printer resp #%d: %02X %02X", notifyCount,
                 length > 0 ? pData[0] : 0, length > 2 ? pData[2] : 0);
        Serial.println(dbg);
    }

    if (length < sizeof(responseBuffer)) {
        memcpy(responseBuffer, pData, length);
        responseLength = length;
    }
    responseReceived = true;
}

class BleClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pclient) override {
        Serial.println("[BLE] *** Connected! ***");
        connected = true;

        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_CONNECTED;
        strncpy(statusMessage, "Connected!", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
    }

    void onDisconnect(NimBLEClient* pclient, int reason) override {
        Serial.printf("[BLE] *** Disconnected! (reason: %d) ***\n", reason);
        connected = false;

        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_DISCONNECTED;
        strncpy(statusMessage, "Disconnected", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
    }
};

// ============================================================================
// Internal Functions
// ============================================================================

static bool doScan() {
    Serial.println("[BLE] Starting scan (30s max)...");

    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
    currentState = BLE_STATE_SCANNING;
    strncpy(statusMessage, "Scanning...", sizeof(statusMessage) - 1);
    xSemaphoreGive(bleStateMutex);

    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setScanCallbacks(new BleScanCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    deviceFound = false;
    pBLEScan->start(0, false);  // Continuous scan

    unsigned long scanStart = millis();
    while (!deviceFound && (millis() - scanStart) < BLE_SCAN_TIMEOUT_MS) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    pBLEScan->stop();
    pBLEScan->clearResults();

    if (!deviceFound) {
        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_FAILED;
        strncpy(statusMessage, "Printer not found!", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
        return false;
    }

    return true;
}

static bool doConnect() {
    if (!targetDevice) {
        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_FAILED;
        strncpy(statusMessage, "No device to connect!", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
        return false;
    }

    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
    currentState = BLE_STATE_CONNECTING;
    strncpy(statusMessage, "Connecting...", sizeof(statusMessage) - 1);
    xSemaphoreGive(bleStateMutex);

    Serial.print("[BLE] Connecting to ");
    Serial.println(targetDevice->getAddress().toString().c_str());

    if (pClient) {
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
    }

    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(new BleClientCallbacks());
    pClient->setConnectionParams(12, 12, 0, 51);
    pClient->setConnectTimeout(BLE_CONNECT_TIMEOUT_S);

    bool connectResult = pClient->connect(targetDevice, false);

    if (!connectResult) {
        NimBLEDevice::deleteClient(pClient);
        vTaskDelay(pdMS_TO_TICKS(500));

        Serial.println("[BLE] Trying with public address type...");
        pClient = NimBLEDevice::createClient();
        pClient->setClientCallbacks(new BleClientCallbacks());
        NimBLEAddress addrPublic(std::string(PRINTER_MAC), 0);
        connectResult = pClient->connect(addrPublic);
    }

    // Wait for connection callback
    Serial.println("[BLE] Waiting for connection (10s)...");
    for (int i = 0; i < 100; i++) {
        if (connected || pClient->isConnected()) {
            Serial.println("[BLE] *** CONNECTED! ***");
            connected = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        if (i % 10 == 0) Serial.print(".");
    }
    Serial.println();

    if (!pClient->isConnected()) {
        Serial.println("[BLE] Connection failed!");
        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_FAILED;
        strncpy(statusMessage, "Connection failed!", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
        return false;
    }

    // Get services
    Serial.println("[BLE] Connected! Getting services...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    NimBLERemoteService* pService = pClient->getService(SERVICE_UUID);
    if (!pService) {
        pService = pClient->getService(SERVICE_UUID_ALT);
    }

    if (!pService) {
        Serial.println("[BLE] No service found!");
        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_FAILED;
        strncpy(statusMessage, "Service not found!", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
        return false;
    }

    pCharacteristic = pService->getCharacteristic(CHAR_UUID);
    if (!pCharacteristic) {
        pCharacteristic = pService->getCharacteristic(CHAR_TX_UUID);
    }

    if (!pCharacteristic) {
        Serial.println("[BLE] Characteristic not found!");
        xSemaphoreTake(bleStateMutex, portMAX_DELAY);
        currentState = BLE_STATE_FAILED;
        strncpy(statusMessage, "Characteristic error!", sizeof(statusMessage) - 1);
        xSemaphoreGive(bleStateMutex);
        return false;
    }

    if (pCharacteristic->canNotify()) {
        pCharacteristic->subscribe(true, bleNotifyCallback);
        Serial.println("[BLE] Subscribed to notifications");
    }

    vTaskDelay(pdMS_TO_TICKS(500));

    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
    currentState = BLE_STATE_CONNECTED;
    strncpy(statusMessage, "Ready", sizeof(statusMessage) - 1);
    xSemaphoreGive(bleStateMutex);

    return true;
}

static void doInitPrinter() {
    Serial.println("[BLE] Initializing printer...");

    if (!connectToPrinter()) {
        Serial.println("[BLE] Connect command failed");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    sendHeartbeat();
    initPrintTask();

    Serial.println("[BLE] Printer ready! (Print task on Core 1)");

    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
    strncpy(statusMessage, "Printer ready", sizeof(statusMessage) - 1);
    xSemaphoreGive(bleStateMutex);
}

// ============================================================================
// BLE Task (runs on Core 1)
// ============================================================================

static bool bleInitialized = false;

static void initBLE() {
    if (bleInitialized) return;

    Serial.println("[BLE] Initializing NimBLE on Core 1...");

    // Initialize NimBLE (this can take some time)
    NimBLEDevice::init("ESP32_Niimbot");
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        NimBLEDevice::setPower(9);
    #else
        NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    #endif
    NimBLEDevice::setMTU(517);

    bleInitialized = true;
    Serial.println("[BLE] NimBLE initialized");
}

static void bleTask(void* parameter) {
    Serial.println("[BLE] Task started on Core 1");
    BleCommand cmd;

    while (true) {
        if (xQueueReceive(bleCommandQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
            switch (cmd.command) {
                case BLE_CMD_SCAN:
                    initBLE();  // Initialize BLE on first scan request
                    if (doScan()) {
                        // Auto-connect after successful scan
                        if (doConnect()) {
                            doInitPrinter();
                        }
                    }
                    break;

                case BLE_CMD_CONNECT:
                    initBLE();  // Ensure BLE is initialized
                    if (doConnect()) {
                        doInitPrinter();
                    }
                    break;

                case BLE_CMD_DISCONNECT:
                    if (pClient && pClient->isConnected()) {
                        pClient->disconnect();
                    }
                    connected = false;
                    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
                    currentState = BLE_STATE_DISCONNECTED;
                    strncpy(statusMessage, "Disconnected", sizeof(statusMessage) - 1);
                    xSemaphoreGive(bleStateMutex);
                    break;

                case BLE_CMD_INIT_PRINTER:
                    if (connected) {
                        doInitPrinter();
                    }
                    break;
            }
        }

        // Auto-reconnect if disconnected
        if (!connected && targetDevice && currentState == BLE_STATE_DISCONNECTED) {
            vTaskDelay(pdMS_TO_TICKS(5000));  // Wait 5 seconds before retry
            if (!connected) {
                Serial.println("[BLE] Auto-reconnecting...");
                xSemaphoreTake(bleStateMutex, portMAX_DELAY);
                strncpy(statusMessage, "Reconnecting...", sizeof(statusMessage) - 1);
                xSemaphoreGive(bleStateMutex);

                if (doConnect()) {
                    doInitPrinter();
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ============================================================================
// Public API
// ============================================================================

void blePrinterInit(void) {
    Serial.println("[BLE] Initializing BLE Printer module...");

    // Create mutex for state access
    bleStateMutex = xSemaphoreCreateMutex();
    if (!bleStateMutex) {
        Serial.println("[BLE] ERROR: Failed to create mutex!");
        return;
    }

    // Create command queue
    bleCommandQueue = xQueueCreate(4, sizeof(BleCommand));
    if (!bleCommandQueue) {
        Serial.println("[BLE] ERROR: Failed to create queue!");
        return;
    }

    // NimBLE initialization is deferred to the BLE task on Core 1
    // This prevents blocking the main thread during startup

    // Create BLE task on Core 1
    xTaskCreatePinnedToCore(
        bleTask,
        "BLETask",
        BLE_TASK_STACK_SIZE,
        NULL,
        1,  // Priority
        &bleTaskHandle,
        1   // Core 1
    );

    Serial.println("[BLE] BLE Printer module initialized");
}

void blePrinterStartScan(void) {
    if (!bleCommandQueue) return;

    BleCommand cmd;
    cmd.command = BLE_CMD_SCAN;
    xQueueSend(bleCommandQueue, &cmd, 0);

    Serial.println("[BLE] Scan command queued");
}

ble_state_t blePrinterGetState(void) {
    ble_state_t state;
    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
    state = currentState;
    xSemaphoreGive(bleStateMutex);
    return state;
}

bool blePrinterIsConnected(void) {
    return connected;
}

const char* blePrinterGetStatusMessage(void) {
    static char msg[64];
    xSemaphoreTake(bleStateMutex, portMAX_DELAY);
    strncpy(msg, statusMessage, sizeof(msg) - 1);
    xSemaphoreGive(bleStateMutex);
    return msg;
}

void blePrinterUpdateUI(void) {
    ble_state_t state = blePrinterGetState();

    switch (state) {
        case BLE_STATE_IDLE:
            displaySetPrinterStatus("Idle");
            displaySetBluetoothStatus(false, false);
            break;
        case BLE_STATE_SCANNING:
            displaySetPrinterStatus("Scanning...");
            displaySetBluetoothStatus(false, true);  // Blink
            break;
        case BLE_STATE_FOUND:
            displaySetPrinterStatus("Found!");
            displaySetBluetoothStatus(false, true);
            break;
        case BLE_STATE_CONNECTING:
            displaySetPrinterStatus("Connecting...");
            displaySetBluetoothStatus(false, true);
            break;
        case BLE_STATE_CONNECTED:
            displaySetPrinterStatus("Ready");
            displaySetBluetoothStatus(true, false);
            break;
        case BLE_STATE_FAILED:
            displaySetPrinterStatus(blePrinterGetStatusMessage());
            displaySetBluetoothStatus(false, false);
            break;
        case BLE_STATE_DISCONNECTED:
            displaySetPrinterStatus("Disconnected");
            displaySetBluetoothStatus(false, false);
            break;
    }
}
