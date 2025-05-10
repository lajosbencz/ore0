#ifndef CAMERA_CONFIG_H
#define CAMERA_CONFIG_H

#include <Arduino.h>
#include "esp_camera.h"
#include "camera_pins.h"

// Enable LED FLASH setting
#define CONFIG_LED_ILLUMINATOR_ENABLED 1

// Function declarations
esp_err_t init_camera();
void deinit_camera();
void setupLedFlash(int pin);
void turnOnLedFlash(int pin);
void turnOffLedFlash(int pin);

#endif // CAMERA_CONFIG_H
