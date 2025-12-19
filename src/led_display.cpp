#include "led_display.h"
#include "FastLED.h"

static CRGB leds[NUM_LEDS];

void Display::init()
{
    FastLED.addLeds<WS2812B, 12, GRB>(leds, NUM_LEDS);
}

void Display::update()
{
    FastLED.show();
}

CRGB* Display::getLeds()
{
    return leds;
}

