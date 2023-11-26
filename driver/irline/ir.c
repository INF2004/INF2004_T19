#include <stdio.h>

#include "pico/stdlib.h"

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
    gpio_init(LEFT_SENSOR_PIN);
    gpio_set_dir(LEFT_SENSOR_PIN, GPIO_IN);

    gpio_init(RIGHT_SENSOR_PIN);
    gpio_set_dir(RIGHT_SENSOR_PIN, GPIO_IN);

    stdio_init_all();

    while (1)
    {
        // Read digital sensor values
        uint16_t left_sensor_value = gpio_get(LEFT_SENSOR_PIN);
        uint16_t right_sensor_value = gpio_get(RIGHT_SENSOR_PIN);

        // printf("Left Sensor: %d, Right Sensor: %d\n", left_sensor_value, right_sensor_value);

        xMessageBufferSend(
            left_ir_to_motor_msg_buff, 
            (void *)&left_sensor_value,               
            sizeof(left_sensor_value),
            0
        );

        xMessageBufferSend(
            right_ir_to_motor_msg_buff, 
            (void *)&right_sensor_value,
            sizeof(right_sensor_value),
            0
        );
        
        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}