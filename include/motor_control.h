#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "config.h"

// Function declarations
void setupMotors();
void controlMotor(int motorNum, const String& command, int speed);

#endif // MOTOR_CONTROL_H
