#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "motor.h"
#include "driver/ultrasonic/ultrasonic.h"

#define CLOCK_DIVIDER 250
#define PWM_WRAP 65535

#define MOTOR_CONTROL_N1 2
#define MOTOR_CONTROL_N2 3
#define MOTOR_CONTROL_N3 4
#define MOTOR_CONTROL_N4 5

#define MOTOR_PWM_RIGHT 6  
#define MOTOR_PWM_LEFT 7   

uint slice_num_left;
uint slice_num_right;
uint64_t pulse_len;
uint64_t cm_distance;

void motor_task(__unused void *params) 
{
    // stdio_init_all();
    motorSetup();
    while(1) {
        // moveForward();
        pulse_len = get_pulse_duration();
        cm_distance = calculate_cm_distance(pulse_len);
        // cm_distance = get_cm_distance(pulse_len);
        // if (cm_distance <= 5)
        // {
        //     stopMotors();
        // }
        printf("cm: %llu from motor.c\n", cm_distance);

        // stopMotors();
        // vTaskDelay(2000);

        // turnLeft();
        // vTaskDelay(2000);

        // stopMotors();
        // vTaskDelay(2000);

        // turnRight();
        // vTaskDelay(2000);

        // stopMotors();
        // vTaskDelay(2000);

        // moveBackward();
        // vTaskDelay(2000);

        // stopMotors();
        // vTaskDelay(2000);
    }
}

void motorSetup(void) {
    gpio_init(MOTOR_CONTROL_N1);
    gpio_init(MOTOR_CONTROL_N2);
    gpio_init(MOTOR_CONTROL_N3);
    gpio_init(MOTOR_CONTROL_N4);

    gpio_set_dir(MOTOR_CONTROL_N1, GPIO_OUT);
    gpio_set_dir(MOTOR_CONTROL_N2, GPIO_OUT);
    gpio_set_dir(MOTOR_CONTROL_N3, GPIO_OUT);
    gpio_set_dir(MOTOR_CONTROL_N4, GPIO_OUT);

    // configure PWM slice and set it running
    gpio_set_function(MOTOR_PWM_LEFT, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR_PWM_RIGHT, GPIO_FUNC_PWM);

    // helper function to get the pwm slice
    slice_num_left = pwm_gpio_to_slice_num(MOTOR_PWM_LEFT);
    slice_num_right = pwm_gpio_to_slice_num(MOTOR_PWM_RIGHT);

    // set the duty cycle 50%, we're using GP2 so it's channel a
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, PWM_WRAP);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, PWM_WRAP);

    // enable pwm
    pwm_set_enabled(slice_num_left, true);
    pwm_set_enabled(slice_num_right, true);
}

void moveForward() {
    gpio_put(MOTOR_CONTROL_N1, 0);
    gpio_put(MOTOR_CONTROL_N2, 1);
    gpio_put(MOTOR_CONTROL_N3, 1);
    gpio_put(MOTOR_CONTROL_N4, 0);
}

void moveBackward() {
    gpio_put(MOTOR_CONTROL_N1, 1);
    gpio_put(MOTOR_CONTROL_N2, 0);
    gpio_put(MOTOR_CONTROL_N3, 0);
    gpio_put(MOTOR_CONTROL_N4, 1);
}

void turnLeft() {
    gpio_put(MOTOR_CONTROL_N1, 1);
    gpio_put(MOTOR_CONTROL_N2, 0);
    gpio_put(MOTOR_CONTROL_N3, 1);
    gpio_put(MOTOR_CONTROL_N4, 0);
}

void turnRight() {
    gpio_put(MOTOR_CONTROL_N1, 1);
    gpio_put(MOTOR_CONTROL_N2, 1);
    gpio_put(MOTOR_CONTROL_N3, 0);
    gpio_put(MOTOR_CONTROL_N4, 0);
}

void stopMotors() {
    gpio_put(MOTOR_CONTROL_N1, 0);
    gpio_put(MOTOR_CONTROL_N2, 0);
    gpio_put(MOTOR_CONTROL_N3, 0);
    gpio_put(MOTOR_CONTROL_N4, 0);
}

