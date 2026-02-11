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

void to_json(json& j, const AutoStartSettings& settings);
void from_json(const json& j, AutoStartSettings& settings);

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

void to_json(json& j, const DetectorSettings& settings);
void from_json(const json& j, DetectorSettings& settings);

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

void to_json(json& j, const LogManagerSettings& settings);
void from_json(const json& j, LogManagerSettings& settings);

struct ProfileSettings {
    bool enabled;
    string name;
};

void to_json(json& j, const ProfileSettings& settings);
void from_json(const json& j, ProfileSettings& settings);

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

void to_json(json& j, const ManagedProfileType& settings);
void from_json(const json& j, ManagedProfileType& settings);

struct ProfileManagerSettings {
    ProfileSettings open_profile;
    ProfileSettings resume_profile;
    ProfileSettings suspend_profile;
    ProfileSettings exit_profile;
};

void to_json(json& j, const ProfileManagerSettings& settings);
void from_json(const json& j, ProfileManagerSettings& settings);

enum class Theme {
    Light,
    Dark,
    Auto,
};

void to_json(json& j, const Theme& settings);
void from_json(const json& j, Theme& settings);

string ThemeToString(Theme theme);
std::optional<Theme> StringToTheme(const std::string& str);

struct ThemeSettings {
    Theme theme = Theme::Light;
};

void to_json(json& j, const ThemeSettings& settings);
void from_json(const json& j, ThemeSettings& settings);

struct WindowGeometrySettings {
    int x = 0; // maybe use uint_32t for these?
    int y = 0;
    int width = 0;
    int height = 0;
    bool load_geometry = false;
    bool save_on_exit = false;
};

void to_json(json& j, const WindowGeometrySettings& settings);
void from_json(const json& j, WindowGeometrySettings& settings);

struct AutoLoadProfileSettings {
    std::optional<ProfileSettings> open_profile;
    std::optional<ProfileSettings> resume_profile;
    std::optional<ProfileSettings> suspend_profile;
    std::optional<ProfileSettings> exit_profile;
};

void to_json(json& j, const AutoLoadProfileSettings& settings);
void from_json(const json& j, AutoLoadProfileSettings& settings);

struct ExitProfileSettings {
    bool set_on_exit;
    string profile_name;
};

void to_json(json& j, const ExitProfileSettings& settings);
void from_json(const json& j, ExitProfileSettings& settings);

enum class HexFormat
{
    RGB,
    BGR,
};

void to_json(json& j, const HexFormat& settings);
void from_json(const json& j, HexFormat& settings);

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

void to_json(json& j, const UserInterfaceSettings& settings);
void from_json(const json& j, UserInterfaceSettings& settings);

// todo: fix naming
struct OnePluginSettings {
    std::string name;
    std::string description;
    bool enabled;
};

void to_json(json& j, const OnePluginSettings& settings);
void from_json(const json& j, OnePluginSettings& settings);

struct PluginSettings {
    std::vector<std::filesystem::path> plugins_remove;
    std::vector<OnePluginSettings> plugins;
};

void to_json(json& j, const PluginSettings& settings);
void from_json(const json& j, PluginSettings& settings);

struct OneClientSettings {
    string ip;
    unsigned short port;
};

void to_json(json& j, const OneClientSettings& settings);
void from_json(const json& j, OneClientSettings& settings);

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

void to_json(json& j, const ClientSettings& settings);
void from_json(const json& j, ClientSettings& settings);

struct ControllerSettings {
    json data;
};

void to_json(json& j, const ControllerSettings& settings);
void from_json(const json& j, ControllerSettings& settings);

struct DriverSettings
{
    bool amd_smbus_reduce_cpu = false;
    bool shared_smbus_access = true;
};

void to_json(json& j, const DriverSettings& settings);
void from_json(const json& j, DriverSettings& settings);

struct ServerSettings
{
    bool legacy_workaround = false;
    // found in ResourceManager::UpdateDeviceList
    bool all_controllers = false;
    // found in OpenRgbSettingsPage()
    bool all_devices = false;
};

void to_json(json& j, const ServerSettings& settings);
void from_json(const json& j, ServerSettings& settings);

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
