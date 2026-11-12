/*---------------------------------------------------------*\
| RGBController_MultichoiceProxy.h                          |
|                                                           |
|   Placeholder RGBController for devices with unresolvable |
|   detection conflicts, only creating the real             |
|   RGBController after receiving the user's consent        |
|   Primary use is for problematic Sinowealth keyboards     |
|                                                           |
|   Dmitry Kychanov (k1-801)                    07 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_MultichoiceProxy.h"

#include "DetectionManager.h"
#include "ResourceManager.h"

RGBController_MultichoiceProxy::RGBController_MultichoiceProxy(device_type placeholder_type, const std::string& placeholder_name, const ControllerSpawnOptions& coptions)
    : RGBController_Proxy(placeholder_type, placeholder_name, nullptr)
{
    chosen_option     = -1;
    options           = coptions;

    flags = CONTROLLER_FLAG_MANUALLY_CONFIGURABLE_DEVICE_SPECIFIC;

    InitDeviceSpecificConfiguration();
}

RGBController_MultichoiceProxy::~RGBController_MultichoiceProxy()
{
}

controller_flags RGBController_MultichoiceProxy::GetFlags()
{
    controller_flags result = CONTROLLER_FLAG_MANUALLY_CONFIGURABLE_DEVICE_SPECIFIC;

    if(target_controller)
    {
        result |= target_controller->GetFlags();
    }
    return result;
}

bool RGBController_MultichoiceProxy::spawnController(int option)
{
    chosen_option = -1;

    if(target_controller)
    {
        /*--------------------------------------------------------*\
        | Note: any additional controllers are NOT tracked         |
        | and are beyond the scope of this proxy controller.       |
        \*--------------------------------------------------------*/
        delete target_controller;
        target_controller = nullptr;
    }

    if(option < 0 || option >= int(options.size()))
    {
        return false;
    }

    DetectedControllers controllers = options[option].spawn(options[option].name);
    chosen_option = option;

    if(!controllers.empty())
    {
        target_controller = controllers[0];

        /*--------------------------------------------------------*\
        | Detectors are allowed to return more than one controller |
        | We only manage one, so any additional controllers are    |
        | registered through ResourceManager (as a precaution)     |
        | However, we do NOT keep track of them beyond this point  |
        | DO NOT register the one at [0]                           |
        \*--------------------------------------------------------*/
        for(size_t i = 1; i < controllers.size(); ++i)
        {
            if(controllers[i])
            {
                DetectionManager::get()->RegisterRGBController(controllers[i]);
            }
        }
    }

    if(target_controller)
    {
        for(size_t callback_idx = 0; callback_idx < ProxyUpdateCallbacks.size(); ++callback_idx)
        {
            target_controller->RegisterUpdateCallback(ProxyUpdateCallbacks[callback_idx], ProxyUpdateCallbackArgs[callback_idx]);
        }
    }
    /*--------------------------------------------------------*\
    | Inform the system that device info has changed           |
    | UI doesn't respond to changes in device NAME through     |
    | controller's callback, so we signal LIST change too      |
    | to force UI to reload tab labels                         |
    \*--------------------------------------------------------*/
    SignalUpdate(RGBCONTROLLER_UPDATE_REASON_DEVICE_CHANGED);
    ResourceManager::get()->SignalResourceManagerUpdate(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);

    return (target_controller != nullptr);
}

static const char* device_model_config_entry_name = "controller_option";
static const char* invalid_string = "Choose device model...";

void RGBController_MultichoiceProxy::InitDeviceSpecificConfiguration()
{
    nlohmann::json configuration_json;
    configuration_json["schema"]        = GetDeviceSpecificConfigurationSchema();
    configuration_json["configuration"] = GetDeviceSpecificConfiguration();

    /*-----------------------------------------------------*\
    | Note: target controller's config is already included  |
    \*-----------------------------------------------------*/
    configuration = configuration_json.dump();
}

void RGBController_MultichoiceProxy::ApplyDeviceSpecificConfiguration()
{
    nlohmann::json configuration_json;

    try
    {
        configuration_json = nlohmann::json::parse(configuration);
    }
    catch(...)
    {
        return;
    }

    if(!configuration_json.contains("configuration") || !configuration_json["configuration"].is_object())
    {
        return;
    }

}

void RGBController_MultichoiceProxy::DeviceUpdateDeviceSpecificConfiguration()
{
    ApplyDeviceSpecificConfiguration();
}

