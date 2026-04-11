/*---------------------------------------------------------*\
| LogitechHIDPPController.h                                 |
|                                                           |
|   Unified Logitech HID++ 2.0 controller                  |
|                                                           |
|   Uses feature discovery (IRoot 0x0000) to dynamically    |
|   determine device capabilities and adapt to any HID++    |
|   2.0 device with RGB lighting support.                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <deque>
#include <condition_variable>
#include <hidapi.h>
#include "RGBController.h"
#include "LogitechProtocolCommon.h"

/*-----------------------------------------------------*\
| HID++ 2.0 Feature Page IDs                            |
\*-----------------------------------------------------*/
#define HIDPP_FEAT_IROOT                    0x0000
#define HIDPP_FEAT_FEATURE_SET              0x0001
#define HIDPP_FEAT_DEVICE_NAME_TYPE         0x0005
#define HIDPP_FEAT_ONBOARD_PROFILES         0x8100
#define HIDPP_FEAT_PROFILE_MANAGEMENT       0x8101
#define HIDPP_FEAT_FIRMWARE_INFO            0x0003
#define HIDPP_FEAT_CENTPPBRIDGE             0x0003  /* same ID, different meaning on Centurion */
#define HIDPP_FEAT_COLOR_LED_EFFECTS        0x8070
#define HIDPP_FEAT_RGB_EFFECTS              0x8071
#define HIDPP_FEAT_PER_KEY_LIGHTING_V1      0x8080
#define HIDPP_FEAT_PER_KEY_LIGHTING_V2      0x8081
#define HIDPP_FEAT_KEYBOARD_LAYOUT          0x4540
#define HIDPP_FEAT_CENTURION_RGB            0x0600
#define HIDPP_FEAT_CENTURION_DEVICE_INFO   0x0100
#define HIDPP_FEAT_CENTURION_DEVICE_NAME   0x0101
#define HIDPP_FEAT_WIRELESS_STATUS          0x1D4B

/*-----------------------------------------------------*\
| HID++ 2.0 Function IDs (byte 3 high nibble)           |
| Function ID is shifted left 4 bits, low nibble = swID |
\*-----------------------------------------------------*/
/*-----------------------------------------------------*\
| HID++ Software ID — identifies our responses.         |
| Must avoid: 0x00 (firmware), 0x01 (GHUB),             |
| 0x02-0x0F (Solaar cycles these).                      |
| There are only 16 values (4-bit field), and all are   |
| claimed. We pick a fixed value and will coordinate    |
| with Solaar to exclude it from its cycle.             |
\*-----------------------------------------------------*/
#define HIDPP_SW_ID                         0x07

/*-----------------------------------------------------*\
| Feature 0x8071 functions                              |
\*-----------------------------------------------------*/
#define FN_8071_GET_INFO                    0x00
#define FN_8071_SET_EFFECT                  0x10
#define FN_8071_SET_PATTERN                 0x20
#define FN_8071_NV_CONFIG                   0x30
#define FN_8071_BIN_INFO                    0x40
#define FN_8071_SW_CONTROL                  0x50
#define FN_8071_SYNC                        0x60
#define FN_8071_PWR_CONFIG                  0x70
#define FN_8071_PWR_MODE                    0x80

/*-----------------------------------------------------*\
| Feature 0x8081 functions                              |
\*-----------------------------------------------------*/
#define FN_8081_GET_INFO                    0x00
#define FN_8081_SET_INDIVIDUAL              0x10
#define FN_8081_SET_CONSECUTIVE             0x20
#define FN_8081_SET_DELTA_5BIT              0x30
#define FN_8081_SET_DELTA_4BIT              0x40
#define FN_8081_SET_RANGE                   0x50
#define FN_8081_SET_SINGLE_VALUE            0x60
#define FN_8081_FRAME_END                   0x70

