#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *debug;
    lv_obj_t *settings;
    lv_obj_t *names;                 // Names input screen
    lv_obj_t *wifi;                  // WiFi scan/connect screen
    // Settings screen buttons
    lv_obj_t *btn_settings_gear;     // Gear icon - placeholder
    lv_obj_t *btn_settings_battery;  // Battery charging toggle
    lv_obj_t *btn_settings_wifi;     // WiFi/OTA toggle
    lv_obj_t *btn_settings_names;    // Names/phonebook button
    lv_obj_t *lbl_settings_title;
    lv_obj_t *lbl_battery_status;    // Battery charging status label
    lv_obj_t *lbl_wifi_status;       // WiFi status label
    // Names screen objects
    lv_obj_t *names_keyboard;        // Keyboard widget
    lv_obj_t *names_textarea;        // Text input area
    lv_obj_t *names_list;            // List of saved names
    lv_obj_t *names_title;           // Title label
    // WiFi screen objects
    lv_obj_t *wifi_title;            // Title label
    lv_obj_t *wifi_status;           // Status label
    lv_obj_t *wifi_list;             // List of scanned networks
    lv_obj_t *wifi_keyboard;         // Keyboard for password
    lv_obj_t *wifi_password_ta;      // Password textarea
    lv_obj_t *wifi_password_label;   // Password prompt label
    lv_obj_t *wifi_connect_btn;      // Connect button
    lv_obj_t *wifi_back_btn;         // Back button
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *v_batt;           // Battery container
    lv_obj_t *v_batt_body;      // Battery body (outline)
    lv_obj_t *v_batt_tip;       // Battery tip (right side)
    lv_obj_t *v_batt_fill;      // Battery fill indicator
    lv_obj_t *v_batt_label;     // Battery percentage label
    lv_obj_t *v_wifi;           // WiFi status symbol
    lv_obj_t *v_bluetooth;      // Bluetooth status symbol
    lv_obj_t *v_usb;            // USB status symbol
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *v_debug;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *print_overlay;       // Printing status overlay
    lv_obj_t *print_overlay_label; // Printing status label
    lv_obj_t *v_o2_percent;        // O2 percentage symbol (smaller font)
    lv_obj_t *v_he_percent;        // He percentage symbol (smaller font)
    lv_obj_t *settings_version;    // Settings screen version label
    lv_obj_t *main_version;        // Main screen version label
    lv_obj_t *main_name;           // Main screen name label (under date)
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_DEBUG = 2,
    SCREEN_ID_SETTINGS = 3,
    SCREEN_ID_NAMES = 4,
    SCREEN_ID_WIFI = 5,
};

void create_screen_main();
void tick_screen_main();

void create_screen_debug();
void tick_screen_debug();

void create_screen_settings();
void tick_screen_settings();

void create_screen_names();
void tick_screen_names();

void create_screen_wifi();
void tick_screen_wifi();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/