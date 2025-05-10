#include "motor_control.h"

// Setup motors
void setupMotors() {
  // Set pin modes
  pinMode(pins.motor1_in1, OUTPUT);
  pinMode(pins.motor1_in2, OUTPUT);
  pinMode(pins.motor2_in1, OUTPUT);
  pinMode(pins.motor2_in2, OUTPUT);
  
  // Initialize motors to stopped state
  digitalWrite(pins.motor1_in1, LOW);
  digitalWrite(pins.motor1_in2, LOW);
  digitalWrite(pins.motor2_in1, LOW);
  digitalWrite(pins.motor2_in2, LOW);
}

// Control a motor
void controlMotor(int motorNum, const String& command, int speed) {
  // Ensure speed is within valid range
  speed = constrain(speed, 0, 255);
  
  uint8_t in1 = (motorNum == 1) ? pins.motor1_in1 : pins.motor2_in1;
  uint8_t in2 = (motorNum == 1) ? pins.motor1_in2 : pins.motor2_in2;
  
  if (command == "forward") {
    analogWrite(in1, speed);
    digitalWrite(in2, LOW);
    Serial.printf("Motor %d forward at speed %d\n", motorNum, speed);
  } else if (command == "backward") {
    digitalWrite(in1, LOW);
    analogWrite(in2, speed);
    Serial.printf("Motor %d backward at speed %d\n", motorNum, speed);
  } else if (command == "stop") {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    Serial.printf("Motor %d stopped\n", motorNum);
  }
}
