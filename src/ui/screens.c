#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 222, 480);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);  // Disable scrolling on main screen
    {
        lv_obj_t *parent_obj = obj;
        {
            // Title - auto-centered based on text width
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 0, 60);
            lv_obj_set_size(obj, 222, LV_SIZE_CONTENT);  // Full width for centering
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_TEAL), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Analyse 2 Print");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 10, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Printer: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 90, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_printer");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 10, 139);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Analyzer: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 90, 139);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_analyzer");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 10, 250);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "o2: ");
        }
        {
            // O2 value (without %)
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 30, 220);
            lv_obj_set_size(obj, 142, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_o2");
        }
        {
            // O2 percent symbol (25% smaller = 36px)
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.v_o2_percent = obj;
            lv_obj_set_pos(obj, 172, 232);  // Right of value, slightly lower to align baseline
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 10, 310);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "HE: ");
        }
        {
            // He value (without %)
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 30, 280);
            lv_obj_set_size(obj, 142, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_HE");
        }
        {
            // He percent symbol (25% smaller = 36px)
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.v_he_percent = obj;
            lv_obj_set_pos(obj, 172, 292);  // Right of value, slightly lower to align baseline
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 10, 355);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "MOD: ");
        }

        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 35, 340);
            lv_obj_set_size(obj, 162, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_mod");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj25 = obj;
            lv_obj_set_pos(obj, 10, 405);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "END: ");
        }

        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj26 = obj;
            lv_obj_set_pos(obj, 35, 390);
            lv_obj_set_size(obj, 162, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_end");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 10, 169);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Name: ");
        }        
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 90, 169);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_name");
        }
        /*{
            // Name label (below date)
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_name = obj;
            lv_obj_set_pos(obj, 10, 193);
            lv_obj_set_size(obj, 202, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Name: ---");
        }*/
        // Version info at bottom
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_version = obj;  // Store reference for dynamic update
            lv_obj_set_pos(obj, 0, 450);
            lv_obj_set_size(obj, 222, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x757575), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "mrDiver.de");  // Will be updated with actual version
        }
        {
            // Battery indicator widget (similar to giobauermeister/battery-indicator-lvgl-editor)
            // Container for the whole battery widget
            lv_obj_t *batt_container = lv_obj_create(parent_obj);
            objects.v_batt = batt_container;
            lv_obj_set_pos(batt_container, 155, 8);
            lv_obj_set_size(batt_container, 60, 22);
            lv_obj_set_style_bg_opa(batt_container, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_width(batt_container, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(batt_container, 0, LV_PART_MAIN);
            lv_obj_clear_flag(batt_container, LV_OBJ_FLAG_SCROLLABLE);

            // Battery body (outline rectangle)
            // Use lv_obj basic object and style it completely to avoid theme colors
            lv_obj_t *batt_body = lv_obj_create(batt_container);
            objects.v_batt_body = batt_body;
            lv_obj_set_pos(batt_body, 0, 0);
            lv_obj_set_size(batt_body, 48, 20);
            lv_obj_set_style_bg_color(batt_body, lv_color_black(), LV_PART_MAIN);  // Black background
            lv_obj_set_style_bg_opa(batt_body, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_color(batt_body, lv_color_hex(0x757575), LV_PART_MAIN);  // Light gray border0xAAAAAA   lv_color_hex(0x757575)
            lv_obj_set_style_border_width(batt_body, 2, LV_PART_MAIN);
            lv_obj_set_style_border_opa(batt_body, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_radius(batt_body, 4, LV_PART_MAIN);
            lv_obj_set_style_pad_all(batt_body, 0, LV_PART_MAIN);
            lv_obj_set_style_outline_width(batt_body, 0, LV_PART_MAIN);  // No outline
            lv_obj_clear_flag(batt_body, LV_OBJ_FLAG_SCROLLABLE);

            // Battery tip (small rectangle on right side)
            lv_obj_t *batt_tip = lv_obj_create(batt_container);
            objects.v_batt_tip = batt_tip;
            lv_obj_set_pos(batt_tip, 48, 5);
            lv_obj_set_size(batt_tip, 5, 10);
            lv_obj_set_style_bg_color(batt_tip,  lv_color_hex(0x757575), LV_PART_MAIN);  // Same gray as border 0xAAAAAA lv_color_hex(0x757575)
            lv_obj_set_style_bg_opa(batt_tip, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(batt_tip, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(batt_tip, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_all(batt_tip, 0, LV_PART_MAIN);
            lv_obj_set_style_outline_width(batt_tip, 0, LV_PART_MAIN);
            lv_obj_clear_flag(batt_tip, LV_OBJ_FLAG_SCROLLABLE);

            // Battery fill indicator (inside the body)
            lv_obj_t *batt_fill = lv_obj_create(batt_body);
            objects.v_batt_fill = batt_fill;
            lv_obj_set_pos(batt_fill, 2, 2);
            lv_obj_set_size(batt_fill, 40, 12);  // Max width when 100%
            lv_obj_set_style_bg_color(batt_fill, lv_color_black(), LV_PART_MAIN);  // Will be updated by ui_set_battery
            lv_obj_set_style_bg_opa(batt_fill, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(batt_fill, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(batt_fill, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_all(batt_fill, 0, LV_PART_MAIN);
            lv_obj_clear_flag(batt_fill, LV_OBJ_FLAG_SCROLLABLE);

            // Battery percentage label (inside body, centered)
            lv_obj_t *batt_label = lv_label_create(batt_body);
            objects.v_batt_label = batt_label;
            lv_obj_center(batt_label);
            lv_obj_set_style_text_color(batt_label, lv_color_black(), LV_PART_MAIN); //505050
            lv_obj_set_style_text_font(batt_label, &lv_font_montserrat_12, LV_PART_MAIN);
            lv_label_set_text(batt_label, "100%");
        }
        {
            // Status symbols row (WiFi, Bluetooth, USB) - positioned top left
            // WiFi symbol (leftmost)
            lv_obj_t *wifi_label = lv_label_create(parent_obj);
            objects.v_wifi = wifi_label;
            lv_obj_set_pos(wifi_label, 20, 10);
            lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_16, LV_PART_MAIN);
            lv_obj_set_style_text_color(wifi_label, lv_color_hex(0x757575), LV_PART_MAIN);  // Gray = disconnected lv_color_hex(0x757575)
            lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);

            // Bluetooth symbol (middle)
            lv_obj_t *bt_label = lv_label_create(parent_obj);
            objects.v_bluetooth = bt_label;
            lv_obj_set_pos(bt_label, 50, 10);
            lv_obj_set_style_text_font(bt_label, &lv_font_montserrat_16, LV_PART_MAIN);
            lv_obj_set_style_text_color(bt_label, lv_color_hex(0x757575), LV_PART_MAIN);  // Gray = disconnected
            lv_label_set_text(bt_label, LV_SYMBOL_BLUETOOTH);

            // USB symbol (rightmost of the three)
            lv_obj_t *usb_label = lv_label_create(parent_obj);
            objects.v_usb = usb_label;
            lv_obj_set_pos(usb_label, 75, 10);
            lv_obj_set_style_text_font(usb_label, &lv_font_montserrat_16, LV_PART_MAIN);
            lv_obj_set_style_text_color(usb_label, lv_color_hex(0x757575), LV_PART_MAIN);  // Gray = disconnected
            lv_label_set_text(usb_label, LV_SYMBOL_USB);
        }
        {
            // Print overlay - covers Printer/Analyzer/Datum area (y: 109-185)
            lv_obj_t *overlay = lv_obj_create(parent_obj);
            objects.print_overlay = overlay;
            lv_obj_set_pos(overlay, 0, 100);
            lv_obj_set_size(overlay, 222, 95);
            lv_obj_set_style_bg_color(overlay, lv_color_hex(0x1565C0), LV_PART_MAIN);  // Blue background
            lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(overlay, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(overlay, 0, LV_PART_MAIN);
            lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);  // Start hidden

            // Print icon and text label
            lv_obj_t *print_label = lv_label_create(overlay);
            objects.print_overlay_label = print_label;
            lv_obj_center(print_label);
            lv_obj_set_style_text_color(print_label, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_text_font(print_label, &lv_font_montserrat_24, LV_PART_MAIN);
            lv_obj_set_style_text_align(print_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_label_set_text(print_label, LV_SYMBOL_DOWNLOAD "\nDrucke...");
        }
    }

    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_debug() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.debug = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 222, 480);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 34, 62);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Printer: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, 114, 62);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_printer");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, 34, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Analyzer: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 114, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_analyzer");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 34, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Datum: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj19 = obj;
            lv_obj_set_pos(obj, 114, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_date");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, 35, 151);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "02: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, 115, 151);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_o2");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 35, 178);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "He: ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj23 = obj;
            lv_obj_set_pos(obj, 115, 178);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v_He");
        }
        {
            // v_debug - simple label instead of msgbox for easier text updates
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.v_debug = obj;
            lv_obj_set_pos(obj, 10, 206);
            lv_obj_set_size(obj, 202, 260);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj24 = obj;
            lv_obj_set_pos(obj, 75, 14);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "debug");
        }
    }
    
    tick_screen_debug();
}

void tick_screen_debug() {
}

void create_screen_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 222, 480);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;

        // Title
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_settings_title = obj;
            lv_obj_set_pos(obj, 0, 30);
            lv_obj_set_size(obj, 222, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_TEAL), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, LV_SYMBOL_SETTINGS " Einstellungen");
        }

        // Battery Charging Button
        {
            lv_obj_t *btn = lv_btn_create(parent_obj);
            objects.btn_settings_battery = btn;
            lv_obj_set_pos(btn, 20, 100);
            lv_obj_set_size(btn, 182, 60);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E7D32), LV_PART_MAIN | LV_STATE_DEFAULT);  // Green
            lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_obj_center(lbl);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_BATTERY_FULL "\nLaden: AUS");
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            objects.lbl_battery_status = lbl;
        }

        // WiFi/OTA Button
        {
            lv_obj_t *btn = lv_btn_create(parent_obj);
            objects.btn_settings_wifi = btn;
            lv_obj_set_pos(btn, 20, 180);
            lv_obj_set_size(btn, 182, 60);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x1565C0), LV_PART_MAIN | LV_STATE_DEFAULT);  // Blue
            lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_obj_center(lbl);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_WIFI "\nWiFi: AUS");
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            objects.lbl_wifi_status = lbl;
        }

        // Names/Phonebook Button
        {
            lv_obj_t *btn = lv_btn_create(parent_obj);
            objects.btn_settings_names = btn;
            lv_obj_set_pos(btn, 20, 260);
            lv_obj_set_size(btn, 182, 60);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x6A1B9A), LV_PART_MAIN | LV_STATE_DEFAULT);  // Purple
            lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_obj_center(lbl);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_LIST "\nNamen");
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        }

        // Gear/Info Button (placeholder for future settings)
        {
            lv_obj_t *btn = lv_btn_create(parent_obj);
            objects.btn_settings_gear = btn;
            lv_obj_set_pos(btn, 20, 340);
            lv_obj_set_size(btn, 182, 60);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x757575), LV_PART_MAIN | LV_STATE_DEFAULT);  // Gray
            lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_obj_center(lbl);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_SETTINGS "\nInfo");
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        }

        // Version info at bottom
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.settings_version = obj;  // Store reference for dynamic update
            lv_obj_set_pos(obj, 0, 450);
            lv_obj_set_size(obj, 222, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x757575), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "v1.0 - mrDiver.de");  // Will be updated with actual version
        }
    }

    tick_screen_settings();
}

