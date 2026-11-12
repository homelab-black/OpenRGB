/*---------------------------------------------------------*\
| CherryKeyboardControllerDetect.cpp                        |
|                                                           |
|   Detector for Cherry keyboard                            |
|                                                           |
|   Sebastian Kraus                             25 Dec 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "CherryKeyboardController.h"
#include "RGBController_CherryKeyboard.h"

/*---------------------------------------------------------*\
| Cherry keyboard VID and usage page                        |
\*---------------------------------------------------------*/
#define CHERRY_KEYBOARD_VID                     0x046A
#define CHERRY_KEYBOARD_USAGE_PAGE              0xFF1C

/*---------------------------------------------------------*\
| On Windows the vendor interface is split into two         |
| collections with the same usage page and usage: one with  |
| 64-byte reports (the lighting interface) and one with     |
| 9-byte reports. Only accept a device whose report         |
| descriptor has an output report large enough for a        |
| lighting packet. If the descriptor cannot be read, accept |
| the device as before.                                     |
\*---------------------------------------------------------*/
static bool CherryKeyboardHasLightingReport(hid_device* dev)
{
    unsigned char   descriptor[HID_API_MAX_REPORT_DESCRIPTOR_SIZE];
    int             descriptor_size;
    int             idx                 = 0;
    unsigned int    report_size         = 0;
    unsigned int    report_count        = 0;
    unsigned int    max_output_bytes    = 0;

    descriptor_size = hid_get_report_descriptor(dev, descriptor, sizeof(descriptor));

    if(descriptor_size <= 0)
    {
        return(true);
    }

    while(idx < descriptor_size)
    {
        unsigned char   prefix      = descriptor[idx];
        unsigned int    data_size   = prefix & 0x03;
        unsigned int    value       = 0;

        if(data_size == 3)
        {
            data_size = 4;
        }

        if(prefix == 0xFE)
        {
            /*---------------------------------------------*\
            | Long item: skip it                            |
            \*---------------------------------------------*/
            if(idx + 1 >= descriptor_size)
            {
                break;
            }

            idx += 3 + descriptor[idx + 1];
            continue;
        }

        if(idx + 1 + (int)data_size > descriptor_size)
        {
            break;
        }

        for(unsigned int byte_idx = 0; byte_idx < data_size; byte_idx++)
        {
            value |= descriptor[idx + 1 + byte_idx] << (8 * byte_idx);
        }

        switch(prefix & 0xFC)
        {
            case 0x74:  /* Report Size  */
                report_size = value;
                break;

            case 0x94:  /* Report Count */
                report_count = value;
                break;

            case 0x90:  /* Output       */
                if((report_size * report_count) / 8 > max_output_bytes)
                {
                    max_output_bytes = (report_size * report_count) / 8;
                }
                break;
        }

        idx += 1 + data_size;
    }

    return(max_output_bytes >= (CHERRY_KB_PACKET_SIZE - 1));
}

DetectedControllers DetectCherryKeyboards(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev && !CherryKeyboardHasLightingReport(dev))
    {
        hid_close(dev);
        dev = NULL;
    }

    if(dev)
    {
        CherryKeyboardController*     controller     = new CherryKeyboardController(dev, info->path, name);
        RGBController_CherryKeyboard* rgb_controller = new RGBController_CherryKeyboard(controller, info->product_id);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Keyboards                                                 |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 3.0S FL NBL",        DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_3_0S_FL_NBL_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 3.0S FL RGB",        DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_3_0S_FL_RGB_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 3.0S FL RGB KOREAN", DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_3_0S_FL_RGB_KOR_PID, 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX 1.0 FL BL",                DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_1_0_FL_BL_PID            , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 1.0 TKL RGB",        DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_1_0_TKL_RGB_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 8.0 TKL RGB",        DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_8_0_TKL_RGB_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 10.0 FL RGB",        DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_10_0_FL_RGB_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000 TKL NBL",            DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000_TKL_NBL_PID        , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 2.0S FL RGB (EU)",   DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_2_0S_FL_RGB_EU_PID , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 2.0S FL RGB (US)",   DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_2_0S_FL_RGB_US_PID , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 2.0S FL NBL",        DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_2_0S_FL_NBL_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000 TKL RGB",            DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000_TKL_RGB_PID        , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MV BOARD 3.0 FL RGB",         DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MV_BOARD_3_0FL_RGB_PID      , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard CCF MX 8.0 TKL BL",           DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   CCF_MX_8_0_TKL_BL_PID       , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard CCF MX 1.0 TKL BL",           DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   CCF_MX_1_0_TKL_BL_PID       , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard CCF MX 1.0 TKL NBL",          DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   CCF_MX_1_0_TKL_NBL_PID      , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000 TKL NBL KOREAN",     DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000_TKL_NBL_KOR_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX 1.0 FL NBL",               DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_1_0_FL_NBL_PID           , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX 1.0 FL RGB",               DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_1_0_FL_RGB_PID           , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000N TKL RGB (EU)",      DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000N_TKL_RGB_EU_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000N TKL RGB (US)",      DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000N_TKL_RGB_US_PID    , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000N FL RGB (EU)",       DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000N_FL_RGB_EU_PID     , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard G80-3000N FL RGB (US)",       DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   G80_3000N_FL_RGB_US_PID     , 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 10.0N FL RGB (EU)",  DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_10_0N_FL_RGB_EU_PID, 1, CHERRY_KEYBOARD_USAGE_PAGE);
REGISTER_HID_DETECTOR_IP("Cherry Keyboard MX BOARD 10.0N FL RGB (US)",  DetectCherryKeyboards, CHERRY_KEYBOARD_VID,   MX_BOARD_10_0N_FL_RGB_US_PID, 1, CHERRY_KEYBOARD_USAGE_PAGE);
