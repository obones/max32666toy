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


#define NUM_LEDS 1//8*8
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

mxc_audio_I2S_config_t getAudioConfig()
{
    mxc_audio_I2S_config_t config = { 0 };

    config.audio = MXC_AUDIO;
    config.masterClockSource = MXC_AUDIO_CLK_SRC_HSCLK;
    config.clock = MXC_AUDIO_CLK_12_288MHz;
    config.BCLKSourceSelect = MXC_AUDIO_BCLK_GENERATOR_TOGGLE;
    config.BCLKSource = MXC_AUDIO_BCLK_SOURCE_F_AUDIO;
    config.BCLKPolarity = MXC_AUDIO_CLK_POL_HIGH;
    config.BCLKDivisor = 6; //0x78;
    config.LRCLKPolarity = MXC_AUDIO_CLK_POL_HIGH;
    config.LRCLKDivider = MXC_AUDIO_LRCLK_DIV_48;
    config.channelSize = MXC_AUDIO_PCM_CHANNEL_SIZE_24; // want 32 but 24 gets out
    config.TxInterfaceSampleRates = MXC_AUDIO_PCM_SAMPLE_RATE_192kHz;
    config.RxInterfaceSampleRates = MXC_AUDIO_PCM_SAMPLE_RATE_192kHz;
    config.TxDataportSampleRates = MXC_AUDIO_PCM_SAMPLE_RATE_192kHz;
    config.RxDataportSampleRates = MXC_AUDIO_PCM_SAMPLE_RATE_192kHz;
    config.TxExtraBitsFormat = MXC_AUDIO_TX_EXTRA_BITS_0;

    return config;
}

int prepareAudioI2S(mxc_audio_I2S_config_t* config)
{
    return MXC_AUDIO_I2S_Configure(config);
}

int prepareAudioPDM(mxc_audio_I2S_config_t* config)
{
    if (config->audio == NULL) {
        return E_NULL_PTR;
    }

    int err;
    if ((err = MXC_AUDIO_SetMasterClock(config)) != E_NO_ERROR) {
        return err;
    }

    // PDM uses MCLK directly
    config->audio->pdm_tx_rate_setup = PDM_TX_CLK_RATE_MCLK | PDM_TX_CLK_DATA_SR_PDM_TX_CLK;

    // Enable PDM Mode, Normal encoding, Tx Modulator bypass, Tx Mono mode, Tx Modulator disabled
    config->audio->modulator_controls = MXC_F_TX_MOD_BYPASS | MXC_F_TX_MONO_ENABLE;

    // Enable PDM TX1 channel 0
    config->audio->pdm_tx_control_1 = PDM_TX_1_CH0_EN;

    // PDM Tx Data length is 1 bit
    config->audio->pdm_tx_control_2 = 0;
}

void prepareAudio()
{
    //Enable audio subsytem peripheral clock
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_AUDIO);
    //Enable gpio clock and configure as alternate function
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_GPIO0);

    const mxc_gpio_cfg_t gpio_cfg_audio = { MXC_GPIO0,
                                            //(/*MXC_GPIO_PIN_25 |*/ MXC_GPIO_PIN_27),  // DOUT and BCLK --> does not work, DOUT stays at 1V
                                            (MXC_GPIO_PIN_24 | MXC_GPIO_PIN_25 | MXC_GPIO_PIN_26 |
                                            MXC_GPIO_PIN_27),
                                            MXC_GPIO_FUNC_ALT1,
                                            MXC_GPIO_PAD_NONE,
                                            MXC_GPIO_VSSEL_VDDIOH,
                                            MXC_GPIO_DRVSTR_3 };
    MXC_GPIO_Config(&gpio_cfg_audio);

