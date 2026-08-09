/*---------------------------------------------------------*\
| RedragonK652Controller.h                                 |
|                                                          |
|   Experimental controller for the wired Redragon K652    |
|                                                          |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <hidapi.h>

#include "RGBController.h"
#include "RedragonK652DeviceRegistry.h"

namespace redragon_k652
{
    static constexpr std::size_t    REPORT_SIZE            = 1032;
    static constexpr unsigned int   LED_COUNT              = 83;
    static constexpr unsigned char  MODE_OFF               = 0x00;
    static constexpr unsigned char  MODE_STATIC            = 0x01;
    static constexpr unsigned char  MODE_RESPIRE           = 0x02;
    static constexpr unsigned char  MODE_RAINBOW           = 0x03;
    static constexpr unsigned char  MODE_FLASH_AWAY        = 0x04;
    static constexpr unsigned char  MODE_RAINDROPS         = 0x05;
    static constexpr unsigned char  MODE_RAINBOW_WHEEL     = 0x06;
    static constexpr unsigned char  MODE_RIPPLES_SHINING   = 0x07;
    static constexpr unsigned char  MODE_STARS_TWINKLE     = 0x08;
    static constexpr unsigned char  MODE_SHADOW_DISAPPEAR  = 0x09;
    static constexpr unsigned char  MODE_RETRO_SNAKE       = 0x0A;
    static constexpr unsigned char  MODE_NEON_STREAM       = 0x0B;
    static constexpr unsigned char  MODE_REACTION          = 0x0C;
    static constexpr unsigned char  MODE_SINE_WAVE         = 0x0D;
    static constexpr unsigned char  MODE_RETINUE_SCANNING  = 0x0E;
    static constexpr unsigned char  MODE_ROTATING_WINDMILL = 0x0F;
    static constexpr unsigned char  MODE_COLORFUL_WATERFALL = 0x10;
    static constexpr unsigned char  MODE_BLOSSOMING        = 0x11;
    static constexpr unsigned char  MODE_ROTATING_STORM    = 0x12;
    static constexpr unsigned char  MODE_COLLISION          = 0x13;
    static constexpr unsigned char  MODE_DIRECT            = 0x15;

    static constexpr std::size_t    MODE_CONFIGURATION_BASE_OFFSET = 0x24;
    static constexpr std::size_t    MODE_PALETTE_BASE_OFFSET       = 0x08;
    static constexpr std::size_t    MODE_PALETTE_SIZE              = 0x15;
    static constexpr unsigned int   MODE_CONTROL_MAX               = 4;

    enum OnboardModeCapability : unsigned char
    {
        ONBOARD_MODE_CAPABILITY_NONE           = 0,
        ONBOARD_MODE_CAPABILITY_SPEED          = (1 << 0),
        ONBOARD_MODE_CAPABILITY_BRIGHTNESS     = (1 << 1),
        ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR = (1 << 2)
    };

    struct OnboardModeDescription
    {
        const char*                 name;
        unsigned char               value;
        unsigned char               capabilities;
    };

    /*
     * These are the only capture-confirmed onboard modes.  Firmware 0x14
     * remains deliberately absent, and Self-define (0x15) is kept on the
     * separate Direct protocol path.
     */
    static constexpr std::array<OnboardModeDescription, 20> ONBOARD_MODE_DESCRIPTIONS =
    {{
        { "Off",                MODE_OFF,                ONBOARD_MODE_CAPABILITY_NONE },
        { "Static",             MODE_STATIC,             ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Respire",            MODE_RESPIRE,            ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Rainbow",            MODE_RAINBOW,            ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS },
        { "Flash Away",         MODE_FLASH_AWAY,         ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Raindrops",          MODE_RAINDROPS,          ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Rainbow Wheel",      MODE_RAINBOW_WHEEL,      ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS },
        { "Ripples Shining",    MODE_RIPPLES_SHINING,    ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Stars Twinkle",      MODE_STARS_TWINKLE,      ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Shadow Disappear",   MODE_SHADOW_DISAPPEAR,   ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Retro Snake",        MODE_RETRO_SNAKE,        ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Neon Stream",        MODE_NEON_STREAM,        ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Reaction",           MODE_REACTION,           ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Sine Wave",          MODE_SINE_WAVE,          ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Retinue Scanning",   MODE_RETINUE_SCANNING,   ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Rotating Windmill",  MODE_ROTATING_WINDMILL,  ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS },
        { "Colorful Waterfall", MODE_COLORFUL_WATERFALL, ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS },
        { "Blossoming",         MODE_BLOSSOMING,         ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS },
        { "Rotating Storm",     MODE_ROTATING_STORM,     ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR },
        { "Collision",          MODE_COLLISION,          ONBOARD_MODE_CAPABILITY_SPEED | ONBOARD_MODE_CAPABILITY_BRIGHTNESS | ONBOARD_MODE_CAPABILITY_COLOR_SELECTOR }
    }};

    constexpr bool IsOnboardMode(unsigned char mode)
    {
        for(std::size_t mode_index = 0; mode_index < ONBOARD_MODE_DESCRIPTIONS.size(); mode_index++)
        {
            if(ONBOARD_MODE_DESCRIPTIONS[mode_index].value == mode)
            {
                return(true);
            }
        }

        return(false);
    }

    constexpr bool IsConfigurableOnboardMode(unsigned char mode)
    {
        return(mode >= MODE_STATIC && mode <= MODE_COLLISION);
    }

    constexpr bool OnboardModeHasCapability(unsigned char mode, unsigned char capability)
    {
        for(std::size_t mode_index = 0; mode_index < ONBOARD_MODE_DESCRIPTIONS.size(); mode_index++)
        {
            if(ONBOARD_MODE_DESCRIPTIONS[mode_index].value == mode)
            {
                return((ONBOARD_MODE_DESCRIPTIONS[mode_index].capabilities & capability) != 0);
            }
        }

        return(false);
    }

    constexpr std::size_t ConfigurationOffset(unsigned char mode)
    {
        return(MODE_CONFIGURATION_BASE_OFFSET + (2 * static_cast<std::size_t>(mode)));
    }

    constexpr std::size_t PaletteOffset(unsigned char mode)
    {
        return(MODE_PALETTE_BASE_OFFSET + (MODE_PALETTE_SIZE * static_cast<std::size_t>(mode)));
    }

    constexpr bool OnboardModeDescriptionsAreComplete()
    {
        for(unsigned char mode = MODE_OFF; mode <= MODE_COLLISION; mode++)
        {
            if(!IsOnboardMode(mode))
            {
                return(false);
            }
        }

        for(std::size_t first_index = 0; first_index < ONBOARD_MODE_DESCRIPTIONS.size(); first_index++)
        {
            for(std::size_t second_index = first_index + 1; second_index < ONBOARD_MODE_DESCRIPTIONS.size(); second_index++)
            {
                if(ONBOARD_MODE_DESCRIPTIONS[first_index].value == ONBOARD_MODE_DESCRIPTIONS[second_index].value)
                {
                    return(false);
                }
            }
        }

        return(true);
    }

    static_assert(ONBOARD_MODE_DESCRIPTIONS.size() == static_cast<std::size_t>(MODE_COLLISION) + 1,
                  "K652 onboard mode table must cover exactly firmware modes 0x00 through 0x13");
    static_assert(OnboardModeDescriptionsAreComplete(), "K652 onboard mode table must contain every ID exactly once");
    static_assert(!IsOnboardMode(0x14) && !IsOnboardMode(MODE_DIRECT),
                  "K652 unknown and Direct firmware IDs must remain outside the onboard-effects table");

    extern const std::array<unsigned char, LED_COUNT> KEY_INDICES;
}

