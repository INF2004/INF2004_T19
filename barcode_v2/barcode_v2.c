#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

const uint8_t ADC_PIN = 0; // Use ADC 0
const uint16_t THRESHOLD = 1000; // ADC threshold value for black vs white
const uint32_t TIMER_INTERVAL = 100000; // Set the timer interval to 10,000us (10 ms)

char current_state = 'W'; // Use 'W' for WHITE, 'B' for BLACK
uint32_t start_time_black = 0; // Start time for the black state
uint32_t start_time_white = 0; // Start time for the white state
char previous_state = 'W'; // Previous state

// Threshold values for classifying line thickness
const uint32_t THIN_LINE_THRESHOLD = 1000000; // Adjust as needed

// Array to store barcode values
char barcode_values[32]; // Assuming a barcode can have up to 32 lines, adjust as needed
uint8_t barcode_index = 0; // Index to keep track of the current position in the array

char determine_State(uint16_t sensor_value) {
    if (sensor_value >= THRESHOLD) {
        // Above or equal threshold
        if (current_state == 'W') {
            // Change of state, black line detected
            printf("%u, BLACK LINE DETECTED\n", sensor_value);
            current_state = 'B';
            start_time_black = time_us_32(); // Record the start time for black state
            return 'B';
        }
    } else {
        // Below threshold
        if (current_state == 'B') {
            // Change of state, white line detected
            printf("%u, WHITE LINE DETECTED\n", sensor_value);
            current_state = 'W';
            start_time_white = time_us_32(); // Record the start time for white state
            return 'W';
        }
    }

    // If no state change, return the current state
    return current_state;
}
void convertAndPrintBarcode() {
    // Check if the barcode array is not empty
    if (strlen(barcode_values) > 0) {
        // Convert each 8-bit binary substring to an integer and then to ASCII
        printf("Converted ASCII: ");
        for (size_t i = 0; i < strlen(barcode_values); i += 8) {
            char binarySubstring[9];
            strncpy(binarySubstring, barcode_values + i, 8);
            binarySubstring[8] = '\0';

            int asciiValue = strtol(binarySubstring, NULL, 2);
            printf("%c", (char)asciiValue);
        }
        printf("\n");
    } else {
        printf("Barcode array is empty. No conversion needed.\n");
    }
}

void resetBarcodeArray() {
    barcode_values[0] = '\0'; // Place a null terminator at the beginning
    barcode_index = 0; // Reset the index
    printf("Barcode array has been emptied.\n");
    printf("The barcode value is %s\n", barcode_values); // Corrected printf statement
}




void classify_Line(uint16_t sensor_value, char state) {
    // Check if the barcode array is already full
    if (barcode_index >= sizeof(barcode_values) - 1) {
        printf("Barcode array is full. Skipping further processing.\n");
        return;
    }

    uint32_t end_time = time_us_32();
    uint32_t pulse_width = end_time - (state == 'B' ? start_time_white : start_time_black);

    printf("Time on %c: %u us\n", state, pulse_width);

    // Determine binary value based on line thickness
    char binary_value = '0';

    if (state == 'B') {
        if (pulse_width <= THIN_LINE_THRESHOLD) {
            printf("Thin Black Line Detected\n\n");
            printf("1");
            binary_value = '1';
        } else {
            printf("Thick Black Line Detected\n\n");
            printf("0");
            binary_value = '0';
        }
    } else {
        if (pulse_width <= THIN_LINE_THRESHOLD) {
            printf("Thin White Line Detected\n\n");
            printf("1");
            binary_value = '1';
        } else {
            printf("Thick White Line Detected\n\n");
            printf("0");
            binary_value = '0';
        }
    }

    // Store binary value in the barcode array
    barcode_values[barcode_index++] = binary_value;

    // Print the binary array after each line is scanned
    printf("Binary Array: %s\n", barcode_values);

    // Check if the barcode array is full after storing the current value
    if (barcode_index >= sizeof(barcode_values) - 1) {
        printf("Barcode array is full. Converting and resetting.\n");
        convertAndPrintBarcode();
        resetBarcodeArray();
    }
}

bool adc_callback(struct repeating_timer *t) {
    uint16_t sensor_value = adc_read();
    char determine_state = determine_State(sensor_value);

    if ( current_state != previous_state) {
        printf("Current state char %c\n", determine_state);
        printf("Previous state char %c\n", previous_state);

        // Call the classify_Line function to determine thickness
        classify_Line(sensor_value, determine_state);

        // Update the previous state
        previous_state = determine_state;
    }

    return true;
}

int main() {
    stdio_init_all();
    // Initialize ADC
    adc_init();
    adc_select_input(ADC_PIN);

    // Initialize and configure a hardware timer
    struct repeating_timer timer;
    add_repeating_timer_us(TIMER_INTERVAL, adc_callback, NULL, &timer);

    while (1);
    return 0;
}
