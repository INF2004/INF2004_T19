#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ultrasonic.h"
#include "driver/motor/motor.h"

int timeout = 26100;
uint64_t pulse_length = 0;
uint64_t distance = 0;

#define TRIGGER_PIN 16
#define ECHO_PIN 17

// Function to measure the duration of the echo pulse
uint64_t get_pulse_duration() {

    gpio_put(TRIGGER_PIN, 0);   // Ensure the trigger pin is set to low first
    gpio_put(TRIGGER_PIN, 1);  // Set the trigger pin to high
    sleep_us(10);               // Generate a 10 microsecond pulse which is the minimum
    gpio_put(TRIGGER_PIN, 0);   // Set the trigger pin to low

    uint64_t width = 0;

    // Wait for the rising edge of the echo pulse
    while (gpio_get(ECHO_PIN) == 0) tight_loop_contents();
    absolute_time_t start_time = get_absolute_time(); // Record the start time

    // Wait for the falling edge of the echo pulsWe
    while (gpio_get(ECHO_PIN) == 1) {
        // Increment to measure the microsecond resolution
        width++;
        sleep_us(1);
        if (width > timeout) {
            break;  // Break out of the loop if timeout happens
        }
    }
    absolute_time_t end_time = get_absolute_time(); // Record the end time

    return absolute_time_diff_us(start_time, end_time); // Calculate the duration
}

// Convert pulse length to cm
uint64_t calculate_cm_distance(uint64_t pulse_length) {

    return pulse_length / 29 / 2;
}

void ultrasonic_task() {
    stdio_init_all();

    // Setup GPIO pins for trigger and echo
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    while (1) {
        pulse_length = get_pulse_duration();
        distance = calculate_cm_distance(pulse_length);
        ultrasonic_to_motor(distance);
        printf("in ultrasonic task\n");
    }
}
