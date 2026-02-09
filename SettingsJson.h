#include <nlohmann/json.hpp>
#include <string>
#include <optional>

using json = nlohmann::json;
using string = std::string;

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
    bool enabled;
    bool setminimized;

    // profile
    bool setprofile;
    string profile;

    // client stuff
    // why not std::optional<string> client ?
    string client;
    bool setclient;

    // server stuff
    bool setserver;
    // why not `std::optional<string>` host?
    bool setserverhost;
    string host;
    // why not `std::optional<string>` port?
    bool setserverportbool;
    string port;

    // custom
    bool setcustom;
    string custom;

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
    std::unordered_map<string, bool> detectors;
    // default false
    bool hid_safe_mode = false;
    // default 0;
    int initial_detection_delay_ms = 0;
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
    // todo: migrate both logfile and log_file to this key
    bool log_file = true;
    // filename to use for logfile
    // The first # symbol is replaced by a timestamp
    std::optional<string> logfile;
    std::optional<unsigned int> loglevel;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LogManagerSettings,
    log_console,
    file_count_limit,
    log_file,
    logfile,
    loglevel
);

struct ProfileSettings {
    bool enabled;
    string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ProfileSettings, enabled, name);

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

enum Theme {
    Light,
    Dark,
    Auto,
};

NLOHMANN_JSON_SERIALIZE_ENUM(Theme, {
    {Light, "light"},
    {Dark, "dark"},
    {Auto, "auto"}
});

struct ThemeSettings {
    Theme theme = Theme::Light;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ThemeSettings, theme);


enum Language {
    SystemDefault,
    German,
    EnglishAU,
    EnglishUk,
    EnglishUs,
    Spanish,
    French,
    Croatian,
    Malaysian,
    Polish,
    Portuguese,
    Russian,
    ChineseSimplified,
    ChineseTraditional,
};

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

struct WindowGeometrySettings {
    int x; // maybe use uint_32t for these?
    int y;
    int width;
    int height;
    bool load_geometry;
    bool save_on_exit;

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
    bool set_on_exit;
    string profile_name;

};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ExitProfileSettings,
    set_on_exit,
    profile_name
);

enum HexFormat
{
    RGB,
    BGR,
};

NLOHMANN_JSON_SERIALIZE_ENUM(HexFormat, {
    {RGB, "RGB"},
    {BGR, "BGR"}
});

struct UserInterfaceSettings {
    // todo: migrate RunZoneChecks to run_zone_checks
    bool run_zone_checks = true;
    bool greyscale_tray_icon;
    bool minimize_on_close;
    bool disable_key_expansion = false;
    bool show_led_view = false;
    HexFormat hex_format = HexFormat::RGB;
    WindowGeometrySettings geometry;
    Language language = Language::SystemDefault;
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
    std::string name;
    std::string description;
    bool enabled;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OnePluginSettings,
    name,
    description,
    enabled
);

struct PluginSettings {
    std::vector<filesystem::path> plugins_remove;
    std::vector<OnePluginSettings> plugins;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PluginSettings,
    plugins_remove,
    plugins
);

struct OneClientSettings {
    string ip;
    unsigned short port;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OneClientSettings,
    ip,
    port
);

struct ClientSettings
{
    std::vector<OneClientSettings> clients;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ClientSettings,
    clients
);

struct ControllerSettings {
    // keys that need to be migrated:
    // "ASRock Polychrome USB"
    // "Asus Aura Core Laptop"
    // "CorsairDominatorSettings"
    // "Gigabyte RGB Fusion 2 SMBus"
    // "Gigabyte RGB Fusion 2 USB"
    // "PhilipsHueDevices"
    // "QMKOpenRGBDevices"
    // "NanoleafDevices"
};

struct DriverSettings
{
    bool amd_smbus_reduce_cpu;
    bool shared_smbus_access;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DriverSettings,
    amd_smbus_reduce_cpu,
    shared_smbus_access
);


struct Settings {
    AutoStartSettings auto_start;
    DetectorSettings detectors;
    LogManagerSettings log_manager;
    ProfileManagerSettings profile_manager;
    ThemeSettings theme;
    UserInterfaceSettings ui;
    PluginSettings plugins;
    ClientSettings clients;
    DriverSettings drivers;
};

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
}
