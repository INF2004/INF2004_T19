#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "message_buffer.h"

#include "maze.h"

#include "driver/ui/ui.h"
#include "driver/magnetometer/magnetometer.h"
#include "driver/motor/motor.h"
#include "driver/ultrasonic/ultrasonic.h"
#include "driver/encoder/encoder.h"
#include "driver/irline/ir.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE 60

void vLaunch(void)
{
    ultrasonic_to_motor_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    left_encoder_to_motor_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    right_encoder_to_motor_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    // magnetometer_to_motor_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    left_ir_to_motor_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    right_ir_to_motor_msg_buff = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    // TaskHandle_t uiTask;
    // TaskHandle_t magnetometerTask;
    TaskHandle_t motorTask;
    TaskHandle_t ultrasonicTask;
    TaskHandle_t encoderTask;
    TaskHandle_t irTask;

    xTaskCreate(motor_task, "TestMotorThread", 4096, NULL, TEST_TASK_PRIORITY, &motorTask);
    // xTaskCreate(ui_task, "TestUiThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &uiTask);
    xTaskCreate(ultrasonic_task, "TestUltrasonicThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &ultrasonicTask);
    // xTaskCreate(magnetometer_task, "TestMagnetometerThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &magnetometerTask);
    xTaskCreate(encoder_task, "TestEncoderThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &encoderTask);
    xTaskCreate(ir_task, "TestIRThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY , &irTask);

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
    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if ( portSUPPORT_SMP == 1 )
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if ( portSUPPORT_SMP == 1 ) && ( configNUM_CORES == 2 )
    vLaunch();
#elif ( RUN_FREERTOS_ON_CORE == 1 )
    multicore_launch_core1(vLaunch);
    while (true);
#else
    vLaunch();
#endif
    return 0;
}
