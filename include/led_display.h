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

    static void init();
    static void update();

    static CRGB* getLeds();
};

#endif