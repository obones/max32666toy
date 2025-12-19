/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. (now owned by
 * Analog Devices, Inc.),
 * Copyright (C) 2023-2024 Analog Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   Hello World (C++ variant)
 * @details This example uses the UART to print to a terminal and flashes an LED.  It also demonstrates
 * basic class creation and instancing available in C++.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"
#include "mxc_sys.h"
#include "rtc.h"
#include "audio.h"

#include "Arduino.h"
#include "FastLED.h"

#include "activity.h"
#include "activity_factory.h"

/***** Definitions *****/

/***** Globals *****/

/***** Functions *****/

class LED {
public:
    explicit LED(int index)
    {
        idx = index;
        blink_count = 0;
    }
    void on()
    {
        LED_On(idx);
        ++blink_count;
    }
    void off()
    {
        LED_Off(idx);
    }
    void print_blink_count()
    {
        printf("Number of blinks: %i\n", blink_count);
    }
    int get_blink_count()
    {
        return blink_count;
    }

private:
    int idx;
    int blink_count;
};


#define NUM_LEDS 8*8
CRGB leds[NUM_LEDS];

void MyDelay(uint32_t us)
{
    //MXC_Delay(500000); return;

    uint32_t startMicros = micros();
    uint32_t endMicros = startMicros;
    while (endMicros - startMicros < us)
        endMicros = micros();

    //printf("startMicros = %d\n", startMicros);
    //printf("endMicros = %d\n", endMicros);
}

void prepareOutputPin()
{
    mxc_gpio_cfg_t pinConfig = {
        MXC_GPIO0,
        MXC_GPIO_PIN_12,
        MXC_GPIO_FUNC_OUT,
        MXC_GPIO_PAD_NONE,
        MXC_GPIO_VSSEL_VDDIOH,
        MXC_GPIO_DRVSTR_3
    };
    MXC_GPIO_Config(&pinConfig);
}

void prepareTimer4()
{
    MXC_SYS_Reset_Periph(MXC_SYS_RESET_TIMER4);
    while (MXC_GCR->rstr0 & MXC_F_GCR_RSTR0_TIMER4) {}
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_T4);
    mxc_tmr_cfg_t config =
    {
        pres: MXC_TMR_PRES_1,
        mode: MXC_TMR_MODE_CONTINUOUS,
        cmp_cnt: 0xFFFFFFFF
    };
    MXC_TMR_Init(MXC_TMR4, &config);
    const mxc_gpio_cfg_t gpio_cfg_tmr4 = { MXC_GPIO0, MXC_GPIO_PIN_4, MXC_GPIO_FUNC_ALT4,
                                        MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIOH, MXC_GPIO_DRVSTR_3 };
    MXC_GPIO_Config(&gpio_cfg_tmr4);
    MXC_TMR_SetCount(MXC_TMR4, 1);
    MXC_TMR_Start(MXC_TMR4);
}

inline void setPin()
{
    MXC_GPIO_OutSet(MXC_GPIO0, MXC_GPIO_PIN_12);
}

inline void clearPin()
{
    MXC_GPIO_OutClr(MXC_GPIO0, MXC_GPIO_PIN_12);
}

inline void __attribute__ ((optimize("-Ofast"))) togglePin()
{
    MXC_GPIO0->out ^= MXC_GPIO_PIN_12;
}

int main(void)
{
    mxc_gpio_cfg_t pinConfig = {MXC_GPIO0, MXC_GPIO_PIN_21, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIOH, MXC_GPIO_DRVSTR_3};
    MXC_GPIO_Config(&pinConfig);
    MXC_GPIO_OutClr(MXC_GPIO0, MXC_GPIO_PIN_21);

    printf("MAX32666 Toy for E14 challenge\n");

    InitArduino();

    prepareOutputPin();
    prepareTimer4();

    typedef struct {
        uint32_t us_ticks;
        uint32_t ns_ticks;
    } Ticks;
    constexpr uint8_t NUM_TICKS = 50;
    Ticks ticks[NUM_TICKS] = {};

    for (uint8_t tick = 0; tick < NUM_TICKS; tick++)
    {
        ticks[tick].us_ticks = MXC_TMR5->cnt;
        ticks[tick].ns_ticks = MXC_TMR4->cnt;
    }
    for (uint8_t tick = 0; tick < NUM_TICKS; tick++)
        printf("%u;%u\n", ticks[tick].us_ticks, ticks[tick].ns_ticks);

    LED led = LED(2);

    FastLED.addLeds<WS2812B, 12, GRB>(leds, NUM_LEDS);

    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = 0x00000F << ((i % 3) * 8);
    }

    MXC_GPIO_OutSet(MXC_GPIO0, MXC_GPIO_PIN_21);

    FastLED.show();

    MXC_GPIO_OutClr(MXC_GPIO0, MXC_GPIO_PIN_21);

    /*
        Note: Use printf instead of std::cout.
        iostream consumes an extreme amount of code space.  Our printf
        function is better optimized for microcontrollers with limited flash
    */

    Activity* activity = ActivityFactory::BuildActivity(0);
    activity->loop();
    delete activity;

    setPin();
    constexpr CRGB pattern[7] = {0x00000F, 0x000F00, 0x0F0000, 0x000F0F, 0x0F000F, 0x0F0F00, 0x0F0F0F};
    while (1) {

        constexpr int delay = 50000;
        led.on();
        MyDelay(delay);
        //togglePin();
        led.off();
        MyDelay(delay);
        //togglePin();
        //led.print_blink_count();
        /*if ((MXC_TMR4->cnt % 10 == 0))
            togglePin();*/

        int blinkCount = led.get_blink_count();
        constexpr int blinkThreshold = 6;
        if (blinkCount == blinkThreshold)
        {
            for (int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++)
                leds[ledIndex] = CRGB::Black;
            leds[0] = pattern[0];
        }
        else if (blinkCount > blinkThreshold)
        {
            CRGB lastLedValue = leds[NUM_LEDS - 1];
            for (int ledIndex = NUM_LEDS - 1; ledIndex > 0; ledIndex--)
                leds[ledIndex] = leds[ledIndex - 1];

            if (blinkCount - blinkThreshold < (int)(sizeof(pattern) /sizeof(pattern[0])))
                leds[0] = pattern[blinkCount - blinkThreshold];
            else
                leds[0] = lastLedValue;
        }

        FastLED.show();
    }

    // https://github.com/Ferki-git-creator/TurboStitchGIF-HeaderOnly-Fast-ZeroAllocation-PlatformIndependent-Embedded-C-GIF-Decoder/blob/main/gif.h
    // https://giflib.sourceforge.net/

    return 0;
}
