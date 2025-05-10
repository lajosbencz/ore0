#include <Arduino.h>
#include "motor.h"

// Default motor pin configuration
const gpio_num_t DEFAULT_M1P1 = GPIO_NUM_12;
const gpio_num_t DEFAULT_M1P2 = GPIO_NUM_13;
const gpio_num_t DEFAULT_M2P1 = GPIO_NUM_15;
const gpio_num_t DEFAULT_M2P2 = GPIO_NUM_14;

motor_config_t motor_config = {
    .m1p1 = DEFAULT_M1P1,
    .m1p2 = DEFAULT_M1P2,
    .m2p1 = DEFAULT_M2P1,
    .m2p2 = DEFAULT_M2P2,
};

gpio_num_t motor_get_p1(motor_t motor)
{
    return (motor == MOTOR_LEFT) ? motor_config.m1p1 : motor_config.m2p1;
}

gpio_num_t motor_get_p2(motor_t motor)
{
    return (motor == MOTOR_LEFT) ? motor_config.m1p2 : motor_config.m2p2;
}

void motor_init(gpio_num_t m1p1, gpio_num_t m1p2, gpio_num_t m2p1, gpio_num_t m2p2)
{
    motor_config.m1p1 = m1p1;
    motor_config.m1p2 = m1p2;
    motor_config.m2p1 = m2p1;
    motor_config.m2p2 = m2p2;
    pinMode(m1p1, OUTPUT);
    digitalWrite(m1p1, LOW);
    pinMode(m1p2, OUTPUT);
    digitalWrite(m1p2, LOW);
    pinMode(m2p1, OUTPUT);
    digitalWrite(m2p1, LOW);
    pinMode(m2p2, OUTPUT);
    digitalWrite(m2p2, LOW);
}

void motor_set(motor_t motor, motor_direction_t direction)
{
    gpio_num_t p1 = motor_get_p1(motor);
    gpio_num_t p2 = motor_get_p2(motor);
    switch (direction)
    {
    default:
    case MOTOR_STOP:
        digitalWrite(p1, LOW);
        digitalWrite(p2, LOW);
        break;
    case MOTOR_FORWARD:
        digitalWrite(p1, HIGH);
        digitalWrite(p2, LOW);
        break;
    case MOTOR_BACKWARD:
        digitalWrite(p1, LOW);
        digitalWrite(p2, HIGH);
        break;
    case MOTOR_BRAKE:
        digitalWrite(p1, HIGH);
        digitalWrite(p2, HIGH);
        break;
    }
    Serial.printf("Motor %d set to %d\n", motor, direction);
}

motor_direction_t motor_get_direction(motor_t motor)
{
    gpio_num_t p1 = motor_get_p1(motor);
    gpio_num_t p2 = motor_get_p2(motor);
    if (digitalRead(p1) == LOW && digitalRead(p2) == LOW)
    {
        return MOTOR_STOP;
    }
    else if (digitalRead(p1) == HIGH && digitalRead(p2) == LOW)
    {
        return MOTOR_FORWARD;
    }
    else if (digitalRead(p1) == LOW && digitalRead(p2) == HIGH)
    {
        return MOTOR_BACKWARD;
    }
    else
    {
        return MOTOR_BRAKE;
    }
}

void motor_off()
{
    motor_set(MOTOR_LEFT, MOTOR_STOP);
    motor_set(MOTOR_RIGHT, MOTOR_STOP);
}

motor_config_t motor_get_config()
{
    return motor_config;
}
