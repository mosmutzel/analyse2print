/**
 * light_sensor.h - LTR553 Ambient Light Sensor for display brightness control
 *
 * Uses the LTR553 sensor on T-Display-S3-Pro to automatically adjust
 * display brightness based on ambient light conditions.
 */

#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the LTR553 light sensor
 * @return true if sensor was found and initialized
 */
bool lightSensorInit(void);

/**
 * Check if light sensor is available
 * @return true if sensor is initialized and working
 */
bool lightSensorAvailable(void);

/**
 * Read ambient light level
 * @return light level in lux (0-64000)
 */
uint16_t lightSensorGetLux(void);

/**
 * Update display brightness based on ambient light
 * Call this periodically from main loop (e.g., every 500ms)
 */
void lightSensorUpdateBrightness(void);

/**
 * Set minimum brightness level (0-255)
 * Default is 10
 */
void lightSensorSetMinBrightness(uint8_t min);

/**
 * Set maximum brightness level (0-255)
 * Default is 255
 */
void lightSensorSetMaxBrightness(uint8_t max);

/**
 * Enable/disable automatic brightness control
 */
void lightSensorSetAutoEnabled(bool enabled);

/**
 * Check if automatic brightness is enabled
 */
bool lightSensorIsAutoEnabled(void);

/**
 * Manually set display brightness (0-255)
 * This disables automatic brightness control
 */
void lightSensorSetManualBrightness(uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif // LIGHT_SENSOR_H
