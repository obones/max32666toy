#include "Arduino.h"
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "tmr.h"

#include "stdio.h"

#define TIMER MXC_TMR5

float timerDivider;

void InitArduino()
{
    // We use TIMER5 as our "micros" source value
    // Note that we tried using HTMR as it could have given more precision but there are lots of issues with
    // ssec and sec being out of sync, in particular with ssec going back in time very often
    MXC_SYS_Reset_Periph(MXC_SYS_RESET_TIMER5);
    while (MXC_GCR->rstr0 & MXC_F_GCR_RSTR0_TIMER5) {}
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_T5);

    mxc_tmr_cfg_t config =
    {
        mode: MXC_TMR_MODE_CONTINUOUS,
    };

    // Only those four can handle microsecond precision, lower frequency sources cannot.
    // For instance, the 32.768kHz source gives a tick duration of 61us even with a :1 prescaler
    // Note that we value rollover duration over precision. For instance, at 96MHz, the rollover
    // is the expected 71.58 minutes but the precision is 1.3333us
    switch(SystemCoreClock)
    {
        case 96000000:
            config.pres = MXC_TMR_PRES_64;
            config.cmp_cnt = 0xC0000000;
            timerDivider = 0.75;
            break;
        case 60000000:
            config.pres = MXC_TMR_PRES_32;
            config.cmp_cnt = 0xF0000000;
            timerDivider = 0.9375;
            break;
        case 32000000:
            config.pres = MXC_TMR_PRES_16;
            config.cmp_cnt = 0xFFFFFFFF;
            timerDivider = 1;
            break;
        case 7372800:
            config.pres = MXC_TMR_PRES_4;
            config.cmp_cnt = 0xEBEDFA43;
            timerDivider = 0.9216;
            break;
    }

    MXC_TMR_Init(TIMER, &config);
    MXC_TMR_SetCount(TIMER, 1);
    MXC_TMR_Start(TIMER);
}

uint32_t millis()
{
    return micros() / 1000;
}

uint32_t micros()
{
    uint32_t fullValue = MXC_TMR_GetCount(TIMER);

    uint32_t result = fullValue / timerDivider;

    return result;
}

void delay(int ms)
{
    MXC_Delay(MXC_DELAY_MSEC(ms));
}

void yield()
{
    // no_op
}
