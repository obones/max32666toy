#include "Arduino.h"
#include "rtc.h"

// Must enable both the RTC clock source and the RTC subsystem outside of FastLED
uint32_t millis()
{
    uint32_t sec, subsec;

    while (MXC_RTC_GetBusyFlag() == E_BUSY) {};

    MXC_RTC_GetTime(&sec, &subsec);

    return sec * 1000 + subsec * 244;
}

uint32_t micros()
{
    return millis() * 1000;
}

void delay(int ms)
{
    MXC_Delay(MXC_DELAY_MSEC(ms));
}

void yield()
{
    // no_op
}
