#include <nlohmann/json.hpp>

#define ELGATO_KEY_LIGHT_CFG_KEY "ElgatoKeyLightDevices"
#define ELGATO_LIGHTSTRIP_CFG_KEY "ElgatoLightStripDevices"

struct ElgatoDeviceSettings
{
    std::string ip;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElgatoDeviceSettings, ip);

struct ElgatoSettings
{
    std::vector<ElgatoDeviceSettings> devices;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElgatoSettings, devices);
