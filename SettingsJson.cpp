#include "SettingsJson.h"

std::string ThemeToString(Theme theme)
{
    switch(theme)
    {
        case Theme::Light: return "Light";
        case Theme::Dark: return "Dark";
        case Theme::Auto: return "Auto";
    }
}

std::optional<Theme> StringToTheme(const std::string& str)
{
    if(str == "Light" || str == "light")
    {
        return Theme::Light;
    }
    else if(str == "Dark" || str == "dark")
    {
        return Theme::Dark;
    }
    else if(str == "Auto" || str == "auto")
    {
        return Theme::Auto;
    }
    else
    {
        return std::nullopt;
    }
}

std::string HexFormatToString(HexFormat hex_format)
{
    switch(hex_format)
    {
        case HexFormat::RGB: return "RGB";
        case HexFormat::BGR: return "BGR";
    }
}
std::optional<HexFormat> StringToHexFormat(const std::string& str)
{
    if(str == "RGB" || str == "rgb")
    {
        return HexFormat::RGB;
    }
        if(str == "BGR" || str == "bgr")
    {
        return HexFormat::BGR;
    }
    else
    {
        return std::nullopt;
    }
}