void tick_screen_settings() {
}

// Keyboard ready callback - called when Enter/OK is pressed
static void names_keyboard_ready_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        // User pressed OK/Enter on keyboard
        // The names.cpp will handle adding the name
    }
}

void create_screen_names() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.names = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 222, 480);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);  // Disable scrolling on main screen
    {
        lv_obj_t *parent_obj = obj;

        // Title
        {
            lv_obj_t *lbl = lv_label_create(parent_obj);
            objects.names_title = lbl;
            lv_obj_set_pos(lbl, 0, 5);
            lv_obj_set_size(lbl, 222, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_palette_main(LV_PALETTE_PURPLE), LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_LIST " Namen");
        }

        // Text input area
        {
            lv_obj_t *ta = lv_textarea_create(parent_obj);
            objects.names_textarea = ta;
            lv_obj_set_pos(ta, 5, 30);
            lv_obj_set_size(ta, 212, 35);
            lv_textarea_set_one_line(ta, true);
            lv_textarea_set_placeholder_text(ta, "Name eingeben...");
            lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_bg_color(ta, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
            lv_obj_set_style_text_color(ta, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_border_color(ta, lv_color_hex(0x6A1B9A), LV_PART_MAIN);
        }

        // List of saved names
        {
            lv_obj_t *list = lv_list_create(parent_obj);
            objects.names_list = list;
            lv_obj_set_pos(list, 5, 70);
            lv_obj_set_size(list, 212, 120);
            lv_obj_set_style_bg_color(list, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
            lv_obj_set_style_border_color(list, lv_color_hex(0x333333), LV_PART_MAIN);
            lv_obj_set_style_border_width(list, 1, LV_PART_MAIN);
            lv_obj_set_style_radius(list, 5, LV_PART_MAIN);
            lv_obj_set_style_pad_all(list, 5, LV_PART_MAIN);
        }

        // Keyboard - create AFTER textarea so it can be linked
        {
            lv_obj_t *kb = lv_keyboard_create(parent_obj);
            objects.names_keyboard = kb;
            lv_obj_set_size(kb, 222, 280);  // Larger keyboard for better touch
            lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);  // Align to bottom
            lv_keyboard_set_textarea(kb, objects.names_textarea);  // Connect to textarea
            lv_obj_add_event_cb(kb, names_keyboard_ready_cb, LV_EVENT_READY, NULL);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Make sure it's visible
        }
    }

    tick_screen_names();
}

void tick_screen_names() {
}

// WiFi screen - for scanning and connecting to networks
void create_screen_wifi() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wifi = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 222, 480);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    {
        lv_obj_t *parent_obj = obj;

        // Title
        {
            lv_obj_t *lbl = lv_label_create(parent_obj);
            objects.wifi_title = lbl;
            lv_obj_set_pos(lbl, 0, 5);
            lv_obj_set_size(lbl, 222, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_WIFI " WLAN");
        }

        // Status label
        {
            lv_obj_t *lbl = lv_label_create(parent_obj);
            objects.wifi_status = lbl;
            lv_obj_set_pos(lbl, 0, 28);
            lv_obj_set_size(lbl, 222, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_PART_MAIN);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x888888), LV_PART_MAIN);
            lv_label_set_text(lbl, "Suche Netzwerke...");
        }

        // Network list
        {
            lv_obj_t *list = lv_list_create(parent_obj);
            objects.wifi_list = list;
            lv_obj_set_pos(list, 5, 45);
            lv_obj_set_size(list, 212, 150);
            lv_obj_set_style_bg_color(list, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
            lv_obj_set_style_border_color(list, lv_color_hex(0x1565C0), LV_PART_MAIN);
            lv_obj_set_style_border_width(list, 1, LV_PART_MAIN);
            lv_obj_set_style_radius(list, 5, LV_PART_MAIN);
            lv_obj_set_style_pad_all(list, 2, LV_PART_MAIN);
        }

        // Password label (initially hidden)
        {
            lv_obj_t *lbl = lv_label_create(parent_obj);
            objects.wifi_password_label = lbl;
            lv_obj_set_pos(lbl, 5, 45);
            lv_obj_set_size(lbl, 212, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
            lv_label_set_text(lbl, "Passwort:");
            lv_obj_add_flag(lbl, LV_OBJ_FLAG_HIDDEN);
        }

        // Password textarea (initially hidden)
        {
            lv_obj_t *ta = lv_textarea_create(parent_obj);
            objects.wifi_password_ta = ta;
            lv_obj_set_pos(ta, 5, 65);
            lv_obj_set_size(ta, 212, 35);
            lv_textarea_set_one_line(ta, true);
            lv_textarea_set_placeholder_text(ta, "Passwort eingeben...");
            lv_textarea_set_password_mode(ta, true);
            lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_bg_color(ta, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
            lv_obj_set_style_text_color(ta, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_border_color(ta, lv_color_hex(0x1565C0), LV_PART_MAIN);
            lv_obj_add_flag(ta, LV_OBJ_FLAG_HIDDEN);
        }

        // Connect button (initially hidden)
        {
            lv_obj_t *btn = lv_btn_create(parent_obj);
            objects.wifi_connect_btn = btn;
            lv_obj_set_pos(btn, 5, 105);
            lv_obj_set_size(btn, 212, 40);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E7D32), LV_PART_MAIN);
            lv_obj_set_style_radius(btn, 5, LV_PART_MAIN);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_obj_center(lbl);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, LV_PART_MAIN);
            lv_label_set_text(lbl, "Verbinden");
        }

        // Back button
        {
            lv_obj_t *btn = lv_btn_create(parent_obj);
            objects.wifi_back_btn = btn;
            lv_obj_set_pos(btn, 5, 150);
            lv_obj_set_size(btn, 212, 40);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x757575), LV_PART_MAIN);
            lv_obj_set_style_radius(btn, 5, LV_PART_MAIN);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_obj_center(lbl);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, LV_PART_MAIN);
            lv_label_set_text(lbl, LV_SYMBOL_LEFT " Zurueck");
        }

        // Keyboard for password (at bottom)
        {
            lv_obj_t *kb = lv_keyboard_create(parent_obj);
            objects.wifi_keyboard = kb;
            lv_obj_set_size(kb, 222, 280);
            lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }
    }

    tick_screen_wifi();
}

void tick_screen_wifi() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_debug,
    tick_screen_settings,
    tick_screen_names,
    tick_screen_wifi,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_color_hex(0xF32196), lv_color_hex(0x36F443), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    create_screen_main();
    create_screen_debug();
    create_screen_settings();
    create_screen_names();
    create_screen_wifi();
}
