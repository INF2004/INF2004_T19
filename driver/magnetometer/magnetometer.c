#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "magnetometer.h"

void magnetometer_task(__unused void *params)
{
    while (true)
    {
        printf("MAGNETOMETER:: printing from magnetometer.c\n");
        vTaskDelay(1000);
    }
}