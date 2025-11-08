/*---------------------------------------------------------*\
| CMARGBStripDevices.cpp                                    |
|                                                           |
|   Device list for Cooler Master ARGB light strip devices  |
|                                                           |
|   Logan Phillips (Eclipse)                    05 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "CMARGBStripDevices.h"

/*-----------------------------------------------------------------*\
| Cooler Master ARGB Strip Device List                              |
|                                                                   |
| This list contains all known devices that use the                 |
| Cooler Master ARGB light strip protocol.                          |
|                                                                   |
| To add a new device:                                              |
| 1. Add REGISTER_HID_DETECTOR in CoolerMasterControllerDetect.cpp  |
| 2. Add definitions for the device here                            |
\*-----------------------------------------------------------------*/

/*-----------------------------------------------------*\
| Strip ID arrays for specific devices                  |
\*-----------------------------------------------------*/

static const uint8_t gd160_strip_ids[]    = { 1, 2 };
static const uint8_t gm27_strip_ids[]     = { 2 };

const cm_argb_strip_device cm_argb_strip_devices[] =
{
    /*--------------------------------------------------------------------------------------------------------------------------*\
    | PID                         Name                                    LEDs  Strips StripIDs           Type                   |
    \*--------------------------------------------------------------------------------------------------------------------------*/
    { COOLERMASTER_GD160_PID,     "Cooler Master GD160 ARGB Gaming Desk", 96,   2,     gd160_strip_ids,   DEVICE_TYPE_ACCESSORY },
    { COOLERMASTER_GM27_FQS_PID,  "Cooler Master GM27-FQS ARGB Monitor",  47,   1,     gm27_strip_ids,    DEVICE_TYPE_MONITOR   },
};

const unsigned int CM_ARGB_STRIP_DEVICE_COUNT = sizeof(cm_argb_strip_devices) / sizeof(cm_argb_strip_devices[0]);
