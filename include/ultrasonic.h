#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

// Ultrasonic sensor pins
#define ULTRASONIC_TRIGGER_PIN 4  // GPIO4 for trigger
#define ULTRASONIC_ECHO_PIN 2     // GPIO2 for echo

// Initialize the ultrasonic sensor
void ultrasonic_init();

// Measure the distance in centimeters
float ultrasonic_measure_distance();

#endif // ULTRASONIC_H
