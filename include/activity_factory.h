#ifndef ACTIVITY_FACTORY_H
#define ACTIVITY_FACTORY_H

#include "activity.h"

class ActivityFactory
{
public:
    static Activity* BuildActivity(int index);
};

#endif