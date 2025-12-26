/**
 * analyzer.cpp - Divesoft Analyzer USB Host Interface
 *
 * This module handles USB Host communication with the Divesoft Analyzer
 * using an FTDI FT232R chip via ESP32-S3 USB OTG.
 *
 * Divesoft Analyzer output format:
 * "He   0.5 %  O2  21.2 %  Ti  24.5 ~C  1004.4 hPa   2025/11/25 18:45:43"
 */

#include "analyzer.h"
#include "display.h"
#include "utilities.h"
#include "usb/usb_host.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_intr_alloc.h"
#include <Wire.h>
#include <XPowersLib.h>

// PMU for USB OTG power control
static PowersSY6970 PMU;

// ============================================================================
// USB Host Configuration
// ============================================================================

#define USB_HOST_TASK_PRIORITY    2
#define CLIENT_NUM_EVENT_MSG      5

// FTDI VID/PID and endpoints
#define FTDI_VID    0x0403
#define FTDI_PID    0x6001  // FT232R
#define FTDI_IF_NUM 0
#define FTDI_EP_IN  0x81
#define FTDI_EP_OUT 0x02

// FTDI control requests
#define FTDI_SIO_RESET          0x00
#define FTDI_SIO_SET_BAUDRATE   0x03
#define FTDI_SIO_SET_DATA       0x04
#define FTDI_SIO_SET_FLOW_CTRL  0x02
#define FTDI_SIO_SET_DTR_RTS    0x01

// ============================================================================
// Global Variables
// ============================================================================

unsigned long analyzerLastDataTime = 0;
bool analyzerConnected = false;

static AnalyzerData currentData = {false, 0.0, 0.0, 0.0, 1013.0, ""};
static SemaphoreHandle_t dataMutex = nullptr;

// USB Host handles
static usb_host_client_handle_t client_hdl = NULL;
static usb_device_handle_t dev_hdl = NULL;
static usb_transfer_t *in_xfer = NULL;
static usb_transfer_t *ctrl_xfer = NULL;
static SemaphoreHandle_t ctrl_sem = NULL;

// Receive buffer
static String inputBuffer = "";

// ============================================================================
// Forward Declarations
// ============================================================================

static void ftdi_init_device();
static void submit_in_transfer();
static void process_line(String &line);

// ============================================================================
// Data Parsing
// ============================================================================

// Parse value after prefix (e.g., "He" -> 0.5)
static float parseValue(String &data, const char* prefix) {
    int idx = data.indexOf(prefix);
    if (idx < 0) return -1;

    idx += strlen(prefix);
    while (idx < (int)data.length() && (data[idx] == ' ' || data[idx] == '\t')) {
        idx++;
    }

    String numStr = "";
    while (idx < (int)data.length()) {
        char c = data[idx];
        if ((c >= '0' && c <= '9') || c == '.' || c == '-') {
            numStr += c;
            idx++;
        } else {
            break;
        }
    }

    if (numStr.length() == 0) return -1;
    return numStr.toFloat();
}

// Process line: "He   0.5 %  O2  21.2 %  Ti  24.5 ~C  1004.4 hPa   2025/11/25 18:45:43"
static void process_line(String &line) {
    if (line.startsWith("He") && line.indexOf("O2") > 0) {
        float he = parseValue(line, "He");
        float o2 = parseValue(line, "O2");

        if (o2 > 0) {
            if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                currentData.helium = he;
                currentData.oxygen = o2;

                // Parse temperature
                float ti = parseValue(line, "Ti");
                if (ti > -1) {
                    currentData.temperature = ti;
                }

                // Parse pressure
                int hpaIdx = line.indexOf("hPa");
                if (hpaIdx > 0) {
                    String pressStr = line.substring(hpaIdx - 8, hpaIdx);
                    pressStr.trim();
                    currentData.pressure = pressStr.toFloat();
                }

                // Parse timestamp
                int dateIdx = line.indexOf("20");
                if (dateIdx > 0) {
                    currentData.timestamp = line.substring(dateIdx);
                    currentData.timestamp.trim();
                }

                currentData.valid = true;
                analyzerLastDataTime = millis();

                xSemaphoreGive(dataMutex);

                // Debug output
                displayDebug("O2=" + String(o2, 1) + "% He=" + String(he, 1) + "%");
            }
        }
    }
}

