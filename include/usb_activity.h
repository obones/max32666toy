#include "activity.h"

class USBActivity : public Activity
{
private:
public:
    USBActivity();

    void loop();

    LoopDelegate getLoopDelegate();
};