/*-----------------------------------------------------*\
| Feature 0x8100 functions                              |
\*-----------------------------------------------------*/
#define FN_8100_SET_ONBOARD_MODE            0x10
#define FN_8100_GET_ONBOARD_MODE            0x20

/*-----------------------------------------------------*\
| Feature 0x8101 functions                              |
\*-----------------------------------------------------*/
#define FN_8101_GET_SET_MODE                0x60

/*-----------------------------------------------------*\
| Zone cluster effect entry                             |
\*-----------------------------------------------------*/
struct HIDPPEffect
{
    uint8_t     index;
    uint16_t    effect_id;
    uint16_t    capabilities;
    uint16_t    default_period;
};

/*-----------------------------------------------------*\
| Zone cluster info from 0x8071 GetRgbClusterInfo       |
\*-----------------------------------------------------*/
struct HIDPPZoneCluster
{
    uint8_t                     index;
    uint16_t                    location;
    uint8_t                     effect_count;
    std::vector<HIDPPEffect>    effects;
};

/*-----------------------------------------------------*\
| Device capabilities discovered via feature probing    |
\*-----------------------------------------------------*/
struct HIDPPDeviceCapabilities
{
    std::string     device_name;
    uint8_t         device_type;
    std::string     firmware_version;
    std::string     serial_number;
    std::string     unit_id;            // stable hardware ID (from FirmwareInfo fn0)
    uint16_t        pid_wireless;       // wireless virtual PID (from FirmwareInfo fn0)
    uint16_t        pid_wired;          // wired USB PID (from FirmwareInfo fn0)

    /*-------------------------------------------------*\
    | Complete feature map (feature_id → runtime index) |
    | Built once by EnumerateFeatures, used by all      |
    | subsequent GetFeatureIndex lookups (no wire).      |
    \*-------------------------------------------------*/
    std::map<uint16_t, uint8_t> feature_map;
    bool        feature_map_complete;   // true after bulk enumeration

    /*-------------------------------------------------*\
    | Feature indices (0 = not supported)               |
    \*-------------------------------------------------*/
    uint8_t         idx_onboard_profiles;
    uint8_t         idx_profile_management;
    uint8_t         idx_rgb_effects;
    uint8_t         idx_perkey_v2;
    uint8_t         idx_perkey_v1;
    uint8_t         idx_wireless_status;
    uint16_t        rgb_feature_page;

    /*-------------------------------------------------*\
    | Resolved function IDs for the RGB effects feature  |
    | Varies between 0x8070, 0x8071, 0x0600              |
    \*-------------------------------------------------*/
    uint8_t         fn_set_effect;
    uint8_t         fn_sw_control;
    uint8_t         fn_pwr_config;
    uint8_t         fn_pwr_mode;
    bool            has_power_mgmt;
    bool            sw_control_simple;

    /*-------------------------------------------------*\
    | Persistent NV settings read from RGBEffects fn3    |
    | (FN_8071_NV_CONFIG). Capability 0x0020 is the      |
    | sleep ramp / off-ramp transition (enabled +        |
    | ramp_seconds).                                     |
    \*-------------------------------------------------*/
    bool            nv_sleep_ramp_known;
    bool            nv_sleep_ramp_enabled;
    uint8_t         nv_sleep_ramp_seconds;

    /*-------------------------------------------------*\
    | Discovered zone and LED data                      |
    \*-------------------------------------------------*/
    std::vector<HIDPPZoneCluster>   zone_clusters;
    std::vector<uint16_t>           perkey_zone_ids;
    bool                            has_perkey;
    bool                            has_zone_effects;
    bool                            has_numpad;
    uint8_t                         keyboard_layout_code;
};

/*-----------------------------------------------------*\
| Transport type — determines wire framing               |
\*-----------------------------------------------------*/
enum HIDPPTransportType
{
    HIDPP_TRANSPORT_STANDARD,       // 0xFF00/0xFF43: report IDs 0x10/0x11, 7/20 bytes
    HIDPP_TRANSPORT_CENTURION       // 0xFFA0: report ID 0x51 or 0x50, 64 bytes,
                                    // with CPL framing and CentPPBridge sub-device routing
};

