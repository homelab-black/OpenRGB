#include <hidapi.h>
#include "DetectionManager.h"
#include "RGBController_ThermaltakeX1RGB.h"
#include "ThermaltakeX1RGBController.h"

#define THERMALTAKE_X1_VID 0x264A
#define THERMALTAKE_X1_PID 0x3013

DetectedControllers DetectThermaltakeX1Controllers(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        ThermaltakeX1RGBController* controller = new ThermaltakeX1RGBController(dev, info->path);
        detected_controllers.push_back(new RGBController_ThermaltakeX1RGB(controller));
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Thermaltake TT Premium X1 RGB", DetectThermaltakeX1Controllers,
                          THERMALTAKE_X1_VID, THERMALTAKE_X1_PID, 1, 0xFF00, 0x01);
