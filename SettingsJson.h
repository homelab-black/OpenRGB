#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <optional>

using json = nlohmann::json;

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

struct AutoStartSettings {
public:
    bool enabled = false;
    bool setminimized = false;

    // profile
    bool setprofile = false;
    std::string profile = "";

    // client stuff
    bool setclient = false;
    std::string client = "";

    // server stuff
    bool setserver = false;
    bool setserverhost = false;
    std::string host = "";
    bool setserverportbool = false;
    int port = 0;

    // custom
    bool setcustom = false;
    std::string custom = "";
};

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

struct DetectorSettings {
    std::unordered_map<std::string, bool> detectors;
    // default false
    bool hid_safe_mode = false;
    // default 0;
    int initial_detection_delay_ms = 0;

    inline bool contains(const std::string& detector_str) const
    {
        return detectors.count(detector_str) > 0;
    }

    /*---------------------------------------------------*\
    | Returns true if the detector exists and is enabled. |
    | Returns false if detector_str does not exist        |
    \*---------------------------------------------------*/
    inline bool detection_enabled_for(const std::string& detector_str) const
    {
        if (detectors.count(detector_str) > 0)
        {
            return detectors.at(detector_str);
        }
        return false;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DetectorSettings,
    detectors,
    hid_safe_mode,
    initial_detection_delay_ms
);

struct LogManagerSettings {
    // whether to enable log console in openrgb
    bool log_console = false;
    // max length of log file
    std::optional<int> file_count_limit;
    // Whether to use a logfile
    bool log_file = true;
    // filename to use for logfile
    // The first # symbol is replaced by a timestamp
    std::optional<std::string> logfile;
    std::optional<unsigned int> loglevel;
    std::vector<size_t> dialog_no_show_hashes;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LogManagerSettings,
    log_console,
    file_count_limit,
    log_file,
    logfile,
    loglevel,
    dialog_no_show_hashes
);

struct ProfileSettings {
    bool enabled = false;
    std::string name = "";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ProfileSettings,
    enabled,
    name
);

/*--------------------------------------------*\
| profile to use on <variant> event            |
| Ie, ManagedProfileType::Open is used on open |
\*--------------------------------------------*/
enum class ManagedProfileType
{
    Open,
    Resume,
    Suspend,
    Exit,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ManagedProfileType, {
    {ManagedProfileType::Open, "open"},
    {ManagedProfileType::Resume, "resume"},
    {ManagedProfileType::Suspend, "suspend"},
    {ManagedProfileType::Exit, "exit"},
});

struct ProfileManagerSettings {
    ProfileSettings open_profile;
    ProfileSettings resume_profile;
    ProfileSettings suspend_profile;
    ProfileSettings exit_profile;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ProfileManagerSettings,
    open_profile,
    resume_profile,
    suspend_profile,
    exit_profile
);

enum class Theme {
    Light,
    Dark,
    Auto,
};

NLOHMANN_JSON_SERIALIZE_ENUM(Theme, {
    {Theme::Light, "light"},
    {Theme::Dark, "dark"},
    {Theme::Auto, "auto"}
});

std::string ThemeToString(Theme theme);
std::optional<Theme> StringToTheme(const std::string& str);

struct ThemeSettings {
    Theme theme = Theme::Light;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeSettings, theme);

struct WindowGeometrySettings {
    int x = 0; // maybe use uint_32t for these?
    int y = 0;
    int width = 0;
    int height = 0;
    bool load_geometry = false;
    bool save_on_exit = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WindowGeometrySettings,
    x,
    y,
    width,
    height,
    load_geometry,
    save_on_exit
);

struct AutoLoadProfileSettings {
    std::optional<ProfileSettings> open_profile;
    std::optional<ProfileSettings> resume_profile;
    std::optional<ProfileSettings> suspend_profile;
    std::optional<ProfileSettings> exit_profile;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AutoLoadProfileSettings,
    open_profile,
    resume_profile,
    suspend_profile,
    exit_profile
);

struct ExitProfileSettings {
    bool set_on_exit = false;
    std::string profile_name = "";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ExitProfileSettings,
    set_on_exit,
    profile_name
);

enum class HexFormat
{
    RGB,
    BGR,
};

NLOHMANN_JSON_SERIALIZE_ENUM(HexFormat, {
    {HexFormat::RGB, "RGB"},
    {HexFormat::BGR, "BGR"}
});

std::string HexFormatToString(HexFormat hex_format);
std::optional<HexFormat> StringToHexFormat(const std::string& str);

struct UserInterfaceSettings {
    // todo: migrate RunZoneChecks to run_zone_checks
    bool run_zone_checks = true;
    bool greyscale_tray_icon = false;
    bool minimize_on_close = false;
    bool disable_key_expansion = false;
    bool show_led_view = false;
    bool numerical_labels = false;
    HexFormat hex_format = HexFormat::RGB;
    WindowGeometrySettings geometry;
    std::string language = "default";
    AutoLoadProfileSettings autoload_profiles;
    ExitProfileSettings exit_profile;
};

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

// todo: fix naming
struct OnePluginSettings {
    std::string name = "";
    std::string description = "";
    bool enabled = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OnePluginSettings,
    name,
    description,
    enabled
);


struct PluginSettings {
    std::vector<std::filesystem::path> plugins_remove;
    std::vector<OnePluginSettings> plugins;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PluginSettings,
    plugins_remove,
    plugins
);

struct OneClientSettings {
    std::string ip = "";
    unsigned short port = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OneClientSettings,
    ip,
    port
);

struct ClientSettings
{
    std::vector<OneClientSettings> clients;

