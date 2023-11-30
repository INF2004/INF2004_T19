#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "FreeRTOS.h"
#include "task.h"
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

#define LEFT_IR_PIN 28
#define RIGHT_IR_PIN 27

#define TARGET_NOTCHES_PER_100_MILLIS 3
#define ULTRASONIC_MIN_DISTANCE_STOP 30
#define ULTRASONIC_MIN_DISTANCE_MOVE 40

#define MIN_PWM_LEVEL 48000
#define BLACK_LINE_ADC_VALUE 1500

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

float pid_left, pid_right, pid;

extern MessageBufferHandle_t ultrasonic_to_motor_msg_buff;
extern MessageBufferHandle_t left_encoder_to_motor_msg_buff;
extern MessageBufferHandle_t right_encoder_to_motor_msg_buff;
extern MessageBufferHandle_t magnetometer_to_motor_msg_buff;

uint64_t ultrasonic_dist = 0;
uint8_t left_encoder_pulses, right_encoder_pulses;
uint16_t left_ir, right_ir;
float compass;

bool turn_left = false;

void motor_task(__unused void *params) 
{
    stdio_init_all();

    motorSetup();

    prevT = get_absolute_time();

    while (1)
    {
        receive_messages_from_tasks();

        if (ultrasonic_dist >= ULTRASONIC_MIN_DISTANCE_MOVE)
        {
            move_backwards = false;
            moveForward();
            pwm_set_chan_level(slice_num_left, PWM_CHAN_A, PWM_WRAP);
            pwm_set_chan_level(slice_num_right, PWM_CHAN_B, PWM_WRAP);
        }


        if (ultrasonic_dist < ULTRASONIC_MIN_DISTANCE_STOP)
        {
            moveBackward();
            pwm_set_chan_level(slice_num_left, PWM_CHAN_A, PWM_WRAP); 
            pwm_set_chan_level(slice_num_right, PWM_CHAN_B, PWM_WRAP);
        }

        currT = get_absolute_time();
        
        // it's faster to just get the state of the IR sensors here instead of passing message buffers
        adc_select_input(2);
        left_ir = adc_read();
        adc_select_input(1);
        right_ir = adc_read();

        // black line detected means value > 1500
        if ((left_ir > BLACK_LINE_ADC_VALUE) && (right_ir < BLACK_LINE_ADC_VALUE)) // detected on the left
        {
            tiltRight();
            pwm_set_chan_level(slice_num_left, PWM_CHAN_A, pwm_right);
            pwm_set_chan_level(slice_num_right, PWM_CHAN_B, pwm_left);
        }

        if ((left_ir < BLACK_LINE_ADC_VALUE) && (right_ir > BLACK_LINE_ADC_VALUE)) // detected on the right
        {
            tiltLeft();
            pwm_set_chan_level(slice_num_left, PWM_CHAN_A, pwm_right);
            pwm_set_chan_level(slice_num_right, PWM_CHAN_B, pwm_left);
        }

        if ((left_ir > BLACK_LINE_ADC_VALUE) && (right_ir > BLACK_LINE_ADC_VALUE)) // when both detected, stop and turn left
        {
            stopMotors();
            turn_left = true;
        }

        if (turn_left)
        {
            uint32_t time_start = to_ms_since_boot(get_absolute_time());
            while (turn_left == true)
            {
                uint32_t time_end = to_ms_since_boot(get_absolute_time());
                if ((time_end - time_start) < 800)
                {
                    turnLeft();
                    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, 0);
                    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, MIN_PWM_LEVEL);
                }
                else
                {
                    turn_left = false;
                    moveForward();
                    pwm_set_chan_level(slice_num_right, PWM_CHAN_B, MIN_PWM_LEVEL);
                    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, MIN_PWM_LEVEL);
                }
            }
        }

        if ((left_ir < BLACK_LINE_ADC_VALUE) && (right_ir < BLACK_LINE_ADC_VALUE)) // when nothing is detected
        {
            // can return a positive or negative value
            pid_left = calculate_pid(left_encoder_pulses, TARGET_NOTCHES_PER_100_MILLIS);
            // signed value add a signed value
            pwm_left = MIN_PWM_LEVEL + pid_left;
            // can return a positive or negative value
            pid_right = calculate_pid(right_encoder_pulses, TARGET_NOTCHES_PER_100_MILLIS);
            // signed value add a signed value
            pwm_right = MIN_PWM_LEVEL +  pid_right;
            moveForward();
            pwm_set_chan_level(slice_num_left, PWM_CHAN_A, pwm_right); // probably a wire issue causing pwm to swap?
            pwm_set_chan_level(slice_num_right, PWM_CHAN_B, pwm_left); // probably a wire issue causing pwm to swap?
        }

        wrap_pwm();

        prevT = currT;

        // print debug messages
        printf(
            "motor.c:: left_pulses:: %d pwm_left:: %d pid_left:: %.2f right_pulses:: %d pwm_right:: %d pid_right:: %.2f\n",
            left_encoder_pulses, pwm_left, pid_left, right_encoder_pulses, pwm_right, pid_right);

        printf("motor.c:: ultrasonic dist:: %llu\n", ultrasonic_dist);
        printf("motor.c:: left_ir:: %d, right_ir:: %d\n", left_ir, right_ir);

        vTaskDelay(pdMS_TO_TICKS(20));
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

   xMessageBufferReceive(
        magnetometer_to_motor_msg_buff,
        (void *)&compass,
        sizeof(compass),
        portMAX_DELAY);
}

float calculate_pid(uint8_t pulses, uint8_t target_pulses)
{
    // deltaT will usually be 1
    int64_t deltaT = absolute_time_diff_us(prevT, currT) / 1000000;

    // Compute the control signal u, with 2500 as ultimate gain
    float kp = 1500; // 2500 * 0.6
    float ki = 600; // (1.2 * 2500) / 5
    float kd = 187.5; // 0.075 * 2500

    prevError = 0;

    // assuming target_pulse is 4, pulses is 3, e will be 1
    float err = target_pulses - pulses;

    eintegral = eintegral + err * deltaT;

    float derivative = kd * (err - prevError);

    float u = (kp * err) + (ki * eintegral) + (derivative);

    prevError = err;

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

// although both tilt left and tilt right is set to turn a single motor,
// it will actually "bunny hop" on the black line.
void tiltLeft(void)
{
    pwm_right = MIN_PWM_LEVEL;
    pwm_left = 0;
}

void tiltRight(void)
{
    pwm_left = MIN_PWM_LEVEL;
    pwm_right = 0;
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

    // set the pwm level to maximum temporarily
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

