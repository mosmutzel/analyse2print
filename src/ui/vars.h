#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_NONE
};

// Native global variables

// UI update functions
void ui_update_vars(void);
void ui_set_printer_status(const char* status);
void ui_set_analyzer_status(const char* status);
void ui_set_date(const char* date);
void ui_set_o2(const char* o2_str, float o2_value);
void ui_set_he(const char* he_str, float he_value);
void ui_set_debug(const char* msg);

// Battery functions
void ui_set_battery(int voltage_mv, bool charging);
void ui_update_battery_blink(void);

// Status symbol functions
void ui_set_wifi_status(bool connected);
void ui_set_bluetooth_status(bool connected, bool scanning);
void ui_set_usb_status(bool connected);
void ui_update_bluetooth_blink(void);  // Call from loop for scanning blink

// Print overlay functions
void ui_show_print_overlay(void);       // Show "Printing..." overlay
void ui_hide_print_overlay(void);       // Hide print overlay

// Settings screen functions
void ui_set_settings_version(const char* version);  // Set version on settings screen

// Main screen name display
void ui_set_name(const char* name);     // Set diver name on main screen

// Global name variable - accessible for printing
extern char v_name[32];

// WiFi button IP display
void ui_set_wifi_button_text(const char* text);  // Set WiFi button text (IP or status)

// END display (below He)
void ui_set_end(const char* end_str, float end_value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/