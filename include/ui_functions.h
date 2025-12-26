/**
 * ui_functions.h - UI Variable Update Functions
 *
 * Function prototypes for updating LVGL UI variables.
 * Implementation is in src/ui/vars.c
 */

#ifndef UI_FUNCTIONS_H
#define UI_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

// Update UI variables
void ui_update_vars(void);

// Set status values
void ui_set_printer_status(const char* status);
void ui_set_analyzer_status(const char* status);

// Set sensor values
void ui_set_date(const char* date);
void ui_set_o2(const char* o2_str, float o2_value);
void ui_set_he(const char* he_str, float he_value);
void ui_set_end(const char* end_str, float end_value);

// Set debug message
void ui_set_debug(const char* msg);

// Get sensor values
float ui_get_o2_value(void);
float ui_get_he_value(void);

#ifdef __cplusplus
}
#endif

#endif // UI_FUNCTIONS_H
