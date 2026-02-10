/*---------------------------------------------------------*\
| PhilipsHueControllerDetect.cpp                            |
|                                                           |
|   Detector for Philips Hue                                |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      15 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "Bridge.h"
#include "HueDeviceTypes.h"

#ifdef _WIN32
#include "WinHttpHandler.h"
#else
#include "LinHttpHandler.h"
#endif

#include "DetectionManager.h"
#include "LogManager.h"
#include "PhilipsHueController.h"
#include "PhilipsHueEntertainmentController.h"
#include "PhilipsHueSettingsHandler.h"
#include "ResourceManager.h"
#include "RGBController_PhilipsHue.h"
#include "RGBController_PhilipsHueEntertainment.h"

DetectedControllers DetectPhilipsHueControllers()
{
    DetectedControllers         detected_controllers;
    PhilipsHueSettingsHandler   hue_settings;

    /*-----------------------------------------------------*\
    | Create an HTTP handler                                |
    \*-----------------------------------------------------*/
#ifdef _WIN32
    using SystemHttpHandler = hueplusplus::WinHttpHandler;
#else
    using SystemHttpHandler = hueplusplus::LinHttpHandler;
#endif

    /*-----------------------------------------------------*\
    | Create a finder and find bridges                      |
    \*-----------------------------------------------------*/
    static hueplusplus::BridgeFinder finder(std::make_shared<SystemHttpHandler>());
    std::vector<hueplusplus::BridgeFinder::BridgeIdentification> bridges;// = finder.findBridges();

    /*-----------------------------------------------------*\
    | If no bridges were detected, manually add bridge IP   |
    | and MAC (need to get these from file)                 |
    \*-----------------------------------------------------*/
    if(hue_settings.GetBridgeCount() > 0)
    {
        hueplusplus::BridgeFinder::BridgeIdentification ident;
        const BridgeSettings& bridge_settings = hue_settings.GetBridge(0);

        ident.ip  = bridge_settings.ip;
        ident.mac = bridge_settings.mac;

        bridges.push_back(ident);
    }

    /*-----------------------------------------------------*\
    | If bridges were found, connect to the first bridge    |
    \*-----------------------------------------------------*/
    if(!bridges.empty())
    {
        /*-------------------------------------------------*\
        | Check if a saved username exists                  |
        \*-------------------------------------------------*/
        if(hue_settings.GetBridgeCount() > 0)
        {
            /*---------------------------------------------*\
            | Add the username & ClientKey if it exists     |
            \*---------------------------------------------*/
            const BridgeSettings& bridge_settings = hue_settings.GetBridge(0);
            finder.addUsername(bridges[0].mac, bridge_settings.username);
            finder.addClientKey(bridges[0].mac, bridge_settings.clientkey);
        }

        /*-------------------------------------------------*\
        | If username was added, this should connect right  |
        | away.  If not, the user will have to push the     |
        | connect button on the bridge.                     |
        \*-------------------------------------------------*/
        try
        {
            static hueplusplus::Bridge bridge = finder.getBridge(bridges[0]);

            bridge.refresh();

            /*---------------------------------------------*\
            | Check to see if we need to save the settings  |
            | Settings need to be saved if either username  |
            | or client key either do not exist or have     |
            | changed                                       |
            \*---------------------------------------------*/
            bool save_settings = false;
            bool use_entertainment = false;
            bool auto_connect = false;

            if(hue_settings.GetBridgeCount() > 0)
            {
                const BridgeSettings& bridge_settings = hue_settings.GetBridge(0);
                if(bridge_settings.username != bridge.getUsername())
                {
                    save_settings = true;
                }
                if(bridge_settings.clientkey != bridge.getClientKey())
                {
                    use_entertainment = true;
                    save_settings = true;
                }
            }

            /*---------------------------------------------*\
            | Save the settings if needed                   |
            \*---------------------------------------------*/
            if(save_settings)
            {
                BridgeSettings bridge_settings
                {
                    .ip = bridge.getBridgeIP(),
                    .mac = bridges[0].mac,
                    .username = bridge.getUsername(),
                    .clientkey = bridge.getClientKey(),
                    .autoconnect = auto_connect,
                    .entertainment = use_entertainment,
                };
                hue_settings.SetBridge(0, bridge_settings);
                hue_settings.SaveSettings();
            }

            /*---------------------------------*\
            | Get bridge settings               |
            \*---------------------------------*/
            const BridgeSettings& bridge_settings = hue_settings.GetBridge(0);

            /*---------------------------------------------*\
            | Get all groups from the bridge                |
            \*---------------------------------------------*/
            if(bridge_settings.entertainment)
            {
                std::vector<hueplusplus::Group> groups = bridge.groups().getAll();

                if(groups.size() > 0)
                {
                    /*-------------------------------------*\
                    | Loop through all available groups and |
                    | check to see if any are Entertainment |
                    | groups                                |
                    \*-------------------------------------*/
                    for(unsigned int group_idx = 0; group_idx < groups.size(); group_idx++)
                    {
                        if(groups[group_idx].getType() == "Entertainment")
                        {
                            PhilipsHueEntertainmentController*     controller     = new PhilipsHueEntertainmentController(bridge, groups[group_idx]);
                            RGBController_PhilipsHueEntertainment* rgb_controller = new RGBController_PhilipsHueEntertainment(controller);

                            detected_controllers.push_back(rgb_controller);
                        }
                    }

                    /*-------------------------------------*\
                    | Loop through RGB Controllers to find  |
                    | the first Entertainment group and Set |
                    | it to "Connect", as only one Stream   |
                    | can be open at a time.                |
                    | TODO: investigate this and rework     |
                    \*-------------------------------------*/
                    //if(bridge_settings.autoconnect)
                    //
                    //   for(unsigned int controller_idx = 0; controller_idx < ResourceManager::get()->GetRGBControllers().size(); controller_idx++)
                    //   {
                    //       if(ResourceManager::get()->GetRGBControllers()[controller_idx]->GetDescription() == "Philips Hue Entertainment Mode Device")
                    //       {
                    //           ResourceManager::get()->GetRGBControllers()[controller_idx]->SetActiveMode(0);
                    //           break;
                    //       }
                    //   }
                    //
                }
            }

            /*---------------------------------------------*\
            | Get all lights from the bridge                |
            \*---------------------------------------------*/
            else
            {
                std::vector<hueplusplus::Light> lights = bridge.lights().getAll();

                if(lights.size() > 0)
                {
                    /*-------------------------------------*\
                    | Loop through all available lights and |
                    | add those that have color (RGB)       |
                    | control capability                    |
                    \*-------------------------------------*/
                    for(unsigned int light_idx = 0; light_idx < lights.size(); light_idx++)
                    {
                        if(lights[light_idx].hasColorControl())
                        {
                            PhilipsHueController*     controller     = new PhilipsHueController(lights[light_idx], bridge.getBridgeIP());
                            RGBController_PhilipsHue* rgb_controller = new RGBController_PhilipsHue(controller);

                            detected_controllers.push_back(rgb_controller);
                        }
                    }
                }
            }
        }
        catch(const std::exception &e)
        {
            LOG_INFO("Exception occurred in Philips Hue detection: %s", e.what());
        }
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Philips Hue", DetectPhilipsHueControllers);
