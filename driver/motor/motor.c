#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "message_buffer.h"

#include "driver/ultrasonic/ultrasonic.h"
#include "motor.h"


#define CLOCK_DIVIDER 250
#define PWM_WRAP 65535

#define MOTOR_CONTROL_N1 2
#define MOTOR_CONTROL_N2 3
#define MOTOR_CONTROL_N3 4
#define MOTOR_CONTROL_N4 5

#define MOTOR_PWM_RIGHT 6
#define MOTOR_PWM_LEFT 7

#define TARGET_NOTCHES_PER_100_MILLIS 3
#define ULTRASONIC_MIN_DISTANCE_STOP 30
#define ULTRASONIC_MIN_DISTANCE_MOVE 40

#define SLIGHT_TILT_PWM 5000

bool move_backwards = false;

uint32_t pwm_left = PWM_WRAP;
uint32_t pwm_right = PWM_WRAP;
uint slice_num_left;
uint slice_num_right;

// pid globals
absolute_time_t currT;
absolute_time_t prevT;
float eintegral = 0;
float prevError = 0;

float pid_left, pid_right;

extern MessageBufferHandle_t ultrasonic_to_motor_msg_buff;
extern MessageBufferHandle_t left_encoder_to_motor_msg_buff;
extern MessageBufferHandle_t right_encoder_to_motor_msg_buff;
// extern MessageBufferHandle_t magnetometer_to_motor_msg_buff;
extern MessageBufferHandle_t left_ir_to_motor_msg_buff;
extern MessageBufferHandle_t right_ir_to_motor_msg_buff;

uint64_t ultrasonic_dist = 0;
uint8_t left_encoder_pulses, right_encoder_pulses;
uint16_t left_ir, right_ir;
float compass;

void motor_task(__unused void *params) 
{
    stdio_init_all();

    motorSetup();

    prevT = get_absolute_time();

    moveForward();

    vTaskDelay(1000);

    while (1)
    {
        receive_messages_from_tasks();

        if (ultrasonic_dist >= ULTRASONIC_MIN_DISTANCE_MOVE)
        {
            moveForward();
            move_backwards = false;
        }

        if (ultrasonic_dist <= ULTRASONIC_MIN_DISTANCE_STOP)
        {
            move_backwards = true;
            stopMotors();
        }

        if (move_backwards && (ultrasonic_dist < ULTRASONIC_MIN_DISTANCE_MOVE))
        {
            moveBackward();
            pwm_right = PWM_WRAP;
            pwm_left = PWM_WRAP;
        }

        currT = get_absolute_time();

        if (!move_backwards)
        {
            // can return a positive or negative value
            pid_left = calculate_pid(left_encoder_pulses, TARGET_NOTCHES_PER_100_MILLIS);
            // signed value add a signed value
            pwm_left += pid_left; 
            // can return a positive or negative value
            pid_right = calculate_pid(right_encoder_pulses, TARGET_NOTCHES_PER_100_MILLIS);
            // signed value add a signed value
            pwm_right += pid_right;
        }

        // if (left_ir && right_ir)
        // {
        //     // turn left?
        // }

        if ((left_ir == 1) && (right_ir == 0)) // detected on the right
        {
            tiltLeft();
        }

        if ((left_ir == 0) && (right_ir == 1)) // detected on the left
        {
            tiltRight();
        }

        wrap_pwm();

        prevT = currT;

        pwm_set_chan_level(slice_num_left, PWM_CHAN_A, pwm_right); // probably a wire issue causing pwm to swap?
        pwm_set_chan_level(slice_num_right, PWM_CHAN_B, pwm_left); // probably a wire issue causing pwm to swap?

        printf(
            "motor.c:: left_pulses:: %d pwm_left:: %d pid_left:: %.2f right_pulses:: %d pwm_right:: %d pid_right:: %.2f\n",
            left_encoder_pulses, pwm_left, pid_left, right_encoder_pulses, pwm_right, pid_right);

        printf("motor.c:: ultrasonic dist:: %llu\n", ultrasonic_dist);
        printf("motor.c:: left_ir:: %d, right_ir:: %d\n", left_ir, right_ir);

        // vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void receive_messages_from_tasks(void)
{
    xMessageBufferReceive(
        ultrasonic_to_motor_msg_buff,
        (void *)&ultrasonic_dist,
        sizeof(ultrasonic_dist),
        portMAX_DELAY);

    xMessageBufferReceive(
        left_encoder_to_motor_msg_buff,
        (void *)&left_encoder_pulses,
        sizeof(left_encoder_pulses),
        portMAX_DELAY);

    xMessageBufferReceive(
        right_encoder_to_motor_msg_buff,
        (void *)&right_encoder_pulses,
        sizeof(right_encoder_pulses),
        portMAX_DELAY);

//    xMessageBufferReceive(
//         magnetometer_to_motor_msg_buff,
//         (void *)&compass,
//         sizeof(compass),
//         portMAX_DELAY);

    xMessageBufferReceive(
        left_ir_to_motor_msg_buff,
        (void *)&left_ir,
        sizeof(left_ir),
        portMAX_DELAY);

    xMessageBufferReceive(
        right_ir_to_motor_msg_buff,
        (void *)&right_ir,
        sizeof(right_ir),
        portMAX_DELAY);
}

float calculate_pid(uint8_t pulses, uint8_t target_pulses)
{
    // deltaT will usually be 1
    int64_t deltaT = absolute_time_diff_us(prevT, currT) / 1000000;

    // Compute the control signal u
    float kp = 5000; // 2500 so far
    float ki = 0;
    float kd = 0;

    prevError = 0;

    // assuming target_nps is 30, nps is 35, e will be -5
    float err = target_pulses - pulses;

    // eintegral = eintegral + err * deltaT;

    // float derivative = kd * (err - prevError);

    // float u = (kp * err) + (ki * eintegral) + (derivative);
    float u = kp * err;

    // prevError = err;

    return u;
}

void wrap_pwm(void)
{
    if (pwm_left > PWM_WRAP)
    {
        pwm_left = PWM_WRAP;
    }

    if (pwm_right > PWM_WRAP)
    {
        pwm_right = PWM_WRAP;
    }

    if (pwm_left < 0)
    {
        pwm_left = 0;
    }

    if (pwm_right < 0)
    {
        pwm_right = 0;
    }
}

void tiltLeft(void)
{
    pwm_right += SLIGHT_TILT_PWM;
    pwm_left -= SLIGHT_TILT_PWM;
}

void tiltRight(void)
{
    pwm_left += SLIGHT_TILT_PWM;
    pwm_right -= SLIGHT_TILT_PWM;
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
    // printf("moving forward::: \n");
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

