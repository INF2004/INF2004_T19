#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"

#include "motor.h"

#define CLOCK_DIVIDER 250
#define PWM_WRAP 25000

#define MOTOR_LEFT_FORWARD 16   
#define MOTOR_RIGHT_FORWARD 17  

void motor_task(__unused void *params) 
{
    motorSetup();
    while (1) {
        // Move forward at full speed for 2 seconds
        moveForward();
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);

        // Move backward at half speed for 2 seconds
        moveBackward();
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);

        // Turn left
        turnLeft();
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);

        // Turn right
        turnRight();
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);
    }
}

void motorSetup() {
    // configure PWM slice and set it running
    gpio_set_function(MOTOR_LEFT_FORWARD, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR_RIGHT_FORWARD, GPIO_FUNC_PWM);

    // helper function to get the pwm slice
    uint slice_num_left = pwm_gpio_to_slice_num(MOTOR_LEFT_FORWARD);
    uint slice_num_right = pwm_gpio_to_slice_num(MOTOR_RIGHT_FORWARD);

    // set the clock division
    pwm_set_clkdiv(slice_num_left, CLOCK_DIVIDER);
    pwm_set_clkdiv(slice_num_right, CLOCK_DIVIDER);

    // set the pwm top to reset
    pwm_set_wrap(slice_num_left, PWM_WRAP);
    pwm_set_wrap(slice_num_right, PWM_WRAP);

    // set the duty cycle 50%, we're using GP2 so it's channel a
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, PWM_WRAP/2);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, PWM_WRAP/2);

    // enable pwm
    pwm_set_enabled(slice_num_left, true);
    pwm_set_enabled(slice_num_right, true);
}

void moveForward() {
    gpio_put(MOTOR_LEFT_FORWARD, 1);
    gpio_put(MOTOR_RIGHT_FORWARD, 1);
}

void moveBackward() {
    gpio_put(MOTOR_LEFT_FORWARD, 0);
    gpio_put(MOTOR_RIGHT_FORWARD, 0);
}

void turnLeft() {
    gpio_put(MOTOR_LEFT_FORWARD, 1);
    gpio_put(MOTOR_RIGHT_FORWARD, 0);
}

void turnRight() {
    gpio_put(MOTOR_LEFT_FORWARD, 0);
    gpio_put(MOTOR_RIGHT_FORWARD, 1);
}

void stopMotors() {
    gpio_put(MOTOR_LEFT_FORWARD, 0);
    gpio_put(MOTOR_RIGHT_FORWARD, 0);
}

