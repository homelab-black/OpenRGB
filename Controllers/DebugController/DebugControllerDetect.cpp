/*---------------------------------------------------------*\
| DebugControllerDetect.cpp                                 |
|                                                           |
|   Detector for debug devices                              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include "Detector.h"
#include "RGBController.h"
#include "RGBController_Debug.h"
#include "SettingsManager.h"

/******************************************************************************************\
*                                                                                          *
*   DetectDebugControllers                                                                 *
*                                                                                          *
*       Add debug controllers based on the DebugDevices key in the settings json           *
*                                                                                          *
\******************************************************************************************/

void DetectDebugControllers()
{
    json            debug_settings;

    /*-------------------------------------------------*\
    | Get Debug Device settings from settings manager   |
    \*-------------------------------------------------*/
    debug_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("DebugDevices");

    /*-------------------------------------------------*\
    | If the Debug settings contains devices, process   |
    \*-------------------------------------------------*/
    if(debug_settings.contains("devices"))
    {
        for(unsigned int device_idx = 0; device_idx < debug_settings["devices"].size(); device_idx++)
        {
            RGBController_Debug * debug_controller = new RGBController_Debug(false, debug_settings["devices"][device_idx]);
            ResourceManager::get()->RegisterRGBController(debug_controller);
        }
    }

//    if (debug_settings.contains("CustomDevices"))
//    {
//        for (int CustomDevice = 0; CustomDevice < (int)debug_settings["CustomDevices"].size(); CustomDevice++)
//        {
//            json CustomDev = debug_settings["CustomDevices"][CustomDevice];
//            /*---------------------------*\
//            | Create a custom controller  |
//            \*---------------------------*/
//            RGBController_Debug* debug_custom = new RGBController_Debug();
//
//            /*----------------------------------------------------------------------*\
//            | if ANY of the attributes are missing then go ahead and skip the entry  |
//            \*----------------------------------------------------------------------*/
//            if (
//                    !CustomDev.contains("DeviceName")        ||
//                    !CustomDev.contains("DeviceType")        ||
//                    !CustomDev.contains("DeviceDescription") ||
//                    !CustomDev.contains("DeviceLocation")    ||
//                    !CustomDev.contains("DeviceVersion")     ||
//                    !CustomDev.contains("DeviceSerial")      ||
//                    !CustomDev.contains("DeviceZones")
//                    )
//            {
//                continue;
//            }
//
//
//            /*-------------*\
//            | Set the name  |
//            \*-------------*/
//            debug_custom->name = CustomDev["DeviceName"];
//
//            /*---------------------*\
//            | Find the device type  |
//            \*---------------------*/
//            if      (CustomDev["DeviceType"] == "motherboard")   debug_custom->type = DEVICE_TYPE_MOTHERBOARD;
//            else if (CustomDev["DeviceType"] == "dram")          debug_custom->type = DEVICE_TYPE_DRAM;
//            else if (CustomDev["DeviceType"] == "gpu")           debug_custom->type = DEVICE_TYPE_GPU;
//            else if (CustomDev["DeviceType"] == "cooler")        debug_custom->type = DEVICE_TYPE_COOLER;
//            else if (CustomDev["DeviceType"] == "led_strip")     debug_custom->type = DEVICE_TYPE_LEDSTRIP;
//            else if (CustomDev["DeviceType"] == "keyboard")      debug_custom->type = DEVICE_TYPE_KEYBOARD;
//            else if (CustomDev["DeviceType"] == "mouse")         debug_custom->type = DEVICE_TYPE_MOUSE;
//            else if (CustomDev["DeviceType"] == "mousemat")      debug_custom->type = DEVICE_TYPE_MOUSEMAT;
//            else if (CustomDev["DeviceType"] == "headset")       debug_custom->type = DEVICE_TYPE_HEADSET;
//            else if (CustomDev["DeviceType"] == "headset_stand") debug_custom->type = DEVICE_TYPE_HEADSET_STAND;
//            else if (CustomDev["DeviceType"] == "gamepad")       debug_custom->type = DEVICE_TYPE_GAMEPAD;
//            else if (CustomDev["DeviceType"] == "light")         debug_custom->type = DEVICE_TYPE_LIGHT;
//            else if (CustomDev["DeviceType"] == "speaker")       debug_custom->type = DEVICE_TYPE_SPEAKER;
//            else if (CustomDev["DeviceType"] == "unknown")       debug_custom->type = DEVICE_TYPE_UNKNOWN;
//
//            /*-----------------------------------------------*\
//            | Set description, location, version, and serial  |
//            \*-----------------------------------------------*/
//            debug_custom->description = CustomDev["DeviceDescription"];
//            debug_custom->location    = CustomDev["DeviceLocation"];
//            debug_custom->version     = CustomDev["DeviceVersion"];
//            debug_custom->serial      = CustomDev["DeviceSerial"];
//
//            /*----------------*\
//            | Create the mode  |
//            \*----------------*/
//            mode debug_custom_direct;
//            debug_custom_direct.name                 = "Direct";
//            debug_custom_direct.value                = 0;
//            debug_custom_direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
//            debug_custom_direct.color_mode           = MODE_COLORS_PER_LED;
//            debug_custom->modes.push_back(debug_custom_direct);
//
//            /*--------------*\
//            | Fill in zones  |
//            \*--------------*/
//            for (int ZoneID = 0; ZoneID < (int)CustomDev["DeviceZones"].size(); ZoneID++)
//            {
//                json ZoneJson = CustomDev["DeviceZones"][ZoneID];
//                if
//                (
//                    !ZoneJson.contains("name")       ||
//                    !ZoneJson.contains("type")       ||
//                    !ZoneJson.contains("leds_min")   ||
//                    !ZoneJson.contains("leds_max")   ||
//                    !ZoneJson.contains("leds_count")
//                )
//                {
//                    continue;
//                }
//                zone custom_zone;
//
//                custom_zone.name = ZoneJson["name"];
//
//                if      (ZoneJson["type"] == "linear") custom_zone.type = ZONE_TYPE_LINEAR;
//                else if (ZoneJson["type"] == "matrix") custom_zone.type = ZONE_TYPE_MATRIX;
//                else if (ZoneJson["type"] == "single") custom_zone.type = ZONE_TYPE_SINGLE;
//                else
//                {
//                    continue;
//                }
//
//                custom_zone.leds_min   = ZoneJson["leds_min"];
//                custom_zone.leds_max   = ZoneJson["leds_max"];
//                custom_zone.leds_count = ZoneJson["leds_count"];
//
//                /*-----------------------*\
//                | Fill in the matrix map  |
//                \*-----------------------*/
//                bool BadVal = false;
//                if (custom_zone.type == ZONE_TYPE_MATRIX)
//                {
//                    if
//                    (
//                        !ZoneJson.contains("matrix_height") ||
//                        !ZoneJson.contains("matrix_width")  ||
//                        !ZoneJson.contains("matrix_map")
//                    )
//                    {
//                        /*--------------------------------------------------------------*\
//                        | If there is no map then the zone can't be valid. Don't add it  |
//                        \*--------------------------------------------------------------*/
//                        continue;
//                    }
//
//                    custom_zone.matrix_map = new matrix_map_type;
//
//                    custom_zone.matrix_map->width  = ZoneJson["matrix_width"];
//                    custom_zone.matrix_map->height = ZoneJson["matrix_height"];
//
//                    int H = custom_zone.matrix_map->height;
//                    int W = custom_zone.matrix_map->width;
//
//                    BadVal = (ZoneJson["matrix_map"].size() != custom_zone.matrix_map->height);
//
//                    unsigned int* MatrixARR = new unsigned int[H * W];
//                    for (int MatrixMapRow = 0; MatrixMapRow < H; MatrixMapRow++)
//                    {
//                        /*-----------------------------------------------------------------------------------------------------*\
//                        | If something went wrong then make no attempt to recover and just move on in a way that doesn't crash  |
//                        | Even 1 bad row can corrupt the map so skip the zone entirely                                          |
//                        \*-----------------------------------------------------------------------------------------------------*/
//                        if ((custom_zone.matrix_map->width != ZoneJson["matrix_map"][MatrixMapRow].size()) || BadVal)
//                        {
//                            BadVal = true;
//                            break;
//                        }
//
//                        for (int MatrixMapCol = 0; MatrixMapCol < W; MatrixMapCol++)
//                        {
//                            int Val = ZoneJson["matrix_map"][MatrixMapRow][MatrixMapCol];
//
//                            if ((signed)Val == -1)
//                            {
//                                MatrixARR[MatrixMapRow * W + MatrixMapCol] = NA;
//                            }
//                            else
//                            {
//                                MatrixARR[MatrixMapRow * W + MatrixMapCol] = (unsigned)Val;
//                            }
//                        }
//                    }
//
//                    custom_zone.matrix_map->map = MatrixARR;
//                }
//
//                /*------------------------------------*\
//                | Don't add the zone if it is invalid  |
//                \*------------------------------------*/
//                if (BadVal)
//                {
//                    continue;
//                }
//
//                bool UseCustomLabels = false;
//                if (ZoneJson.contains("custom_labels"))
//                {
//                    /*-------------------------------------------------------*\
//                    | If the count is correct and the zone is non-resizeable  |
//                    \*-------------------------------------------------------*/
//                    if ((ZoneJson["custom_labels"].size() == custom_zone.leds_count) && (custom_zone.leds_min == custom_zone.leds_max))
//                    {
//                        UseCustomLabels = true;
//                    }
//                }
//
//                /*------------------*\
//                | Set the LED names  |
//                \*------------------*/
//                for (int LED_ID = 0; LED_ID < (int)custom_zone.leds_count; LED_ID++)
//                {
//                    led custom_led;
//                    if (UseCustomLabels)
//                    {
//                        /*----------------------------------------*\
//                        | Set the label to the user defined label  |
//                        \*----------------------------------------*/
//                        custom_led.name = ZoneJson["custom_labels"][LED_ID];
//                    }
//                    else
//                    {
//                        /*------------------------------------------------*\
//                        | Set default labels because something went wrong  |
//                        \*------------------------------------------------*/
//                        custom_led.name = ("Custom LED. Zone " + std::to_string(ZoneID) + ", LED " + std::to_string(LED_ID));
//                    }
//                    debug_custom->leds.push_back(custom_led);
//                }
//
//                debug_custom->zones.push_back(custom_zone);
//            }
//
//            debug_custom->SetupColors();
//
//            ResourceManager::get()->RegisterRGBController(debug_custom);
//        }
//    }

}   /* DetectDebugControllers() */

REGISTER_DETECTOR("Debug Controllers", DetectDebugControllers);
