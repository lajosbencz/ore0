#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <WebServer.h>

// DNS server for captive portal
const byte DNS_PORT = 53;
extern DNSServer dnsServer;

// Web server for WiFi configuration
extern WebServer server;

// Preferences for storing WiFi credentials
extern Preferences preferences;

// AP mode credentials
extern const char* AP_SSID;
extern const char* AP_PASSWORD;

// Function declarations
bool connectToWiFi();
void setupAP();
void handleWiFiConfigRoot();
void handleWiFiConfigSave();
void handleWiFiClient();

// WebSocket server configuration
String getWsHost();
uint16_t getWsPort();
String getWsPath();
bool getWsTls();

#endif // WIFI_MANAGER_H
