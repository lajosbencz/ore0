#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>
#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "websocket_client.h"
#include "sensors.h"
#include "motor_control.h"
#include "camera.h"

// Global variables
WebSocketsClient wsClient;
AsyncWebServer server(WEB_SERVER_PORT);

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nESP32-CAM Robot Controller");
  
  // Load configuration
  loadDefaults();
  loadPrefs();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
  }

  // Setup camera
  cameraSetup();
  Serial.println("Camera initialized");

  // Setup motors
  setupMotors();

  // Setup I2C
  Wire.begin(pins.i2c_sda, pins.i2c_scl);
  
  // Setup ultrasonic sensor
  pinMode(pins.sonar_trigger, OUTPUT);
  pinMode(pins.sonar_echo, INPUT);
  
  // Connect to WiFi or start AP mode
  bool connected = connectToWiFi();
  
  // Setup web server
  setupWebServer();
  
  // Setup WebSocket client
  setupWebSocketClient();
  
  // Initial connection attempt to WebSocket server if WiFi is connected
  if (connected) {
    connectToWebSocket();
  }
}

void loop() {
  // Maintain WebSocket connection
  wsClient.loop();
  
  // Check if we need to reconnect to WebSocket server
  if (!wsConnected && WiFi.status() == WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = currentMillis;
      Serial.println("Attempting to reconnect to WebSocket server...");
      connectToWebSocket();
    }
  }
  
  // Handle camera streaming at 30fps
  handleCameraStream(&wsClient);
  
  // Small delay to prevent watchdog timer issues
  delay(1); // Reduced delay to ensure smoother frame rate
}
