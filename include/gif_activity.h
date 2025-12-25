#ifndef GIF_ACTIVITY_H
#define GIF_ACTIVITY_H

#include "activity.h"
#include "ff.h"
#include <vector>
#include <string>
#include "AnimatedGif.h"

#define FF_MAXLEN 256

class GifActivity : public Activity
{
    friend void GifBMIInterrupt(void* data);
private:
    std::vector<std::string> files;
    FATFS fs;
    AnimatedGIF gif;
    int currentFileIndex = 0;
    bool moveToNextFile = false;

    bool mounted;

    bool initSDCard();
    FRESULT mount();
    FRESULT listFiles();
    bool loadFile(size_t index);
public:
    GifActivity();

    void loop();
};

#endif