/*-----------------------------------------------------*\
| Transport layer — abstracts wire format differences    |
|                                                        |
| Standard HID++ and Centurion both carry the same       |
| feature/function/data payload, but with different      |
| report framing. This struct holds transport state      |
| so SendMessage/ReadMessage can adapt.                  |
\*-----------------------------------------------------*/
struct HIDPPTransport
{
    HIDPPTransportType  type;
    uint16_t            usage_page;         // 0xFF00, 0xFF43, or 0xFFA0
    uint8_t             report_id;          // 0x10/0x11 for standard, 0x51/0x50 for Centurion
    bool                addressed;          // Centurion 0x50 has device address byte
    uint8_t             device_address;     // Centurion 0x50: device address (e.g., 0x23)
    uint8_t             bridge_feat_idx;    // CentPPBridge feature index on parent (0 if N/A)
    uint8_t             sub_device_id;      // CentPPBridge sub-device ID (typically 0)
};

/*-----------------------------------------------------*\
| Power management state machine                        |
| Matches Solaar's RGBPowerManager states               |
\*-----------------------------------------------------*/
enum HIDPPPowerState
{
    HIDPP_POWER_ACTIVE   = 0,
    HIDPP_POWER_DIMMING  = 1,
    HIDPP_POWER_IDLE     = 2,
    HIDPP_POWER_SLEEPING = 3,
};

/*-----------------------------------------------------*\
| Parsed HID++ message for the response queue           |
\*-----------------------------------------------------*/
struct HIDPPRawMessage
{
    uint8_t feat;
    uint8_t func;
    uint8_t data[60];
    int     result;
};

class LogitechHIDPPController
{
public:
    LogitechHIDPPController(hid_device* dev, const char* path,
                            uint8_t device_index, bool wireless,
                            std::shared_ptr<std::mutex> mutex_ptr,
                            uint16_t usage_page = 0xFF00);
    ~LogitechHIDPPController();

    /*-------------------------------------------------*\
    | Lifecycle                                         |
    \*-------------------------------------------------*/
    bool            Probe();
    void            Initialize();
    void            Shutdown();

    /*-------------------------------------------------*\
    | Accessors                                         |
    \*-------------------------------------------------*/
    const HIDPPDeviceCapabilities&  GetCapabilities() const;
    std::string     GetDeviceLocation();
    std::string     GetSerialString();
    uint32_t        GetInitGeneration() const;

    /*-------------------------------------------------*\
    | Per-key lighting (0x8081)                          |
    \*-------------------------------------------------*/
    void            SetPerKeyColors(const std::vector<std::pair<uint16_t, RGBColor>>& zone_colors);
    void            SetAllPerKeyColor(RGBColor color);
    void            SendPerKeyData(uint8_t perkey_idx, uint8_t function,
                                   const uint8_t* data, size_t len);
    bool            PerKeyFrameEnd(bool persist = false);

    /*-------------------------------------------------*\
    | Zone effects (0x8071 / 0x8070)                    |
    \*-------------------------------------------------*/
    void            SetZoneEffect(uint8_t cluster_idx, uint8_t effect_idx,
                                  uint16_t effect_id,
                                  unsigned char r, unsigned char g, unsigned char b,
                                  uint16_t period, bool persist);

    /*-------------------------------------------------*\
    | SW control management                             |
    \*-------------------------------------------------*/
    void            SetSWControl(uint8_t mode, uint8_t flags);
    void            SetRGBPowerMode(uint8_t mode);
    void            SetHostMode();
    void            ClaimSWControlIfNeeded();

