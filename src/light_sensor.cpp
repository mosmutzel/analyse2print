/**
 * light_sensor.cpp - LTR553 Ambient Light Sensor for display brightness control
 *
 * Uses the LTR553 sensor on T-Display-S3-Pro to automatically adjust
 * display brightness based on ambient light conditions.
 */

#include "light_sensor.h"
#include "utilities.h"
#include <Wire.h>
#include <SensorLTR553.hpp>

// Sensor instance
static SensorLTR553 als;
static bool sensorInitialized = false;

// Brightness control settings
static uint8_t minBrightness = 10;
static uint8_t maxBrightness = 255;
static bool autoEnabled = true;
static uint8_t currentBrightness = 128;

// Update timing
static unsigned long lastUpdateTime = 0;
static const unsigned long UPDATE_INTERVAL = 500;  // 500ms update interval

// Smoothing filter
static uint16_t luxHistory[4] = {0, 0, 0, 0};
static uint8_t luxHistoryIndex = 0;

/**
 * Apply brightness to display backlight
 */
static void applyBrightness(uint8_t brightness) {
    if (brightness == currentBrightness) return;

    currentBrightness = brightness;

#ifdef USING_DISPLAY_PRO_V1
    // V1.0 uses direct PWM
    analogWrite(BOARD_TFT_BL, brightness);
#else
    // V1.1 uses different backlight driver with 16 levels
    // Map 0-255 to 0-16
    uint8_t level = map(brightness, 0, 255, 0, BRIGHTNESS_MAX_LEVEL);
    analogWrite(BOARD_TFT_BL, level);
#endif
}

/**
 * Map lux value to brightness
 * Low light = low brightness, high light = high brightness
 */
static uint8_t luxToBrightness(uint16_t lux) {
    // Mapping curve:
    // 0-10 lux (dark) -> minimum brightness
    // 10-100 lux (indoor dim) -> 30-50% brightness
    // 100-500 lux (indoor normal) -> 50-80% brightness
    // 500-5000 lux (bright indoor/shade) -> 80-100% brightness
    // 5000+ lux (direct sunlight) -> max brightness

    uint8_t brightness;

    if (lux < 10) {
        brightness = minBrightness;
    } else if (lux < 100) {
        // Linear interpolation from min to 50%
        brightness = map(lux, 10, 100, minBrightness, 128);
    } else if (lux < 500) {
        // Linear interpolation from 50% to 80%
        brightness = map(lux, 100, 500, 128, 200);
    } else if (lux < 5000) {
        // Linear interpolation from 80% to max
        brightness = map(lux, 500, 5000, 200, maxBrightness);
    } else {
        brightness = maxBrightness;
    }

    return brightness;
}

bool lightSensorInit(void) {
    log_i("Initializing LTR553 light sensor...");

    // Use the existing Wire instance that PMU already initialized
    // Don't re-initialize I2C to avoid conflicts
    if (!als.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, LTR553_SLAVE_ADDRESS)) {
        log_w("LTR553 sensor not found!");
        sensorInitialized = false;
        return false;
    }

    log_i("LTR553 sensor found!");

    // Configure ambient light sensor
    // Gain 1X: 1 lux to 64k lux range (good for general use)
    als.setLightSensorGain(SensorLTR553::ALS_GAIN_1X);

    // Integration time 100ms and measurement rate 500ms
    als.setLightSensorRate(SensorLTR553::ALS_INTEGRATION_TIME_100MS,
                           SensorLTR553::ALS_MEASUREMENT_TIME_500MS);

    // Enable ambient light sensor
    als.enableLightSensor();

    // We don't use proximity sensor, so keep it disabled
    // als.enableProximity();

    sensorInitialized = true;
    log_i("LTR553 sensor initialized successfully");

    // Don't change backlight pin - it's already configured by lvgl_hal
    // Just set initial brightness to max
    currentBrightness = maxBrightness;

    return true;
}

bool lightSensorAvailable(void) {
    return sensorInitialized;
}

uint16_t lightSensorGetLux(void) {
    if (!sensorInitialized) return 0;

    // Read both channels
    int ch0 = als.getLightSensor(0);  // CH0: visible + IR
    int ch1 = als.getLightSensor(1);  // CH1: IR only

    if (ch0 < 0 || ch1 < 0) return 0;

    // Simple lux calculation from CH0 and CH1
    // CH0 = visible + IR, CH1 = IR only
    // Lux is approximated from CH0 with some adjustment for IR
    uint16_t lux;
    if (ch0 > 0 && ch1 > 0) {
        float ratio = (float)ch1 / (float)ch0;
        if (ratio < 0.5) {
            lux = (uint16_t)(1.7743 * ch0 - 1.1059 * ch1);
        } else if (ratio < 0.61) {
            lux = (uint16_t)(0.9824 * ch0 - 0.9824 * ch1);
        } else if (ratio < 0.8) {
            lux = (uint16_t)(0.7050 * ch0 - 0.7050 * ch1);
        } else if (ratio < 1.3) {
            lux = (uint16_t)(0.1767 * ch0 - 0.1767 * ch1);
        } else {
            lux = 0;
        }
    } else {
        lux = (uint16_t)ch0;  // Fallback: use raw CH0 value
    }

    return lux;
}

void lightSensorUpdateBrightness(void) {
    if (!sensorInitialized || !autoEnabled) return;

    unsigned long now = millis();
    if (now - lastUpdateTime < UPDATE_INTERVAL) return;
    lastUpdateTime = now;

    // Get current lux reading
    uint16_t lux = lightSensorGetLux();

    // Add to history for smoothing
    luxHistory[luxHistoryIndex] = lux;
    luxHistoryIndex = (luxHistoryIndex + 1) % 4;

    // Calculate average
    uint32_t avgLux = 0;
    for (int i = 0; i < 4; i++) {
        avgLux += luxHistory[i];
    }
    avgLux /= 4;

    // Convert to brightness and apply
    uint8_t brightness = luxToBrightness((uint16_t)avgLux);
    applyBrightness(brightness);
}

void lightSensorSetMinBrightness(uint8_t min) {
    minBrightness = min;
}

void lightSensorSetMaxBrightness(uint8_t max) {
    maxBrightness = max;
}

void lightSensorSetAutoEnabled(bool enabled) {
    autoEnabled = enabled;
}

bool lightSensorIsAutoEnabled(void) {
    return autoEnabled;
}

void lightSensorSetManualBrightness(uint8_t brightness) {
    autoEnabled = false;
    applyBrightness(brightness);
}
