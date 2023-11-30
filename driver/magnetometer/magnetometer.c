#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "magnetometer.h"

#define I2C_PORT i2c0
#define I2C_DATA_RATE 400000
#define MAGNETOMETER_SCL_PIN 5
#define MAGNETOMETER_SDA_PIN 4

#define PI 3.14159265358979323846;

uint8_t ACCEL_ADDRESS = 0x19; // 0011001b
uint8_t MAG_ADDRESS = 0x1E;   // 0011110b

extern MessageBufferHandle_t magnetometer_to_motor_msg_buff;

void magnetometer_task(__unused void *params)
{
    magnetometer_init();

    vTaskDelay(1000);

    while (true)
    {
        // accel_t accel = accelerometer_read();

        float compass = compass_read_degrees();

        xMessageBufferSend(
            magnetometer_to_motor_msg_buff, // The message buffer to write to
            (void *)&compass,    // The source of data to send
            sizeof(compass),     // The length of the data to send
            0                    // The block time; 0 = no block
        );

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void magnetometer_init(void)
{
    i2c_init(I2C_PORT, I2C_DATA_RATE);

    gpio_set_function(MAGNETOMETER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAGNETOMETER_SDA_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(MAGNETOMETER_SCL_PIN);
    gpio_pull_up(MAGNETOMETER_SDA_PIN);

    accelerometer_init();
    compass_init();
}

void accelerometer_init(void)
{
    uint8_t config[2];

    uint8_t CTRL_REG1_A = 0x20;
    uint8_t ACCEL_ENABLE = 0x57; // enables 100 Hz data rate, low power off, enable x,y,z axes
    config[0] = CTRL_REG1_A;
    config[1] = ACCEL_ENABLE;

    // write to enable accelerometer
    i2c_write_blocking(I2C_PORT, ACCEL_ADDRESS, config, sizeof(config), true);

    uint8_t ACC_CTRL_REG4_A = 0x23;
    uint8_t FULL_SCALE = 0x00; // +/- 2g scale
    config[0] = ACC_CTRL_REG4_A;
    config[1] = FULL_SCALE;

    i2c_write_blocking(I2C_PORT, ACCEL_ADDRESS, config, sizeof(config), true);
}

accel_t accelerometer_read(void) 
{
    uint8_t xla, xha, yla, yha, zla, zha;
    int16_t raw_xa, raw_ya, raw_za;

    // Set the accelerometer register address
    // Set the most significant bit to 1 by writing 0x80 for multi-byte (6 bytes) read
    // i2c_read_blocking must block, otherwise magnetometer *will* freeze

    uint8_t ACC_OUT_X_L_A = 0x28 | 0x80;
    uint8_t accel_data[6] = {0};

    i2c_write_blocking(I2C_PORT, ACCEL_ADDRESS, &ACC_OUT_X_L_A, sizeof(ACC_OUT_X_L_A), true);

    i2c_read_blocking(I2C_PORT, ACCEL_ADDRESS, accel_data, sizeof(accel_data), true);

    xla = accel_data[0]; // x axis low bits
    xha = accel_data[1]; // x axis high bits
    yla = accel_data[2]; // y axis low bits
    yha = accel_data[3]; // y axis high bits
    zla = accel_data[4]; // z axis low bits
    zha = accel_data[5]; // z axis high bits

    // raw values are in cm/s^2 units
    raw_xa = (int16_t)(xla | (xha << 8)) >> 4;
    raw_ya = (int16_t)(yla | (yha << 8)) >> 4;
    raw_za = (int16_t)(zla | (zha << 8)) >> 4;

    // printf("raw_xa:: %d, raw_ya:: %d, raw_za:: %d\n", raw_xa, raw_ya, raw_za);

    accel_t data = {
        .raw_xa = raw_xa,
        .raw_ya = raw_ya,
        .raw_za = raw_za
    };

    return data;
}

void compass_init(void)
{
    uint8_t config[2];

    uint8_t MAG_MR_REG_M = 0x02;
    uint8_t CONTINUOUS_CONVERSION = 0x00;
    config[0] = MAG_MR_REG_M;
    config[1] = CONTINUOUS_CONVERSION;

    // write to enable compass
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, config, sizeof(config), true);

    uint8_t CRA_REG_M = 0x00;
    uint8_t DATA_RATE_M = 0x1C; // 15Hz (0x10)
    config[0] = CRA_REG_M;
    config[1] = DATA_RATE_M;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, config, sizeof(config), true);

    uint8_t CRB_REG_M = 0x01;
    uint8_t GAIN_M = 0x20; // +/- 1.3g (0x20)
    config[0] = CRB_REG_M;
    config[1] = GAIN_M;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, config, sizeof(config), true);
}

float compass_read_degrees(void)
{
    uint8_t reg[1] = {0};
    uint8_t data[1] = {0};

    uint8_t config[2];

    uint8_t MAG_MR_REG_M = 0x02;
    uint8_t SINGLE_SHOT_MODE = 0x01;
    config[0] = MAG_MR_REG_M;
    config[1] = SINGLE_SHOT_MODE;

    // write to enable compass
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, config, sizeof(config), true);
    vTaskDelay(10);

    // Read 6 bytes of data
    // msb first
    // Read xMag msb data from register(0x03)
    reg[0] = 0x03;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, reg, sizeof(reg), false);
    i2c_read_blocking(I2C_PORT, MAG_ADDRESS, data, sizeof(data), false);
    char data1_0 = data[0];

    // Read xMag lsb data from register(0x04)
    reg[0] = 0x04;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, reg, sizeof(reg), false);
    i2c_read_blocking(I2C_PORT, MAG_ADDRESS, data, sizeof(data), false);
    char data1_1 = data[0];

    // Read yMag msb data from register(0x05)
    reg[0] = 0x07;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, reg, sizeof(reg), false);
    i2c_read_blocking(I2C_PORT, MAG_ADDRESS, data, sizeof(data), false);
    char data1_2 = data[0];

    // Read yMag lsb data from register(0x06)
    reg[0] = 0x08;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, reg, sizeof(reg), false);
    i2c_read_blocking(I2C_PORT, MAG_ADDRESS, data, sizeof(data), false);
    char data1_3 = data[0];

    // Read zMag msb data from register(0x07)
    reg[0] = 0x05;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, reg, sizeof(reg), false);
    i2c_read_blocking(I2C_PORT, MAG_ADDRESS, data, sizeof(data), false);
    char data1_4 = data[0];

    // Read zMag lsb data from register(0x08)
    reg[0] = 0x06;
    i2c_write_blocking(I2C_PORT, MAG_ADDRESS, reg, sizeof(reg), false);
    i2c_read_blocking(I2C_PORT, MAG_ADDRESS, data, sizeof(data), false);
    char data1_5 = data[0];

    // Convert the data
    int xMag = (data1_0 << 8) | data1_1;
    if (xMag > 32767)
    {
        xMag -= 65536;
    }

    int yMag = (data1_2 << 8) | data1_3;
    if (yMag > 32767)
    {
        yMag -= 65536;
    }

    int zMag = (data1_4 << 8) | data1_5;
    if (zMag > 32767)
    {
        zMag -= 65536;
    }

    // Calculate the angle of the vector y,x
    float heading = (atan2(yMag, xMag) * 180.0) / PI;

    // Normalize to 0-360
    if (heading < 0)
    {
        heading += 360;
    }

    return heading;
}