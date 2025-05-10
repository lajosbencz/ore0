#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include "esp_camera.h"

// WebSocket client class using the WebSockets library
class WebSocketClient {
private:
  WebSocketsClient webSocket;
  const char* host;
  uint16_t port;
  const char* path;
  bool useTls;
  bool connected;
  unsigned long lastFrameTime;
  unsigned long lastHeartbeatTime;
  bool isStreaming;
  bool cameraInitialized;
  
  // WebSocket event handler
  static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

public:
  static WebSocketClient* instance; // Singleton instance for callback
  static int cameraInitFailCount; // Counter for camera initialization failures
  static unsigned long lastCameraInitAttempt; // Timestamp of last camera init attempt

public:
  WebSocketClient(const char* _host, uint16_t _port, const char* _path, bool _useTls = true);
  
  // Connection methods
  bool connect();
  bool isConnected() { return connected; }
  void disconnect();
  void setUseTls(bool _useTls) { useTls = _useTls; }
  
  // Camera streaming methods
  void startStreaming() { isStreaming = true; }
  void stopStreaming() { isStreaming = false; }
  bool streaming() { return isStreaming; }
  void sendCameraFrame();
  
  // Binary data methods
  void sendBinaryData(uint8_t* data, size_t length);
  
  // Update method (to be called in loop)
  void update();
  
  // Event handling
  void handleWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};

#endif // WEBSOCKET_CLIENT_H
