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
#include <cstdlib>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"
#include "mxc_sys.h"
#include "rtc.h"
#include "audio.h"
#include "i2c.h"
#include "adc.h"
#include "adc_regs.h"
#include <nvic_table.h>

#include "Arduino.h"

#include "led_display.h"

#include "activity.h"
#include "activity_factory.h"

#include "BMI160Gen.h"

#define I2C_MASTER MXC_I2C0_BUS0 ///< I2C instance
#define I2C_FREQ 100000 ///< I2C clock frequency

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

void initBMI160()
{
    int error = MXC_I2C_Init(I2C_MASTER, 1, 0);
    if (error != E_NO_ERROR) {
        printf("I2C master configure failed with error %i\n", error);
        return;
    }
    error = MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);
    if (error < E_NO_ERROR) {
        printf("I2C set frequency failed with error %i\n", error);
        return;
    }

    printf("begin BMI160\n");
    if (!BMI160.begin(I2C_MASTER, 0x68)) //, MXC_GPIO_PIN_11))
    {
        printf("BMI160 failed\n");
        return;
    }

    uint8_t pmuStatus = BMI160.getRegister(BMI160_RA_PMU_STATUS);
    printf("BMI PMU status: 0x%.2x\n", pmuStatus);
}

void startBrightnessLevelRead()
{
    // Can't use MXC_ADC_StartConversion(MXC_ADC_CH_0); as it is waiting for adc to become inactive where we want to poll the status later on

    // Clear ADC done interrupt flag
    MXC_ADC_ClearFlags(MXC_F_ADC_INTR_DONE_IF);

    // Set start bit
    MXC_ADC->ctrl |= MXC_F_ADC_CTRL_START;
}

void initBrightnessLevelRead()
{
    int error = MXC_ADC_Init();
    if (error != E_NO_ERROR)
        printf("MXC_ADC_Init failed with error %i\n", error);

    MXC_GPIO_Config(&gpio_cfg_adc0);

    // We are measuring a voltage that goes from 0V to the full 3.3V.
    // We thus decided to use the unscaled 1.22V internal band-gap reference (ADC_CTRL.ref_sel = 0)
    // which means that we should divide the input by 3 (ADC_CTRL.adc_divsel = 0x2) to get a maximum value
    // of 1.1V and thus avoid overflows.
    // But real world experience showed that we overflowed at midpoint!
    // So, for some weird reason, we actually have to further scale the input by two (ADC_CTRL.scale = 1)
    // in order to get our entire voltage scale properly.
    // Note: as the divisions gets us a voltage value just below 1.22V, we go from 0 to 234

    // clear required flags
    MXC_ADC->ctrl &= ~(
        MXC_F_ADC_CTRL_CH_SEL |      // reset channel selection
        MXC_F_ADC_CTRL_REF_SCALE |   // do not scale reference
        MXC_F_ADC_CTRL_REF_SEL       // use internal band-gap reference
    );

    // set required flags for conversion
    MXC_ADC->ctrl |=
        MXC_F_ADC_CTRL_DATA_ALIGN |        // MSB align to get 8bit conversion for free
        MXC_S_ADC_CTRL_ADC_DIVSEL_DIV3 |   // divide 3.3v by 3 to be just below the 1.22V band gap reference
        MXC_F_ADC_CTRL_SCALE |             // scale input (see above)
        ((0 << MXC_F_ADC_CTRL_CH_SEL_POS) & MXC_F_ADC_CTRL_CH_SEL); // convert on channel 0

    printf("ADC initialized\n");

    startBrightnessLevelRead();
}

bool brightnessLevelReady(uint8_t& value)
{
    bool result = ((MXC_ADC->status & MXC_F_ADC_STATUS_ACTIVE) == 0) && (MXC_ADC->intr & MXC_F_ADC_INTR_DONE_IF);
    if (result)
    {
        value = *(((uint8_t*)&(MXC_ADC->data)) + 1);
    }

    return result;
}

volatile bool activityChanged = true;

static void activitySelectorISR()
{
    activityChanged = true;

    MXC_GPIO0->int_clr = MXC_GPIO_PIN_4;
}

