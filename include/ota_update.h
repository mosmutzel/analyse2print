#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

// GitHub raw URL for version check
#define VERSION_CHECK_URL "https://raw.githubusercontent.com/mosmutzel/analyse2print/main/firmware/version.json"

// Check for firmware update
// Returns true if a new version is available
bool otaCheckForUpdate(void);

// Start OTA update from GitHub
// Returns true if update started successfully
bool otaStartUpdate(void);

// Get the latest available version string
const char* otaGetLatestVersion(void);

// Get changelog for latest version
const char* otaGetChangelog(void);

// Check if update is in progress
bool otaUpdateInProgress(void);

// Get update progress (0-100)
int otaGetProgress(void);

#ifdef __cplusplus
}
#endif

#endif // OTA_UPDATE_H
