#include "websocket_client.h"
#include <ArduinoJson.h>
#include "sensors.h"
#include "motor_control.h"
#include "camera.h"
#include "ca.h"

// Function to connect to WebSocket server
void connectToWebSocket() {
  if (WiFi.status() == WL_CONNECTED) {
    const char* server = wsConfig.server.c_str();
    Serial.printf("Connecting to WebSocket server: %s:%d%s (Secure: %s)\n", 
                  server, wsConfig.port, wsConfig.path, wsConfig.useSSL ? "Yes" : "No");
    
    if(wsConfig.useSSL) {
      wsClient.beginSSL(server, wsConfig.port, wsConfig.path, root_ca);
    } else {
      wsClient.begin(server, wsConfig.port, wsConfig.path);
    }
    wsClient.setReconnectInterval(reconnectInterval); // Try to reconnect every 5 seconds
  }
}

// WebSocket event handler
void handleWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket disconnected");
      wsConnected = false;
      break;
    
    case WStype_CONNECTED:
      Serial.println("WebSocket connected");
      wsConnected = true;
      // Send initial status message
      wsClient.sendTXT("{\"type\":\"status\",\"device\":\"esp32cam\",\"ready\":true}");
      break;
    
    case WStype_TEXT:
      Serial.printf("WebSocket received: %s\n", payload);
      
      // Parse JSON command
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Handle motor control commands
        if (doc.containsKey("motor") && doc.containsKey("cmd")) {
          int motorNum = doc["motor"];
          String cmd = doc["cmd"].as<String>();
          int speed = doc.containsKey("speed") ? doc["speed"] : 255;
          
          // Control the motor
          controlMotor(motorNum, cmd, speed);
        }
        
        // Handle sensor read requests
        if (doc.containsKey("read")) {
          String sensorType = doc["read"].as<String>();
          
          if (sensorType == "distance") {
            float distance = measureDistance();
            String response = "{\"type\":\"sensor\",\"sensor\":\"distance\",\"value\":" + String(distance) + "}";
            wsClient.sendTXT(response);
          }
        }
        
        // Handle camera streaming commands
        if (doc.containsKey("camera")) {
          String cameraCmd = doc["camera"].as<String>();
          
          if (cameraCmd == "start_stream") {
            startCameraStream();
            String response = "{\"type\":\"camera\",\"status\":\"streaming\",\"fps\":30}";
            wsClient.sendTXT(response);
          } 
          else if (cameraCmd == "stop_stream") {
            stopCameraStream();
            String response = "{\"type\":\"camera\",\"status\":\"stopped\"}";
            wsClient.sendTXT(response);
          }
          else if (cameraCmd == "capture") {
            // Single frame capture
            String response = "{\"type\":\"camera\",\"status\":\"capturing\"}";
            wsClient.sendTXT(response);
            cameraCapture(&wsClient);
          }
        }
      }
      break;
  }
}

// Setup WebSocket client
void setupWebSocketClient() {
  wsClient.onEvent(handleWebSocketEvent);
}
