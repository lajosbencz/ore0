#include "wifi_manager.h"

// Function to start AP mode for configuration
void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  apMode = true;
  
  Serial.println("AP Mode started");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

// Function to connect to WiFi
bool connectToWiFi() {
  if (!wifiConfig.configured) {
    Serial.println("WiFi not configured. Starting AP mode.");
    startAPMode();
    return false;
  }
  
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(wifiConfig.ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());
  
  // Wait for connection
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    apMode = false;
    return true;
  } else {
    Serial.println("\nWiFi connection failed. Starting AP mode for configuration.");
    startAPMode();
    return false;
  }
}
