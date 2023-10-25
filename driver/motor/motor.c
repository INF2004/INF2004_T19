#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"

#include "motor.h"

#define PWM_PIN_LEFT 0 
#define PWM_PIN_RIGHT 1 
#define PWM_FREQ 5000 
#define PWM_RANGE 255

#define MOTOR_LEFT_FORWARD 2   // Example GPIO for left motor forward
#define MOTOR_LEFT_BACKWARD 3  // Example GPIO for left motor backward
#define MOTOR_RIGHT_FORWARD 4  // Example GPIO for right motor forward
#define MOTOR_RIGHT_BACKWARD 5 // Example GPIO for right motor backward

void motor_task(__unused void *params) 
{
    stdio_init_all();
    motorSetup();
    while (1) {
        // Move forward at full speed for 2 seconds
        moveForward();
        setMotorSpeed(PWM_RANGE);
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);

        // Move backward at half speed for 2 seconds
        moveBackward();
        setMotorSpeed(PWM_RANGE / 2);
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);
    }
}

void motorSetup() {
    gpio_set_function(PWM_PIN_LEFT, GPIO_FUNC_PWM);
    gpio_set_function(PWM_PIN_RIGHT, GPIO_FUNC_PWM);

    uint slice_left = pwm_gpio_to_slice_num(PWM_PIN_LEFT);
    uint slice_right = pwm_gpio_to_slice_num(PWM_PIN_RIGHT);

    pwm_set_clkdiv(slice_left, 2);
    pwm_set_clkdiv(slice_right, 2);

    pwm_set_wrap(slice_left, PWM_RANGE); 
    pwm_set_wrap(slice_right, PWM_RANGE);

    pwm_set_enabled(slice_left, true);
    pwm_set_enabled(slice_right, true);

    gpio_init(MOTOR_LEFT_FORWARD);
    gpio_set_dir(MOTOR_LEFT_FORWARD, GPIO_OUT);

    gpio_init(MOTOR_LEFT_BACKWARD);
    gpio_set_dir(MOTOR_LEFT_BACKWARD, GPIO_OUT);

    gpio_init(MOTOR_RIGHT_FORWARD);
    gpio_set_dir(MOTOR_RIGHT_FORWARD, GPIO_OUT);

    gpio_init(MOTOR_RIGHT_BACKWARD);
    gpio_set_dir(MOTOR_RIGHT_BACKWARD, GPIO_OUT);
}

void moveForward() {
    gpio_put(MOTOR_LEFT_FORWARD, 1);
    gpio_put(MOTOR_LEFT_BACKWARD, 0);
    gpio_put(MOTOR_RIGHT_FORWARD, 1);
    gpio_put(MOTOR_RIGHT_BACKWARD, 0);
}

void moveBackward() {
    gpio_put(MOTOR_LEFT_FORWARD, 0);
    gpio_put(MOTOR_LEFT_BACKWARD, 1);
    gpio_put(MOTOR_RIGHT_FORWARD, 0);
    gpio_put(MOTOR_RIGHT_BACKWARD, 1);
}

void stopMotors() {
    gpio_put(MOTOR_LEFT_FORWARD, 0);
    gpio_put(MOTOR_LEFT_BACKWARD, 0);
    gpio_put(MOTOR_RIGHT_FORWARD, 0);
    gpio_put(MOTOR_RIGHT_BACKWARD, 0);
}

void setMotorSpeed(int speed) {
    pwm_set_gpio_level(PWM_PIN_LEFT, speed);
    pwm_set_gpio_level(PWM_PIN_RIGHT, speed);
}
