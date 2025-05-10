#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <driver/gpio.h>

// Default motor pin configuration
extern const gpio_num_t DEFAULT_M1P1;
extern const gpio_num_t DEFAULT_M1P2;
extern const gpio_num_t DEFAULT_M2P1;
extern const gpio_num_t DEFAULT_M2P2;

typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKE,
} motor_direction_t;

typedef enum {
    MOTOR_LEFT = 0,
    MOTOR_RIGHT,
} motor_t;

typedef struct {
    gpio_num_t m1p1;
    gpio_num_t m1p2;
    gpio_num_t m2p1;
    gpio_num_t m2p2;
} motor_config_t;

void motor_init(gpio_num_t m1p1, gpio_num_t m1p2, gpio_num_t m2p1, gpio_num_t m2p2);
void motor_set(motor_t motor, motor_direction_t direction);
motor_direction_t motor_get_direction(motor_t motor);
void motor_off();
motor_config_t motor_get_config();

#endif // MOTOR_H
