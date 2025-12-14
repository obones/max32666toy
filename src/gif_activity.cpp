#include "gif_activity.h"
#include "sdhc_lib.h"

const TCHAR* FF_ERRORS[] =
{
    "FR_OK",
    "FR_DISK_ERR",
    "FR_INT_ERR",
    "FR_NOT_READY",
    "FR_NO_FILE",
    "FR_NO_PATH",
    "FR_INVALID_NAME",
    "FR_DENIED",
    "FR_EXIST",
    "FR_INVALID_OBJECT",
    "FR_WRITE_PROTECTED",
    "FR_INVALID_DRIVE",
    "FR_NOT_ENABLED",
    "FR_NO_FILESYSTEM",
    "FR_MKFS_ABORTED",
    "FR_TIMEOUT",
    "FR_LOCKED",
    "FR_NOT_ENOUGH_CORE",
    "FR_TOO_MANY_OPEN_FILES",
    "FR_INVALID_PARAMETER"
};

int GifActivity::mount(void)
{
    FRESULT err;
    if ((err = f_mount(&fs, "", 1)) != FR_OK) //Mount the default drive to fs now
    {
        printf("Error opening SD card: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
    }
    else
    {
        printf("SD card mounted.\n");
        mounted = true;
    }

    f_getcwd(cwd, sizeof(cwd)); //Set the Current working directory

    return err;
}

GifActivity::GifActivity()
{
    mount();
}

void GifActivity::loop()
{

}