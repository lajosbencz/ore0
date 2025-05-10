#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// Constants
#define WEB_SERVER_PORT 80
#define WS_SECURE_PORT 443       // Default secure WebSocket port
#define WS_PATH "/ws"         // WebSocket path
#define ULTRASONIC_TIMEOUT_US 30000  // 30ms timeout for ultrasonic sensor
#define AP_SSID "ORE0-CONFIG-AP"    // Default AP name for configuration
#define AP_PASSWORD "ore0"       // Default AP password

struct WebSocketConfig {
  String server;
  int port;
  String path;
  bool useSSL;
};

// ——— Pin map —————————————————————
struct PinMap {
  uint8_t motor1_in1, motor1_in2;
  uint8_t motor2_in1, motor2_in2;
  uint8_t sonar_trigger, sonar_echo;  // Ultrasonic sensor pins
  uint8_t i2c_sda, i2c_scl;           // I2C pins for DAC
};

// ——— WiFi credentials ————————————————
struct WiFiConfig {
  String ssid;
  String password;
  bool configured;
};

// Global variables (extern declarations)
extern PinMap pins;
extern WiFiConfig wifiConfig;
extern WebSocketConfig wsConfig;
extern Preferences prefs;
extern bool wsConnected;
extern bool apMode;
extern unsigned long lastReconnectAttempt;
extern const unsigned long reconnectInterval;

// Function declarations
void loadDefaults();
void loadPrefs();
void savePinMap(const PinMap& pinMap);
void saveWiFiConfig(const String& ssid, const String& password);
void saveWebSocketConfig(const String& server, int port, const String& path, bool useSSL);

#endif // CONFIG_H
