#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ultrasonic.h"

int timeout = 26100;

#define TRIGGER_PIN 0
#define ECHO_PIN 1

// Function to measure the duration of the echo pulse
uint64_t get_pulse_duration() {

    gpio_put(TRIGGER_PIN, 1);  // Set the trigger pin to high
    sleep_us(10);               // Generate a 10 microsecond pulse which is the minimum
    gpio_put(TRIGGER_PIN, 0);   // Set the trigger pin to low

    uint64_t width = 0;

    // Wait for the rising edge of the echo pulse
    while (gpio_get(ECHO_PIN) == 0) tight_loop_contents();
    absolute_time_t start_time = get_absolute_time(); // Record the start time

    // Wait for the falling edge of the echo pulse
    while (gpio_get(ECHO_PIN) == 1) {
        // Increment to measure the microsecond resolution
        width++;
        sleep_us(1);
        if (width > timeout) return 0;
    }
    absolute_time_t end_time = get_absolute_time(); // Record the end time

    return absolute_time_diff_us(start_time, end_time); // Calculate the duration
}

// Convert pulse length to inches
uint64_t calculate_inch_distance(uint64_t pulse_length) {

    return (long)pulse_length / 74.f / 2.f;
}

void ultrasonic_task() {
    stdio_init_all();

    // Setup GPIO pins for trigger and echo
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    while (1) {
        uint64_t pulse_length = get_pulse_duration();
        uint64_t distance = calculate_inch_distance(pulse_length);
        printf("Distance: %llu inch\n", distance);
        sleep_ms(1000);  // Delay between measurements

        if (distance < 5) {

            printf("Obstacle detected!\n");
            // Car to stop and move backwards

        }

    }
}
