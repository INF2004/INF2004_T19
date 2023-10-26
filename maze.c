#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "driver/ui/ui.h"
#include "driver/magnetometer/magnetometer.h"
#include "driver/motor/motor.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE 60


void vLaunch(void) 
{
    TaskHandle_t uiTask;
    TaskHandle_t magnetometerTask;
    TaskHandle_t motorTask;

    xTaskCreate(ui_task, "TestUiThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &uiTask);
    xTaskCreate(motor_task, "TestMotorThread", configMINIMAL_STACK_SIZE, NULL, 9, &motorTask);
    xTaskCreate(magnetometer_task, "TestMagnetometerThread", configMINIMAL_STACK_SIZE, NULL, 8, &magnetometerTask);

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
