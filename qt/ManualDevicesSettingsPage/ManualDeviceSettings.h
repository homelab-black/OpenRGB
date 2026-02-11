#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*------------------------------------------------------------*\
| We could use this struct in the ManualDeviceSettingsPage     |
| Instead of dealing with json, but I'm leaving that for later |
\*------------------------------------------------------------*/

/*-----------------------------------------------------------------------*\
| Settings struct for controllers                                         |
| included on the manual device settings page                             |
| These all have a single "devices" list, which holds settings per-device |
|                                                                         |
| `D` must be serializable _and_ deserializable,                          |
| meaning these two functions must exist:                                 |
|    `void to_json(json& j, const D& settings);`                          |
|    `void from_json(const json& j, D& settings);`                        |
\*-----------------------------------------------------------------------*/
template <typename D>
struct ManualDeviceSettings
{
    std::vector<D> devices;
};

template<typename D>
void to_json(json& j, const ManualDeviceSettings<D>& settings);

template<typename D>
void from_json(const json& j, ManualDeviceSettings<D>& settings);
