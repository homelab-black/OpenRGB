#include "PhilipsHueSettingsHandler.h"
#include "ResourceManager.h"
#include "SettingsManager.h"



PhilipsHueSettingsHandler::PhilipsHueSettingsHandler()
{
    /*-------------------------------------------------*\
    | Get Philips Hue settings from settings manager    |
    \*-------------------------------------------------*/
    std::optional<HueSettings> cfg_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettings<HueSettings>(PHILIPSHUE_SETTINGS_KEY);
    settings = cfg_settings.value_or(HueSettings {});
}

PhilipsHueSettingsHandler::~PhilipsHueSettingsHandler()
{
}

std::size_t PhilipsHueSettingsHandler::GetBridgeCount()
{
    return settings.bridges.size();
}

const BridgeSettings& PhilipsHueSettingsHandler::GetBridge(unsigned int bridge_idx)
{
    /*---------------------------------------------------------*\
    | Maybe return a std::optional instead of default settings? |
    \*---------------------------------------------------------*/
    return settings.bridges[bridge_idx];
}

void PhilipsHueSettingsHandler::SetBridge(unsigned int bridge_idx, BridgeSettings new_settings)
{
    if(settings.bridges.size() < bridge_idx)
    {
        settings.bridges[bridge_idx] = new_settings;
    }
}


void PhilipsHueSettingsHandler::SaveSettings()
{
    ResourceManager::get()->GetSettingsManager()->SetControllerSettingsAndSave(PHILIPSHUE_SETTINGS_KEY, settings);
}
