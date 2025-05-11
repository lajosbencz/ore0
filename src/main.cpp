#include <Arduino.h>
#include "camera_config.h"
#include "wifi_manager.h"
#include "websocket_client.h"
#include "motor.h"
#include "ultrasonic.h"

// Global objects
WebSocketClient* wsClient = nullptr;
unsigned long lastReconnectAttempt = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  
  // Initialize preferences for WiFi credentials and other settings
  preferences.begin("config", false);

  // Initialize motors with default pins
  // Motor pin configuration can be changed via WebSocket commands
  motor_init(DEFAULT_M1P1, DEFAULT_M1P2, DEFAULT_M2P1, DEFAULT_M2P2);
  
  // Initialize ultrasonic sensor
  ultrasonic_init();
  
  // Initialize GPIO pins 12-15 as outputs
  // These pins are also used for motors by default, but can be controlled directly via GPIO commands
  for (int pin = 12; pin <= 15; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    Serial.printf("Initialized GPIO pin %d as OUTPUT\n", pin);
  }
  
  // Camera and LED will be initialized when a WebSocket client connects
  Serial.println("Camera and LED will be initialized when a client connects");
  
  // Try to connect to WiFi
  bool wifiConnected = connectToWiFi();
  
  // If WiFi connection fails, start AP mode
  if (!wifiConnected) {
    setupAP();
  } else {
    // Connected to WiFi, try to connect to WebSocket server
    Serial.print("Camera Ready! Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
    
    // Get WebSocket server configuration from preferences
    String wsHost = getWsHost();
    uint16_t wsPort = getWsPort();
    String wsPath = getWsPath();
    bool wsTls = getWsTls();
    
    Serial.printf("WebSocket server: %s:%d%s (TLS: %s)\n", 
                  wsHost.c_str(), wsPort, wsPath.c_str(), 
                  wsTls ? "Yes" : "No");
    
    // Create WebSocket client
    wsClient = new WebSocketClient(wsHost.c_str(), wsPort, wsPath.c_str(), wsTls);
    
    // Try to connect to WebSocket server
    if (wsClient->connect()) {
      Serial.println("WebSocket connected");
      wsClient->startStreaming(); // Auto-start streaming
    } else {
      Serial.println("WebSocket connection failed");
    }
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Connected to WiFi
    if (wsClient) {
      // Update WebSocket client (handles data, ping, and streaming)
      // The WebSockets library handles reconnection automatically
      wsClient->update();
    } else {
      // Create new client if needed
      // Get WebSocket server configuration from preferences
      String wsHost = getWsHost();
      uint16_t wsPort = getWsPort();
      String wsPath = getWsPath();
      bool wsTls = getWsTls();
      
      wsClient = new WebSocketClient(wsHost.c_str(), wsPort, wsPath.c_str(), wsTls);
      
      if (wsClient->connect()) {
        Serial.println("WebSocket connected");
        wsClient->startStreaming();
      } else {
        Serial.println("WebSocket connection failed");
        delete wsClient;
        wsClient = nullptr;
        lastReconnectAttempt = millis();
      }
    }
  } else {
    // In AP mode, handle WiFi configuration
    handleWiFiClient();
  }
  
  delay(10);
}