struct RedragonK652HIDOperations
{
    int  (*send_feature_report)(hid_device*, const unsigned char*, std::size_t);
    int  (*get_feature_report)(hid_device*, unsigned char*, std::size_t);
    void (*close)(hid_device*);
    void (*sleep_ms)(unsigned int);
};

class RedragonK652Controller
{
public:
    RedragonK652Controller(hid_device* dev_cmd_handle, hid_device* dev_data_handle, const char* path, std::string dev_name,
                          RedragonK652DeviceClaim device_claim = {}, const RedragonK652HIDOperations* operations = nullptr);
    ~RedragonK652Controller();

    /* Active validation; call only after the complete passive K652 fingerprint succeeds. */
    static bool     ValidateConfiguration(hid_device* dev_cmd_handle, hid_device* dev_data_handle,
                                          const RedragonK652HIDOperations* operations = nullptr);

    std::string     GetLocation();
    std::string     GetName();
    unsigned int    GetLEDCount();

    /* Permanently enables user/API lighting control for this instance. */
    void            EnableUserLightingControl();
    bool            ApplyStatic(RGBColor color);
    bool            ApplyOnboardMode(unsigned char mode, RGBColor color, unsigned int speed, unsigned int brightness, bool rgb_mode);
    bool            ApplyDirect(const std::vector<RGBColor>& colors);

private:
    bool            QueryConfiguration();
    bool            SendReport(const std::array<unsigned char, redragon_k652::REPORT_SIZE>& report);

    hid_device*     dev_cmd;
    hid_device*     dev_data;
    RedragonK652DeviceClaim device_claim;
    RedragonK652HIDOperations operations;
    std::atomic<bool> user_lighting_control_enabled;
    std::mutex      access_mutex;
    std::array<RGBColor, redragon_k652::MODE_COLLISION + 1> onboard_palette_colors;
    std::array<std::array<unsigned char, 2>, redragon_k652::MODE_COLLISION + 1> onboard_mode_configurations;
    std::string     location;
    std::string     name;
};
