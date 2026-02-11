/*-----------------------------------------*\
| Not using the NLOHMANN_DEFINE macro here  |
| to make error messages better (hopefully) |
\*-----------------------------------------*/
template<typename D>
void to_json(json& j, const ManualDeviceSettings<D>& settings)
{
    j = json { "devices", setttings.devices };
}

template<typename D>
void from_json(const json& j, ManualDeviceSettings<D>& settings)
{
    j.at("devices").get_to(settings);
}
