#include "esp_camera.h"
#include "sensor.h"
#include "camera.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include <Arduino.h>
#include "config.h"

void cameraSetup() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;  // Use VGA (640x480) for better streaming performance
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  config.jpeg_quality = 12;  // Lower quality (higher number) for smaller file size
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.fb_count = 2;  // Use 2 frame buffers for better performance

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
    // s->set_vflip(s, 1);        // flip it back
    // s->set_brightness(s, 1);   // up the brightness just a bit
    // s->set_saturation(s, -2);  // lower the saturation
  s->set_framesize(s, FRAMESIZE_VGA);  // Ensure we use VGA resolution
}

// Global variables for camera streaming
unsigned long lastFrameTime = 0;
const unsigned long frameInterval = 33; // ~30fps (1000ms/30)
bool streamingEnabled = false;

void cameraCapture(WebSocketsClient *webSocket) {
  camera_fb_t *fb = NULL;
  struct timeval _timestamp;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[128];

  int64_t fr_start = esp_timer_get_time();

  fb = esp_camera_fb_get();
  if (!fb) {
    log_e("Camera capture failed");
    res = ESP_FAIL;
  } else {
    _timestamp.tv_sec = fb->timestamp.tv_sec;
    _timestamp.tv_usec = fb->timestamp.tv_usec;
    if (fb->format != PIXFORMAT_JPEG) {
      bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
      esp_camera_fb_return(fb);
      fb = NULL;
      if (!jpeg_converted) {
        log_e("JPEG compression failed");
        res = ESP_FAIL;
      }
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }
  }
  if (res == ESP_OK && wsConnected) {
    // Send the full JPEG buffer
    webSocket->sendBIN(_jpg_buf, _jpg_buf_len);
  }
  if (fb) {
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  if (_jpg_buf && fb == NULL) {
    // Only free the buffer if it was allocated by frame2jpg
    free(_jpg_buf);
    _jpg_buf = NULL;
  }
  if (res != ESP_OK) {
    log_e("Send frame failed");
    return;
  }
  int64_t fr_time = esp_timer_get_time() - fr_start;
  Serial.printf("MJPG: %uB %ums\n", (uint32_t)(_jpg_buf_len), (uint32_t)(fr_time/1000));
}

// Function to start camera streaming
void startCameraStream() {
  streamingEnabled = true;
  Serial.println("Camera streaming started at 30fps");
}

// Function to stop camera streaming
void stopCameraStream() {
  streamingEnabled = false;
  Serial.println("Camera streaming stopped");
}

// Function to handle camera streaming at 30fps
void handleCameraStream(WebSocketsClient *webSocket) {
  if (streamingEnabled && wsConnected) {
    unsigned long currentMillis = millis();
    
    // Check if it's time to send a new frame (30fps = ~33ms between frames)
    if (currentMillis - lastFrameTime >= frameInterval) {
      lastFrameTime = currentMillis;
      cameraCapture(webSocket);
    }
  }
}
