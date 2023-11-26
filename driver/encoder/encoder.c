#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/time.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "driver/motor/motor.h"
#include "encoder.h"

#define ENCODER_PIN_A 0
#define ENCODER_PIN_B 1
#define ENCODER_PULSES_PER_REV 20

volatile uint8_t left_encoder_count = 0;
volatile uint8_t right_encoder_count = 0;
struct repeating_timer speed_timer;

extern MessageBufferHandle_t left_encoder_to_motor_msg_buff;
extern MessageBufferHandle_t right_encoder_to_motor_msg_buff;

void encoder_isr(uint gpio, uint32_t events)
{
    if ((gpio == ENCODER_PIN_A) && (events & GPIO_IRQ_EDGE_RISE))
    {
        left_encoder_count++;
    }
    if ((gpio == ENCODER_PIN_B) && (events & GPIO_IRQ_EDGE_RISE))
    {
        right_encoder_count++;
    }
}

// speed is calculated in notches per sec
// this method will callback every second, sending both left and right
// speeds

bool calculate_speed_callback(struct repeating_timer *t)
{
    //  send pulses to motor task
    xMessageBufferSend(
        left_encoder_to_motor_msg_buff, // The message buffer to write to
        (void *)&left_encoder_count,              // The source of data to send
        sizeof(left_encoder_count),               // The length of the data to send
        0                               // The block time; 0 = no block
    );
    //  send pulses to motor task
    xMessageBufferSend(
        right_encoder_to_motor_msg_buff, // The message buffer to write to
        (void *)&right_encoder_count,              // The source of data to send
        sizeof(right_encoder_count),               // The length of the data to send
        0                                // The block time; 0 = no block
    );
    // printf("encoder.c:: left:: %.2f right:: %.2f\n", left_rpm, right_rpm);
    left_encoder_count = 0;
    right_encoder_count = 0;
    return true;
}

void encoder_task(__unused void *params)
{
    // Initialize GPIO pins for encoder A and B
    gpio_init(ENCODER_PIN_A);
    gpio_set_dir(ENCODER_PIN_A, GPIO_IN);
    gpio_pull_up(ENCODER_PIN_A);

    gpio_init(ENCODER_PIN_B);
    gpio_set_dir(ENCODER_PIN_B, GPIO_IN);
    gpio_pull_up(ENCODER_PIN_B);

    // Set up interrupt handler for both A and B pins
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_RISE, true, &encoder_isr);
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_B, GPIO_IRQ_EDGE_RISE, true, &encoder_isr);

    add_repeating_timer_ms(100, calculate_speed_callback, NULL, &speed_timer);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
