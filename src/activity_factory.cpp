#include "activity_factory.h"

#include "gif_activity.h"
#include "dice_activity.h"
#include "trace_activity.h"
#include "empty_activity.h"

Activity* ActivityFactory::BuildActivity(int index)
{
    switch (index)
    {
        case 0:
            return new GifActivity();
        case 1:
            return new DiceActivity();
        case 2:
            return new TraceActivity();
        default:
            return new EmptyActivity(index);
    }
}
