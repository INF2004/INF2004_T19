#include "FreeRTOS.h"

void motor_task(__unused void *params);
void ultrasonic_to_motor(uint64_t distance);
void motorSetup(void);
void moveForward(void);
void moveBackward(void);
void turnLeft(void);
void turnRight(void);
void stopMotors(void);
