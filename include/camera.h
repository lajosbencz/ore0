#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include <WebSocketsClient.h>

// Camera model definition
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// Function declarations
void cameraSetup();
void cameraCapture(WebSocketsClient *webSocket);
void startCameraStream();
void stopCameraStream();
void handleCameraStream(WebSocketsClient *webSocket);

// Global variables for camera streaming
extern unsigned long lastFrameTime;
extern const unsigned long frameInterval;
extern bool streamingEnabled;

#endif // CAMERA_H
