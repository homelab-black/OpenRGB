#include "PhilipsHueSettingsHandler.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BridgeSettings,
    ip,
    mac,
    username,
    clientkey,
    autoconnect,
    entertainment
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HueSettings,
    bridges
);

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
    if(settings.bridges.size() < bridge_idx)
    {
        return settings.bridges[bridge_idx];
    }
    else
    {
        /*---------------------------------------------------------*\
        | Maybe return a std::optional instead of default settings? |
        \*---------------------------------------------------------*/
        return BridgeSettings {};
    }
}

void PhilipsHueSettingsHandler::SaveSettings()
{
    ResourceManager::get()->GetSettingsManager()->SetControllerSettingsAndSave(PHILIPSHUE_SETTINGS_KEY, settings);
}
