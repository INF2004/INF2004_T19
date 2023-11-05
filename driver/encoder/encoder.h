#include "FreeRTOS.h"

void encoder_task(__unused void *params);
void gpio_event_string(char *buf, uint32_t events);
void gpio_callback(uint gpio, uint32_t events);
void gpio_callback_1(uint gpio, uint32_t events);
void gpio_callback_2(uint gpio, uint32_t events);
void gpio_event_string(char *buf, uint32_t events);