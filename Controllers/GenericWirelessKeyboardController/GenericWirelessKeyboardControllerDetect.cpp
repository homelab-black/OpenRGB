#include "Detector.h"
#include "GenericWirelessKeyboardController.h"
#include "RGBController_GenericWirelessKeyboard.h"
#include "LogManager.h"

#define GENERIC_KEYBOARD_VID 0x3554
#define GENERIC_KEYBOARD_PID 0xfa09

void DetectGenericWirelessKeyboard(hid_device_info *info, const std::string &name)
{
    hid_device *dev = hid_open_path(info->path);

    if (dev)
    {
        GenericWirelessKeyboardController *controller = new GenericWirelessKeyboardController(dev, info->path, name);
        RGBController_GenericWirelessKeyboard *rgb_controller = new RGBController_GenericWirelessKeyboard(controller);
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

REGISTER_HID_DETECTOR_IPU("Generic Wireless Keyboard", DetectGenericWirelessKeyboard, GENERIC_KEYBOARD_VID, GENERIC_KEYBOARD_PID, 1, 0xff02, 0x0002);