/*-----------------------------------------------------*\
| Device-Specific Configuration Functions               |
\*-----------------------------------------------------*/
nlohmann::json RGBController_MultichoiceProxy::GetDeviceSpecificConfigurationSchema()
{
    nlohmann::json schema;

    /*--------------------------------------------------------*\
    | Always insert a "None-selected" option which builds NO   |
    | controller, and make it the default                      |
    \*--------------------------------------------------------*/
    nlohmann::json enum_options = {invalid_string};
    for(size_t i = 0; i < options.size(); ++i)
    {
        enum_options.push_back(options[i].name);
    }

    schema[device_model_config_entry_name]["title"]        = "Device model [WARNING: BRICKING RISK]";
    schema[device_model_config_entry_name]["description"]  = "BY CHOOSING DEVICE YOU ACCEPT THE RISKS OF DEVICE DAMAGE.\n\n"
                                                            "This device could not be identified automatically and requires manual selection.\n"
                                                            "Controller will be initialized after the model is selected.\n"
                                                            "If your model is not listed, do not select anything!";

    schema[device_model_config_entry_name]["type"]         = "string";
    schema[device_model_config_entry_name]["default"]      = invalid_string;
    schema[device_model_config_entry_name]["order"]        = 0;
    schema[device_model_config_entry_name]["enum"]         = enum_options;

    if(target_controller)
    {
        nlohmann::json patch = target_controller->GetDeviceSpecificConfigurationSchema();
        if(!patch.is_null())
        {
            schema.merge_patch(patch);
        }
    }
    return schema;
}

nlohmann::json RGBController_MultichoiceProxy::GetDeviceSpecificConfiguration()
{
    nlohmann::json config;
    config[device_model_config_entry_name] = (chosen_option == -1) ? invalid_string : options[chosen_option].name;

    if(target_controller)
    {
        nlohmann::json patch = target_controller->GetDeviceSpecificConfiguration();
        if(!patch.is_null())
        {
            config.merge_patch(patch);
        }
    }
    return config;
}

void RGBController_MultichoiceProxy::SetDeviceSpecificConfiguration(nlohmann::json config)
{
    if(config.contains(device_model_config_entry_name))
    {
        std::string opt = config[device_model_config_entry_name];
        int option_idx;
        for(option_idx = 0; option_idx < int(options.size()); ++option_idx)
        {
            if(options[option_idx].name == opt)
            {
                break;
            }
        }
        if(option_idx >= int(options.size()))
        {
            option_idx = -1;
        }

        spawnController(option_idx);
    }


    /*--------------------------------------------------------*\
    | Rewrite the "configuration" string to the new config     |
    \*--------------------------------------------------------*/
    InitDeviceSpecificConfiguration();

    if(target_controller)
    {
        /*--------------------------------------------------------*\
        | Pass down everything EXCEPT the controller_option        |
        | so that the managed controller CAN have a configuration  |
        | of it's own and be configurable, BUT it doesn't override |
        | the chosen device model                                  |
        \*--------------------------------------------------------*/
        config.erase(device_model_config_entry_name);
        target_controller->SetDeviceSpecificConfiguration(config);
    }
}

/*---------------------------------------------------------*\
| Update Callback Functions                                 |
| Re-implemented bc the vectors in base are private         |
\*---------------------------------------------------------*/
void RGBController_MultichoiceProxy::RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg)
{
    std::lock_guard<std::mutex> lock(ProxyUpdateMutex);

    /*-----------------------------------------------------*\
    | Check to see if this is already registered to avoid   |
    | registering the same callback multiple times          |
    \*-----------------------------------------------------*/
    for(unsigned int callback_idx = 0; callback_idx < ProxyUpdateCallbackArgs.size(); callback_idx++ )
    {
        if((ProxyUpdateCallbacks[callback_idx] == new_callback) && (ProxyUpdateCallbackArgs[callback_idx] == new_callback_arg))
        {
            return;
        }
    }

    ProxyUpdateCallbacks.push_back(new_callback);
    ProxyUpdateCallbackArgs.push_back(new_callback_arg);

    if(target_controller)
    {
        target_controller->RegisterUpdateCallback(new_callback, new_callback_arg);
    }
}

void RGBController_MultichoiceProxy::UnregisterUpdateCallback(void * callback_arg)
{
    std::lock_guard<std::mutex> lock(ProxyUpdateMutex);
    for(unsigned int callback_idx = 0; callback_idx < ProxyUpdateCallbackArgs.size(); callback_idx++)
    {
        if(ProxyUpdateCallbackArgs[callback_idx] == callback_arg)
        {
            ProxyUpdateCallbackArgs.erase(ProxyUpdateCallbackArgs.begin() + callback_idx);
            ProxyUpdateCallbacks.erase(ProxyUpdateCallbacks.begin() + callback_idx);
        }
    }
    if(target_controller)
    {
        target_controller->UnregisterUpdateCallback(callback_arg);
    }
}

