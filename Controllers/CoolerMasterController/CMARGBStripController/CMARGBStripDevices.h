/*---------------------------------------------------------*\
| CMARGBStripDevices.h                                      |
|                                                           |
|   Device list for Cooler Master ARGB light strip devices  |
|                                                           |
|   Logan Phillips (Eclipse)                    05 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include <stdint.h>

/*-----------------------------------------------------*\
| Product IDs                                           |
\*-----------------------------------------------------*/
#define COOLERMASTER_GM27_FQS_PID           0x01BB
#define COOLERMASTER_GD160_PID              0x01A9

/*-----------------------------------------------------*\
| Device configuration structure                        |
\*-----------------------------------------------------*/
typedef struct
{
    uint16_t        product_id;
    const char*     name;
    unsigned int    leds_per_strip;
    unsigned int    num_strips;
    const uint8_t*  strip_ids;
    device_type     type;
} cm_argb_strip_device;

/*-----------------------------------------------------*\
| Device list                                           |
\*-----------------------------------------------------*/
extern const unsigned int CM_ARGB_STRIP_DEVICE_COUNT;
extern const cm_argb_strip_device cm_argb_strip_devices[];
