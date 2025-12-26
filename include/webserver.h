#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>

// Initialize the web server
void webserverInit(void);

// Handle web server requests (call in loop)
void webserverLoop(void);

// Start web server (call when WiFi connected)
void webserverStart(void);

// Stop web server (call when WiFi disconnected)
void webserverStop(void);

// Check if web server is running
bool webserverIsRunning(void);

#endif // WEBSERVER_H