void initActivitySelector()
{
    mxc_gpio_cfg_t pinConfig =
    {
        MXC_GPIO0,
        MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_4,
        MXC_GPIO_FUNC_IN,
        MXC_GPIO_PAD_NONE,
        MXC_GPIO_VSSEL_VDDIOH,
        MXC_GPIO_DRVSTR_3
    };
    MXC_GPIO_Config(&pinConfig);

    mxc_gpio_cfg_t interruptPinConfig =
    {
        MXC_GPIO0,
        MXC_GPIO_PIN_4,
        MXC_GPIO_FUNC_IN,
        MXC_GPIO_PAD_NONE,
        MXC_GPIO_VSSEL_VDDIOH,
        MXC_GPIO_DRVSTR_3
    };
    int error;
    error = MXC_GPIO_Config(&interruptPinConfig);
    if (error != E_NO_ERROR)
        printf("MXC_ADC_Init failed with error %i\n", error);

    // The GPIO demo shows usage of MXC_GPIO_RegisterCallback but we could not get it to work at all so we use
    // the tried and tested MXC_NVIC_SetVector calls
    constexpr IRQn_Type gpioIrqNumber = GPIO0_IRQn;

    NVIC_ClearPendingIRQ(gpioIrqNumber);
    NVIC_DisableIRQ(gpioIrqNumber);
    MXC_NVIC_SetVector(gpioIrqNumber, activitySelectorISR);
    NVIC_SetPriority(gpioIrqNumber, 0);

    interruptPinConfig.port->int_en = 0;
    interruptPinConfig.port->int_clr = interruptPinConfig.mask;

    // We ask for "rising" but it seems it does not care as we get an interrupt when 74HC148 E0 goes to 0
    // making us read a register value of the form 0x07 which should not happen.
    MXC_GPIO_IntConfig(&interruptPinConfig, MXC_GPIO_INT_RISING);

    MXC_GPIO_EnableInt(interruptPinConfig.port, interruptPinConfig.mask);

    NVIC_EnableIRQ(gpioIrqNumber);

    printf("Selector initialized\n");
}

int8_t readActivitySelector()
{
    uint32_t portValue = MXC_GPIO_InGet(MXC_GPIO0, MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_4);

    printf("portValue: %X\n", portValue);

    if (portValue & MXC_GPIO_PIN_4)
        return (portValue & 0x07);
    else
        return -1;
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

    /*typedef struct {
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
        printf("%u;%u\n", ticks[tick].us_ticks, ticks[tick].ns_ticks);*/

    //LED led = LED(2);

    Display::init();

    CRGB* leds = Display::leds;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = 0x00000F << ((i % 3) * 8);
    }

    MXC_GPIO_OutSet(MXC_GPIO0, MXC_GPIO_PIN_21);

    Display::update();

    MXC_GPIO_OutClr(MXC_GPIO0, MXC_GPIO_PIN_21);

    initBMI160();
    initBrightnessLevelRead();
    initActivitySelector();

    /*
        Note: Use printf instead of std::cout.
        iostream consumes an extreme amount of code space.  Our printf
        function is better optimized for micro-controllers with limited flash
    */

    Activity* activity = nullptr;

    /*typedef void (*LoopPtr)(Activity*);
    LoopPtr loopPtr = activity->*loop;

    // https://www.codeproject.com/articles/The-Impossibly-Fast-Cplusplus-Delegates-Fixed#comments-section
    // https://www.codeproject.com/articles/The-Impossibly-Fast-C-Delegates#comments-section
    // https://www.codeproject.com/articles/Fast-C-Delegate-Boost-Function-drop-in-replacement#comments-section

    loopPtr(activity);*/
/*
    typedef void (*LoopPtr)(Activity*);
    auto a = (*activity);

    LoopPtr loopPtr = (LoopPtr)(activity->*loop);
    loopPtr(activity);*/

    setPin();
    uint8_t previousBrightness = 0;
    //constexpr CRGB pattern[7] = {0x00000F, 0x000F00, 0x0F0000, 0x000F0F, 0x0F000F, 0x0F0F00, 0x0F0F0F};
    while (1) {
        uint8_t newBrightness;
        if (brightnessLevelReady(newBrightness))
        {
            if (abs(newBrightness - previousBrightness) > 3)
            {
                previousBrightness = newBrightness;
                printf("Brightness read: %d\n", newBrightness);
            }
            // start next read
            startBrightnessLevelRead();
        }

        if (activityChanged)
        {
            int8_t activityIndex = readActivitySelector();
            printf("Activity index: %d\n", activityIndex);
            activityChanged = false;

            delete activity;
            activity = ActivityFactory::BuildActivity(activityIndex);
        }

        /*constexpr int delay = 50000;
        led.on();
        MyDelay(delay);
        //togglePin();
        led.off();
        MyDelay(delay);*/
        //togglePin();
        //led.print_blink_count();
        /*if ((MXC_TMR4->cnt % 10 == 0))
            togglePin();*/

        /*int blinkCount = led.get_blink_count();
        constexpr int blinkThreshold = 6;
        if (blinkCount == blinkThreshold)
        {
            for (int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++)
                leds[ledIndex] = CRGB::Black;
            leds[0] = pattern[0];

            Display::update();
        }
        else if (blinkCount > blinkThreshold)*/
        {
            activity->loop();

            /*CRGB lastLedValue = leds[NUM_LEDS - 1];
            for (int ledIndex = NUM_LEDS - 1; ledIndex > 0; ledIndex--)
                leds[ledIndex] = leds[ledIndex - 1];

            if (blinkCount - blinkThreshold < (int)(sizeof(pattern) /sizeof(pattern[0])))
                leds[0] = pattern[blinkCount - blinkThreshold];
            else
                leds[0] = lastLedValue;*/
        }
    }

    delete activity;

    return 0;
}
