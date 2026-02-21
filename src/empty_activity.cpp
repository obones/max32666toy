#include "empty_activity.h"
#include "led_display.h"

#include <stdio.h>

const uint8_t digits[9][8] =
{
    {
        0b00111100,
        0b01000010,
        0b10000001,
        0b10000001,
        0b10000001,
        0b10000001,
        0b01000010,
        0b00111100
    },
    {
        0b00011000,
        0b00111000,
        0b01011000,
        0b10011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b11111111
    },
    {
        0b00111100,
        0b01100110,
        0b11001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b11000000,
        0b11111111
    },
    {
        0b00111100,
        0b11000011,
        0b00000011,
        0b00011110,
        0b00001110,
        0b00000011,
        0b11000011,
        0b00111100
    },
    {
        0b00011000,
        0b00111000,
        0b01011000,
        0b10011000,
        0b11111111,
        0b00011000,
        0b00011000,
        0b00011000
    },
    {
        0b11111110,
        0b11000000,
        0b11000000,
        0b11111100,
        0b00000110,
        0b00000011,
        0b00000110,
        0b11111100
    },
    {
        0b00111110,
        0b01100000,
        0b11000000,
        0b11111100,
        0b11000110,
        0b11000011,
        0b01100110,
        0b00111100
    },
    {
        0b11111111,
        0b00000110,
        0b00001100,
        0b00011000,
        0b0011000,
        0b01100000,
        0b11000000,
        0b10000000
    },
    {
        0b00111100,
        0b11000011,
        0b11000011,
        0b01111110,
        0b01111110,
        0b11000011,
        0b11000011,
        0b00111100
    }
};

const uint8_t cross[8] =
{
    0b11000011,
    0b01100110,
    0b00111100,
    0b00011000,
    0b00011000,
    0b00111100,
    0b01100110,
    0b11000011
};

EmptyActivity::EmptyActivity(int index)
{
    this->index = index;

    if (index < 0)
        Display::displayOneBitImage(cross, CRGB::Red4);
    else
        Display::displayOneBitImage(digits[index], CRGB::Red4);
}

LoopDelegate EmptyActivity::getLoopMethod()
{
    auto tmp = LoopDelegate::from_method<EmptyActivity, &EmptyActivity::nonVirtualLoop>(this);

    return tmp;
}

FFLoopDelegate EmptyActivity::getFFLoopMethod()
{
    FFLoopDelegate tmp(this, &EmptyActivity::nonVirtualLoop);
    return tmp;
}

SALoopDelegate EmptyActivity::getSALoopMethod()
{
    auto tmp = SALoopDelegate::create<EmptyActivity, &EmptyActivity::nonVirtualLoop>(this);
    return tmp;
}

/*void EmptyActivity::getLoopMethod()
{
    typedef void (*loopPtr)(Activity *);
    loopPtr tmp2 = static_cast<void*>(loop);
    auto tmp = loop;
}*/

void EmptyActivity::loop()
{
    //printf("virtual loop\t");
}

void EmptyActivity::nonVirtualLoop()
{
    //printf("non virtual loop\t");
}

