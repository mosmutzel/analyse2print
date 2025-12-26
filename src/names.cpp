/**
 * names.cpp - Names Manager
 *
 * Handles name storage and retrieval using NVS (Non-Volatile Storage)
 * Provides UI callbacks for the names screen
 */

#include "names.h"
#include "ui/screens.h"
#include <Preferences.h>

// Forward declaration of C function from ui.c
extern "C" {
  void loadScreen(enum ScreensEnum screenId);
}

// NVS namespace for names storage
static Preferences prefs;
#define NVS_NAMESPACE "names"
#define MAX_NAMES 10
#define MAX_NAME_LENGTH 20

// Current selected name (for printing)
static char currentName[MAX_NAME_LENGTH + 1] = "";

// Stored names
static char storedNames[MAX_NAMES][MAX_NAME_LENGTH + 1];
static int nameCount = 0;

// Forward declarations
static void namesButtonEventCb(lv_event_t *e);
static void keyboardEventCb(lv_event_t *e);
static void listItemEventCb(lv_event_t *e);

/**
 * Load names from NVS storage
 */
void namesLoad() {
    prefs.begin(NVS_NAMESPACE, true);  // Read-only mode

    nameCount = prefs.getInt("count", 0);
    if (nameCount > MAX_NAMES) nameCount = MAX_NAMES;

    for (int i = 0; i < nameCount; i++) {
        char key[8];
        snprintf(key, sizeof(key), "name%d", i);
        String name = prefs.getString(key, "");
        strncpy(storedNames[i], name.c_str(), MAX_NAME_LENGTH);
        storedNames[i][MAX_NAME_LENGTH] = '\0';
    }

    // Load current selected name
    String current = prefs.getString("current", "");
    strncpy(currentName, current.c_str(), MAX_NAME_LENGTH);
    currentName[MAX_NAME_LENGTH] = '\0';

    prefs.end();

    Serial.printf("[NAMES] Loaded %d names from storage\n", nameCount);
}

/**
 * Save names to NVS storage
 */
void namesSave() {
    prefs.begin(NVS_NAMESPACE, false);  // Read-write mode

    prefs.putInt("count", nameCount);

    for (int i = 0; i < nameCount; i++) {
        char key[8];
        snprintf(key, sizeof(key), "name%d", i);
        prefs.putString(key, storedNames[i]);
    }

    prefs.putString("current", currentName);

    prefs.end();

    Serial.printf("[NAMES] Saved %d names to storage\n", nameCount);
}

/**
 * Add a new name to storage
 */
bool namesAdd(const char* name) {
    if (name == NULL || strlen(name) == 0) return false;
    if (nameCount >= MAX_NAMES) {
        Serial.println("[NAMES] Storage full!");
        return false;
    }

    // Check for duplicates
    for (int i = 0; i < nameCount; i++) {
        if (strcmp(storedNames[i], name) == 0) {
            Serial.println("[NAMES] Name already exists");
            return false;
        }
    }

    strncpy(storedNames[nameCount], name, MAX_NAME_LENGTH);
    storedNames[nameCount][MAX_NAME_LENGTH] = '\0';
    nameCount++;

    namesSave();
    return true;
}

/**
 * Delete a name from storage
 */
bool namesDelete(int index) {
    if (index < 0 || index >= nameCount) return false;

    // Shift remaining names
    for (int i = index; i < nameCount - 1; i++) {
        strcpy(storedNames[i], storedNames[i + 1]);
    }
    nameCount--;

    namesSave();
    return true;
}

/**
 * Set current selected name
 */
void namesSetCurrent(const char* name) {
    if (name == NULL) {
        currentName[0] = '\0';
    } else {
        strncpy(currentName, name, MAX_NAME_LENGTH);
        currentName[MAX_NAME_LENGTH] = '\0';
    }
    namesSave();
}

/**
 * Get current selected name
 */
const char* namesGetCurrent() {
    return currentName;
}

/**
 * Get number of stored names
 */
int namesGetCount() {
    return nameCount;
}

/**
 * Get name at index
 */
const char* namesGetAt(int index) {
    if (index < 0 || index >= nameCount) return NULL;
    return storedNames[index];
}

/**
 * Update the names list UI
 */
void namesUpdateList() {
    extern objects_t objects;

    if (objects.names_list == NULL) return;

    // Clear existing items
    lv_obj_clean(objects.names_list);

    // Add stored names
    for (int i = 0; i < nameCount; i++) {
        lv_obj_t *btn = lv_list_add_btn(objects.names_list, LV_SYMBOL_OK, storedNames[i]);
        lv_obj_add_event_cb(btn, listItemEventCb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, LV_PART_MAIN);

        // Highlight current selection
        if (strcmp(storedNames[i], currentName) == 0) {
            lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_PURPLE), LV_PART_MAIN);
        }
    }
}

/**
 * Keyboard event callback - handles Enter key
 */
static void keyboardEventCb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target(e);

    if (code == LV_EVENT_READY) {
        // Enter pressed - save the name
        extern objects_t objects;
        if (objects.names_textarea != NULL) {
            const char* text = lv_textarea_get_text(objects.names_textarea);
            if (text != NULL && strlen(text) > 0) {
                if (namesAdd(text)) {
                    lv_textarea_set_text(objects.names_textarea, "");
                    namesUpdateList();
                    Serial.printf("[NAMES] Added: %s\n", text);
                }
            }
        }
    } else if (code == LV_EVENT_CANCEL) {
        // Cancel pressed - clear textarea
        extern objects_t objects;
        if (objects.names_textarea != NULL) {
            lv_textarea_set_text(objects.names_textarea, "");
        }
    }
}

/**
 * List item click callback - select name
 */
static void listItemEventCb(lv_event_t *e) {
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    if (index >= 0 && index < nameCount) {
        namesSetCurrent(storedNames[index]);
        namesUpdateList();
        Serial.printf("[NAMES] Selected: %s\n", storedNames[index]);
    }
}

/**
 * Names button click callback (from settings screen)
 */
static void namesButtonEventCb(lv_event_t *e) {
    loadScreen(SCREEN_ID_NAMES);
    namesUpdateList();
}

/**
 * Initialize names manager
 */
void namesInit() {
    extern objects_t objects;

    // Load saved names
    namesLoad();

    // Setup button callback on settings screen
    if (objects.btn_settings_names != NULL) {
        lv_obj_add_event_cb(objects.btn_settings_names, namesButtonEventCb, LV_EVENT_CLICKED, NULL);
    }

    // Setup keyboard callback
    if (objects.names_keyboard != NULL) {
        lv_obj_add_event_cb(objects.names_keyboard, keyboardEventCb, LV_EVENT_ALL, NULL);
    }

    // Update list
    namesUpdateList();

    Serial.println("[NAMES] Initialized");
}
