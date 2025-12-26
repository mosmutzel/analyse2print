/**
 * webserver.cpp - Configuration Web Server
 *
 * Provides a web interface for configuring:
 * - Diver names (add, delete, select)
 * - WiFi networks (add, delete)
 *
 * Uses raw WiFiServer for maximum compatibility with ESP32-S3 USB Host mode
 */

#include "webserver.h"
#include <Arduino.h>
#include <WiFi.h>
#include "names.h"
#include "wifi_manager.h"
#include "version.h"

// Create server on port 80
static WiFiServer webServer(80);
static bool serverRunning = false;

// URL decode helper
static String urlDecode(const String& text) {
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = text.length();
    unsigned int i = 0;
    while (i < len) {
        char c = text.charAt(i);
        if (c == '+') {
            decoded += ' ';
        } else if (c == '%' && i + 2 < len) {
            temp[2] = text.charAt(i + 1);
            temp[3] = text.charAt(i + 2);
            decoded += (char)strtol(temp, NULL, 16);
            i += 2;
        } else {
            decoded += c;
        }
        i++;
    }
    return decoded;
}

// Parse query parameters from URL
static String getParam(const String& url, const String& param) {
    int start = url.indexOf(param + "=");
    if (start < 0) return "";
    start += param.length() + 1;
    int end = url.indexOf("&", start);
    if (end < 0) end = url.length();
    return urlDecode(url.substring(start, end));
}

// Parse POST body parameters
static String getPostParam(const String& body, const String& param) {
    int start = body.indexOf(param + "=");
    if (start < 0) return "";
    start += param.length() + 1;
    int end = body.indexOf("&", start);
    if (end < 0) end = body.length();
    return urlDecode(body.substring(start, end));
}

// Build main HTML page
static String buildPage() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Analyse2Print</title>";
    html += "<style>";
    html += "body{font-family:Arial;max-width:600px;margin:0 auto;padding:20px;background:#1a1a2e;color:#eee}";
    html += "h1,h2{color:#00d4aa}";
    html += ".card{background:#16213e;border-radius:10px;padding:20px;margin:20px 0}";
    html += "input[type=text],input[type=password]{width:100%;padding:10px;margin:10px 0;border:1px solid #0f3460;border-radius:5px;background:#0f3460;color:#fff;box-sizing:border-box}";
    html += "button,input[type=submit]{background:#00d4aa;color:#1a1a2e;border:none;padding:10px 20px;border-radius:5px;cursor:pointer;font-weight:bold;margin:5px}";
    html += ".delete-btn{background:#e74c3c;color:white}";
    html += ".select-btn{background:#3498db;color:white}";
    html += "ul{list-style:none;padding:0}";
    html += "li{display:flex;justify-content:space-between;align-items:center;padding:10px;margin:5px 0;background:#0f3460;border-radius:5px}";
    html += ".current{color:#00d4aa;font-weight:bold}";
    html += ".version{text-align:center;color:#666;margin-top:30px}";
    html += "</style></head><body>";
    html += "<h1>Analyse2Print</h1>";

    // Names section
    html += "<div class='card'><h2>Taucher-Namen</h2>";
    html += "<form action='/add_name' method='POST'>";
    html += "<input type='text' name='name' placeholder='Neuer Name' maxlength='20' required>";
    html += "<input type='submit' value='Hinzufuegen'></form><ul>";

    const char* currentName = namesGetCurrent();
    int count = namesGetCount();
    for (int i = 0; i < count; i++) {
        const char* name = namesGetAt(i);
        bool isSelected = (strcmp(name, currentName) == 0);
        html += "<li><span";
        if (isSelected) html += " class='current'";
        html += ">" + String(name);
        if (isSelected) html += " (aktiv)";
        html += "</span><div>";
        if (!isSelected) {
            html += "<a href='/select_name?idx=" + String(i) + "'><button class='select-btn'>Waehlen</button></a>";
        }
        html += "<a href='/delete_name?idx=" + String(i) + "'><button class='delete-btn'>Loeschen</button></a>";
        html += "</div></li>";
    }
    if (count == 0) {
        html += "<li>Keine Namen gespeichert</li>";
    }
    html += "</ul></div>";

    // WiFi section
    html += "<div class='card'><h2>WiFi-Netzwerke</h2>";
    html += "<form action='/add_wifi' method='POST'>";
    html += "<input type='text' name='ssid' placeholder='SSID' maxlength='32' required>";
    html += "<input type='password' name='password' placeholder='Passwort' maxlength='64'>";
    html += "<input type='submit' value='Speichern'></form><ul>";

    int wifiCount = wifiManagerGetSavedCount();
    for (int i = 0; i < wifiCount; i++) {
        const char* ssid = wifiManagerGetSavedSSID(i);
        html += "<li><span>" + String(ssid) + "</span>";
        html += "<a href='/delete_wifi?ssid=" + String(ssid) + "'><button class='delete-btn'>Loeschen</button></a></li>";
    }
    if (wifiCount == 0) {
        html += "<li>Keine Netzwerke gespeichert</li>";
    }
    html += "</ul></div>";

    html += "<div class='version'>Version " + String(FIRMWARE_VERSION) + "</div>";
    html += "</body></html>";

    return html;
}

