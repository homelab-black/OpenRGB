#include "Detector.h"
#include "SinowealthWirelessFullColorDevices.h"
#include "SinowealthWirelessKeyboardController.h"
#include "LogManager.h"
#include "RGBController_SinowealthWirelessKeyboardSolid.h"
#include "RGBController_SinowealthWirelessKeyboardFullColor.h"

#define SINOWEALTH_KEYBOARD_VID 0x3554
#define SINOWEALTH_KEYBOARD_PID 0xfa09
#define SINOWEALTH_KEYBOARD_PAGE 0xff02
#define SINOWEALTH_KEYBOARD_USAGE 0x0002
#define SINOWEALTH_KEYBOARD_PACKET_SIZE 20

#define MODEL_AULA_F99 0xa4
#define MODEL_AULA_F75 0xcd

unsigned char GetModelId(hid_device* dev)
{
    unsigned char usb_buf[SINOWEALTH_KEYBOARD_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, SINOWEALTH_KEYBOARD_PACKET_SIZE);

    /*-----------------------------------------------------*\
    | Set up config request packet                          |
    \*-----------------------------------------------------*/
    usb_buf[0]  = 0x13;
    usb_buf[1]  = 0x05;
    usb_buf[2]  = 0x01;
    usb_buf[19] = 0x19;

    int bytes_read = 0;
    do
    {
        hid_write(dev, usb_buf, SINOWEALTH_KEYBOARD_PACKET_SIZE);
        bytes_read = hid_read_timeout(dev, usb_buf, SINOWEALTH_KEYBOARD_PACKET_SIZE, 100);
    } while(bytes_read <= 0);

    return usb_buf[10];
}

void DetectSinowealthWirelessKeyboard(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        unsigned char model_id = GetModelId(dev);

        if(sinowealth_wireless_keyboards.find(model_id) != sinowealth_wireless_keyboards.end())
        {
            SinowealthWirelessKeyboardController* controller = new SinowealthWirelessKeyboardController(
                dev, info->path, sinowealth_wireless_keyboards.at(model_id).device_name);

            RGBController_SinowealthWirelessKeyboardFullColor* rgb_controller =
                new RGBController_SinowealthWirelessKeyboardFullColor(controller, model_id);
            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
        else
        {
            LOG_WARNING("Model ID 0x%02x not recognized. Using solid color mode.", model_id);
            SinowealthWirelessKeyboardController* controller =
                new SinowealthWirelessKeyboardController(dev, info->path, name);

            RGBController_SinowealthWirelessKeyboardSolid* rgb_controller =
                new RGBController_SinowealthWirelessKeyboardSolid(controller);
            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
    }
}

REGISTER_HID_DETECTOR_IPU("Sinowealth Wireless Keyboard", DetectSinowealthWirelessKeyboard, SINOWEALTH_KEYBOARD_VID,
                          SINOWEALTH_KEYBOARD_PID, 1, SINOWEALTH_KEYBOARD_PAGE, SINOWEALTH_KEYBOARD_USAGE);
