#ifndef GIF_ACTIVITY_H
#define GIF_ACTIVITY_H

#include "activity.h"
#include "ff.h"

#define FF_MAXLEN 256

class GifActivity : public Activity
{
private:
    TCHAR cwd[FF_MAXLEN];
    FATFS fs;

    bool mounted;

    int mount();
public:
    GifActivity();

    void loop();
};

#endif