    /*-------------------------------------------------*\
    | Power management (idle/dim/sleep/wake)             |
    \*-------------------------------------------------*/
    void            StartPowerManager();
    void            StopPowerManager();
    void            StartEventWatcher();
    void            StartProbeWatcher();
    bool            HasBridge() const;
    void            SetRepaintCallback(std::function<void()> repaint);
    void            SetReinitCallback(std::function<void()> cb);
    void            SetRegisterCallback(std::function<void(RGBController*)> cb);
    HIDPPPowerState GetPowerState() const;
    int             GetDimBrightness() const;
    void            WakeIfNeeded();
    bool            IsOnline() const;
    void            FlushResponseQueue();

private:
    hid_device*                 dev;
    std::string                 location;
    uint8_t                     device_index;
    bool                        wireless;
    std::shared_ptr<std::mutex> mutex;
    HIDPPDeviceCapabilities     caps;
    HIDPPTransport              transport;
    bool                        initialized;
    bool                        sw_control_claimed;
    uint32_t                    frame_counter;
    uint32_t                    init_generation;
    std::string                 log_tag;

    /*-------------------------------------------------*\
    | Transport-layer I/O                               |
    |                                                   |
    | SendMessage/ReadMessage handle wire framing based  |
    | on transport.type. Upper layers pass feature index,|
    | function ID, and payload — the transport layer     |
    | wraps them in the correct report format.           |
    \*-------------------------------------------------*/
    int             SendMessage(uint8_t feat_idx, uint8_t function,
                                const uint8_t* data, size_t len);
    int             ReadMessage(uint8_t* feat_idx_out, uint8_t* function_out,
                                uint8_t* data_out, size_t data_max,
                                int timeout_ms = LOGITECH_PROTOCOL_TIMEOUT);

    /*-------------------------------------------------*\
    | Standard HID++ transport (0xFF00/0xFF43)           |
    \*-------------------------------------------------*/
    int             SendStandard(uint8_t feat_idx, uint8_t function,
                                 const uint8_t* data, size_t len);
    int             ReadStandardDirect(uint8_t* feat_idx_out, uint8_t* function_out,
                                       uint8_t* data_out, size_t data_max,
                                       int timeout_ms);

    /*-------------------------------------------------*\
    | Centurion transport (0xFFA0)                       |
    | Wraps messages in CPL framing, routes through      |
    | CentPPBridge for sub-device access.                |
    \*-------------------------------------------------*/
    int             SendCenturion(uint8_t feat_idx, uint8_t function,
                                  const uint8_t* data, size_t len);
    int             ReadCenturionDirect(uint8_t* feat_idx_out, uint8_t* function_out,
                                        uint8_t* data_out, size_t data_max,
                                        int timeout_ms);

    /*-------------------------------------------------*\
    | Reader thread dispatch layer                       |
    | ReadHIDDirect: raw HID read (used by reader thread |
    |   and during Probe before reader starts).          |
    | ReadFromQueue: waits on response queue filled by   |
    |   the reader thread.                               |
    \*-------------------------------------------------*/
    int             ReadHIDDirect(uint8_t* feat_idx_out, uint8_t* function_out,
                                  uint8_t* data_out, size_t data_max,
                                  int timeout_ms);
    int             ReadFromQueue(uint8_t* feat_idx_out, uint8_t* function_out,
                                  uint8_t* data_out, size_t data_max,
                                  int timeout_ms);

    /*-------------------------------------------------*\
    | High-level helpers                                 |
    | SendLong/ReadResponse are transport-aware wrappers |
    | that dispatch through the transport layer above.   |
    | All existing feature code calls these.             |
    \*-------------------------------------------------*/
    int             SendLong(uint8_t feat_idx, uint8_t function,
                             const uint8_t* data, size_t len);
    int             ReadResponse(blankFAPmessage& response,
                                 uint8_t expected_feat_idx,
                                 uint8_t expected_function,
                                 int timeout_ms = LOGITECH_PROTOCOL_TIMEOUT,
                                 int retries_left = 10);
    int             SendAndReceive(uint8_t feat_idx, uint8_t function,
                                   const uint8_t* send_data, size_t send_len,
                                   uint8_t* recv_data, size_t recv_max);

