#include "usb_activity.h"

#include "bsp/board_api.h"
#include "tusb.h"

// WARNING: MUST be included BEFORE mx_device/mx_sys as they redefine the __R symbol which breaks stl_tree.h
#include "e14toy_interchange.h"

#include "mxc_device.h"
#include "mxc_sys.h"
#include "usbhs_regs.h"

#include "led_display.h"

//------------------------------------------------------------------------------
// Wrapper functions to bridge TinyUSB BSP with MSDK BSP
//------------------------------------------------------------------------------
void board_init(void)
{
    // 1ms tick timer
//    SysTick_Config(SystemCoreClock / 1000);

    // Startup the HIRC96M clock if it's not on already
    if (!(MXC_GCR->clkcn & MXC_F_GCR_CLKCN_HIRC96M_EN)) {
        MXC_GCR->clkcn |= MXC_F_GCR_CLKCN_HIRC96M_EN;
    }

    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_USB);

    // allow High Speed USB and connect the D+/D- lines
    MXC_USBHS->power |= MXC_F_USBHS_POWER_HS_ENABLE | MXC_F_USBHS_POWER_SOFTCONN;
}

uint32_t board_millis(void)
{
    return millis();
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void send_cdc_reply(const void* buffer, uint32_t size)
{
    tud_cdc_write(buffer, size);
    tud_cdc_write_flush();
}

void cdc_task(void)
{
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
        // connected and there are data available
        if (tud_cdc_available())
        {
            // read buffer size
            uint32_t bufferSize;
            uint32_t count = tud_cdc_read(&bufferSize, sizeof(bufferSize));
            if (count == sizeof(bufferSize))
            {
                if (bufferSize < 5*1024)
                {
                    // read data
                    uint8_t buf[bufferSize];
                    uint32_t count = tud_cdc_read(buf, bufferSize);

                    if (count >= bufferSize)
                    {
                        auto message = e14toy::GetMessage(buf);

                        auto messageId = message->id();
                        switch (messageId)
                        {
                            case e14toy::MessageId::FullDisplay :
                            {
                                auto display = message->display();
                                auto content = display->content();

                                for (int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++)
                                {
                                    auto contentPixel = (*content)[ledIndex];
                                    Display::leds[ledIndex].setRGB(contentPixel->r(), contentPixel->g(), contentPixel->b());
                                }
                                Display::update();

                                break;
                            }
                            default:
                                printf("Unknown message id : %d", messageId);
                                break;
                        }

                        constexpr char const * reply = "Received buffer";
                        send_cdc_reply(reply, sizeof(reply));
                    }
                    else
                    {
                        constexpr char const * reply = "Buffer too small";
                        send_cdc_reply(reply, sizeof(reply));
                    }
                }
                else
                {
                    printf("unexpectedly large buffer (%d)", bufferSize);
                }
            }
            else
            {
                printf("Only read %d bytes instead of %d for count", count, sizeof(bufferSize));
                constexpr char const * reply = "Wrong count size in buffer";
                send_cdc_reply(reply, sizeof(reply));
            }
        }
    }
}

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
void USB_IRQHandler(void)
{
    tud_int_handler(0);
}

//--------------------------------------------------------------------+
// Activity
//--------------------------------------------------------------------+
const uint8_t usb_icon[8] =
{
    0b00010000,
    0b00111000,
    0b00010000,
    0b00010100,
    0b01010100,
    0b01011000,
    0b00110000,
    0b00010000
};

USBActivity::USBActivity()
{
    Display::displayOneBitImage(usb_icon, CRGB::Blue1);

    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    if (board_init_after_tusb)
        board_init_after_tusb();
}

void USBActivity::loop()
{
    tud_task(); // tinyusb device task

    cdc_task();
}

LoopDelegate USBActivity::getLoopDelegate()
{
    return LoopDelegate::from_method<USBActivity, &USBActivity::loop>(this);
}
