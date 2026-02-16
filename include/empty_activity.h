#include "activity.h"

class EmptyActivity : public Activity
{
private:
    int index;
public:
    EmptyActivity(int index);

    void loop();
};