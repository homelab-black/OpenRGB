/*---------------------------------------------------------*\
| ASRockPolychromeUSBController.h                           |
|                                                           |
|   Driver for ASRock Polychrome USB motherboards           |
|                                                           |
|   Ed Kambulow (dredvard)                      20 Dec 2020 |
|   Shady Nawara (ShadyNawara)                  16 Jan 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*----------------------------------------------------------------------------------------------*\
| Definitions for Polychrome USB                                                                 |
\*----------------------------------------------------------------------------------------------*/
#define POLYCHROME_USB_NUM_MODES            16          /* Number of Polychrome USB modes       */

enum
{
    POLYCHROME_USB_MODE_OFF                 = 0x00,     /* OFF mode                             */
    POLYCHROME_USB_MODE_STATIC              = 0x01,     /* Static color mode                    */
    POLYCHROME_USB_MODE_BREATHING           = 0x02,     /* Breathing effect mode                */
    POLYCHROME_USB_MODE_STROBE              = 0x03,     /* Strobe effect mode                   */
    POLYCHROME_USB_MODE_SPECTRUM_CYCLE      = 0x04,     /* Spectrum Cycle effect mode           */
    POLYCHROME_USB_MODE_RANDOM              = 0x05,     /* Random effect mode                   */
    POLYCHROME_USB_MODE_MUSIC               = 0x06,     /* Random effect mode                   */
    POLYCHROME_USB_MODE_WAVE                = 0x07,     /* Wave effect mode                     */
    POLYCHROME_USB_MODE_SPRING              = 0x08,     /* Spring effect mode                   */
    POLYCHROME_USB_MODE_STACK               = 0x09,     /* Stack effect mode                    */
    POLYCHROME_USB_MODE_CRAM                = 0x0A,     /* Cram effect mode                     */
    POLYCHROME_USB_MODE_SCAN                = 0x0B,     /* Scan effect mode                     */
    POLYCHROME_USB_MODE_NEON                = 0x0C,     /* Neon effect mode                     */
    POLYCHROME_USB_MODE_WATER               = 0x0D,     /* Water effect mode                    */
    POLYCHROME_USB_MODE_RAINBOW             = 0x0E,     /* Rainbow effect mode                  */
    POLYCHROME_USB_MODE_DIRECT              = 0x0F,     /* CHROMA CONNECT effect mode           */
};

enum
{
    POLYCHROME_USB_SPEED_MIN                = 0xFF,     /* Slowest speed                        */
    POLYCHROME_USB_SPEED_DEFAULT            = 0xE0,     /* Default speed                        */
    POLYCHROME_USB_SPEED_MAX                = 0x00,     /* Fastest speed                        */
};

enum
{
    POLYCHROME_USB_ZONE_MAX_NUM             = 0x08,     /* Total Max number of zones            */
    POLYCHROME_USB_ZONE_ADDRESSABLE_MAX     = 0x64,     /* Maxinum number of ARGB LEDs          */
};

enum
{
    POLYCHROME_USB_LEDCOUNT_CFG             = 0x02,     // Config for LED Count
    POLYCHROME_USB_RGSWAP_CFG               = 0x03,     // Config for RGSWAP
    POLYCHROME_USB_ZONE_UNAVAILABLE         = 0x1E,     // Value from LEDCOUNT CFG if zone not present
};

extern const char* polychrome_USB_zone_names[];

enum
{
    POLYCHROME_USB_ZONE_RGB1         = 0x00,  // RGB Header 1
    POLYCHROME_USB_ZONE_RGB2         = 0X01,  // RGB Header 2
    POLYCHROME_USB_ZONE_ARGB1        = 0X02,  // ARGB Header 1
    POLYCHROME_USB_ZONE_ARGB2        = 0X03,  // ARGB Header 2
    POLYCHROME_USB_ZONE_PCH          = 0X04,  // PCH
    POLYCHROME_USB_ZONE_IOCOVER      = 0X05,  // IOCOVER
    POLYCHROME_USB_ZONE_PCB          = 0X06,  // PCB - Could be mixed swapped with 0x07
    POLYCHROME_USB_ZONE_AUDIO        = 0X07   // AUDIO/ARGB Header 3
};

