#include "led_display.h"
#include "FastLED.h"

CRGB Display::leds[NUM_LEDS] = {};

void Display::init()
{
    FastLED.addLeds<WS2812B, 12, GRB>(leds, NUM_LEDS);
}

void Display::update()
{
    FastLED.show();
}

void Display::displayOneBitImage(const uint8_t* bits, CRGB color)
{
    uint8_t currentByte = *bits;
    int pixelIndex = 0;
    while(pixelIndex < NUM_LEDS)
    {
        if (currentByte & 0x80)
            leds[pixelIndex] = color;
        else
            leds[pixelIndex] = CRGB::Black;

        currentByte <<= 1;

        pixelIndex++;

        if (pixelIndex % 8 == 0)
        {
            bits++;
            currentByte = *bits;
        }
    }

    update();
}

