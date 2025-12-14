#include "activity_factory.h"

#include "gif_activity.h"

Activity* ActivityFactory::BuildActivity(int index)
{
    return new GifActivity();
}
