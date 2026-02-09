#include <nlohmann/json.hpp>
#include <string>
#include <optional>

using json = nlohmann::json;
using string = std::string;

struct AutoStartSettings {
public:
    bool enabled;
    bool setminimized;

    // profile
    bool setprofile;
    string profile;

    // client stuff
    bool setclient;
    string client;

    // server stuff
    bool setserver;
    bool setserverhost;
    string host;
    bool setserverportbool;
    int port;

    // custom
    bool setcustom;
    string custom;

};

struct DetectorSettings {
    std::unordered_map<string, bool> detectors;
    // default false
    bool hid_safe_mode = false;
    // default 0;
    int initial_detection_delay_ms = 0;
};

struct LogManagerSettings {
    // whether to enable log console in openrgb
    bool log_console = false;
    // max length of log file
    std::optional<int> file_count_limit;
    // Whether to use a logfile
    bool log_file = true;
    // filename to use for logfile
    // The first # symbol is replaced by a timestamp
    std::optional<string> logfile;
    std::optional<unsigned int> loglevel;
};


struct ProfileSettings {
    bool enabled;
    string name;
};

/*--------------------------------------------*\
| profile to use on <variant> event            |
| Ie, ManagedProfileType::Open is used on open |
\*--------------------------------------------*/
enum ManagedProfileType
{
    Open,
    Resume,
    Suspend,
    Exit,
};

struct ProfileManagerSettings {
    ProfileSettings open_profile;
    ProfileSettings resume_profile;
    ProfileSettings suspend_profile;
    ProfileSettings exit_profile;
};

enum Theme {
    Light,
    Dark,
    Auto,
};

string ThemeToString(Theme theme);
std::optional<Theme> StringToTheme(const std::string& str);

struct ThemeSettings {
    Theme theme = Theme::Light;
};

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

struct WindowGeometrySettings {
    int x; // maybe use uint_32t for these?
    int y;
    int width;
    int height;
    bool load_geometry;
    bool save_on_exit;

};

struct AutoLoadProfileSettings {
    std::optional<ProfileSettings> open_profile;
    std::optional<ProfileSettings> resume_profile;
    std::optional<ProfileSettings> suspend_profile;
    std::optional<ProfileSettings> exit_profile;
};


struct ExitProfileSettings {
    bool set_on_exit;
    string profile_name;

};

enum HexFormat
{
    RGB,
    BGR,
};

string HexFormatToString(HexFormat hex_format);
std::optional<HexFormat> StringToHexFormat(const std::string& str);

struct UserInterfaceSettings {
    // todo: migrate RunZoneChecks to run_zone_checks
    bool run_zone_checks = true;
    bool greyscale_tray_icon;
    bool minimize_on_close;
    bool disable_key_expansion = false;
    bool show_led_view = false;
    HexFormat hex_format = HexFormat::RGB;
    WindowGeometrySettings geometry;
    // Language language = Language::SystemDefault;
    string language = "default";
    AutoLoadProfileSettings autoload_profiles;
    ExitProfileSettings exit_profile;
};

// todo: fix naming
struct OnePluginSettings {
    std::string name;
    std::string description;
    bool enabled;
};

struct PluginSettings {
    std::vector<std::filesystem::path> plugins_remove;
    std::vector<OnePluginSettings> plugins;
};

struct OneClientSettings {
    string ip;
    unsigned short port;
};

struct ClientSettings
{
    std::vector<OneClientSettings> clients;
};

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

struct ServerSettings
{
    bool legacy_workaround = false;
    // found in ResourceManager::UpdateDeviceList
    bool all_controllers = false;
    // found in OpenRgbSettingsPage()
    bool all_devices = false;
};

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
    ServerSettings servers;
};

/* *\
We can't use the macro's here since we need to rename the fields.
I prefer snake_case in for field names.
\* */
void to_json(nlohmann::json &j, const Settings& st);
void from_json(const json& j, Settings& st);
