/*---------------------------------------------------------*\
| DasKeyboardControllerDetect.cpp                           |
|                                                           |
|   Detector for Das Keyboard keyboard                      |
|                                                           |
|   Frank Niessen (denk_mal)                    16 Dec 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "DasKeyboardController.h"
#include "DasKeyboardX50Controller.h"
#include "RGBController_DasKeyboard.h"
#include "RGBController_DasKeyboardX50.h"
#include "LogManager.h"
#include <hidapi.h>
#include <libusb.h>

/*---------------------------------------------------------*\
| Das Keyboard vendor ID                                    |
\*---------------------------------------------------------*/
#define DAS_KEYBOARD_VID                0x24F0

/*---------------------------------------------------------*\
| Keyboard product IDs                                      |
\*---------------------------------------------------------*/
#define DAS_KEYBOARD_Q4_PID             0x2037
#define DAS_KEYBOARD_Q5_PID             0x2020
#define DAS_KEYBOARD_Q5S_PID            0x209A
#define DAS_KEYBOARD_X50_PID            0x202B

DetectedControllers DetectDasKeyboardControllers(hid_device_info *info, const std::string &name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        DasKeyboardController* controller = new DasKeyboardController(dev, info->path, name);

        if(controller->GetLayoutString() == "NONE")
        {
            delete controller;
        }
        else
        {
            RGBController_DasKeyboard* rgb_controller = new RGBController_DasKeyboard(controller);

            detected_controllers.push_back(rgb_controller);
        }
    }

    return(detected_controllers);
}

DetectedControllers DetectDasKeyboardX50()
{
    DetectedControllers detected_controllers;
    libusb_context* context = nullptr;
    libusb_device** devices = nullptr;
    int result = libusb_init(&context);

    if(result != LIBUSB_SUCCESS)
    {
        LOG_ERROR("[DasKeyboardX50] libusb_init failed: %s", libusb_error_name(result));
        return(detected_controllers);
    }

    ssize_t device_count = libusb_get_device_list(context, &devices);
    if(device_count < 0)
    {
        LOG_ERROR("[DasKeyboardX50] libusb_get_device_list failed: %s", libusb_error_name(static_cast<int>(device_count)));
        libusb_exit(context);
        return(detected_controllers);
    }
    LOG_INFO("[DasKeyboardX50] Scanning %zd USB devices", device_count);

    for(ssize_t device_idx = 0; device_idx < device_count; device_idx++)
    {
        libusb_device_descriptor descriptor;
        if(libusb_get_device_descriptor(devices[device_idx], &descriptor) != LIBUSB_SUCCESS ||
           descriptor.idVendor != DAS_KEYBOARD_VID || descriptor.idProduct != DAS_KEYBOARD_X50_PID)
        {
            continue;
        }

        LOG_INFO("[DasKeyboardX50] Found 24F0:202B on bus %u address %u", libusb_get_bus_number(devices[device_idx]), libusb_get_device_address(devices[device_idx]));

        libusb_device_handle* handle = nullptr;
        result = libusb_open(devices[device_idx], &handle);
        if(result != LIBUSB_SUCCESS)
        {
            LOG_ERROR("[DasKeyboardX50] libusb_open failed: %s", libusb_error_name(result));
            continue;
        }

        LOG_INFO("[DasKeyboardX50] Device opened; interface 2 claim deferred until the first color update");

        std::string path = std::to_string(libusb_get_bus_number(devices[device_idx])) + "-" +
                           std::to_string(libusb_get_device_address(devices[device_idx]));
        DasKeyboardX50Controller* controller = new DasKeyboardX50Controller(context, handle, path);
        detected_controllers.push_back(new RGBController_DasKeyboardX50(controller));
        context = nullptr;
        break;
    }

    if(devices != nullptr)
    {
        libusb_free_device_list(devices, 1);
    }

    if(context != nullptr)
    {
        libusb_exit(context);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Das Keyboard Q4 RGB",  DetectDasKeyboardControllers, DAS_KEYBOARD_VID, DAS_KEYBOARD_Q4_PID,  1,    0x01,   0x80);
REGISTER_HID_DETECTOR_I  ("Das Keyboard Q5 RGB",  DetectDasKeyboardControllers, DAS_KEYBOARD_VID, DAS_KEYBOARD_Q5_PID,  1);
REGISTER_HID_DETECTOR_I  ("Das Keyboard Q5S RGB", DetectDasKeyboardControllers, DAS_KEYBOARD_VID, DAS_KEYBOARD_Q5S_PID, 1);
REGISTER_DETECTOR        ("Das Keyboard X50 RGB", DetectDasKeyboardX50);

/*---------------------------------------------------------*\
| Udev rule metadata for the libusb-based X50 detector      |
| DUMMY_DEVICE_DETECTOR("Das Keyboard X50 RGB", DetectDasKeyboardX50, 0x24F0, 0x202B )                    |
\*---------------------------------------------------------*/
