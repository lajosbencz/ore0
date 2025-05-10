#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// Function declarations
void startAPMode();
bool connectToWiFi();

#endif // WIFI_MANAGER_H