// Send HTTP response
static void sendResponse(WiFiClient& client, int code, const String& contentType, const String& body) {
    client.print("HTTP/1.1 ");
    client.print(code);
    client.println(code == 200 ? " OK" : (code == 302 ? " Found" : " Error"));
    client.println("Content-Type: " + contentType);
    client.print("Content-Length: ");
    client.println(body.length());
    if (code == 302) {
        client.println("Location: /");
    }
    client.println("Connection: close");
    client.println();
    client.print(body);
}

// Send redirect response
static void sendRedirect(WiFiClient& client) {
    client.println("HTTP/1.1 302 Found");
    client.println("Location: /");
    client.println("Connection: close");
    client.println();
}

void webserverInit(void) {
    Serial.println("[WEB] Webserver initialized");
}

void webserverLoop(void) {
    if (!serverRunning) return;

    WiFiClient client = webServer.available();
    if (!client) return;

    // Wait for data
    unsigned long timeout = millis() + 3000;
    while (!client.available() && millis() < timeout) {
        delay(1);
    }
    if (!client.available()) {
        client.stop();
        return;
    }

    // Read first line (request line)
    String request = client.readStringUntil('\r');
    client.readStringUntil('\n'); // consume LF

    Serial.printf("[WEB] Request: %s\n", request.c_str());

    // Parse method and path
    String method = request.substring(0, request.indexOf(' '));
    int pathStart = request.indexOf(' ') + 1;
    int pathEnd = request.indexOf(' ', pathStart);
    String path = request.substring(pathStart, pathEnd);

    // Read headers
    int contentLength = 0;
    while (client.available()) {
        String line = client.readStringUntil('\r');
        client.readStringUntil('\n');
        if (line.length() == 0) break;
        if (line.startsWith("Content-Length:")) {
            contentLength = line.substring(15).toInt();
        }
    }

    // Read POST body if present
    String body = "";
    if (method == "POST" && contentLength > 0) {
        body = client.readString();
    }

    // Route handling
    if (path == "/" || path.startsWith("/?")) {
        sendResponse(client, 200, "text/html", buildPage());
    }
    else if (path.startsWith("/add_name") && method == "POST") {
        String name = getPostParam(body, "name");
        name.trim();
        if (name.length() > 0) {
            namesAdd(name.c_str());
            Serial.printf("[WEB] Added name: %s\n", name.c_str());
        }
        sendRedirect(client);
    }
    else if (path.startsWith("/delete_name")) {
        String idx = getParam(path, "idx");
        if (idx.length() > 0) {
            namesDelete(idx.toInt());
            Serial.printf("[WEB] Deleted name at index: %s\n", idx.c_str());
        }
        sendRedirect(client);
    }
    else if (path.startsWith("/select_name")) {
        String idx = getParam(path, "idx");
        if (idx.length() > 0) {
            const char* name = namesGetAt(idx.toInt());
            if (name) {
                namesSetCurrent(name);
                Serial.printf("[WEB] Selected name: %s\n", name);
            }
        }
        sendRedirect(client);
    }
    else if (path.startsWith("/add_wifi") && method == "POST") {
        String ssid = getPostParam(body, "ssid");
        String password = getPostParam(body, "password");
        ssid.trim();
        if (ssid.length() > 0) {
            wifiManagerSaveNetwork(ssid.c_str(), password.c_str());
            Serial.printf("[WEB] Saved WiFi: %s\n", ssid.c_str());
        }
        sendRedirect(client);
    }
    else if (path.startsWith("/delete_wifi")) {
        String ssid = getParam(path, "ssid");
        if (ssid.length() > 0) {
            wifiManagerDeleteNetwork(ssid.c_str());
            Serial.printf("[WEB] Deleted WiFi: %s\n", ssid.c_str());
        }
        sendRedirect(client);
    }
    else {
        sendResponse(client, 404, "text/plain", "Not Found");
    }

    client.stop();
}

void webserverStart(void) {
    if (!serverRunning && wifiManagerIsConnected()) {
        webServer.begin();
        serverRunning = true;
        Serial.printf("[WEB] Webserver started at http://%s/\n", wifiManagerGetIP());
    }
}

void webserverStop(void) {
    if (serverRunning) {
        webServer.end();
        serverRunning = false;
        Serial.println("[WEB] Webserver stopped");
    }
}

bool webserverIsRunning(void) {
    return serverRunning;
}
