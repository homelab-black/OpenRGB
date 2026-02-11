#include "SettingsJson.h"

// /*-----------------------------------------------------------------------*\
// | Specialisation, required to parse std::optional<T>                      |
// | https://github.com/nlohmann/json/issues/1910#issuecomment-615843605     |
// \*-----------------------------------------------------------------------*/
// namespace nlohmann {
//     template <typename T>
//     struct adl_serializer<std::optional<T>> {
//         static void to_json(json& j, const std::optional<T>& opt) {
//             if (opt == std::nullopt) {
//                 j = nullptr;
//             } else {
//               j = *opt;
//             }
//         }

//         static void from_json(const json& j, std::optional<T>& opt) {
//             if (j.is_null()) {
//                 opt = std::nullopt;
//             } else {
//                 opt = j.get<T>();
//             }
//         }
//     };
// }

string ThemeToString(Theme theme)
{
    switch (theme)
    {
        case Theme::Light: return "Light";
        case Theme::Dark: return "Dark";
        case Theme::Auto: return "Auto";
    }
}

std::optional<Theme> StringToTheme(const std::string& str)
{
    if (str == "Light" || str == "light")
    {
        return Theme::Light;
    }
    else if (str == "Dark" || str == "dark")
    {
        return Theme::Dark;
    }
    else if (str == "Auto" || str == "auto")
    {
        return Theme::Auto;
    }
    else
    {
        return std::nullopt;
    }
}

string HexFormatToString(HexFormat hex_format)
{
    switch (hex_format)
    {
        case HexFormat::RGB: return "RGB";
        case HexFormat::BGR: return "BGR";
    }
}
std::optional<HexFormat> StringToHexFormat(const std::string& str)
{
    if (str == "RGB" || str == "rgb")
    {
        return HexFormat::RGB;
    }
        if (str == "BGR" || str == "bgr")
    {
        return HexFormat::BGR;
    }
    else
    {
        return std::nullopt;
    }
}

















