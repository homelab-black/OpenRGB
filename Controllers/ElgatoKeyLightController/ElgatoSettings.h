#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define ELGATO_KEY_LIGHT_CFG_KEY "ElgatoKeyLightDevices"
#define ELGATO_LIGHTSTRIP_CFG_KEY "ElgatoLightStripDevices"

struct ElgatoDeviceSettings
{
    std::string ip;
};

void to_json(json& j, const ElgatoDeviceSettings& settings);
void from_json(const json& j, ElgatoDeviceSettings& settings);

struct ElgatoSettings
{
    std::vector<ElgatoDeviceSettings> devices;
};

void to_json(json& j, const ElgatoSettings& settings);
void from_json(const json& j, ElgatoSettings& settings);
