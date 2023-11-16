#include "FreeRTOS.h"

uint64_t get_pulse_duration();
uint64_t calculate_cm_distance(uint64_t pulse_length);
uint64_t get_cm_distance();
void ultrasonic_task(__unused void *params); 