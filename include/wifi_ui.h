#ifndef WIFI_UI_H
#define WIFI_UI_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize WiFi UI callbacks
void wifiUIInit(void);

// Update WiFi list with scan results
void wifiUIUpdateList(void);

// Called periodically to check scan status and update UI
void wifiUITick(void);

// Show WiFi screen and start scan
void wifiUIShow(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_UI_H
