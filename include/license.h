#ifndef LICENSE_H
#define LICENSE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// License status
typedef enum {
    LICENSE_VALID,
    LICENSE_INVALID,
    LICENSE_NOT_FOUND,
    LICENSE_EXPIRED
} license_status_t;

// Initialize license manager
void licenseInit(void);

// Check if device is licensed
license_status_t licenseCheck(void);

// Get device ID (based on ESP32 eFuse MAC)
const char* licenseGetDeviceId(void);

// Activate license with key
bool licenseActivate(const char* licenseKey);

// Clear license (for testing)
void licenseClear(void);

// Check if license is valid (quick check)
bool licenseIsValid(void);

// Show license activation screen
void licenseShowActivation(void);

#ifdef __cplusplus
}
#endif

#endif // LICENSE_H
