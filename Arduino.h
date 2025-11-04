/* Header to help FastLED */
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>

extern "C"
{
    // Must enable both the RTC clock source and the RTC subsystem outside of FastLED
    uint32_t millis();
    uint32_t micros();
    void delay(int ms);
    void yield();
}

#endif
