#include "activity.h"

class TraceActivity : public Activity
{
private:
    int8_t lastPitchBin = 0;
    int8_t lastRollBin = 0;
    uint32_t pitchRepeat = 0;
    uint32_t rollRepeat = 0;

    uint8_t x;
    uint8_t y;

    uint32_t lastMillis;

    void updateCoordinate(float angle, int8_t& lastBin, uint32_t& repeat, uint8_t axisSize, uint8_t& coordinate);
public:
    TraceActivity();

    void loop();

    LoopDelegate getLoopDelegate();
};