/*    const mxc_gpio_cfg_t tmp_gpio_cfg = { MXC_GPIO0,
                                            MXC_GPIO_PIN_25,
                                            MXC_GPIO_FUNC_ALT1,
                                            MXC_GPIO_PAD_NONE,
                                            MXC_GPIO_VSSEL_VDDIOH,
                                            MXC_GPIO_DRVSTR_3 };
    MXC_GPIO_Config(&tmp_gpio_cfg);*/
    //MXC_GPIO_OutSet(MXC_GPIO0, MXC_GPIO_PIN_25);

    /*MXC_GPIO0->en |= MXC_GPIO_PIN_25;
    MXC_GPIO0->en1 &= ~MXC_GPIO_PIN_25;
    MXC_GPIO0->en2 &= ~MXC_GPIO_PIN_25;

    MXC_GPIO_SetVSSEL(MXC_GPIO0, MXC_GPIO_VSSEL_VDDIOH, MXC_GPIO_PIN_25);
    MXC_GPIO_SetDriveStrength(MXC_GPIO0, MXC_GPIO_DRVSTR_3, MXC_GPIO_PIN_25);

    MXC_GPIO_OutClr(MXC_GPIO0, MXC_GPIO_PIN_25);*/

    /*MXC_GPIO0->en &= ~MXC_GPIO_PIN_25;
    MXC_GPIO0->en1 &= ~MXC_GPIO_PIN_25;
    MXC_GPIO0->en2 &= ~MXC_GPIO_PIN_25;*/

    mxc_audio_I2S_config_t config = getAudioConfig();

    int error = prepareAudioI2S(&config);
    //int error = prepareAudioPDM(&config);
    if (error != E_NO_ERROR)
        printf("Error initializing audio: %d", error);
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
    //MXC_GPIO_OutToggle(MXC_GPIO0, MXC_GPIO_PIN_12);
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
    prepareAudio();

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


    /*clearPin();
    while(1)
    {
        //if (MXC_TMR4->cnt % 10 == 0)
            togglePin();
    }*/

    LED led = LED(2);

    FastLED.addLeds<WS2812B, 12, GRB>(leds, NUM_LEDS);

    leds[0] = CRGB::Red;

    MXC_GPIO_OutSet(MXC_GPIO0, MXC_GPIO_PIN_21);

    FastLED.show();

    /*while(1)
    {
    MXC_AUDIO->tx_pcm_ch0_addr = 0b01111111111110100111111111110110; //0xEFFFEFFF; // 0xAAAAAAAA;//0xF0F0F0F0;
    MXC_AUDIO->tx_pcm_ch1_addr = 0b01111111111011100111111111011110; //0xEFFFFDFF; 0xAAAAAAAA;//0b11100111001110011000110001100000;

    MXC_AUDIO->tx_pdm_ch0_addr = 0xFFFFFFFF;
    }*/

    /*
chipset frequency: 800.000000 Khz
smallest: 250
pgcd: 125
precision:0
nb pulse per bit: 10
    T1Pulses: 2
    T2Pulses: 5
    T3Pulses: 3
needed frequency (nbPulse per bit) * (chipset // frequency): 8.000000 Mhz
SystemCoreClock: 96000000
needed BitCLK divider: 12
gOneBit:  00000000'00000000'00000011'11111000
gOneBit:  00000000'00000000'00000011'00000000
pixelsPulsesSize: 13
  remainingBits: 4 - gPulsesPerBit: 10 - currentPulsePos: 2, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 8 - gPulsesPerBit: 10 - currentPulsePos: 6, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 2 - gPulsesPerBit: 10 - currentPulsePos: 0, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 6 - gPulsesPerBit: 10 - currentPulsePos: 4, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 0 - gPulsesPerBit: 10 - currentPulsePos: -2, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 4 - gPulsesPerBit: 10 - currentPulsePos: 2, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 8 - gPulsesPerBit: 10 - currentPulsePos: 6, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 2 - gPulsesPerBit: 10 - currentPulsePos: 0, LOWEST_USABLE_BIT_INDEX: 8
  remainingBits: 6 - gPulsesPerBit: 10 - currentPulsePos: 4, LOWEST_USABLE_BIT_INDEX: 8
pixelsPulses:
11111110'00111111'10001111'00000000 11100011'11111000'11111110'00000000
00111111'10001111'11100011'00000000 11111000'11000000'00110000'00000000
00001100'00000011'00000000'00000000 11000000'00110000'00001100'00000000
00000011'00000000'11000000'00000000 00110000'00001100'00000011'00000000
00000000'11000000'00110000'00000000 00001100'00000011'00000000'00000000
00000000'00000000'00000000'00000000 00000000'00000000'00000000'00000000
00000000'00000000'00000000'00000000

1111111000 1111111000 1111111000 1111111000 1111111000 1111111000 1111111000 1111111000
1100000000 1100000000 1100000000 1100000000 1100000000 1100000000 1100000000 1100000000
1100000000 1100000000 1100000000 1100000000 1100000000 1100000000 1100000000 1100000000
0000000000 0000000000 0000000000 0000000000 0000000000 0000000000 0000000000 00
    */

    MXC_GPIO_OutClr(MXC_GPIO0, MXC_GPIO_PIN_21);

    /*
        Note: Use printf instead of std::cout.
        iostream consumes an extreme amount of code space.  Our printf
        function is better optimized for microcontrollers with limited flash
    */

    setPin();
    while (1) {

        led.on();
        MyDelay(500000);
        //togglePin();
        led.off();
        MyDelay(500000);
        //togglePin();
        //led.print_blink_count();
        /*if ((MXC_TMR4->cnt % 10 == 0))
            togglePin();*/
    }

    return 0;
}
