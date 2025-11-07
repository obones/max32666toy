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

#include "Arduino.h"
#include "FastLED.h"

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

int main(void)
{
    printf("MAX32666 Toy for E14 challenge\n");

    InitArduino();

    LED led = LED(2);

    MXC_SYS_RTCClockEnable();
    MXC_RTC_Init(0, 0);

    FastLED.addLeds<WS2812B, 12>(leds, NUM_LEDS);

    leds[0] = CRGB::Red; FastLED.show();

    /*
        Note: Use printf instead of std::cout.
        iostream consumes an extreme amount of code space.  Our printf
        function is better optimized for microcontrollers with limited flash
    */

    while (1) {
        led.on();
        MyDelay(500000);
        led.off();
        MyDelay(500000);
        led.print_blink_count();
    }

    return 0;
}
