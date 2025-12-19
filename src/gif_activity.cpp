#include "gif_activity.h"
#include "led_display.h"

extern "C"
{
    #include "sdhc_lib.h"
}

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

const TCHAR* GIF_ERRORS[] =
{
   "GIF_SUCCESS",
   "GIF_DECODE_ERROR",
   "GIF_TOO_WIDE",
   "GIF_INVALID_PARAMETER",
   "GIF_UNSUPPORTED_FEATURE",
   "GIF_FILE_NOT_OPEN",
   "GIF_EARLY_EOF",
   "GIF_EMPTY_FRAME",
   "GIF_BAD_FILE",
   "GIF_ERROR_MEMORY"
};

bool GifActivity::initSDCard()
{
    // Enable Power To Card
    mxc_gpio_cfg_t SDPowerEnablePin = { MXC_GPIO1, MXC_GPIO_PIN_6, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE,
                                        MXC_GPIO_VSSEL_VDDIO };
    MXC_GPIO_Config(&SDPowerEnablePin);
    MXC_GPIO_OutClr(SDPowerEnablePin.port, SDPowerEnablePin.mask);

    // Initialize SDHC peripheral
    mxc_sdhc_cfg_t cfg = {};
    cfg.bus_voltage = MXC_SDHC_Bus_Voltage_3_3;
    cfg.block_gap = 0;
    cfg.clk_div = 0x0b0; // Maximum divide ratio, frequency must be >= 400 kHz during Card Identification phase
    if (MXC_SDHC_Init(&cfg) != E_NO_ERROR) {
        printf("Unable to initialize SDHC driver.\n");
        return false;
    }

    MXC_GPIO_Config(&SDPowerEnablePin);
    MXC_GPIO_OutClr(SDPowerEnablePin.port, SDPowerEnablePin.mask);

    // wait for card to be inserted
    while (!MXC_SDHC_Card_Inserted()) {}
    printf("Card inserted.\n");

    // set up card to get it ready for a transaction
    if (MXC_SDHC_Lib_InitCard(10) == E_NO_ERROR)
    {
        printf("Card Initialized.\n");
    }
    else
    {
        printf("No card response! Remove card, reset EvKit, and try again.\n");
        return false;
    }

    if (MXC_SDHC_Lib_Get_Card_Type() == CARD_SDHC)
        printf("Card type: SDHC\n");
    else
        printf("Card type: MMC/eMMC\n");

    return true;
}

FRESULT GifActivity::listFiles()
{
    FRESULT err; //FFat Result (Struct)
    DIR dir; //FFat Directory Object
    FILINFO fno; //FFat File Information Object

    if ((err = f_opendir(&dir, "/gif")) == FR_OK)
    {
        while (1)
        {
            err = f_readdir(&dir, &fno);
            if (err != FR_OK || fno.fname[0] == 0)
                break;

            printf("/gif/%s", fno.fname);

            files.push_back(fno.fname);

            if (fno.fattrib & AM_DIR)
            {
                printf("/");
            }

            printf("\n");
        }
        f_closedir(&dir);
        return FR_OK;
    }
    else
    {
        printf("Error opening directory: %s\n", FF_ERRORS[err]);
        return err;
    }
}

FRESULT GifActivity::mount(void)
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

    return err;
}

static void* GIFOpenFile(const char *fname, int32_t *pSize)
{
    FRESULT err;
    FIL* file = new FIL();
    if ((err = f_open(file, fname, FA_OPEN_EXISTING | FA_READ)) != FR_OK)
    {
        printf("Error opening file %s: %s\n", fname, FF_ERRORS[err]);
        return NULL;
    }
    *pSize = f_size(file);
    return file;
}

static void GIFCloseFile(void *pHandle)
{
    FIL *file = static_cast<FIL *>(pHandle);
    if (file != NULL)
    {
        f_close(file);
        delete file;
    }
}

static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    UINT iBytesRead;
    iBytesRead = iLen;
    FIL *file = static_cast<FIL *>(pFile->fHandle);

    // Note: If you read a file all the way to the last byte, seek() stops working
    /*if ((pFile->iSize - pFile->iPos) < iLen)
        iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around*/
    if (iBytesRead <= 0)
        return 0;

    FRESULT err;
    if ((err = f_read(file, pBuf, iLen, &iBytesRead)) != FR_OK) {
        printf("Error reading file: %s\n", FF_ERRORS[err]);
        return 0;
    }

    return iBytesRead;
}

static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
    FIL *file = static_cast<FIL *>(pFile->fHandle);

    f_lseek(file, iPosition);

    pFile->iPos = f_tell(file);
    return pFile->iPos;
}

static void GIFDraw(GIFDRAW *pDraw)
{
}

bool GifActivity::loadFile(size_t index)
{
    if (index > files.size())
        return false;

    if (gif.open(("/gif/" + files[index]).c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
    {
        if ((gif.getCanvasWidth() != Display::Width) && (gif.getCanvasHeight() != Display::Height))
        {
            printf("GIF is not of appropriate size\n");
            gif.close();
            return false;
        }
    }
    else
    {
        printf("Could not open gif %s: %s\n", files[index].c_str(), GIF_ERRORS[gif.getLastError()]);
        return false;
    }

    return true;
}

const uint8_t SDImage[] =
{
    0b01101100,
    0b10001010,
    0b10001001,
    0b10001001,
    0b01001001,
    0b00101001,
    0b00101010,
    0b11001100,
};

GifActivity::GifActivity()
{
    Display::displayOneBitImage(SDImage, 0xF0F000);

    if (!initSDCard())
    {
        Display::displayOneBitImage(SDImage, 0xF00000);
        return;
    }

    if (mount() != FR_OK)
    {
        Display::displayOneBitImage(SDImage, 0xF0F000);
        return;
    }

    if (listFiles() != FR_OK)
    {
        Display::displayOneBitImage(SDImage, 0xF08000);
        return;
    }

    gif.begin(GIF_PALETTE_RGB888);

    loadFile(currentFileIndex);

    Display::displayOneBitImage(SDImage, 0x008000);
}

void GifActivity::loop()
{
    if (gif.getLastError() == GIF_SUCCESS)
    {
        gif.setFrameBuf(Display::leds);
        gif.setDrawType(GIF_DRAW_COOKED);

        int frameDelay;
        if (gif.playFrame(true, &frameDelay))
            Display::update();
    }
    else
    {
        currentFileIndex++;
        loadFile(currentFileIndex);
    }
}