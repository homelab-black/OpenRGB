#include "SettingsJson.h"

/*-----------------------------------------------------------------------*\
| Specialisation, required to parse std::optional<T>                      |
| https://github.com/nlohmann/json/issues/1910#issuecomment-615843605     |
\*-----------------------------------------------------------------------*/
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt == std::nullopt) {
                j = nullptr;
            } else {
              j = *opt;
            }
        }

        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };
}

// NO TRAILING COMMAS!
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AutoStartSettings,
    enabled,
    setminimized,
    setprofile,
    profile,
    client,
    setclient,
    setserver,
    setserverhost,
    host,
    setserverportbool,
    port,
    setcustom,
    custom
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DetectorSettings,
    detectors,
    hid_safe_mode,
    initial_detection_delay_ms
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LogManagerSettings,
    log_console,
    file_count_limit,
    log_file,
    logfile,
    loglevel,
    dialog_no_show_hashes
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ProfileSettings,
    enabled,
    name
);


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ProfileManagerSettings,
    open_profile,
    resume_profile,
    suspend_profile,
    exit_profile
);

NLOHMANN_JSON_SERIALIZE_ENUM(Theme, {
    {Light, "light"},
    {Dark, "dark"},
    {Auto, "auto"}
});

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeSettings, theme);

NLOHMANN_JSON_SERIALIZE_ENUM(Language, {
    {SystemDefault, "default"},
    {German, "Deutsch"},
    {EnglishAU, "English - AU"},
    {EnglishUk, "English - Uk"},
    {EnglishUs, "English - Us"},
    {Spanish, "Español"},
    {French, "Français"},
    {Croatian, "Hrvatski"},
    {Malaysian, "Melayu"},
    {Polish, "Polski"},
    {Portuguese, "Português"},
    {Russian, "Русский"},
    {ChineseSimplified, "简体中文"},
    {ChineseTraditional, "繁體中文"}
});

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WindowGeometrySettings,
    x,
    y,
    width,
    height,
    load_geometry,
    save_on_exit
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AutoLoadProfileSettings,
    open_profile,
    resume_profile,
    suspend_profile,
    exit_profile
);


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ExitProfileSettings,
    set_on_exit,
    profile_name
);

NLOHMANN_JSON_SERIALIZE_ENUM(HexFormat, {
    {RGB, "RGB"},
    {BGR, "BGR"}
});

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UserInterfaceSettings,
    run_zone_checks,
    greyscale_tray_icon,
    minimize_on_close,
    disable_key_expansion,
    geometry,
    language,
    autoload_profiles,
    exit_profile
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OnePluginSettings,
    name,
    description,
    enabled
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PluginSettings,
    plugins_remove,
    plugins
);


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OneClientSettings,
    ip,
    port
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ClientSettings,
    clients
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ControllerSettings,
    data
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DriverSettings,
    amd_smbus_reduce_cpu,
    shared_smbus_access
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ServerSettings,
    legacy_workaround,
    all_controllers,
    all_devices
);


/* *\
We can't use the macro's here since we need to rename the fields.
I prefer snake_case in for field names.
\* */

void to_json(nlohmann::json &j, const Settings& st) {
    j = json {
        {"AutoStart", st.auto_start},
        {"Detectors", st.detectors},
        {"LogManager", st.log_manager},
        {"ProfileManager", st.profile_manager},
        {"Theme", st.theme},
        {"UserInterface", st.ui},
        {"Plugins", st.plugins},
        {"Clients", st.clients},
        {"Drivers", st.drivers},
        {"Servers", st.servers},
        {"Controllers", st.controllers},
    };
}

void from_json(const json& j, Settings& st) {
    j.at("AutoStart").get_to(st.auto_start);
    j.at("Detectors").get_to(st.detectors);
    j.at("LogManager").get_to(st.log_manager);
    j.at("ProfileManager").get_to(st.profile_manager);
    j.at("Theme").get_to(st.theme);
    j.at("UserInterface").get_to(st.ui);
    j.at("Plugins").get_to(st.plugins);
    j.at("Clients").get_to(st.clients);
    j.at("Drivers").get_to(st.drivers);
    j.at("Servers").get_to(st.servers);
    j.at("Controllers").get_to(st.controllers);
}