// ============================================================================
// USB Transfer Callbacks
// ============================================================================

// IN transfer callback
static void in_xfer_callback(usb_transfer_t *transfer) {
    if (transfer->status != USB_TRANSFER_STATUS_COMPLETED) {
        if (analyzerConnected) submit_in_transfer();
        return;
    }

    // FTDI: first 2 bytes are Modem/Line Status, data follows after
    if (transfer->actual_num_bytes > 2) {
        uint8_t *data = transfer->data_buffer + 2;
        size_t len = transfer->actual_num_bytes - 2;

        analyzerLastDataTime = millis();

        // Process characters
        for (size_t i = 0; i < len; i++) {
            char c = (char)data[i];

            if (c == '\n') {
                inputBuffer.trim();
                if (inputBuffer.length() > 0) {
                    process_line(inputBuffer);
                }
                inputBuffer = "";
            } else if (c != '\r' && c >= 32 && inputBuffer.length() < 128) {
                inputBuffer += c;
            }
        }
    }

    if (analyzerConnected) {
        submit_in_transfer();
    }
}

// Control transfer callback
static void ctrl_xfer_callback(usb_transfer_t *transfer) {
    xSemaphoreGive(ctrl_sem);
}

// ============================================================================
// USB Transfer Functions
// ============================================================================

// Submit IN transfer
static void submit_in_transfer() {
    if (in_xfer && analyzerConnected) {
        in_xfer->num_bytes = 64;
        esp_err_t err = usb_host_transfer_submit(in_xfer);
        if (err != ESP_OK) {
            displayDebug("IN err: " + String(esp_err_to_name(err)));
        }
    }
}

// Send control transfer to FTDI
static esp_err_t ftdi_control_transfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex) {
    if (!ctrl_xfer || !analyzerConnected) return ESP_FAIL;

    usb_setup_packet_t *setup = (usb_setup_packet_t *)ctrl_xfer->data_buffer;
    setup->bmRequestType = USB_BM_REQUEST_TYPE_DIR_OUT | USB_BM_REQUEST_TYPE_TYPE_VENDOR | USB_BM_REQUEST_TYPE_RECIP_DEVICE;
    setup->bRequest = bRequest;
    setup->wValue = wValue;
    setup->wIndex = wIndex;
    setup->wLength = 0;

    ctrl_xfer->num_bytes = sizeof(usb_setup_packet_t);

    esp_err_t err = usb_host_transfer_submit_control(client_hdl, ctrl_xfer);
    if (err == ESP_OK) {
        xSemaphoreTake(ctrl_sem, pdMS_TO_TICKS(1000));
    }
    return err;
}

