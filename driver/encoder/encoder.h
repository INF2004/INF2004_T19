#include "FreeRTOS.h"
#include "pico/time.h"

void encoder_isr(uint gpio, uint32_t events);
bool calculate_speed_callback(struct repeating_timer *t);
void encoder_task(__unused void *params);
