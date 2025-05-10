#include "websocket_client.h"
#include "commands.h"
#include "motor.h"

// Function to handle binary motor commands
void handle_binary_command(uint8_t *payload, size_t length)
{
  if (length < 1) return;
  // print the command type
  Serial.printf("Received command type: %02X\n", payload[0]);
  
  uint8_t cmd = payload[0];
  
  switch (cmd)
  {
    case CMD_MOTOR_CONTROL:
      if (length >= sizeof(motor_control_cmd_t))
      {
        motor_control_cmd_t *motor_cmd = (motor_control_cmd_t *)payload;
        
        // Convert command direction to motor_direction_t
        motor_direction_t direction;
        switch (motor_cmd->direction)
        {
          case MOTOR_CMD_FORWARD:
            direction = MOTOR_FORWARD;
            break;
          case MOTOR_CMD_BACKWARD:
            direction = MOTOR_BACKWARD;
            break;
          case MOTOR_CMD_BRAKE:
            direction = MOTOR_BRAKE;
            break;
          case MOTOR_CMD_STOP:
          default:
            direction = MOTOR_STOP;
            break;
        }
        
        // Apply command to the specified motor(s)
        switch (motor_cmd->motor)
        {
          case MOTOR_CMD_LEFT:
            motor_set(MOTOR_LEFT, direction);
            Serial.printf("Left motor set to direction %d\n", direction);
            break;
          case MOTOR_CMD_RIGHT:
            motor_set(MOTOR_RIGHT, direction);
            Serial.printf("Right motor set to direction %d\n", direction);
            break;
          case MOTOR_CMD_BOTH:
            motor_set(MOTOR_LEFT, direction);
            motor_set(MOTOR_RIGHT, direction);
            Serial.printf("Both motors set to direction %d\n", direction);
            break;
        }
      }
      break;
      
    case CMD_SET_MOTOR_PINS:
      if (length >= sizeof(motor_pins_cmd_t))
      {
        motor_pins_cmd_t *pins_cmd = (motor_pins_cmd_t *)payload;
        
        // Initialize motors with the new pin configuration
        motor_init(
          (gpio_num_t)pins_cmd->m1p1,
          (gpio_num_t)pins_cmd->m1p2,
          (gpio_num_t)pins_cmd->m2p1,
          (gpio_num_t)pins_cmd->m2p2
        );
        
        Serial.printf("Motor pins reconfigured: M1P1=%d, M1P2=%d, M2P1=%d, M2P2=%d\n",
                      pins_cmd->m1p1, pins_cmd->m1p2, pins_cmd->m2p1, pins_cmd->m2p2);
      }
      break;
      
    case CMD_GET_MOTOR_PINS:
      {
        // Create response with current motor pin configuration
        motor_pins_cmd_t response;
        motor_config_t config = motor_get_config();
        response.command = CMD_SET_MOTOR_PINS;
        response.m1p1 = config.m1p1;
        response.m1p2 = config.m1p2;
        response.m2p1 = config.m2p1;
        response.m2p2 = config.m2p2;
      }
      break;
  }
}

// Initialize static instance pointer for callback
WebSocketClient *WebSocketClient::instance = nullptr;

WebSocketClient::WebSocketClient(const char *_host, uint16_t _port, const char *_path, bool _useTls)
    : host(_host), port(_port), path(_path), useTls(_useTls), connected(false),
      lastFrameTime(0), isStreaming(false)
{
  // Set this instance as the singleton for callback
  instance = this;
}

bool WebSocketClient::connect()
{
  // Configure WebSocket client
  if (useTls) {
    Serial.println("Connecting to WebSocket server with TLS");
    webSocket.beginSSL(host, port, path);
  } else {
    Serial.println("Connecting to WebSocket server without TLS");
    webSocket.begin(host, port, path);
  }
  
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  // Set a short timeout for the initial connection
  unsigned long timeout = millis();
  while (!connected)
  {
    webSocket.loop();
    if (millis() - timeout > 5000)
    {
      Serial.println("WebSocket connection timeout");
      return false;
    }
    delay(10);
  }

  // Send initial message to identify as camera
  webSocket.sendTXT("CAMERA_CONNECTED");

  return true;
}

void WebSocketClient::disconnect()
{
  if (connected)
  {
    webSocket.disconnect();
    connected = false;
  }
}

void WebSocketClient::sendCameraFrame()
{
  if (!connected || !isStreaming)
    return;

  // Limit frame rate
  if (millis() - lastFrameTime < 33)
    return; // 30 fps max

  // Get a frame from the camera
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  // Send image data as binary
  sendBinaryData(fb->buf, fb->len);

  static uint8_t frameCount = 0;
  frameCount++;
  if (frameCount % 10 == 0)
  {
    Serial.printf("Last frame: %dx%d, size: %zu bytes\n", fb->width, fb->height, fb->len);
    frameCount = 0;
  }

  // Return the frame buffer to be reused
  esp_camera_fb_return(fb);
  lastFrameTime = millis();
}

void WebSocketClient::sendBinaryData(uint8_t* data, size_t length)
{
  if (!connected)
    return;
    
  webSocket.sendBIN(data, length);
}

void WebSocketClient::update()
{
  // Handle WebSocket events and keep connection alive
  webSocket.loop();

  // Send camera frame if streaming
  if (connected && isStreaming)
  {
    sendCameraFrame();
  }
}

// Static event handler that forwards to the instance method
void WebSocketClient::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  if (instance)
  {
    instance->handleWebSocketEvent(type, payload, length);
  }
}

// Instance method to handle WebSocket events
void WebSocketClient::handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("WebSocket disconnected");
    connected = false;
    break;

  case WStype_CONNECTED:
    Serial.println("WebSocket connected");
    connected = true;
    break;

  case WStype_TEXT:
    // Null-terminate the payload for string operations
    payload[length] = 0;
    Serial.printf("Received text len=%d\n", length);

    // Handle commands
    if (strcmp((char *)payload, "START_STREAM") == 0)
    {
      isStreaming = true;
      Serial.println("Streaming started");
    }
    else if (strcmp((char *)payload, "STOP_STREAM") == 0)
    {
      isStreaming = false;
      Serial.println("Streaming stopped");
    }
    else
    {
      Serial.printf("Unknown text command: %s\n", (char *)payload);
    }
    break;

  case WStype_BIN:
    Serial.printf("Received binary data of length %d\n", length);
    handle_binary_command(payload, length);
    break;

  case WStype_PING:
    // Handled automatically by the library
    Serial.println("Received ping");
    break;

  case WStype_PONG:
    Serial.println("Received pong");
    break;

  case WStype_ERROR:
    Serial.println("WebSocket error");
    break;
  }
}