    inline bool contains(std::string ip, unsigned short port) const
    {
        return find(ip, port) != std::nullopt;
    }

    inline std::optional<size_t> find(std::string ip, unsigned short port) const
    {
        for (size_t i = 0; i < clients.size(); i++)
        {
            if (clients[i].ip == ip && clients[i].port == port)
            {
                return i;
            }
        }
        return std::nullopt;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ClientSettings,
    clients
);

struct ControllerSettings {
    json data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ControllerSettings,
    data
);

struct DriverSettings
{
    bool amd_smbus_reduce_cpu = false;
    bool shared_smbus_access = true;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DriverSettings,
    amd_smbus_reduce_cpu,
    shared_smbus_access
);

struct ServerSettings
{
    bool legacy_workaround = false;
    // found in ResourceManager::UpdateDeviceList
    bool all_controllers = false;
    // found in OpenRgbSettingsPage()
    bool all_devices = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ServerSettings,
    legacy_workaround,
    all_controllers,
    all_devices
);

#define SETTINGS_VERSION "1.0"
#define VERSION_CFG_KEY "Version"

struct Settings {
    /*----------------------------------------------------------*\
    | default to "0.0" so we know this field hasn't been set yet |
    \*----------------------------------------------------------*/
    std::string version = "0.0";
    AutoStartSettings auto_start;
    DetectorSettings detectors;
    LogManagerSettings log_manager;
    ProfileManagerSettings profile_manager;
    ThemeSettings theme;
    UserInterfaceSettings ui;
    PluginSettings plugins;
    ClientSettings clients;
    DriverSettings drivers;
    ServerSettings servers;
    ControllerSettings controllers;
};

/*-----------------------------------------------------------------*\
| We can't use the macro's here since we need to rename the fields. |
| I prefer snake_case in for field names.                           |
|                                                                   |
| Inlined these for consistency with macros                         |
\*-----------------------------------------------------------------*/
inline void to_json(nlohmann::json &j, const Settings& st) {
    j = json {
        {VERSION_CFG_KEY, st.version},
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

inline void from_json(const json& j, Settings& st) {
    /*-----------------------------------------*\
    | Assign default value if key doesn't exist |
    \*-----------------------------------------*/
    st.version         = j.value(VERSION_CFG_KEY, SETTINGS_VERSION);
    st.auto_start      = j.value("AutoStart", AutoStartSettings {});
    st.detectors       = j.value("Detectors", DetectorSettings {});
    st.log_manager     = j.value("LogManager", LogManagerSettings {});
    st.profile_manager = j.value("ProfileManager", ProfileManagerSettings {});
    st.theme           = j.value("Theme", ThemeSettings {});
    st.ui              = j.value("UserInterface", UserInterfaceSettings {});
    st.plugins         = j.value("Plugins", PluginSettings {});
    st.clients         = j.value("Clients", ClientSettings {});
    st.drivers         = j.value("Drivers", DriverSettings {});
    st.servers         = j.value("Servers", ServerSettings {});
    st.controllers     = j.value("Controllers", ControllerSettings {});
}

