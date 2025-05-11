#include "ultrasonic.h"

// Initialize the ultrasonic sensor
void ultrasonic_init() {
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  
  // Ensure trigger pin is LOW
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  
  Serial.printf("Ultrasonic sensor initialized (Trigger: GPIO%d, Echo: GPIO%d)\n", 
                ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
}

// Measure the distance in centimeters
float ultrasonic_measure_distance() {
  // Clear the trigger pin
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  
  // Set the trigger pin HIGH for 10 microseconds
  digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  
  // Read the echo pin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout
  
  // Calculate the distance
  // Speed of sound is approximately 343 meters per second or 0.0343 cm/µs
  // Distance = (duration / 2) * 0.0343
  float distance = (duration / 2.0) * 0.0343;
  
  // If the distance is out of range or timeout occurred, return -1
  if (distance <= 0 || distance > 400) {
    return -1.0;
  }
  
  return distance;
}
