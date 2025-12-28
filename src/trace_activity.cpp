#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "BMI160Gen.h"

#include "trace_activity.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define ACCEL_SENSITIVITY 16384.0 // Sensitivity for ±2g in LSB/g (adjust based on your configuration)

TraceActivity::TraceActivity()
{
    // auto-calibrate accelerometer
    printf("Calibrating accelerometer...");
    BMI160.setRegister(BMI160_RA_CMD, 0x37);
    delay(1000);
    printf("Done.\n");

    //BMI160.setAccelRate(0b1000);
}

void TraceActivity::loop()
{
    int16_t ax, ay, az;

    // Read accelerometer data
    /*uint8_t buffer[6] = {};

    _i2c_req_t req;
    req.addr = 0b1101000;
    req.i2c = MXC_I2C0_BUS0;
    req.tx_buf = buffer;
    req.tx_len = 1;
    req.rx_buf = buffer;
    req.rx_len = 6;

    buffer[0] = 0x12; // first accelerometer register

    int err = MXC_I2C_MasterTransaction(&req);
    switch (err)
    {
        case E_NO_ERROR:
            break;
        case E_BUSY:
            printf("x");
            return;
        default:
            printf("TraceActivity::loop(): I2C failed: %d.\n", err);
            return;
    }

    ax = ((int16_t)buffer[1]) << 8 | buffer[0];
    ay = ((int16_t)buffer[3]) << 8 | buffer[2];
    az = ((int16_t)buffer[5]) << 8 | buffer[4];

    /*/BMI160.getAcceleration(&ax, &ay, &az);
    printf("ax: %d, ay: %d, az: %d\n", ax, ay, az);

    // Convert raw accelerometer values to g
    float ax_g = ax / ACCEL_SENSITIVITY;
    float ay_g = ay / ACCEL_SENSITIVITY;
    float az_g = az / ACCEL_SENSITIVITY;

    // Calculate tilt angles (pitch and roll) in degrees
    float pitch = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / M_PI;
    float roll = atan2(-ax_g, az_g) * 180.0 / M_PI;

    // Print tilt angles
    printf("Pitch: %.2f°, Roll: %.2f°\n", double(pitch), double(roll));

    delay(500);

}