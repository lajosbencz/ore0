#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "config.h"

// Global variables
extern AsyncWebServer server;

// Function declarations
void setupWebServer();
String processor(const String& var);

// HTML content
extern const char* configHtml;

#endif // WEB_SERVER_H
