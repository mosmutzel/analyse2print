/**
 * vars.c - EEZ Studio UI Variables
 *
 * This module provides the variable update functions for the UI.
 * Variables:
 *   v_printer  - Printer connection status
 *   v_analyzer - Analyzer connection status
 *   v_date     - Date from analyzer
 *   v_o2       - O2 percentage
 *   v_he       - He percentage
 *   v_mod      - Maximum Operating Depth (calculated)
 */

#include "vars.h"
#include "screens.h"
#include <lvgl.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <Arduino.h>

// UI variable values
static char v_printer_str[32] = "Disconnected";
static char v_analyzer_str[32] = "Disconnected";
static char v_batt_str[16] = "0";
static char v_date_str[32] = "--";
static char v_o2_str[16] = "00.0";
static char v_he_str[16] = "00.0";
static char v_mod_str[16] = "0";
static char v_end_str[16] = "0";
static char v_debug_str[512] = "Debug ready...";

// Global name variable - accessible from main.cpp
char v_name[32] = "---";
#define DEBUG_LOG_MAX_LINES 12
#define DEBUG_LOG_LINE_LEN 40

// Numeric values for calculations
static float v_o2_value = 21.0f;
static float v_he_value = 0.0f;
static float ppO2max = 1.4f;             // Maximum ppO2 in bar
static float ppO2maxDeco = 1.6f;  
/**
 * Calculate MOD (Maximum Operating Depth) for a given O2 percentage
 * MOD = ((ppO2max / FO2) - 1) * 10
 * Using ppO2max = 1.2 bar for technical diving
 */
static float calculate_mod(float o2_percent) {
    if (o2_percent <= 0.0f || o2_percent > 100.0f) {
        return 0.0f;
    }
    if (o2_percent > 0.0f && o2_percent < 40.0f){
    float fo2 = o2_percent / 100.0f;  // Fraction of O2

    float mod = ((ppO2max / fo2) - 1.0f) * 10.0f;  // Depth in meters
    return mod > 0 ? mod : 0;
    }
    else if (o2_percent >= 40.0f){
        float fo2 = o2_percent / 100.0f;  // Fraction of O2

    float mod = ((ppO2maxDeco / fo2) - 1.0f) * 10.0f;  // Depth in meters
    return mod > 0 ? mod : 0;
    }    
    else {
        float mod = 0;
        return mod;
    }
}

/**
 * Calculate END (Equivalent Narcotic Depth) for a given depth and He percentage
 * END = (Depth + 10) * (1 - fHe) - 10
 * This calculates the equivalent air depth for narcosis purposes
 * Helium is non-narcotic, so higher He% means lower END
 */
static float calculate_end(float depth, float o2_percent, float he_percent) {
    if (depth <= 0.0f) {
        return 0;
    }
    float f02 = o2_percent / 100.0f;
    float fhe = he_percent / 100.0f;
    float fn2 = 1.0f - (f02 + fhe);
    //float fHe = he_percent / 100.0f;  // Fraction of Helium
    //float end = (depth + 10.0f) * (1.0f - fHe) - 10.0f;
    float end = (depth + 10.0f) * (fn2 / 0.79f) - 10.0f;
    return end > 0 ? end : 0;
}


/**
 * Update all UI labels with current variable values
 */
void ui_update_vars(void) {
    // Update printer status (obj2)
    if (objects.obj2) {
        lv_label_set_text(objects.obj2, v_printer_str);
    }

    // Update analyzer status (obj4)
    if (objects.obj4) {
        lv_label_set_text(objects.obj4, v_analyzer_str);
    }

    // Update date (obj12)
    if (objects.obj12) {
        lv_label_set_text(objects.obj12, v_date_str);
    }

    // Update O2 value (obj7) - value only, % is separate label
    if (objects.obj7) {
        lv_label_set_text(objects.obj7, v_o2_str);
    }

    // Update He value (obj8) - value only, % is separate label
    if (objects.obj8) {
        lv_label_set_text(objects.obj8, v_he_str);
    }

    // Update MOD value (obj11)
    if (objects.obj11) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_mod_str);
        lv_label_set_text(objects.obj11, buf);
    }

    // Update END value (obj26)
    if (objects.obj26) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_end_str);
        lv_label_set_text(objects.obj26, buf);
    }
}

/**
 * Set printer status
 */
