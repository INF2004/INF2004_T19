#include "FreeRTOS.h"

void motor_task(__unused void *params);
void receive_messages_from_tasks(void);
float calculate_pid(uint8_t pulses, uint8_t target_pulses);
void wrap_pwm(void);
void motorSetup(void);
void moveForward(void);
void moveBackward(void);
void tiltLeft(void);
void tiltRight(void);
void turnLeft(void);
void turnRight(void);
void stopMotors(void);
