#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"

#include "motor.h"

#define PWM_PIN_LEFT_ENABLED 18 
#define PWM_PIN_RIGHT_ENABLED 13 
#define PWM_FREQ 5000 
#define PWM_RANGE 255

#define MOTOR_LEFT_FORWARD 16   
#define MOTOR_LEFT_BACKWARD 17 
#define MOTOR_RIGHT_FORWARD 14  
#define MOTOR_RIGHT_BACKWARD 15

void motor_task(__unused void *params) 
{
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
        setMotorSpeed(1);
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);

        // Turn left
        turnLeft();
        setMotorSpeed(PWM_RANGE);
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);

        // Turn right
        turnRight();
        setMotorSpeed(PWM_RANGE);
        sleep_ms(2000);

        // Stop for 1 second
        stopMotors();
        sleep_ms(1000);
    }
}

void motorSetup() {
    // Tell GPIO 18 and 13 they are allocatd to the PWN
    gpio_set_function(PWM_PIN_LEFT_ENABLED, GPIO_OUT);
    gpio_set_function(PWM_PIN_RIGHT_ENABLED, GPIO_OUT);
    /* LEFT MOTOR CONFIG
    GPIO 16 */
    gpio_init(MOTOR_LEFT_FORWARD);
    gpio_set_dir(MOTOR_LEFT_FORWARD, GPIO_OUT);
    // GPIO 17
    gpio_init(MOTOR_LEFT_BACKWARD);
    gpio_set_dir(MOTOR_LEFT_BACKWARD, GPIO_OUT);
    // GPIO 18
    gpio_init(PWM_PIN_LEFT_ENABLED);
    gpio_set_dir(PWM_PIN_LEFT_ENABLED, GPIO_OUT);

    /* RIGHT MOTOR CONFIG
    GPIO 14*/
    gpio_init(MOTOR_RIGHT_FORWARD);
    gpio_set_dir(MOTOR_RIGHT_FORWARD, GPIO_OUT);

    // GPIO 15
    gpio_init(MOTOR_RIGHT_BACKWARD);
    gpio_set_dir(MOTOR_RIGHT_BACKWARD, GPIO_OUT);

    gpio_put(PWM_PIN_LEFT_ENABLED, 1);
    gpio_put(PWM_PIN_RIGHT_ENABLED, 1);
    pwm_set_enabled(MOTOR_LEFT_FORWARD, true);
    pwm_set_enabled(MOTOR_RIGHT_FORWARD, true);
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

void turnLeft() {
    gpio_put(MOTOR_LEFT_FORWARD, 0);
    gpio_put(MOTOR_LEFT_BACKWARD, 1);
    gpio_put(MOTOR_RIGHT_FORWARD, 1);
    gpio_put(MOTOR_RIGHT_BACKWARD, 0);
}

void turnRight() {
    gpio_put(MOTOR_LEFT_FORWARD, 1);
    gpio_put(MOTOR_LEFT_BACKWARD, 0);
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
    pwm_set_gpio_level(PWM_PIN_LEFT_ENABLED, speed);
    pwm_set_gpio_level(PWM_PIN_RIGHT_ENABLED, speed);
}
