#include "config.h"
#include <ArduinoJson.h>

// Global variables
PinMap pins;
WiFiConfig wifiConfig;
WebSocketConfig wsConfig;
Preferences prefs;
bool wsConnected = false;
bool apMode = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 seconds between reconnection attempts

void loadDefaults() {
  // default pin assignments; overwritten by NVS if present
  pins = {
    25, 26,           // Motor 1 (IN1, IN2)
    27, 14,           // Motor 2 (IN1, IN2)
    12, 13,           // Ultrasonic (Trigger, Echo)
    21, 22,           // I2C (SDA, SCL)
  };
  
  // Default WiFi config (empty)
  wifiConfig = {"", "", false};

  wsConfig = {
    "ore0.lazos.me",  // Default WebSocket server
    80,               // Default port
    "/ws",            // Default path
    false             // No SSL by default
  };
}

void loadPrefs() {
  prefs.begin("config", true);
  
  // Load pin configuration
  if (prefs.isKey("pinmap")) {
    String json = prefs.getString("pinmap");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, json);
    pins.motor1_in1 = doc["m1i1"];
    pins.motor1_in2 = doc["m1i2"];
    pins.motor2_in1 = doc["m2i1"];
    pins.motor2_in2 = doc["m2i2"];
    pins.sonar_trigger = doc["sonar_trig"];
    pins.sonar_echo = doc["sonar_echo"];
    pins.i2c_sda    = doc["sda"];
    pins.i2c_scl    = doc["scl"];
  }
  
  // Load WiFi configuration
  if (prefs.isKey("wifi_ssid") && prefs.isKey("wifi_pass")) {
    wifiConfig.ssid = prefs.getString("wifi_ssid");
    wifiConfig.password = prefs.getString("wifi_pass");
    wifiConfig.configured = true;
  }

  // Load WebSocket configuration
  if (prefs.isKey("ws_server")) {
    String server = prefs.getString("ws_server");
    int colonPos = server.indexOf(':');
    if (colonPos > 0) {
      wsConfig.port = server.substring(colonPos + 1).toInt();
      wsConfig.server = server.substring(0, colonPos);
    } else {
      wsConfig.server = server;
    }
    wsConfig.path = prefs.getString("ws_path", "/ws");
    wsConfig.useSSL = prefs.getBool("ws_ssl", false);
  }
  
  prefs.end();
}

void savePinMap(const PinMap& pinMap) {
  prefs.begin("config", false);
  DynamicJsonDocument doc(256);
  doc["m1i1"]  = pinMap.motor1_in1;
  doc["m1i2"]  = pinMap.motor1_in2;
  doc["m2i1"]  = pinMap.motor2_in1;
  doc["m2i2"]  = pinMap.motor2_in2;
  doc["sonar_trig"] = pinMap.sonar_trigger;
  doc["sonar_echo"] = pinMap.sonar_echo;
  doc["sda"]   = pinMap.i2c_sda;
  doc["scl"]   = pinMap.i2c_scl;
  
  String out; 
  serializeJson(doc, out);
  prefs.putString("pinmap", out);
  
  pins = pinMap; // Update global pins variable
}

void saveWiFiConfig(const String& ssid, const String& password) {
  prefs.begin("config", false);
  prefs.putString("wifi_ssid", ssid);
  prefs.putString("wifi_pass", password);
  prefs.end();
  
  wifiConfig.ssid = ssid;
  wifiConfig.password = password;
  wifiConfig.configured = true;
}

void saveWebSocketConfig(const String& server, int port, const String& path, bool useSSL) {
  prefs.begin("config", false);
  prefs.putString("ws_server", server + ":" + String(port));
  prefs.putString("ws_path", path);
  prefs.putBool("ws_ssl", useSSL);
  prefs.end();
  
  wsConfig.server = server;
  wsConfig.port = port;
  wsConfig.path = path;
  wsConfig.useSSL = useSSL;
}
