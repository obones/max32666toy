#include "activity.h"

class DiceActivity : public Activity
{
private:
    friend void DiceBMIInterrupt(void* data);

    bool shaken;
public:
    DiceActivity();

    void loop();

    LoopDelegate getLoopDelegate();
};