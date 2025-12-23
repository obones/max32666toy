/* Header to help FastLED */
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>

void InitArduino();

extern "C"
{
    // InitArduino must have been called beforehand
    uint32_t millis();
    uint32_t micros();
    void delay(int ms);
    void delayMicroseconds(int us);
    void yield();
}

#endif
