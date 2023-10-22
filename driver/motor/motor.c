#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

// Define GPIO pins for motor control
#define LEFT_MOTOR_FORWARD 0
#define LEFT_MOTOR_BACKWARD 1
#define RIGHT_MOTOR_FORWARD 2
#define RIGHT_MOTOR_BACKWARD 3

void initializeMotors() {
    gpio_init(LEFT_MOTOR_FORWARD);
    gpio_init(LEFT_MOTOR_BACKWARD);
    gpio_init(RIGHT_MOTOR_FORWARD);
    gpio_init(RIGHT_MOTOR_BACKWARD);

    gpio_set_dir(LEFT_MOTOR_FORWARD, GPIO_OUT);
    gpio_set_dir(LEFT_MOTOR_BACKWARD, GPIO_OUT);
    gpio_set_dir(RIGHT_MOTOR_FORWARD, GPIO_OUT);
    gpio_set_dir(RIGHT_MOTOR_BACKWARD, GPIO_OUT);
}

void moveForward() {
    gpio_put(LEFT_MOTOR_FORWARD, 1);
    gpio_put(LEFT_MOTOR_BACKWARD, 0);
    gpio_put(RIGHT_MOTOR_FORWARD, 1);
    gpio_put(RIGHT_MOTOR_BACKWARD, 0);
}

void moveBackward() {
    gpio_put(LEFT_MOTOR_FORWARD, 0);
    gpio_put(LEFT_MOTOR_BACKWARD, 1);
    gpio_put(RIGHT_MOTOR_FORWARD, 0);
    gpio_put(RIGHT_MOTOR_BACKWARD, 1);
}

void stopMotors() {
    gpio_put(LEFT_MOTOR_FORWARD, 0);
    gpio_put(LEFT_MOTOR_BACKWARD, 0);
    gpio_put(RIGHT_MOTOR_FORWARD, 0);
    gpio_put(RIGHT_MOTOR_BACKWARD, 0);
}

void turnLeft() {
    gpio_put(LEFT_MOTOR_FORWARD, 0);
    gpio_put(LEFT_MOTOR_BACKWARD, 1);
    gpio_put(RIGHT_MOTOR_FORWARD, 1);
    gpio_put(RIGHT_MOTOR_BACKWARD, 0);
}

void turnRight() {
    gpio_put(LEFT_MOTOR_FORWARD, 1);
    gpio_put(LEFT_MOTOR_BACKWARD, 0);
    gpio_put(RIGHT_MOTOR_FORWARD, 0);
    gpio_put(RIGHT_MOTOR_BACKWARD, 1);
}

int main() {
    stdio_init_all();

    initializeMotors();

    while (1) {
        // Move forward
        moveForward();
        sleep_ms(2000);

        // Move backward
        moveBackward();
        sleep_ms(2000);

        // Stop
        stopMotors();
        sleep_ms(1000);

        // Turn left
        turnLeft();
        sleep_ms(1000);

        // Turn right
        turnRight();
        sleep_ms(1000);
    }

    return 0;
}
