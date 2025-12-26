/**
 * names.h - Names Manager
 */

#ifndef NAMES_H
#define NAMES_H

#include <Arduino.h>
#include <lvgl.h>

/**
 * Initialize names manager
 */
void namesInit();

/**
 * Load names from NVS storage
 */
void namesLoad();

/**
 * Save names to NVS storage
 */
void namesSave();

/**
 * Add a new name to storage
 */
bool namesAdd(const char* name);

/**
 * Delete a name from storage
 */
bool namesDelete(int index);

/**
 * Set current selected name
 */
void namesSetCurrent(const char* name);

/**
 * Get current selected name
 */
const char* namesGetCurrent();

/**
 * Get number of stored names
 */
int namesGetCount();

/**
 * Get name at index
 */
const char* namesGetAt(int index);

/**
 * Update the names list UI
 */
void namesUpdateList();

#endif // NAMES_H
