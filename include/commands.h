#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum {
    // Motor control commands
    CMD_MOTOR_CONTROL = 0x10,   // Command to control motors
    
    // Motor pin configuration commands
    CMD_SET_MOTOR_PINS = 0x20,  // Command to set motor pins
    CMD_GET_MOTOR_PINS = 0x21,  // Command to get current motor pin configuration
    
    // GPIO control commands
    CMD_SET_GPIO = 0x30,        // Command to set GPIO pin state
    CMD_GET_GPIO = 0x31         // Command to get GPIO pin state
} command_t;

// Motor direction values
typedef enum {
    MOTOR_CMD_STOP = 0,
    MOTOR_CMD_FORWARD,
    MOTOR_CMD_BACKWARD,
    MOTOR_CMD_BRAKE
} motor_cmd_direction_t;

// Motor selection values
typedef enum {
    MOTOR_CMD_LEFT = 0,
    MOTOR_CMD_RIGHT,
    MOTOR_CMD_BOTH
} motor_cmd_selection_t;

// Structure for motor control command
typedef struct {
    uint8_t command;            // CMD_MOTOR_CONTROL
    uint8_t motor;              // MOTOR_CMD_LEFT, MOTOR_CMD_RIGHT, or MOTOR_CMD_BOTH
    uint8_t direction;          // MOTOR_CMD_STOP, MOTOR_CMD_FORWARD, MOTOR_CMD_BACKWARD, or MOTOR_CMD_BRAKE
} motor_control_cmd_t;

// Structure for motor pin configuration command
typedef struct {
    uint8_t command;            // CMD_SET_MOTOR_PINS
    uint8_t m1p1;               // Left motor pin 1
    uint8_t m1p2;               // Left motor pin 2
    uint8_t m2p1;               // Right motor pin 1
    uint8_t m2p2;               // Right motor pin 2
} motor_pins_cmd_t;

// Structure for GPIO control command
typedef struct {
    uint8_t command;            // CMD_SET_GPIO
    uint8_t pin;                // GPIO pin number
    uint8_t state;              // 0 = LOW, 1 = HIGH
} gpio_control_cmd_t;

// Structure for GPIO state response
typedef struct {
    uint8_t command;            // CMD_GET_GPIO
    uint8_t pin;                // GPIO pin number
    uint8_t state;              // 0 = LOW, 1 = HIGH
} gpio_state_cmd_t;

#endif // COMMANDS_H
