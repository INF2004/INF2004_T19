/* 
    Develop an application using FreeRTOS that contains a task 
    that reads the temperature data from the RP2040's built-in temperature
    sensor and sends it to two tasks every 1 second. The second task will 
    perform a moving average on a buffer of ten data points, and the third task
    will perform a simple averaging. Additionally, create a fourth task exclusively
    for executing all the printf statements. No printf statements are allowed in
    any other task.
*/

#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE 60

// message ids to use in the print queue
#define SIMPLE_AVG_MSG_ID 0
#define MOVING_AVG_MSG_ID 1

#define MOVING_AVG_BUFF_SIZE 10

#define PRINT_QUEUE_MAX_ELEMENTS 10


// message buffer from temperature task to moving avg task
static MessageBufferHandle_t temp_to_moving_avg_msg_buff;
// message buffer from temperature task to simple avg task
static MessageBufferHandle_t temp_to_simple_avg_msg_buff;

QueueHandle_t print_queue;

// structure for use in print queue
typedef struct {
    uint8_t id;
    float temp_celsius;
} temp_msg_t;

float read_onboard_temperature() 
{
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V*/
    const float conversionFactor = 3.3f / (1 << 12);
    // map the adc_reading from 0 - 4095 to a voltage between 0 and 3.3
    float adc = (float) adc_read() * conversionFactor;
    // these "magic numbers" is specific to the sensor on the pico
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    return tempC;
}

/*
    continuously reads the onboard temperature sensor at regular intervals 
    and sends the temperature readings to two message buffers for further
    processing or use by other tasks in the system.
*/

void temp_task(__unused void *params)
{
    float temperature_celsius = 0;

    // initialise temp adc stuffs
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Temperature sensor is on input 4

    while (true) 
    {
        vTaskDelay(1000);
        temperature_celsius = read_onboard_temperature();

        // send temperature to simple_avg and moving_avg tasks
        xMessageBufferSend(
            temp_to_moving_avg_msg_buff,  // The message buffer to write to
            (void *)&temperature_celsius, // The source of data to send
            sizeof(temperature_celsius),  // The length of the data to send
            0                             // The block time; 0 = no block
        );

        xMessageBufferSend(
            temp_to_simple_avg_msg_buff,
            (void *)&temperature_celsius,
            sizeof(temperature_celsius),
            0
        );
    }
}

/*
    compute a moving average of temperature data received from temp_task
    and then sending the moving average value to a print queue
*/

void moving_avg_task(__unused void *params)
{
    // variables to receive from message buffer
    float fReceivedData;
    size_t xReceivedBytes;
    
    // static type to make the variables persist across runs.
    // data buffer will be set in a circular fashion with index, e.g.
    // 0,1,2...9,0,1,2...9
    static float data[10] = {0};
    static int index = 0;
    static int count = 0;

    float sum = 0;
    float moving_avg_celsius = 0;

    while (true)
    {
        // receive the message from temp_task
        xReceivedBytes = xMessageBufferReceive(
            temp_to_moving_avg_msg_buff,
            (void *) &fReceivedData,
            sizeof(fReceivedData),
            portMAX_DELAY
        );

        if (xReceivedBytes > 0) 
        {
            sum -= data[index];  
            data[index] = fReceivedData;
            sum += data[index];

            // index can be from 0 to MOVING_AVG_BUFF_SIZE - 1
            index = (index + 1) % MOVING_AVG_BUFF_SIZE;

            // once count reaches 10, it wont decrease, since it will store the last 10 data points.
            if (count < MOVING_AVG_BUFF_SIZE) 
            {   
                count++;
            }

            moving_avg_celsius = sum / count;

            temp_msg_t temp_msg_print;
            temp_msg_print.id = MOVING_AVG_MSG_ID;
            temp_msg_print.temp_celsius = moving_avg_celsius;

            xQueueSend(print_queue, &temp_msg_print, 0);
        }
    }
}

/*
    compute a simple average of temperature data received from temp_task 
    and then sending the simple average value to a print queue.
*/

void simple_avg_task(__unused void *params) 
{
    // variables to receive from message buffer
    float fReceivedData;
    size_t xReceivedBytes;

    // persist the variables across runs
    static float sum = 0;
    static uint32_t count = 0;

    float simple_average_celsius = 0;

    while (true) 
    {
        // receive the message from temp_task
        xReceivedBytes = xMessageBufferReceive(
            temp_to_simple_avg_msg_buff,
            (void *) &fReceivedData,
            sizeof(fReceivedData),
            portMAX_DELAY
        );
        
        if (xReceivedBytes > 0) 
        {
            sum += fReceivedData;
            count++;

            simple_average_celsius = sum / count;

            temp_msg_t temp_msg_print;
            temp_msg_print.id = SIMPLE_AVG_MSG_ID;
            temp_msg_print.temp_celsius = simple_average_celsius;

            xQueueSend(print_queue, &temp_msg_print, 0);
        }
    }
}

/*
    receive messages from the tasks and print the corresponding message format
*/

void print_task(__unused void *params) 
{
    while (true) 
    {
        temp_msg_t print_msg;

        // if message received fails, skip this iteration
        if (xQueueReceive(print_queue, &print_msg, 0) != pdPASS)
        {
            continue;
        }

        switch (print_msg.id)
        {
            case SIMPLE_AVG_MSG_ID:
                printf("Simple Average Temperature = %0.2f C\n", print_msg.temp_celsius);
                break;
            case MOVING_AVG_MSG_ID:
                printf("Moving Average Temperature = %0.2f C\n", print_msg.temp_celsius);
                break;
            default:
                printf("INVALID MESSAGE ID:: %d IN QUEUE\n", print_msg.id);
        }

        // it is important that the queue task processes messages
        // faster than both simple_avg and moving_avg tasks can send messages,
        // otherwise it will lose the data, so we have a faster delay.
        vTaskDelay(10);
    }
}

void vLaunch(void) 
{
    TaskHandle_t tempTask;
    TaskHandle_t movingAvgTask;
    TaskHandle_t simpleAvgTask;
    TaskHandle_t printTask;

    // create a print queue that can store max 10 temp_msg_t structs
    print_queue = xQueueCreate(PRINT_QUEUE_MAX_ELEMENTS, sizeof(temp_msg_t));

    xTaskCreate(temp_task, "TestTempThread", configMINIMAL_STACK_SIZE, NULL, 8, &tempTask);
    xTaskCreate(moving_avg_task, "TestMovingAvgThread", configMINIMAL_STACK_SIZE, NULL, 9, &movingAvgTask);
    xTaskCreate(simple_avg_task, "TestSimpleAvgThread", configMINIMAL_STACK_SIZE, NULL, 10, &simpleAvgTask);
    xTaskCreate(print_task, "TestPrintThread", configMINIMAL_STACK_SIZE, NULL, 11, &printTask);

    temp_to_moving_avg_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    temp_to_simple_avg_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    // (note we only do this in NO_SYS mode, because cyw43_arch_freertos
    // takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if ( portSUPPORT_SMP == 1 )
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if ( portSUPPORT_SMP == 1 ) && ( configNUM_CORES == 2 )
    printf("Starting %s on both cores:\n", rtos_name);
    vLaunch();
#elif ( RUN_FREERTOS_ON_CORE == 1 )
    printf("Starting %s on core 1:\n", rtos_name);
    multicore_launch_core1(vLaunch);
    while (true);
#else
    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
#endif
    return 0;
}
