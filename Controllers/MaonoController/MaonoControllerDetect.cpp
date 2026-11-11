/*---------------------------------------------------------*\
| MaonoControllerDetect.cpp                                 |
|                                                           |
|   Detector for Maono microphones                          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "MaonoController.h"
#include "RGBController_Maono.h"

/*---------------------------------------------------------*\
| Maono product IDs (Maono Link 3 MaonoDevicesInfo.txt).    |
| Products without controllable RGB are not listed:         |
| PD300X, A04 Gen2, PD100 Gen2, PD500X, PD500W, PM450 Gen2. |
\*---------------------------------------------------------*/
#define MAONO_PD200X_PID                 0x0104
#define MAONO_PD100X_PID                 0x010C
#define MAONO_PD100X_RGB_PID             0x0108
#define MAONO_PD100X_LITE_PID            0x0109
#define MAONO_DGM20_OLD_PID              0x0105
#define MAONO_DGM20_PID                  0x0112
#define MAONO_DGM20G_PID                 0x0113
#define MAONO_PD200W_PID                 0x0411
#define MAONO_PD200W_RX1_PID             0x040E
#define MAONO_PD200W_RX2_PID             0x040F
#define MAONO_PD200W_RX3_PID             0x0410
#define MAONO_PD100W_PID                 0x0417
#define MAONO_PD100W_RX_PID              0x0414
#define MAONO_PD100W_LIGHTNING_PID       0x0415
#define MAONO_PD100W_CRX_PID             0x0416
#define MAONO_DGM20W_PID                 0x041D
#define MAONO_DGM20W_RX_PID              0x041A
#define MAONO_DGM20W_LIGHTNING_PID       0x041B
#define MAONO_DGM20W_CRX_PID             0x041C
#define MAONO_T1_MINI_PRO_1_PID          0x0418
#define MAONO_T1_MINI_PRO_2_PID          0x0419
#define MAONO_DM40_PRO_1_PID             0x0405
#define MAONO_DM40_PRO_2_PID             0x0406
#define MAONO_DM40_1_PID                 0x040C
#define MAONO_DM40_2_PID                 0x040D
#define MAONO_DM40_LIVE_1_PID            0x0412
#define MAONO_DM40_LIVE_2_PID            0x0413
#define MAONO_DM40_LIVE_3_PID            0x0420
#define MAONO_DM40_LIVE_4_PID            0x0421
#define MAONO_DM40_LIVE_5_PID            0x0422
#define MAONO_DM40_LIVE_GEN2_PID         0x0425
#define MAONO_DM40_LIVE_GEN2_RX1_PID     0x0424
#define MAONO_DM40_LIVE_GEN2_RX2_PID     0x0423

/*---------------------------------------------------------*\
| The microphones also expose audio-control HID collections |
| (consumer page). When the same product has a vendor       |
| defined collection (usage page 0xFFxx), only that one is  |
| opened, so each microphone appears once in OpenRGB.       |
\*---------------------------------------------------------*/
static bool MaonoHasVendorCollection(unsigned short pid)
{
    bool             found = false;
    hid_device_info* list  = hid_enumerate(MAONO_VID, pid);

    for(hid_device_info* it = list; it != nullptr; it = it->next)
    {
        if(it->usage_page >= 0xFF00)
        {
            found = true;
        }
    }

    hid_free_enumeration(list);

    return(found);
}

