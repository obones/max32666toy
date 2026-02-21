#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H
#include <FastLED.h>
#define NUM_LEDS Display::Width * Display::Height

class Display
{
public:
    Display() = delete;

    static constexpr uint8_t Width = 8;
    static constexpr uint8_t Height = 8;

    // direct access, be careful what you do with this
    static CRGB leds[NUM_LEDS];

    static void init();
    static void update();
    static uint8_t getBrightness();
    static void setBrightness(uint8_t value);

    static void displayOneBitImage(const uint8_t* bits, CRGB color);
    static void setPixel(uint8_t x, uint8_t y, CRGB color);
};

#endif