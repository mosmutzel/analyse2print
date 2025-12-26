#if defined(EEZ_FOR_LVGL)
#include <eez/core/vars.h>
#endif

#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"







#if defined(EEZ_FOR_LVGL)

void ui_init() {
    eez_flow_init(assets, sizeof(assets), (lv_obj_t **)&objects, sizeof(objects), images, sizeof(images), actions);
}

void ui_tick() {
    eez_flow_tick();
    tick_screen(g_currentScreen);
}

#else

#include <string.h>

static int16_t currentScreen = -1;

static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId) {
    currentScreen = screenId - 1;

    // Direct access to screen objects instead of array indexing
    lv_obj_t *screen = NULL;
    if (screenId == SCREEN_ID_MAIN) {
        screen = objects.main;
    } else if (screenId == SCREEN_ID_DEBUG) {
        screen = objects.debug;
    } else if (screenId == SCREEN_ID_SETTINGS) {
        screen = objects.settings;
    } else if (screenId == SCREEN_ID_NAMES) {
        screen = objects.names;
    } else if (screenId == SCREEN_ID_WIFI) {
        screen = objects.wifi;
    }

    // Safety check: ensure screen object is valid
    if (screen == NULL) {
        // Don't load NULL screen - this would crash
        return;
    }

    // Additional safety: check if screen is a valid LVGL object
    if (!lv_obj_is_valid(screen)) {
        return;
    }

    // Use lv_scr_load instead of lv_scr_load_anim to avoid animation issues
    lv_scr_load(screen);
}

void ui_init() {
    create_screens();
    loadScreen(SCREEN_ID_MAIN);

}

void ui_tick() {
    tick_screen(currentScreen);
}

#endif
