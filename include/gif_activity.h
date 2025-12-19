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
private:
    std::vector<std::string> files;
    FATFS fs;
    AnimatedGIF gif;
    int currentFileIndex = 0;

    bool mounted;

    int initSDCard();
    int mount();
    int listFiles();
    bool loadFile(int index);
public:
    GifActivity();

    void loop();
};

#endif