void ui_set_printer_status(const char* status) {
    strncpy(v_printer_str, status, sizeof(v_printer_str) - 1);
    v_printer_str[sizeof(v_printer_str) - 1] = '\0';

    // Update color based on status
    if (objects.obj2) {
        if (strstr(status, "Ready") || strstr(status, "Connected")) {
            lv_obj_set_style_text_color(objects.obj2, lv_palette_main(LV_PALETTE_LIGHT_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
        } else if (strstr(status, "Disconnected") || strstr(status, "Error")) {
            lv_obj_set_style_text_color(objects.obj2, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT); //red
        } else {
            lv_obj_set_style_text_color(objects.obj2, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT); //blue lv_palette_main(LV_PALETTE_LIGHT_BLUE)
        }
        lv_label_set_text(objects.obj2, v_printer_str);
    }
    // Also update debug screen label (obj15)
    if (objects.obj15) {
        lv_label_set_text(objects.obj15, v_printer_str);
    }
}

/**
 * Set analyzer status
 */
void ui_set_analyzer_status(const char* status) {
    strncpy(v_analyzer_str, status, sizeof(v_analyzer_str) - 1);
    v_analyzer_str[sizeof(v_analyzer_str) - 1] = '\0';

    // Update color based on status
    if (objects.obj4) {
        if (strstr(status, "Connected") || strstr(status, "Ready")) {
            lv_obj_set_style_text_color(objects.obj4, lv_palette_main(LV_PALETTE_LIGHT_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT); //green
        } else if (strstr(status, "Disconnected") || strstr(status, "Error")) {
            lv_obj_set_style_text_color(objects.obj4, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT); // red
        } else {
            lv_obj_set_style_text_color(objects.obj4, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT); //blue
        }
        lv_label_set_text(objects.obj4, v_analyzer_str);
    }
    // Also update debug screen label (obj17)
    if (objects.obj17) {
        lv_label_set_text(objects.obj17, v_analyzer_str);
    }
}

// Battery state
static int v_batt_percent = 0;
static bool v_batt_charging = false;
static bool v_batt_blink_state = false;
static unsigned long v_batt_last_blink = 0;

// Battery widget dimensions (must match screens.c)
#define BATT_FILL_MAX_WIDTH 40   // Max fill width at 100%
#define BATT_LOW_THRESHOLD 20    // Low battery threshold

/**
 * Set battery voltage and charging status
 * Uses battery indicator widget style from giobauermeister/battery-indicator-lvgl-editor
 * Simple linear mapping: 3200mV = 0%, 4200mV = 100%
 */
void ui_set_battery(int voltage_mv, bool charging) {
    // Simple linear mapping: 3200mV = 0%, 4200mV = 100%
    int percent = (voltage_mv - 3200) * 100 / 1000;
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    v_batt_percent = percent;
    v_batt_charging = charging;

    // Determine color based on state
    // Display uses BGR color order, so we swap R and B components
    // Original RGB -> BGR: 0xRRGGBB -> 0xBBGGRR
    lv_color_t fill_color;
    lv_color_t text_color;

    if (charging) {
        fill_color = lv_palette_main(LV_PALETTE_LIGHT_GREEN);  // Green when charging
        text_color = lv_color_black();
    } else if (percent <= BATT_LOW_THRESHOLD) {
        fill_color = lv_palette_main(LV_PALETTE_RED);  // Red for critical
        text_color = lv_color_white();
    } else if (percent <= 40) {
        fill_color = lv_palette_main(LV_PALETTE_YELLOW);  // Yellow for low
        text_color = lv_color_black();
    } else {
        fill_color = lv_color_white();  // White for normal
        text_color = lv_color_black();
    }

    // Update battery fill bar width
    if (objects.v_batt_fill && lv_obj_is_valid(objects.v_batt_fill)) {
        int fill_width = (BATT_FILL_MAX_WIDTH * percent) / 100;
        if (fill_width < 2) fill_width = 2;  // Minimum visible width
        lv_obj_set_width(objects.v_batt_fill, fill_width);
        lv_obj_set_style_bg_color(objects.v_batt_fill, fill_color, LV_PART_MAIN);
    }

    // Update battery tip color (colored when 100%, light gray otherwise)
    if (objects.v_batt_tip && lv_obj_is_valid(objects.v_batt_tip)) {
        if (percent >= 100) {
            lv_obj_set_style_bg_color(objects.v_batt_tip, fill_color, LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(objects.v_batt_tip, lv_color_hex(0x757575), LV_PART_MAIN);
        }
    }

    // Update percentage label
    if (objects.v_batt_label && lv_obj_is_valid(objects.v_batt_label)) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", percent);
        lv_label_set_text(objects.v_batt_label, buf);
        lv_obj_set_style_text_color(objects.v_batt_label, text_color, LV_PART_MAIN);
    }
}

/**
 * Update battery blink animation (call from loop)
 * Blinks the fill bar when charging
 */
void ui_update_battery_blink(void) {
    if (!v_batt_charging) {
        // Not charging - ensure fill is visible
        if (objects.v_batt_fill && lv_obj_is_valid(objects.v_batt_fill)) {
            lv_obj_clear_flag(objects.v_batt_fill, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    // Charging - blink the fill bar
    unsigned long now = millis();
    if (now - v_batt_last_blink >= 500) {  // 500ms blink interval
        v_batt_last_blink = now;
        v_batt_blink_state = !v_batt_blink_state;

        if (objects.v_batt_fill && lv_obj_is_valid(objects.v_batt_fill)) {
            if (v_batt_blink_state) {
                lv_obj_clear_flag(objects.v_batt_fill, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(objects.v_batt_fill, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

// ============================================================================
// Status Symbol Functions
// ============================================================================

// Bluetooth blink state
static bool v_bt_scanning = false;
static bool v_bt_blink_state = false;
static unsigned long v_bt_last_blink = 0;

/**
 * Set WiFi status symbol color
 * @param connected true = white (active), false = gray (inactive)
 */
void ui_set_wifi_status(bool connected) {
    if (objects.v_wifi && lv_obj_is_valid(objects.v_wifi)) {
        lv_color_t color = connected ? lv_color_white()
                                     : lv_color_hex(0x757575);
        lv_obj_set_style_text_color(objects.v_wifi, color, LV_PART_MAIN);
    }
}

/**
 * Set Bluetooth status symbol color
 * @param connected true = white (connected to printer)
 * @param scanning true = blink gray (searching for printer)
 */
void ui_set_bluetooth_status(bool connected, bool scanning) {
    v_bt_scanning = scanning && !connected;  // Only scan-blink if not connected

    if (objects.v_bluetooth && lv_obj_is_valid(objects.v_bluetooth)) {
        if (connected) {
            lv_obj_set_style_text_color(objects.v_bluetooth,
                lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
        } else if (!scanning) {
            lv_obj_set_style_text_color(objects.v_bluetooth,
                lv_color_hex(0x757575), LV_PART_MAIN);
        }
        // If scanning, color is handled by ui_update_bluetooth_blink()
    }
}

/**
 * Set USB status symbol color
 * @param connected true = white (analyzer connected), false = gray (no analyzer)
 */
void ui_set_usb_status(bool connected) {
    if (objects.v_usb && lv_obj_is_valid(objects.v_usb)) {
        lv_color_t color = connected ? lv_color_white()
                                     : lv_palette_main(LV_PALETTE_BLUE);
        lv_obj_set_style_text_color(objects.v_usb, color, LV_PART_MAIN);
    }
}

/**
 * Update Bluetooth blink animation (call from loop)
 * Blinks the symbol when scanning for printer
 */
void ui_update_bluetooth_blink(void) {
    if (!v_bt_scanning) return;

    unsigned long now = millis();
    if (now - v_bt_last_blink >= 300) {  // 300ms blink interval (faster than battery)
        v_bt_last_blink = now;
        v_bt_blink_state = !v_bt_blink_state;

        if (objects.v_bluetooth && lv_obj_is_valid(objects.v_bluetooth)) {
            lv_color_t color = v_bt_blink_state ? lv_color_white()
                                                : lv_color_hex(0x757575);
            lv_obj_set_style_text_color(objects.v_bluetooth, color, LV_PART_MAIN);
        }
    }
}


/**
 * Set date
 */
void ui_set_date(const char* date) {
    strncpy(v_date_str, date, sizeof(v_date_str) - 1);
    v_date_str[sizeof(v_date_str) - 1] = '\0';

    if (objects.obj12) {
        lv_label_set_text(objects.obj12, v_date_str);
    }
    // Also update debug screen label (obj19)
    if (objects.obj19) {
        lv_label_set_text(objects.obj19, v_date_str);
    }
}

/**
 * Set O2 value and recalculate MOD and END
 */
void ui_set_o2(const char* o2_str, float o2_value) {
    strncpy(v_o2_str, o2_str, sizeof(v_o2_str) - 1);
    v_o2_str[sizeof(v_o2_str) - 1] = '\0';
    v_o2_value = o2_value;

    // Update O2 display (value only, % is separate)
    if (objects.obj7) {
        lv_label_set_text(objects.obj7, v_o2_str);

        // Color based on O2 level
        lv_color_t o2_color;
        if (o2_value > 40.0f) {
            o2_color = lv_color_hex(0x00FF98);  // Orange for high O2
        } else if (o2_value > 21.0f) {
            o2_color = lv_color_hex(0xFBE040);  // Cyan for nitrox
        } else if (o2_value <= 21.0f && o2_value > 20.7) {
            o2_color = lv_color_white();  // white for Air
        } else {
            o2_color = lv_palette_main(LV_PALETTE_RED);  // red for hypoxic
        }
        lv_obj_set_style_text_color(objects.obj7, o2_color, LV_PART_MAIN | LV_STATE_DEFAULT);

        // Also update the % symbol color
        if (objects.v_o2_percent) {
            lv_obj_set_style_text_color(objects.v_o2_percent, o2_color, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    // Recalculate and update MOD
    float mod = calculate_mod(o2_value);
    snprintf(v_mod_str, sizeof(v_mod_str), "%.0f", mod);

    if (objects.obj11) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_mod_str);
        lv_label_set_text(objects.obj11, buf);

        // Color based on MOD
        if (mod < 30.0f) {
            lv_obj_set_style_text_color(objects.obj11, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);  // Orange for shallow MOD
        } else {
            lv_obj_set_style_text_color(objects.obj11, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);  // white for good MOD
        }
    }

    // Recalculate and update END based on MOD and current He value
    float end = calculate_end(mod, o2_value, v_he_value);
    snprintf(v_end_str, sizeof(v_end_str), "%.0f", end);

    if (objects.obj26) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_end_str);
        lv_label_set_text(objects.obj26, buf);

        // Color based on END - warn if END > 30m (narcosis risk)
        if (end > 40.0f) {
            lv_obj_set_style_text_color(objects.obj26, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);  // Red for high narcosis
        } else if (end > 30.0f) {
            lv_obj_set_style_text_color(objects.obj26, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);  // Yellow for moderate narcosis
        } else {
            lv_obj_set_style_text_color(objects.obj26, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);  // White for safe END
        }
    }

    // Also update debug screen label (obj21)
    if (objects.obj21) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%s%%", v_o2_str);
        lv_label_set_text(objects.obj21, buf);
    }
}

/**
 * Set He value and recalculate END
 */
void ui_set_he(const char* he_str, float he_value) {
    strncpy(v_he_str, he_str, sizeof(v_he_str) - 1);
    v_he_str[sizeof(v_he_str) - 1] = '\0';
    v_he_value = he_value;

    // Update He display (value only, % is separate)
    if (objects.obj8) {
        lv_label_set_text(objects.obj8, v_he_str);

        // Color based on He level
        lv_color_t he_color;
        if (he_value > 0.8f) {
            he_color = lv_color_hex(0x3BFFEB);  // Yellow for trimix
        } else {
            he_color = lv_color_white();  // White for no helium
        }
        lv_obj_set_style_text_color(objects.obj8, he_color, LV_PART_MAIN | LV_STATE_DEFAULT);

        // Also update the % symbol color
        if (objects.v_he_percent) {
            lv_obj_set_style_text_color(objects.v_he_percent, he_color, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    // Recalculate and update END based on current MOD and new He value
    float mod = calculate_mod(v_o2_value);
    float end = calculate_end(mod, v_o2_value, he_value);
    snprintf(v_end_str, sizeof(v_end_str), "%.0f", end);

    if (objects.obj26) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_end_str);
        lv_label_set_text(objects.obj26, buf);

        // Color based on END - warn if END > 30m (narcosis risk)
        if (end > 40.0f) {
            lv_obj_set_style_text_color(objects.obj26, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);  // Red for high narcosis
        } else if (end > 30.0f) {
            lv_obj_set_style_text_color(objects.obj26, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);  // Yellow for moderate narcosis
        } else {
            lv_obj_set_style_text_color(objects.obj26, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);  // White for safe END
        }
    }

    // Also update debug screen label (obj23)
    if (objects.obj23) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%s%%", v_he_str);
        lv_label_set_text(objects.obj23, buf);
    }
}
/**
 * Set END value
 */
void ui_set_end(const char* end_str, float end_value) {
    strncpy(v_end_str, end_str, sizeof(v_end_str));
    //v_end_str[sizeof(v_end_str) - 1] = '\0';
    //v_he_value = end_value;

    /*
     // Recalculate and update MOD
    float mod = calculate_mod(o2_value);
    snprintf(v_mod_str, sizeof(v_mod_str), "%.0f", mod);

    if (objects.obj11) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_mod_str);
        lv_label_set_text(objects.obj11, buf);

        // Color based on MOD
        if (mod < 30.0f) {
    */

    if (objects.obj26) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%sm", v_end_str);
        lv_label_set_text(objects.obj26, buf);

        // Color based on END level
        if (end_value > 5) {
            lv_obj_set_style_text_color(objects.obj26, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);  // Yellow for trimix
        } else {
            lv_obj_set_style_text_color(objects.obj26, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);  // White for no helium
        }
    }
}

/**
 * Get current O2 value
 */
float ui_get_o2_value(void) {
    return v_o2_value;
}

/**
 * Get current He value
 */
float ui_get_he_value(void) {
    return v_he_value;
}

/**
 * Set debug message - appends to debug log on debug screen
 * Shows last DEBUG_LOG_MAX_LINES lines
 */
void ui_set_debug(const char* msg) {
    // Find first newline in current buffer to remove oldest line
    char* firstNewline = strchr(v_debug_str, '\n');

    // Count current lines
    int lineCount = 0;
    for (char* p = v_debug_str; *p; p++) {
        if (*p == '\n') lineCount++;
    }

    // If we have max lines, remove the first line
    if (lineCount >= DEBUG_LOG_MAX_LINES && firstNewline) {
        memmove(v_debug_str, firstNewline + 1, strlen(firstNewline + 1) + 1);
    }

    // Append new message with newline
    size_t currentLen = strlen(v_debug_str);
    size_t msgLen = strlen(msg);

    // Truncate message if too long
    if (msgLen > DEBUG_LOG_LINE_LEN) {
        msgLen = DEBUG_LOG_LINE_LEN;
    }

    // Check if we have space
    if (currentLen + msgLen + 2 < sizeof(v_debug_str)) {
        if (currentLen > 0 && v_debug_str[currentLen - 1] != '\n') {
            strcat(v_debug_str, "\n");
        }
        strncat(v_debug_str, msg, msgLen);
    }

    // Only update if v_debug object exists and is a valid LVGL object
    if (objects.v_debug && lv_obj_is_valid(objects.v_debug)) {
        lv_label_set_text(objects.v_debug, v_debug_str);
    }
}

/**
 * Show print overlay with printer symbol
 * Overlays the Printer/Analyzer/Datum area
 */
void ui_show_print_overlay(void) {
    if (objects.print_overlay && lv_obj_is_valid(objects.print_overlay)) {
        lv_obj_clear_flag(objects.print_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * Hide print overlay
 */
void ui_hide_print_overlay(void) {
    if (objects.print_overlay && lv_obj_is_valid(objects.print_overlay)) {
        lv_obj_add_flag(objects.print_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * Set version on main screen and settings screen
 */
void ui_set_settings_version(const char* version) {
    char buf[32];
    snprintf(buf, sizeof(buf), "v%s - mrDiver.de", version);

    // Update main screen version label
    if (objects.main_version) {
        lv_label_set_text(objects.main_version, buf);
    }

    // Update settings screen version label
    if (objects.settings_version) {
        lv_label_set_text(objects.settings_version, buf);
    }
}

/**
 * Set diver name on main screen (below date)
 * Also updates the global v_name variable for use in printing
 */
void ui_set_name(const char* name) {
    // Update global v_name variable
    strncpy(v_name, name, sizeof(v_name) - 1);
    v_name[sizeof(v_name) - 1] = '\0';

  //  if (objects.main_name && lv_obj_is_valid(objects.main_name)) {
  //      char buf[48];
  //      snprintf(buf, sizeof(buf), "Name: %s", name);
  //      lv_label_set_text(objects.main_name, buf);
  //  }

    lv_label_set_text(objects.obj12, v_name);
}

/**
 * Set WiFi button text (IP or status)
 */
void ui_set_wifi_button_text(const char* text) {
    extern objects_t objects;
    if (objects.lbl_wifi_status && lv_obj_is_valid(objects.lbl_wifi_status)) {
        lv_label_set_text(objects.lbl_wifi_status, text);
    }
}
