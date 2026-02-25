#include "activity.h"
#include "BMI160Gen.h"
#include "led_display.h"

Activity::Activity()
{
    Display::clear();
}

Activity::~Activity()
{
    BMI160.attachInterrupt(nullptr, nullptr);
}
