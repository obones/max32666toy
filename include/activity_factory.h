#ifndef ACTIVITY_FACTORY_H
#define ACTIVITY_FACTORY_H

#include "activity.h"

class ActivityFactory
{
    static Activity* BuildActivity(int index);
};

#endif