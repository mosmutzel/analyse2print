#include "ota_update.h"
#include "wifi_manager.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Arduino.h>

// Version check URL
static const char* VERSION_URL = VERSION_CHECK_URL;

// Latest version info
static char latestVersion[16] = {0};
static char firmwareUrl[256] = {0};
static char changelog[256] = {0};
static bool updateAvailable = false;
static bool updateInProgress = false;
static int updateProgress = 0;

// Compare version strings (returns 1 if v1 > v2, -1 if v1 < v2, 0 if equal)
static int compareVersions(const char* v1, const char* v2) {
    int major1, minor1, patch1;
    int major2, minor2, patch2;

    sscanf(v1, "%d.%d.%d", &major1, &minor1, &patch1);
    sscanf(v2, "%d.%d.%d", &major2, &minor2, &patch2);

    if (major1 != major2) return major1 > major2 ? 1 : -1;
    if (minor1 != minor2) return minor1 > minor2 ? 1 : -1;
    if (patch1 != patch2) return patch1 > patch2 ? 1 : -1;
    return 0;
}

bool otaCheckForUpdate(void) {
    if (!wifiManagerIsConnected()) {
        Serial.println("[OTA] Not connected to WiFi");
        return false;
    }

    Serial.println("[OTA] Checking for updates...");

    HTTPClient http;
    http.begin(VERSION_URL);
    http.setTimeout(10000);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] HTTP error: %d\n", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.printf("[OTA] Response: %s\n", payload.c_str());

    // Parse JSON manually (simple parser for our format)
    // Find "version": "x.x.x"
    int versionStart = payload.indexOf("\"version\"");
    if (versionStart < 0) {
        Serial.println("[OTA] Version not found in response");
        return false;
    }

    int versionValueStart = payload.indexOf("\"", versionStart + 10) + 1;
    int versionValueEnd = payload.indexOf("\"", versionValueStart);
    String version = payload.substring(versionValueStart, versionValueEnd);
    strncpy(latestVersion, version.c_str(), sizeof(latestVersion) - 1);

    // Find "firmware_url": "..."
    int urlStart = payload.indexOf("\"firmware_url\"");
    if (urlStart >= 0) {
        int urlValueStart = payload.indexOf("\"", urlStart + 15) + 1;
        int urlValueEnd = payload.indexOf("\"", urlValueStart);
        String url = payload.substring(urlValueStart, urlValueEnd);
        strncpy(firmwareUrl, url.c_str(), sizeof(firmwareUrl) - 1);
    }

    // Find "changelog": "..."
    int changelogStart = payload.indexOf("\"changelog\"");
    if (changelogStart >= 0) {
        int changelogValueStart = payload.indexOf("\"", changelogStart + 12) + 1;
        int changelogValueEnd = payload.indexOf("\"", changelogValueStart);
        String cl = payload.substring(changelogValueStart, changelogValueEnd);
        strncpy(changelog, cl.c_str(), sizeof(changelog) - 1);
    }

    Serial.printf("[OTA] Current: %s, Latest: %s\n", FIRMWARE_VERSION, latestVersion);

    // Compare versions
    if (compareVersions(latestVersion, FIRMWARE_VERSION) > 0) {
        updateAvailable = true;
        Serial.println("[OTA] Update available!");
        return true;
    }

    Serial.println("[OTA] Already up to date");
    updateAvailable = false;
    return false;
}

bool otaStartUpdate(void) {
    if (!updateAvailable || strlen(firmwareUrl) == 0) {
        Serial.println("[OTA] No update available or URL missing");
        return false;
    }

    if (!wifiManagerIsConnected()) {
        Serial.println("[OTA] Not connected to WiFi");
        return false;
    }

    Serial.printf("[OTA] Starting update from: %s\n", firmwareUrl);
    updateInProgress = true;
    updateProgress = 0;

    HTTPClient http;
    http.begin(firmwareUrl);
    http.setTimeout(30000);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] HTTP error: %d\n", httpCode);
        http.end();
        updateInProgress = false;
        return false;
    }

    int contentLength = http.getSize();

    if (contentLength <= 0) {
        Serial.println("[OTA] Invalid content length");
        http.end();
        updateInProgress = false;
        return false;
    }

    Serial.printf("[OTA] Firmware size: %d bytes\n", contentLength);

    if (!Update.begin(contentLength)) {
        Serial.println("[OTA] Not enough space for update");
        http.end();
        updateInProgress = false;
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();

    uint8_t buff[1024];
    int bytesWritten = 0;

    while (http.connected() && bytesWritten < contentLength) {
        size_t available = stream->available();
        if (available) {
            size_t readBytes = stream->readBytes(buff, min(available, sizeof(buff)));
            size_t written = Update.write(buff, readBytes);
            if (written != readBytes) {
                Serial.println("[OTA] Write error");
                break;
            }
            bytesWritten += written;
            updateProgress = (bytesWritten * 100) / contentLength;

            // Update display every 10%
            if (updateProgress % 10 == 0) {
                Serial.printf("[OTA] Progress: %d%%\n", updateProgress);
            }
        }
        delay(1);
    }

    http.end();

    if (bytesWritten == contentLength && Update.end(true)) {
        Serial.println("[OTA] Update successful! Rebooting...");
        updateProgress = 100;
        delay(1000);
        ESP.restart();
        return true;
    } else {
        Serial.printf("[OTA] Update failed: %s\n", Update.errorString());
        Update.abort();
        updateInProgress = false;
        return false;
    }
}

const char* otaGetLatestVersion(void) {
    return latestVersion;
}

const char* otaGetChangelog(void) {
    return changelog;
}

bool otaUpdateInProgress(void) {
    return updateInProgress;
}

int otaGetProgress(void) {
    return updateProgress;
}
