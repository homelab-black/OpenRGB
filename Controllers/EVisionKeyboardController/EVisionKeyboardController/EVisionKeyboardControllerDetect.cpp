/*---------------------------------------------------------*\
| EVisionKeyboardControllerDetect.cpp                       |
|                                                           |
|   Detector for EVision/Huafenda keyboards                 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "EVisionKeyboardController.h"
#include "RGBController_EVisionKeyboard.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

/*---------------------------------------------------------*\
| EVision vendor ID                                         |
|   NOTE: EVision (and Huafenda) use rebranded Sonix MCUs   |
|   Some EVision/Huafenda keyboards use the Sonix VID while |
|   others use EVision's VID                                |
\*---------------------------------------------------------*/
#define SONIX_VID                                   0x0C45
#define EVISION_VID                                 0x320F

/*---------------------------------------------------------*\
| EVision Usage Page                                        |
\*---------------------------------------------------------*/
#define EVISION_KEYBOARD_USAGE_PAGE                 0xFF1C

/*---------------------------------------------------------*\
| Keyboard product IDs                                      |
\*---------------------------------------------------------*/
#define EVISION_SONIX_5004_PID                      0x5004
#define EVISION_SONIX_5104_PID                      0x5104
#define EVISION_SONIX_5204_PID                      0x5204
#define EVISION_SONIX_652F_PID                      0x652F
#define EVISION_SONIX_7672_PID                      0x7672
#define EVISION_SONIX_7698_PID                      0x7698
#define EVISION_SONIX_8520_PID                      0x8520
#define EVISION_5000_PID                            0x5000
#define EVISION_502A_PID                            0x502A
#define EVISION_505B_PID                            0x505B
#define EVISION_5064_PID                            0x5064
#define EVISION_5078_PID                            0x5078
#define EVISION_5084_PID                            0x5084

/*---------------------------------------------------------*\
| 320F:5000 firmware revisions known to accept the RAM-only |
| dynamic color command (0x12).  0x0103 is the K551RGB-1 V2 |
| revision documented by community research; 0x0104 is the  |
| K552RGB-1 V2 / Kumara-class board verified in this change.|
\*---------------------------------------------------------*/
#define EVISION_5000_DIRECT_RELEASE_0103            0x0103
#define EVISION_5000_DIRECT_RELEASE_0104            0x0104

DetectedControllers DetectEVisionKeyboards(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        bool direct_supported =
            (info->vendor_id == EVISION_VID)
            && (info->product_id == EVISION_5000_PID)
            && ((info->release_number == EVISION_5000_DIRECT_RELEASE_0103)
             || (info->release_number == EVISION_5000_DIRECT_RELEASE_0104));

        EVisionKeyboardController*     controller     = new EVisionKeyboardController(dev, info->path, name, direct_supported);
        RGBController_EVisionKeyboard* rgb_controller = new RGBController_EVisionKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Keyboards                                                 |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:5004",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_5004_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:5104",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_5104_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:5204",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_5204_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:652F",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_652F_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:7672",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_7672_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:7698",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_7698_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 0C45:8520",  DetectEVisionKeyboards, SONIX_VID,      EVISION_SONIX_8520_PID, 1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:5000",  DetectEVisionKeyboards, EVISION_VID,    EVISION_5000_PID,       1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:502A",  DetectEVisionKeyboards, EVISION_VID,    EVISION_502A_PID,       1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:505B",  DetectEVisionKeyboards, EVISION_VID,    EVISION_505B_PID,       1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:5078",  DetectEVisionKeyboards, EVISION_VID,    EVISION_5078_PID,       1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:5064",  DetectEVisionKeyboards, EVISION_VID,    EVISION_5064_PID,       1, EVISION_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("EVision Keyboard 320F:5084",  DetectEVisionKeyboards, EVISION_VID,    EVISION_5084_PID,       1, EVISION_KEYBOARD_USAGE_PAGE);
