#include "FreeRTOS.h"

void motor_task(__unused void *params);
void motorSetup(void);
void moveForward(void);
void moveBackward(void);
void stopMotors(void);
void setMotorSpeed(int speed);
