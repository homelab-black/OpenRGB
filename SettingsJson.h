#pragma once

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

    inline bool contains2(const std::string& detector_str) const
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
    std::vector<size_t> dialog_no_show_hashes;
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
    int x = 0; // maybe use uint_32t for these?
    int y = 0;
    int width = 0;
    int height = 0;
    bool load_geometry = false;
    bool save_on_exit = false;

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
    bool numerical_labels = false;
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

    inline bool contains(string ip, unsigned short port) const
    {
        return find(ip, port) != std::nullopt;
    }

    inline std::optional<size_t> find(string ip, unsigned short port) const
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

struct ControllerSettings {
    // keys that need to be migrated:
    // "ASRock Polychrome USB"
    // "Asus Aura Core Laptop"
    // "CorsairDominatorSettings"
    // "DDPDevices"
    // "DebugDevices"
    // "DMXDevices"
    // "E131Devices"
    // "Gigabyte RGB Fusion 2 SMBus"
    // "Gigabyte RGB Fusion 2 USB"
    // "ElgatoLightStripDevices"
    // "ENESMBusSettings"
    // "EspurnaDevices"
    // "EVision2Settings"
    // "EndorfySettings"
    // "FanBusDevices"
    // "GoveeDevices"
    // "KasaSmartDevices"
    // "LEDStripDevices"
    // "LIFXDevices"
    // "LinuxLEDDevices"
    // "PhilipsHueDevices"
    // "PhilipsWizDevices"
    // "QMKOpenRGBDevices"
    // "NanoleafDevices"
    // "QMKVialRGBDevices"
    // "YeelightDevices"
    json data;
};

struct DriverSettings
{
    bool amd_smbus_reduce_cpu = false;
    bool shared_smbus_access = true;
};

struct ServerSettings
{
    bool legacy_workaround = false;
    // found in ResourceManager::UpdateDeviceList
    bool all_controllers = false;
    // found in OpenRgbSettingsPage()
    bool all_devices = false;
};

#define SETTINGS_VERSION "1.0"

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

/* *\
We can't use the macro's here since we need to rename the fields.
I prefer snake_case in for field names.
\* */
void to_json(nlohmann::json &j, const Settings& st);
void from_json(const json& j, Settings& st);
