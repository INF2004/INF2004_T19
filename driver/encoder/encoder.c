#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <math.h>
// #include "../motor/motor.h"

#include "FreeRTOS.h"
#include "task.h"

#include "encoder.h"

static absolute_time_t prev_time[2]; // Array for two sensors
static char event_str[2][128];       // Array for two sensors
static int notch_count[2] = {0, 0};  // Array for two sensors
static float wheel_diameter = 0.065; // 65mm wheel diameter in meters

void encoder_task(__unused void *params) {
    stdio_init_all();

    // motorSetup();

    // gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_LEVEL_LOW, true, &gpio_callback);

    printf("Hello GPIO IRQ\n");

    // Set up GPIO interrupt for the first encoder (GPIO0)
    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Wait forever
    while (true)
    {
        printf("encoder thread:: \n");
    }
}

void gpio_event_string(char *buf, uint32_t events);

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}