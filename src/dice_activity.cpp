#include "dice_activity.h"
#include "led_display.h"

#include <stdint.h>
#include <stdio.h>
#include <algorithm>

#include "mxc_device.h"
#include "tpu.h"

#include "BMI160Gen.h"

const uint8_t digits[6][8] =
{
    {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000
    },
    {
        0b00000011,
        0b00000011,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000000,
        0b11000000
    },
    {
        0b00000011,
        0b00000011,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b11000000,
        0b11000000
    },
    {
        0b11000011,
        0b11000011,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11000011,
        0b11000011
    },
    {
        0b11000011,
        0b11000011,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b11000011,
        0b11000011
    },
    {
        0b11000011,
        0b11000011,
        0b00000000,
        0b11000011,
        0b11000011,
        0b00000000,
        0b11000011,
        0b11000011
    }
};

void DiceBMIInterrupt(void* data)
{
    static_cast<DiceActivity*>(data)->shaken = true;
}

DiceActivity::DiceActivity()
{
    printf("Starting dice\n");
    MXC_TPU_Init(MXC_SYS_PERIPH_CLOCK_TRNG);

    // To get a "shaken" effect we rely on the "significant motion" interrupt as it automatically detects
    // movement for a long time.
    // This is done by selecting "significant motion", meaning that the "motion" interrupt no longer is for any motion as
    // indicated in section 2.6.2 of the datasheet.
    BMI160.setSignificantMotionDetectionInterruptSelected(true);
    BMI160.setSignificantMotionDetectionProofTime(BMI160SignificantMotionProofTime::BMI160_SIGNIFICANT_MOTION_PROOF_TIME_0_25S);
    BMI160.setSignificantMotionDetectionSkipTime(BMI160SignificantMotionSkipTime::BMI160_SIGNIFICANT_MOTION_SKIP_TIME_1_5S);
    BMI160.setIntMotionEnabled(true);

    BMI160.attachInterrupt(DiceBMIInterrupt, this);

    // force an initial draw so that the display does not stay blank
    shaken = true;
}

void DiceActivity::loop()
{
    if (shaken)
    {
        shaken = false;

        constexpr int faceCount = 6;
        int maxCount;
        int maxFaceIndex;
        do
        {
            printf("Drawing...");
            maxFaceIndex = -1;
            maxCount = 0;
            uint32_t maxFaceValue = 0;
            for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
            {
                uint32_t faceValue = MXC_TPU_TRNG_Read32BIT(MXC_TRNG);

                if (faceValue > maxFaceValue)
                {
                    maxFaceIndex = faceIndex;
                    maxFaceValue = faceValue;
                    maxCount = 1;
                }
                else if (faceValue == maxFaceValue)
                {
                    maxCount++;
                }
            }
        } while (maxCount != 1);
        printf("  -> %d\n", maxFaceIndex + 1);

        // Show a dice "roll"
        for (int loopIndex = 0; loopIndex < faceCount * 4; loopIndex++)
        {
            Display::displayOneBitImage(digits[loopIndex % faceCount], CRGB::Yellow4);
            MXC_Delay(MXC_DELAY_MSEC(50));
        }

        // show the final value
        Display::displayOneBitImage(digits[maxFaceIndex], CRGB::Green4);
    }
}

LoopDelegate DiceActivity::getLoopDelegate()
{
    return LoopDelegate::from_method<DiceActivity, &DiceActivity::loop>(this);
}
