#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

#include <FastLED.h>
#include <LEDMatrix.h>

#define NUM_LEDS Display::Width * Display::Height

class Display
{
public:
    Display() = delete;

    static constexpr uint8_t Width = 8;
    static constexpr uint8_t Height = 8;

    // direct access, be careful what you do with this
    static cLEDMatrix<-Display::Width, -Display::Height, HORIZONTAL_MATRIX> leds;

    static void init();
    static void update();
    static uint8_t getBrightness();
    static void setBrightness(uint8_t value);

    static void clear();

    static void displayOneBitImage(const uint8_t* bits, CRGB color);
    static void setPixel(uint8_t x, uint8_t y, CRGB color);
};

#endif