#include "activity.h"

class DiceActivity : public Activity
{
private:
    friend void DiceBMIInterrupt(void* data);

    bool shaken = false;
public:
    DiceActivity();

    void loop();
};