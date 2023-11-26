#include "FreeRTOS.h"
#include "message_buffer.h"

uint64_t get_distance();
uint64_t get_pulse_duration();
uint64_t calculate_cm_distance(uint64_t pulse_length);
void ultrasonic_task(__unused void *params); 