DetectedControllers DetectMaonoControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers     detected_controllers;
    const MaonoDeviceInfo*  device_info = MaonoFindDevice(info->product_id);

    if(device_info == nullptr)
    {
        return(detected_controllers);
    }

    if(info->usage_page < 0xFF00 && MaonoHasVendorCollection(info->product_id))
    {
        return(detected_controllers);
    }

    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        MaonoController*     controller     = new MaonoController(dev, info->path, name, device_info);
        RGBController_Maono* rgb_controller = new RGBController_Maono(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR("Maono PD200X",                    DetectMaonoControllers, MAONO_VID, MAONO_PD200X_PID);
REGISTER_HID_DETECTOR("Maono PD100X",                    DetectMaonoControllers, MAONO_VID, MAONO_PD100X_PID);
REGISTER_HID_DETECTOR("Maono PD100X RGB",                DetectMaonoControllers, MAONO_VID, MAONO_PD100X_RGB_PID);
REGISTER_HID_DETECTOR("Maono PD100X Lite",               DetectMaonoControllers, MAONO_VID, MAONO_PD100X_LITE_PID);
REGISTER_HID_DETECTOR("Maono DGM20",                     DetectMaonoControllers, MAONO_VID, MAONO_DGM20_OLD_PID);
REGISTER_HID_DETECTOR("Maono DGM20",                     DetectMaonoControllers, MAONO_VID, MAONO_DGM20_PID);
REGISTER_HID_DETECTOR("Maono DGM20G",                    DetectMaonoControllers, MAONO_VID, MAONO_DGM20G_PID);
REGISTER_HID_DETECTOR("Maono PD200W",                    DetectMaonoControllers, MAONO_VID, MAONO_PD200W_PID);
REGISTER_HID_DETECTOR("Maono PD200W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_PD200W_RX1_PID);
REGISTER_HID_DETECTOR("Maono PD200W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_PD200W_RX2_PID);
REGISTER_HID_DETECTOR("Maono PD200W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_PD200W_RX3_PID);
REGISTER_HID_DETECTOR("Maono PD100W",                    DetectMaonoControllers, MAONO_VID, MAONO_PD100W_PID);
REGISTER_HID_DETECTOR("Maono PD100W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_PD100W_RX_PID);
REGISTER_HID_DETECTOR("Maono PD100W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_PD100W_LIGHTNING_PID);
REGISTER_HID_DETECTOR("Maono PD100W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_PD100W_CRX_PID);
REGISTER_HID_DETECTOR("Maono DGM20W",                    DetectMaonoControllers, MAONO_VID, MAONO_DGM20W_PID);
REGISTER_HID_DETECTOR("Maono DGM20W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_DGM20W_RX_PID);
REGISTER_HID_DETECTOR("Maono DGM20W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_DGM20W_LIGHTNING_PID);
REGISTER_HID_DETECTOR("Maono DGM20W Receiver",           DetectMaonoControllers, MAONO_VID, MAONO_DGM20W_CRX_PID);
REGISTER_HID_DETECTOR("Maono Wave T1 Mini Pro",          DetectMaonoControllers, MAONO_VID, MAONO_T1_MINI_PRO_1_PID);
REGISTER_HID_DETECTOR("Maono Wave T1 Mini Pro",          DetectMaonoControllers, MAONO_VID, MAONO_T1_MINI_PRO_2_PID);
REGISTER_HID_DETECTOR("Maono DM40 Pro",                  DetectMaonoControllers, MAONO_VID, MAONO_DM40_PRO_1_PID);
REGISTER_HID_DETECTOR("Maono DM40 Pro",                  DetectMaonoControllers, MAONO_VID, MAONO_DM40_PRO_2_PID);
REGISTER_HID_DETECTOR("Maono DM40",                      DetectMaonoControllers, MAONO_VID, MAONO_DM40_1_PID);
REGISTER_HID_DETECTOR("Maono DM40",                      DetectMaonoControllers, MAONO_VID, MAONO_DM40_2_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live",                 DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_1_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live",                 DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_2_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live",                 DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_3_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live",                 DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_4_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live",                 DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_5_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live Gen2",            DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_GEN2_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live Gen2 Receiver",   DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_GEN2_RX1_PID);
REGISTER_HID_DETECTOR("Maono DM40 Live Gen2 Receiver",   DetectMaonoControllers, MAONO_VID, MAONO_DM40_LIVE_GEN2_RX2_PID);
