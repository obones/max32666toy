#include "activity.h"
#include "BMI160Gen.h"

Activity::Activity()
{

}

Activity::~Activity()
{
    BMI160.attachInterrupt(nullptr, nullptr);
}
