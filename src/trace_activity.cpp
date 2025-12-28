#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "BMI160Gen.h"

#include "trace_activity.h"
#include "led_display.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define ACCEL_SENSITIVITY 16384.0 // Sensitivity for ±2g in LSB/g (adjust based on your configuration)

constexpr uint8_t requiredRepeats[] = {0, 20, 15, 10, 5, 3, 1};
constexpr uint8_t binCount = sizeof(requiredRepeats) / sizeof(requiredRepeats[0]);

TraceActivity::TraceActivity()
{
    // auto-calibrate accelerometer
    printf("Calibrating accelerometer...");
    fflush(stdout);
    BMI160.setRegister(BMI160_RA_CMD, 0x37);
    delay(1000);
    printf(" Done.\n");

    //BMI160.setAccelRate(0b1000);

    x = Display::Width / 2;
    y = Display::Height / 2;
}

void TraceActivity::updateDiscretization(float angle, int8_t& lastBin, uint32_t& repeat)
{
    // discretize -80;80 into the bins defined above.
    constexpr float maxAngle = 80;
    if (angle > maxAngle)
        angle = maxAngle;
    else if (angle < -maxAngle)
        angle = -maxAngle;

    int8_t bin = angle / maxAngle * (binCount - 1);

    if (bin != lastBin)
    {
        lastBin = bin;
        repeat = 0;
    }
    else
    {
        repeat++;
    }

    //printf("angle: %.2f°, bin: %d, repeat: %d\n", double(angle), bin, repeat);
}

void TraceActivity::updateCoordinate(int8_t lastBin, uint32_t& repeat, uint8_t axisSize, uint8_t& coordinate)
{
    uint8_t requiredRepeat = requiredRepeats[abs(lastBin)];
    if (requiredRepeat != 0 && repeat > requiredRepeat)
    {
        repeat = 0;

        if (lastBin > 0)
        {
            if (coordinate != axisSize - 1)
                coordinate++;
        }
        else
        {
            if (coordinate != 0)
                coordinate--;
        }
    }
}

void TraceActivity::loop()
{
    int16_t ax, ay, az;

    // Read accelerometer data
    // Inspired by https://how2electronics.com/interfacing-bmi160-accelerometer-gyroscope-with-arduino/
    BMI160.getAcceleration(&ax, &ay, &az);
    //printf("ax: %d, ay: %d, az: %d\n", ax, ay, az);

    // Convert raw accelerometer values to g
    float ax_g = ax / ACCEL_SENSITIVITY;
    float ay_g = ay / ACCEL_SENSITIVITY;
    float az_g = az / ACCEL_SENSITIVITY;

    // Calculate tilt angles (pitch and roll) in degrees
    float pitch = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / M_PI;
    float roll = atan2(-ax_g, az_g) * 180.0 / M_PI;

    // Print tilt angles
    //printf("Pitch: %.2f°, Roll: %.2f°\n", double(pitch), double(roll));

    updateDiscretization(pitch, lastPitchBin, pitchRepeat);
    updateDiscretization(roll, lastRollBin, rollRepeat);

    updateCoordinate(lastPitchBin, pitchRepeat, Display::Width, x);
    updateCoordinate(lastRollBin, rollRepeat, Display::Height, y);

    //printf("x: %d, y: %d\n", x, y);

    if (millis() - lastMillis > 250)
    {
        for(int ledIndex = 0; ledIndex < Display::Height * Display::Width; ledIndex++)
            Display::leds[ledIndex].fadeToBlackBy(32);

        lastMillis = millis();
    }

    Display::setPixel(x, y, CRGB::Blue4);
    Display::update();

    //delay(250);
}