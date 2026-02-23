#include "led_display.h"
#include "FastLED.h"

cLEDMatrix<-Display::Width, -Display::Height, HORIZONTAL_MATRIX> Display::leds = {};

void Display::init()
{
    FastLED.addLeds<WS2812B, 12, GRB>(leds[0], NUM_LEDS);
}

void Display::update()
{
    FastLED.show();
}

uint8_t Display::getBrightness()
{
    return FastLED.getBrightness();
}

void Display::setBrightness(uint8_t value)
{
    FastLED.setBrightness(value);
}

void Display::displayOneBitImage(const uint8_t* bits, CRGB color)
{
    uint8_t currentByte = *bits;
    for (int y = 0; y < Height; y++)
    {
        for (int x = 0; x < Width; x++)
        {
            if (currentByte & 0x80)
                leds(x, y) = color;
            else
                leds(x, y) = CRGB::Black;

            currentByte <<= 1;
        }

        bits++;
        currentByte = *bits;
    }

    update();
}

void Display::setPixel(uint8_t x, uint8_t y, CRGB color)
{
    leds(x, y) = color;
}