// Initialize FTDI device
static void ftdi_init_device() {
    displayDebug("FTDI init...");

    // Set baudrate 115200: divisor 0x001A (3MHz / 26 = 115384 Hz)
    ftdi_control_transfer(FTDI_SIO_SET_BAUDRATE, 0x001A, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Activate DTR and RTS - THIS IS CRITICAL!
    ftdi_control_transfer(FTDI_SIO_SET_DTR_RTS, 0x0303, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    displayDebug("FTDI ready (115200 8N1)");
}

// ============================================================================
// USB Client Event Callback
// ============================================================================

static void client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg) {
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV: {
            displayDebug("USB device found");

            uint8_t addr = event_msg->new_dev.address;
            esp_err_t err = usb_host_device_open(client_hdl, addr, &dev_hdl);
            if (err != ESP_OK) {
                displayDebug("Open err: " + String(esp_err_to_name(err)));
                return;
            }

            // Check if FTDI
            const usb_device_desc_t *desc;
            usb_host_get_device_descriptor(dev_hdl, &desc);

            displayDebug("VID:" + String(desc->idVendor, HEX) + " PID:" + String(desc->idProduct, HEX));

            if (desc->idVendor == FTDI_VID && desc->idProduct == FTDI_PID) {
                displayDebug("FTDI FT232R detected!");

                // Claim interface
                err = usb_host_interface_claim(client_hdl, dev_hdl, FTDI_IF_NUM, 0);
                if (err != ESP_OK) {
                    displayDebug("Claim err: " + String(esp_err_to_name(err)));
                    usb_host_device_close(client_hdl, dev_hdl);
                    dev_hdl = NULL;
                    return;
                }

                // Allocate transfers
                usb_host_transfer_alloc(64, 0, &in_xfer);
                usb_host_transfer_alloc(64, 0, &ctrl_xfer);

                in_xfer->device_handle = dev_hdl;
                in_xfer->bEndpointAddress = FTDI_EP_IN;
                in_xfer->callback = in_xfer_callback;
                in_xfer->context = NULL;

                ctrl_xfer->device_handle = dev_hdl;
                ctrl_xfer->bEndpointAddress = 0;
                ctrl_xfer->callback = ctrl_xfer_callback;
                ctrl_xfer->context = NULL;

                analyzerConnected = true;

                // Initialize FTDI (baudrate + DTR/RTS)
                ftdi_init_device();

                // Start receiving
                submit_in_transfer();

                displayDebug("Analyzer connected!");
                displayDebug("Waiting for data...");
            } else {
                displayDebug("Not FTDI, closing");
                usb_host_device_close(client_hdl, dev_hdl);
                dev_hdl = NULL;
            }
            break;
        }

        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            displayDebug("USB disconnected");
            analyzerConnected = false;
            currentData.valid = false;

            if (in_xfer) {
                usb_host_transfer_free(in_xfer);
                in_xfer = NULL;
            }
            if (ctrl_xfer) {
                usb_host_transfer_free(ctrl_xfer);
                ctrl_xfer = NULL;
            }
            if (dev_hdl) {
                usb_host_interface_release(client_hdl, dev_hdl, FTDI_IF_NUM);
                usb_host_device_close(client_hdl, dev_hdl);
                dev_hdl = NULL;
            }
            break;

        default:
            break;
    }
}

// ============================================================================
// USB Host Library Task
// ============================================================================

static void usb_lib_task(void *arg) {
    usb_host_config_t config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&config));

    while (true) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
    }
}

// ============================================================================
// Public Functions
// ============================================================================

void analyzerInit() {
    displayDebug("Init USB Host...");

    // Initialize I2C for PMU
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);

    // Initialize PMU and enable OTG power for USB Host
    if (PMU.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, SY6970_SLAVE_ADDRESS)) {
        displayDebug("PMU initialized");
        PMU.enableOTG(); //braucht der Analyzer nicht. Funktionier ohne
        displayDebug("OTG power enabled");
    } else {
        displayDebug("PMU init failed!");
    }

    inputBuffer.reserve(128);
    dataMutex = xSemaphoreCreateMutex();
    ctrl_sem = xSemaphoreCreateBinary();

    // Start USB Host Task
    xTaskCreatePinnedToCore(usb_lib_task, "usb_lib", 4096, NULL, USB_HOST_TASK_PRIORITY, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Register client
    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = CLIENT_NUM_EVENT_MSG,
        .async = {
            .client_event_callback = client_event_callback,
            .callback_arg = NULL,
        }
    };

    esp_err_t err = usb_host_client_register(&client_config, &client_hdl);
    if (err != ESP_OK) {
        displayDebug("Client err: " + String(esp_err_to_name(err)));
        return;
    }

    displayDebug("USB Host ready");
    displayDebug("Waiting for Analyzer...");
}

void analyzerLoop() {
    // Process USB client events - THIS IS CRITICAL!
    if (client_hdl) {
        usb_host_client_handle_events(client_hdl, pdMS_TO_TICKS(10));
    }
}

bool isAnalyzerConnected() {
    return analyzerConnected;
}

AnalyzerData getAnalyzerData() {
    AnalyzerData data;

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        data = currentData;
        xSemaphoreGive(dataMutex);
    } else {
        data.valid = false;
    }

    return data;
}