    /*-------------------------------------------------*\
    | Feature discovery                                 |
    \*-------------------------------------------------*/
    uint8_t         GetFeatureIndex(uint16_t feature_page);
    void            DiscoverTransport();
    void            DiscoverDeviceName();
    void            DiscoverDeviceType();
    void            EnumerateFeatures(uint8_t feature_set_idx);
    void            DiscoverFirmwareInfo();
    void            DiscoverRGBEffects();
    void            DiscoverPerKeyZones();
    void            DiscoverKeyboardLayout();

    /*-------------------------------------------------*\
    | Power management internals                        |
    \*-------------------------------------------------*/
    void            ReaderThreadFunc();
    void            PowerThreadFunc();
    void            DispatchEvent(uint8_t feat, uint8_t func, const uint8_t* data);
    void            OnUserActivity(uint8_t activity_type);
    void            StartDimRamp();
    void            DimRampStep();
    void            StartSleep();
    void            Wake();
    void            ReadFirmwareTimers();
    void            ReadNvSleepRampConfig();
    void            WritePowerConfig(uint16_t idle_s, uint16_t sleep_s);
    void            ReprobeSubDevice();
    void            ReconnectDevice();
    void            FullReprobe();
    bool            ScanForDevice();
    void            SwapHIDHandle(hid_device* new_dev, const std::string& new_path);

    /*-------------------------------------------------*\
    | Reader thread + response queue                    |
    \*-------------------------------------------------*/
    std::thread*            reader_thread;
    std::atomic<bool>       reader_running;
    std::mutex              response_mutex;
    std::condition_variable response_cv;
    std::deque<HIDPPRawMessage> response_queue;

    /*-------------------------------------------------*\
    | Power thread (state machine + command sender)     |
    \*-------------------------------------------------*/
    std::thread*            power_thread;
    std::atomic<bool>       power_thread_running;
    std::atomic<int>        pending_activity;       // -1=none, 0=idle, 1+=active
    std::atomic<int>        pending_connection;     // 0=none, +1=connected, -1=disconnected
    std::atomic<bool>       device_online;          // false when device is unreachable
    std::atomic<int>        consecutive_timeouts;   // reset on successful response
    std::atomic<bool>       watcher_mode;           // true when retrying failed probe

    /*-------------------------------------------------*\
    | Power management state                            |
    \*-------------------------------------------------*/
    HIDPPPowerState         power_state;
    std::mutex              power_mutex;

    /*-------------------------------------------------*\
    | Dim ramp state                                    |
    | dim_brightness_pct is applied by DeviceUpdateLEDs |
    | to scale colors before pushing to device.         |
    | This is our own host-side animation, independent  |
    | of any firmware dim/sleep timers.                 |
    \*-------------------------------------------------*/
    static const int DIM_STEPS       = 25;
    static const int DIM_INTERVAL_MS = 200;
    static const int DIM_TARGET_PCT  = 50;
    std::atomic<int> dim_brightness_pct;    // 100=full, 50=dimmed
    int              dim_step;
    std::chrono::steady_clock::time_point next_dim_time;

    /*-------------------------------------------------*\
    | Sleep timer                                       |
    \*-------------------------------------------------*/
    std::chrono::steady_clock::time_point sleep_deadline;

    /*-------------------------------------------------*\
    | Firmware power config (read from device)          |
    \*-------------------------------------------------*/
    uint16_t        idle_timeout_s;
    uint16_t        sleep_timeout_s;

    /*-------------------------------------------------*\
    | Callbacks                                         |
    \*-------------------------------------------------*/
    std::function<void()> request_repaint_fn;
    std::function<void()> reinit_direct_fn;
    std::function<void(RGBController*)> register_controller_fn;
};
