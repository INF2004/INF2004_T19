#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "driver/motor/motor.h"

#define LEFT_SENSOR_PIN 28
#define RIGHT_SENSOR_PIN 27

extern MessageBufferHandle_t left_ir_to_motor_msg_buff;
extern MessageBufferHandle_t right_ir_to_motor_msg_buff;

void ir_task(__unused void *params)
{
    adc_init();
    adc_gpio_init(LEFT_SENSOR_PIN);
    adc_gpio_init(RIGHT_SENSOR_PIN);

    stdio_init_all();

    while (1)
    {
        // it's faster to just get the state of the IR sensors in your task instead of message buffers
        // after initialising the gpios
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}