enum class PolychromeDeviceType
{
    FIXED,
    ADDRESSABLE,
};

struct PolychromeZoneInfo
{
    unsigned char   mode;
    unsigned char   zone;
    unsigned char   speed;
    RGBColor        color;
};

struct PolychromeDeviceInfo
{
    unsigned char           effect_channel;
    unsigned char           num_leds;
	unsigned char			zone_type;
    bool        			rgswap;
    PolychromeDeviceType    device_type;
};

struct PolyChromeRGSwapZones
{
    bool rgb_led_1_header;
    bool rgb_led_2_header;
    bool addressable_header_1;
    bool addressable_header_2;
    bool pch;
    bool io_cover;
    bool pcb;
    bool addressable_header_3;
};

inline void to_json(json& j, const PolyChromeRGSwapZones& settings)
{
    j = json {
        {"RGB LED 1 Header", settings.rgb_led_1_header},
        {"RGB LED 2 Header", settings.rgb_led_2_header},
        {"Addressable Header 1", settings.addressable_header_1},
        {"Addressable Header 2", settings.addressable_header_2},
        {"PCH", settings.pch},
        {"IO Cover", settings.io_cover},
        {"PCB", settings.pcb},
        {"Addressable Header 3/Audio", settings.addressable_header_3},
    };
}

inline void from_json(const json& j, PolyChromeRGSwapZones& settings)
{
    j.at("RGB LED 1 Header").get_to(settings.rgb_led_1_header);
    j.at("RGB LED 2 Header").get_to(settings.rgb_led_2_header);
    j.at("Addressable Header 1").get_to(settings.addressable_header_1);
    j.at("Addressable Header 2").get_to(settings.addressable_header_2);
    j.at("PCH").get_to(settings.pch);
    j.at("IO Cover").get_to(settings.io_cover);
    j.at("PCB").get_to(settings.pcb);
    j.at("Addressable Header 3/Audio").get_to(settings.addressable_header_3);
}

struct PolyChromeUSBSettings
{
    PolyChromeRGSwapZones rg_swap;
};

inline void to_json(json& j, const PolyChromeUSBSettings& settings)
{
    j = json {
        {"RGSwap", settings.rg_swap},
    };
}

inline void from_json(const json& j, PolyChromeUSBSettings& settings)
{
    j.at("RGSwap").get_to(settings.rg_swap);
}

class PolychromeUSBController
{
public:
    unsigned char   zone_led_count[8];

    PolychromeUSBController(hid_device* dev_handle, const char* path);
    ~PolychromeUSBController();

    unsigned int                                GetChannelCount();
    std::string                                 GetDeviceLocation();
    std::string                                 GetDeviceName();
    const std::vector<PolychromeDeviceInfo>&    GetPolychromeDevices() const;
    std::string                                 GetSerialString();
    PolychromeZoneInfo                          GetZoneConfig (unsigned char zone);

    void                                        WriteZone
                                                    (
                                                    unsigned char   zone,
                                                    unsigned char   mode,
                                                    unsigned char   speed,
                                                    RGBColor        rgb,
                                                    bool            allzone
                                                    );

    void                                        WriteAllZones
                                                    (
                                                    const std::vector<PolychromeZoneInfo> &zones_info,
                                                    const std::vector<zone> &zones
                                                    );

    void                                        WriteHeader
                                                    (
                                                    unsigned char   cfg,
                                                    unsigned char*  configstring,
                                                    unsigned int    configsize
                                                    );

    void                                        DeviceResizeZone(int zone, int new_size);
    void                                        SetRGSwap(bool reset);

protected:
    hid_device*                         dev;
    std::vector<PolychromeDeviceInfo>   device_info;
    std::string                         location;

    void WriteRGSwap
        (
        bool hdr0,
        bool hdr1,
        bool ahdr0,
        bool ahdr1,
        bool pch,
        bool io,
        bool pcb,
        bool chnl8
        );

private:
    std::string   device_name;
    unsigned char configtable[12];
    bool          rgswapconfig[8] = { 0 };

    void SetDeviceInfo();
    void ReadConfigTables();
    void Commit();
};
