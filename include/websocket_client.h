#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include "config.h"

// Global variables
extern WebSocketsClient wsClient;
extern unsigned long lastReconnectAttempt;
extern const unsigned long reconnectInterval;

// Function declarations
void setupWebSocketClient();
void connectToWebSocket();
void handleWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);

#endif // WEBSOCKET_CLIENT_H
