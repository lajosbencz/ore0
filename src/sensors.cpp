#include "sensors.h"

// Function to measure distance using ultrasonic sensor
float measureDistance() {
  // Clear the trigger pin
  digitalWrite(pins.sonar_trigger, LOW);
  delayMicroseconds(2);
  
  // Set the trigger pin HIGH for 10 microseconds
  digitalWrite(pins.sonar_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pins.sonar_trigger, LOW);
  
  // Read the echo pin, returns the sound wave travel time in microseconds
  long duration = pulseIn(pins.sonar_echo, HIGH, ULTRASONIC_TIMEOUT_US);
  
  // Calculate the distance
  float distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  
  return distance; // Return distance in cm
}
