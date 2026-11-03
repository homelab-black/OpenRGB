/*---------------------------------------------------------*\
| RGBController_Lenovo_Gen7_8.cpp                           |
|                                                           |
|   RGBController for Lenovo Gen7 and Gen8 devices          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <vector>
#include <hidapi.h>
#include "RGBController_Lenovo_Gen7_8.h"
#include "LenovoDevices.h"
#include "Legion9ProfileGuard.h"
#include "HIDLampArrayController.h"
#include "RGBController_HIDLampArray.h"

using namespace std;

/**------------------------------------------------------------------*\
    @name Lenovo Legion Gen 7-10
    @category Keyboard
    @type USB
    @save :robot:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectLenovoLegionUSBControllersGen7And8
    @comment Legion 9 18IAX10 is 048D:C909. Direct on that keyboard is a per-key static profile. The chassis strip is 048D:C903 and shares this device view. Tested on Nobara Linux 44.
\*-------------------------------------------------------------------*/

static bool UsesGen10PacketFormat(uint16_t pid)
{
    return pid == LEGION_7GEN10 || pid == LEGION_5GEN10 || pid == LEGION_9GEN10;
}

/*---------------------------------------------------------*\
| Legion 9 18IAX10 ignores Aurora direct packets. Per-key   |
| colors apply only as a static profile, which is what      |
| Static already sends. Direct on this keyboard uses that   |
| write so a color edit shows up without changing mode.     |
\*---------------------------------------------------------*/
static bool DirectUsesStaticProfile(uint16_t pid)
{
    return pid == LEGION_9GEN10;
}

static bool Is24ZoneDevice(uint16_t pid)
{
    return pid == LEGION_5GEN10;
}

static bool IsKeyboardOnlyDevice(uint16_t pid)
{
    return pid == LEGION_5GEN10;
}

#define LEGION9_LIGHT_STRIP_VID 0x048D
#define LEGION9_LIGHT_STRIP_PID 0xC903
#define HID_LAMPARRAY_PURPOSE_CONTROL   0x01
#define HID_LAMPARRAY_PURPOSE_ACCENT    0x02
#define HID_LAMPARRAY_PURPOSE_BRANDING  0x04

/*---------------------------------------------------------*\
| Open the chassis LampArray (048d:c903, usage page 0x59).  |
| The keyboard keys are advertised there too; the Spectrum  |
| controller keeps those. This handle is the strip and logo.|
\*---------------------------------------------------------*/
static HIDLampArrayController* OpenLegion9LightStrip()
{
    hid_device_info*        infos   = hid_enumerate(LEGION9_LIGHT_STRIP_VID, LEGION9_LIGHT_STRIP_PID);
    HIDLampArrayController* strip   = nullptr;
    hid_device_info*        chosen  = nullptr;

    for(hid_device_info* info = infos; info != nullptr; info = info->next)
    {
        if(info->usage_page == 0x59 && info->usage == 0x01)
        {
            chosen = info;
            break;
        }
    }

    if(chosen == nullptr)
    {
        for(hid_device_info* info = infos; info != nullptr; info = info->next)
        {
            if(info->interface_number == 1)
            {
                chosen = info;
                break;
            }
        }
    }

    if(chosen != nullptr)
    {
        hid_device* dev = hid_open_path(chosen->path);

        if(dev)
        {
            strip = new HIDLampArrayController(dev, chosen->path);

            if(strip->GetLampCount() == 0)
            {
                delete strip;
                strip = nullptr;
            }
        }
    }

    hid_free_enumeration(infos);

    if(strip == nullptr)
    {
        LOG_WARNING("[Lenovo Legion 9] light strip 048D:C903 did not open. Using the keyboard-only map.");
    }
    else
    {
        LOG_INFO("[Lenovo Legion 9] light strip 048D:C903 opened with %u lamps.", strip->GetLampCount());
    }

    return(strip);
}

static std::vector<unsigned short> LampIdsInRange(const std::vector<LampAttributes>& lamps, size_t begin, size_t end)
{
    std::vector<unsigned short> ids;

    for(size_t index = begin; index < end && index < lamps.size(); index++)
    {
        ids.push_back(lamps[index].LampId);
    }

    return(ids);
}

static const lenovo_led legion_5gen10_24zone_leds[] =
{
    {0x01, "Zone R1C1"},
    {0x02, "Zone R1C2"},
    {0x03, "Zone R1C3"},
    {0x04, "Zone R1C4"},
    {0x05, "Zone R1C5"},
    {0x06, "Zone R1C6"},
    {0x07, "Zone R2C1"},
    {0x08, "Zone R2C2"},
    {0x09, "Zone R2C3"},
    {0x0A, "Zone R2C4"},
    {0x0B, "Zone R2C5"},
    {0x0C, "Zone R2C6"},
    {0x0D, "Zone R3C1"},
    {0x0E, "Zone R3C2"},
    {0x0F, "Zone R3C3"},
    {0x10, "Zone R3C4"},
    {0x11, "Zone R3C5"},
    {0x12, "Zone R3C6"},
    {0x13, "Zone R4C1"},
    {0x14, "Zone R4C2"},
    {0x15, "Zone R4C3"},
    {0x16, "Zone R4C4"},
    {0x17, "Zone R4C5"},
    {0x18, "Zone R4C6"},
};

static const unsigned int legion_5gen10_24zone_matrix_map[] =
{
    0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23,
};

static const lenovo_zone legion_5gen10_kbd_24zone =
{
    "Keyboard (24-zone)",
    ZONE_TYPE_MATRIX,
    0,
    4,
    6,
    legion_5gen10_24zone_matrix_map,
    legion_5gen10_24zone_leds,
    0,
    23,
};

static const RGBColor legion_5gen10_zone_visualization_colors[24] =
{
    0xFF0000, 0x00FFFF, 0xFFFF00, 0x0000FF, 0x00FF00, 0xFF00FF,
    0xFF7F00, 0x007FFF, 0x7FFF00, 0x7F00FF, 0xFF007F, 0x00FF7F,
    0xFFD700, 0x0055FF, 0x55FF00, 0xFF0055, 0x00BFFF, 0xBF00FF,
    0xFF3B00, 0x00FF3B, 0x3B00FF, 0xFF1493, 0x14FF93, 0x9314FF,
};

LenovoRGBController_Gen7_8::LenovoRGBController_Gen7_8(LenovoGen7And8USBController* controller_ptr)
{
    controller              = controller_ptr;
    light_strip             = nullptr;
    use_frame_colors        = false;
    preserve_key_lamps      = false;
    direct_profile_restore  = false;

    mode Screw;
    Screw.name = "Screw Rainbow";
    Screw.value = LENOVO_LEGION_GEN7_8_MODE_SCREW_RAINBOW;
    Screw.flags = MODE_FLAG_HAS_SPEED |
        MODE_FLAG_HAS_DIRECTION_LR |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    Screw.speed_min = 0x01;
    Screw.speed_max = 0x03;
    Screw.speed = 2;
    Screw.color_mode = MODE_COLORS_NONE;
    Screw.brightness_min = 0;
    Screw.brightness_max = 9;
    Screw.brightness = brightness;
    Screw.direction = MODE_DIRECTION_RIGHT;
    modes.push_back(Screw);

    mode Rainbow;
    Rainbow.name = "Rainbow Wave";
    Rainbow.value = LENOVO_LEGION_GEN7_8_MODE_RAINBOW_WAVE;
    Rainbow.flags = MODE_FLAG_HAS_SPEED |
        MODE_FLAG_HAS_DIRECTION_LR |
        MODE_FLAG_HAS_DIRECTION_UD |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    Rainbow.speed_min = 0x01;
    Rainbow.speed_max = 0x03;
    Rainbow.speed = 2;
    Rainbow.color_mode = MODE_COLORS_NONE;
    Rainbow.brightness_min = 0;
    Rainbow.brightness_max = 9;
    Rainbow.brightness = brightness;
    Rainbow.direction = MODE_DIRECTION_RIGHT;
    modes.push_back(Rainbow);

    mode ColorChange;
    ColorChange.name = "Color Change";
    ColorChange.value = LENOVO_LEGION_GEN7_8_MODE_COLOR_CHANGE;
    ColorChange.flags = MODE_FLAG_HAS_SPEED |
        MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
        MODE_FLAG_HAS_RANDOM_COLOR |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    ColorChange.speed_min = 0x01;
    ColorChange.speed_max = 0x03;
    ColorChange.speed = 2;
    ColorChange.colors_min = 1;
    ColorChange.colors_max = 4;
    ColorChange.colors.resize(4);
    ColorChange.colors[0] = 0xFFF500;
    ColorChange.color_mode = MODE_COLORS_RANDOM;
    ColorChange.brightness_min = 0;
    ColorChange.brightness_max = 9;
    ColorChange.brightness = brightness;
    modes.push_back(ColorChange);

    mode ColorPulse;
    ColorPulse.name = "Color Pulse";
    ColorPulse.value = LENOVO_LEGION_GEN7_8_MODE_COLOR_PULSE;
    ColorPulse.flags = MODE_FLAG_HAS_SPEED |
        MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
        MODE_FLAG_HAS_RANDOM_COLOR |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    ColorPulse.speed_min = 0x01;
    ColorPulse.speed_max = 0x03;
    ColorPulse.speed = 2;
    ColorPulse.colors_min = 1;
    ColorPulse.colors_max = 4;
    ColorPulse.colors.resize(4);
    ColorPulse.colors[0] = 0xFFF500;
    ColorPulse.color_mode = MODE_COLORS_RANDOM;
    ColorPulse.brightness_min = 0;
    ColorPulse.brightness_max = 9;
    ColorPulse.brightness = brightness;
    modes.push_back(ColorPulse);

    mode ColorWave;
    ColorWave.name = "Color Wave";
    ColorWave.value = LENOVO_LEGION_GEN7_8_MODE_COLOR_WAVE;
    ColorWave.flags = MODE_FLAG_HAS_SPEED |
        MODE_FLAG_HAS_DIRECTION_LR |
        MODE_FLAG_HAS_DIRECTION_UD |
        MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
        MODE_FLAG_HAS_RANDOM_COLOR |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    ColorWave.speed_min = 0x01;
    ColorWave.speed_max = 0x03;
    ColorWave.speed = 2;
    ColorWave.colors_min = 1;
    ColorWave.colors_max = 4;
    ColorWave.colors.resize(4);
    ColorWave.colors[0] = 0xFFF500;
    ColorWave.color_mode = MODE_COLORS_RANDOM;
    ColorWave.brightness_min = 0;
    ColorWave.brightness_max = 9;
    ColorWave.brightness = brightness;
    ColorWave.direction = MODE_DIRECTION_RIGHT;
    modes.push_back(ColorWave);

    mode Smooth;
    Smooth.name = "Smooth";
    Smooth.value = LENOVO_LEGION_GEN7_8_MODE_SMOOTH;
    Smooth.flags = MODE_FLAG_HAS_SPEED |
        MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
        MODE_FLAG_HAS_RANDOM_COLOR |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    Smooth.speed_min = 0x01;
    Smooth.speed_max = 0x03;
    Smooth.speed = 2;
    Smooth.colors_min = 1;
    Smooth.colors_max = 4;
    Smooth.colors.resize(4);
    Smooth.colors[0] = 0xFFF500;
    Smooth.color_mode = MODE_COLORS_RANDOM;
    Smooth.brightness_min = 0;
    Smooth.brightness_max = 9;
    Smooth.brightness = brightness;
    modes.push_back(Smooth);

    if(!Is24ZoneDevice(controller->getPid()))
    {
        mode Rain;
        Rain.name = "Rain";
        Rain.value = LENOVO_LEGION_GEN7_8_MODE_RAIN;
        Rain.flags = MODE_FLAG_HAS_SPEED |
            MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
            MODE_FLAG_HAS_RANDOM_COLOR |
            MODE_FLAG_HAS_BRIGHTNESS |
            MODE_FLAG_AUTOMATIC_SAVE;
        Rain.speed_min = 0x01;
        Rain.speed_max = 0x03;
        Rain.speed = 2;
        Rain.colors_min = 1;
        Rain.colors_max = 4;
        Rain.colors.resize(4);
        Rain.colors[0] = 0xFFF500;
        Rain.color_mode = MODE_COLORS_RANDOM;
        Rain.brightness_min = 0;
        Rain.brightness_max = 9;
        Rain.brightness = brightness;
        modes.push_back(Rain);
    }

    if(!IsKeyboardOnlyDevice(controller->getPid()))
    {
        mode Ripple;
        Ripple.name = "Ripple";
        Ripple.value = LENOVO_LEGION_GEN7_8_MODE_RIPPLE;
        Ripple.flags = MODE_FLAG_HAS_SPEED |
            MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
            MODE_FLAG_HAS_RANDOM_COLOR |
            MODE_FLAG_HAS_BRIGHTNESS |
            MODE_FLAG_AUTOMATIC_SAVE;
        Ripple.speed_min = 0x01;
        Ripple.speed_max = 0x03;
        Ripple.speed = 2;
        Ripple.colors_min = 1;
        Ripple.colors_max = 4;
        Ripple.colors.resize(4);
        Ripple.colors[0] = 0xFFF500;
        Ripple.color_mode = MODE_COLORS_RANDOM;
        Ripple.brightness_min = 0;
        Ripple.brightness_max = 9;
        Ripple.brightness = brightness;
        modes.push_back(Ripple);

        mode AudioBounce;
        AudioBounce.name = "Audio Bounce Lighting";
        AudioBounce.value = LENOVO_LEGION_GEN7_8_MODE_AUDIO_BOUNCE;
        AudioBounce.flags = MODE_FLAG_HAS_BRIGHTNESS;
        AudioBounce.color_mode = MODE_COLORS_NONE;
        AudioBounce.brightness_min = 0;
        AudioBounce.brightness_max = 9;
        AudioBounce.brightness = brightness;
        modes.push_back(AudioBounce);

        mode AudioRipple;
        AudioRipple.name = "Audio Ripple Lighting";
        AudioRipple.value = LENOVO_LEGION_GEN7_8_MODE_AUDIO_RIPPLE;
        AudioRipple.flags = MODE_FLAG_HAS_BRIGHTNESS;
        AudioRipple.color_mode = MODE_COLORS_NONE;
        AudioRipple.brightness_min = 0;
        AudioRipple.brightness_max = 9;
        AudioRipple.brightness = brightness;
        modes.push_back(AudioRipple);
    }

    mode Static;
    Static.name = "Static";
    Static.value = LENOVO_LEGION_GEN7_8_MODE_STATIC;
    Static.flags = MODE_FLAG_HAS_PER_LED_COLOR |
        MODE_FLAG_HAS_RANDOM_COLOR |
        MODE_FLAG_HAS_BRIGHTNESS |
        MODE_FLAG_AUTOMATIC_SAVE;
    Static.color_mode = MODE_COLORS_PER_LED;
    Static.brightness_min = 0;
    Static.brightness_max = 9;
    Static.brightness = brightness;
    modes.push_back(Static);

    if(!Is24ZoneDevice(controller->getPid()))
    {
        mode Type;
        Type.name = "Type Lighting";
        Type.value = LENOVO_LEGION_GEN7_8_MODE_TYPE;
        Type.flags = MODE_FLAG_HAS_SPEED |
            MODE_FLAG_HAS_MODE_SPECIFIC_COLOR |
            MODE_FLAG_HAS_RANDOM_COLOR |
            MODE_FLAG_HAS_BRIGHTNESS |
            MODE_FLAG_AUTOMATIC_SAVE;
        Type.speed_min = 0x01;
        Type.speed_max = 0x03;
        Type.speed = 2;
        Type.colors_min = 1;
        Type.colors_max = 4;
        Type.colors.resize(4);
        Type.colors[0] = 0xFFF500;
        Type.color_mode = MODE_COLORS_RANDOM;
        Type.brightness_min = 0;
        Type.brightness_max = 9;
        Type.brightness = brightness;
        modes.push_back(Type);
    }

    if(!IsKeyboardOnlyDevice(controller->getPid()))
    {
        mode Direct;
        Direct.name = "Direct";
        Direct.value = LENOVO_LEGION_GEN7_8_MODE_DIRECT;
        Direct.flags = MODE_FLAG_HAS_PER_LED_COLOR |
            MODE_FLAG_HAS_BRIGHTNESS;
        Direct.color_mode = MODE_COLORS_PER_LED;
        Direct.brightness_min = 0;
        Direct.brightness_max = 9;
        Direct.brightness = brightness;
        modes.push_back(Direct);
    }

    name   = controller->getName();
    type   = DEVICE_TYPE_KEYBOARD;
    vendor = "Lenovo";

    switch (controller->getPid())
    {
    case LEGION_S7GEN7:
        description = "Lenovo Legion 7 Slim Generation 7";
        break;

    case LEGION_7GEN7:
        description = "Lenovo Legion 7 Generation 7";
        break;

    case LEGION_S7GEN8:
        description = "Lenovo Legion 7 Slim Generation 8";
        break;

    case LEGION_7GEN8:
        description = "Lenovo Legion 7 Generation 8";
        break;

    case LEGION_7GEN9:
    case LEGION_7GEN9_H:
        description = "Lenovo Legion 7 Generation 9";
        break;

    case LEGION_7GEN10:
        description = "Lenovo Legion 7 Generation 10";
        break;

    case LEGION_5GEN10:
        description = "Lenovo Legion 5 Gen 10";
        break;

    case LEGION_9GEN10:
        description = "Lenovo Legion 9 18IAX10";
        break;
    }

    brightness = controller->getCurrentBrightness();
    profile_id = controller->getCurrentProfileId();

    if(controller->getPid() == LEGION_9GEN10)
    {
        light_strip = OpenLegion9LightStrip();
        LOG_INFO("[Lenovo Legion 9] 18IAX10 keyboard 048D:C909 profile %u brightness %u.", profile_id, brightness);
    }
    for(mode &m : modes)
    {
        m.brightness = brightness;
    }

    SetupZones();

    /*-----------------------------------------------------*\
    | Initiliaze Static                                     |
    \*-----------------------------------------------------*/
    active_mode = 0;
    for(size_t i = 0; i < modes.size(); i++)
    {
        if(modes[i].value == LENOVO_LEGION_GEN7_8_MODE_STATIC)
        {
            active_mode = (int)i;
            break;
        }
    }
    ReadDeviceSettings();
    last_mode = active_mode;
    committed_key_colors = colors;
    key_touched.assign(colors.size(), 0);
    chassis_hold.assign(colors.size(), 0);
    chassis_held.assign(colors.size(), 0);
}

LenovoRGBController_Gen7_8::~LenovoRGBController_Gen7_8()
{
    Shutdown();

    /* Direct overwrites the saved effect. Put that effect back if the user did not choose Static. */
    if(direct_profile_restore &&
       active_mode >= 0 &&
       active_mode < (int)modes.size() &&
       modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT)
    {
        controller->restoreProfileReport(direct_profile);
        /* A profile write can leave brightness at 0. Put back the level already selected. */
        controller->setBrightness(brightness);
    }

    if(light_strip != nullptr)
    {
        /* Give the chassis effect back to the firmware. */
        light_strip->SetLampArrayControlReport(1);
        delete light_strip;
        light_strip = nullptr;
    }

    delete controller;
}

void LenovoRGBController_Gen7_8::SetupZones()
{
    vector<lenovo_zone> lenovo_zones;
    if(controller->getPid() == LEGION_9GEN10)
    {
        /*-------------------------------------------------*\
        | The strip and logo share this view. Side and      |
        | front squares own the LampArray runs. The logo    |
        | square owns the branding lamps. Without the strip |
        | device, keep the full matrix including the top    |
        | lens row.                                         |
        \*-------------------------------------------------*/
        if(light_strip != nullptr)
        {
            lenovo_zones.push_back(legion9_gen10_kbd_with_strip);
        }
        else
        {
            lenovo_zones.push_back(legion9_gen10_kbd);
        }
        lenovo_zones.push_back(lenovo_legion_7gen7_logo);
    }
    else if(Is24ZoneDevice(controller->getPid()))
    {
        lenovo_zones.push_back(legion_5gen10_kbd_24zone);
    }
    else
    {
        lenovo_zones.push_back(legion7_gen7and8_kbd_ansi);
    }

    if(controller->getPid() != LEGION_9GEN10 && !IsKeyboardOnlyDevice(controller->getPid()))
    {
        lenovo_zones.push_back(legion7_gen7and8_neon);
    }

    if (controller->getPid() == LEGION_7GEN7)
    {
        lenovo_zones.push_back(lenovo_legion_7gen7_logo);
        lenovo_zones.push_back(lenovo_legion_7gen7_vents);
    }

    if (controller->getPid() == LEGION_7GEN10)
    {
        lenovo_zones.push_back(lenovo_legion_7gen7_logo);
        lenovo_zones.push_back(lenovo_legion_7gen10_vents);
    }

    for(unsigned int i = 0; i < lenovo_zones.size(); i++)
    {
        zone new_zone;
        new_zone.name       = lenovo_zones[i].name;
        new_zone.type       = lenovo_zones[i].type;
        new_zone.leds_count = lenovo_zones[i].end - lenovo_zones[i].start + 1;
        new_zone.leds_max   = new_zone.leds_count;
        new_zone.leds_min   = new_zone.leds_count;

        LOG_DEBUG("[Lenovo Gen7/8 Controller] adding zone: %s with %u LEDs", new_zone.name.c_str(), new_zone.leds_count);

        if(lenovo_zones[i].type == ZONE_TYPE_MATRIX)
        {
            if(lenovo_zones[i].matrix_map != NULL)
            {
                new_zone.matrix_map.Set(lenovo_zones[i].height, lenovo_zones[i].width, (unsigned int *)lenovo_zones[i].matrix_map);
            }
        }

        zones.push_back(new_zone);

        for(unsigned int led_idx = lenovo_zones[i].start; led_idx <= lenovo_zones[i].end; led_idx++ )
        {
            led new_led;
            new_led.name = lenovo_zones[i].leds[led_idx].name;
            uint16_t led_num = lenovo_zones[i].leds[led_idx].led_num;
            if(led_num > 0xFF)
            {
                new_led.value = led_num;
            }
            else
            {
                new_led.value = lenovo_zones[i].id << 8 | led_num;
            }
            leds.push_back(new_led);

            /*---------------------------------------------*\
            | create led id to index map for fast look up   |
            \*---------------------------------------------*/
            led_id_to_index[new_led.value]=leds.size() - 1;
        }
    }

    SetupColors();
    BuildStripGroups();
}

void LenovoRGBController_Gen7_8::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void LenovoRGBController_Gen7_8::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void LenovoRGBController_Gen7_8::DeviceUpdateMode()
{
    uint8_t hw_profile_id = controller->getCurrentProfileId();
    if(hw_profile_id != profile_id)
    {
        profile_id = hw_profile_id;
        ReadDeviceSettings();
        last_mode = active_mode;
        direct_enabled = false;
        direct_profile_restore = false;
        direct_profile.clear();
    }

    if(brightness != modes[active_mode].brightness)
    {
        brightness = modes[active_mode].brightness;
        controller->setBrightness(brightness);
        for(mode &m : modes)
        {
            m.brightness = brightness;
        }
    }

    if(last_mode != active_mode)
    {
        /* Static is an intentional save. Do not restore the pre-Direct profile over it. */
        if(DirectUsesStaticProfile(controller->getPid()) &&
           modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_STATIC)
        {
            direct_profile_restore = false;
            direct_profile.clear();
        }

        if(modes[last_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT)
        {
            controller->setLedsDirectOff(profile_id);
            direct_enabled = false;
        }

        if(modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT)
        {
            if(!DirectUsesStaticProfile(controller->getPid()))
            {
                controller->setLedsDirectOn(profile_id);
                if(!UsesGen10PacketFormat(controller->getPid()))
                {
                    controller->setLedsByGroup(profile_id, GetLedGroups());
                }
            }
            direct_enabled = true;
        }

        last_mode = active_mode;
    }
    else if((modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT) && !direct_enabled)
    {
        if(!DirectUsesStaticProfile(controller->getPid()))
        {
            controller->setLedsDirectOn(profile_id);
        }
        direct_enabled = true;
    }

    if(modes[active_mode].value != LENOVO_LEGION_GEN7_8_MODE_DIRECT)
    {
        DeviceUpdateLEDs();
    }
}

bool LenovoRGBController_Gen7_8::HostPerLedColorMode() const
{
    if(active_mode < 0 || active_mode >= (int)modes.size())
    {
        return(false);
    }

    if(modes[active_mode].color_mode != MODE_COLORS_PER_LED)
    {
        return(false);
    }

    unsigned int value = modes[active_mode].value;

    return(value == LENOVO_LEGION_GEN7_8_MODE_STATIC ||
           (value == LENOVO_LEGION_GEN7_8_MODE_DIRECT && DirectUsesStaticProfile(controller->getPid())));
}

bool LenovoRGBController_Gen7_8::IsChassisLed(unsigned int led_value) const
{
    return(strip_groups.find((uint16_t)led_value) != strip_groups.end());
}

bool LenovoRGBController_Gen7_8::LedColorEdited(size_t led_index) const
{
    if(led_index < key_touched.size() && key_touched[led_index])
    {
        return(true);
    }

    if(led_index < committed_key_colors.size() && led_index < colors.size() &&
       colors[led_index] != committed_key_colors[led_index])
    {
        return(true);
    }

    return(false);
}

void LenovoRGBController_Gen7_8::PrepareFrameColors()
{
    frame_colors = colors;
    preserve_key_lamps = false;
    use_frame_colors = false;

    /* The strip device is not required. c909 still has to fill untouched keys. */
    if(!DirectUsesStaticProfile(controller->getPid()) || !HostPerLedColorMode())
    {
        return;
    }

    if(committed_key_colors.size() != colors.size())
    {
        committed_key_colors = colors;
        key_touched.assign(colors.size(), 0);
    }

    if(chassis_hold.size() != colors.size())
    {
        chassis_hold.assign(colors.size(), 0);
        chassis_held.assign(colors.size(), 0);
    }

    bool any_edit = false;

    for(size_t led_index = 0; led_index < leds.size() && led_index < colors.size(); led_index++)
    {
        if(!IsChassisLed(leds[led_index].value) && LedColorEdited(led_index))
        {
            any_edit = true;
            break;
        }
    }

    std::unordered_map<uint16_t, RGBColor> hardware = controller->getLiveLedColors();

    /*-----------------------------------------------------*\
    | The UI buffer starts black. A strip edit used to save |
    | a profile that listed only the strip, and the keyboard|
    | firmware turned every missing key blue. Fill untouched|
    | keys from the live colors, and keep untouched strip   |
    | squares on the color they already have.               |
    \*-----------------------------------------------------*/
    if(!Legion9HaveKeyColors(hardware.empty() ? 1 : 0, any_edit ? 1 : 0))
    {
        return;
    }

    for(size_t led_index = 0; led_index < leds.size() && led_index < frame_colors.size(); led_index++)
    {
        bool chassis = IsChassisLed(leds[led_index].value);

        if(LedColorEdited(led_index))
        {
            frame_colors[led_index] = colors[led_index];

            if(chassis)
            {
                chassis_hold[led_index] = colors[led_index];
                chassis_held[led_index] = 1;
            }

            continue;
        }

        if(chassis && chassis_held[led_index])
        {
            frame_colors[led_index] = chassis_hold[led_index];
            continue;
        }

        std::unordered_map<uint16_t, RGBColor>::iterator found = hardware.find((uint16_t)leds[led_index].value);

        if(found != hardware.end())
        {
            frame_colors[led_index] = found->second;

            if(chassis)
            {
                chassis_hold[led_index] = found->second;
                chassis_held[led_index] = 1;
            }
        }
    }

    use_frame_colors = true;
    preserve_key_lamps = true;
}

void LenovoRGBController_Gen7_8::RememberColorEdits()
{
    if(committed_key_colors.size() != colors.size())
    {
        committed_key_colors = colors;
        key_touched.assign(colors.size(), 0);
        return;
    }

    for(size_t led_index = 0; led_index < colors.size() && led_index < leds.size(); led_index++)
    {
        if(colors[led_index] != committed_key_colors[led_index])
        {
            committed_key_colors[led_index] = colors[led_index];

            if(key_touched.size() != colors.size())
            {
                key_touched.assign(colors.size(), 0);
            }

            key_touched[led_index] = 1;
        }
    }
}

void LenovoRGBController_Gen7_8::DeviceUpdateLEDs()
{
    bool skip_keyboard = false;

    PrepareFrameColors();

    /*------------------------------------------------------*\
    | No live key colors and the user has not painted a key.|
    | Saving the black UI buffer is what turns the keyboard |
    | blue, so leave the keyboard profile alone.            |
    \*------------------------------------------------------*/
    if(DirectUsesStaticProfile(controller->getPid()) && HostPerLedColorMode() && !use_frame_colors)
    {
        bool any_edit = false;

        for(size_t led_index = 0; led_index < leds.size() && led_index < colors.size(); led_index++)
        {
            if(!IsChassisLed(leds[led_index].value) && LedColorEdited(led_index))
            {
                any_edit = true;
                break;
            }
        }

        skip_keyboard = !any_edit;
    }

    if(!skip_keyboard)
    {
        if(modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT)
        {
            if(DirectUsesStaticProfile(controller->getPid()))
            {
                /* Snapshot once, before the first Direct save, so quit can put the previous effect back. */
                if(!direct_profile_restore)
                {
                    direct_profile = controller->getProfileReport(profile_id);

                    if(direct_profile.size() >= 5 && direct_profile[0] == REPORT_ID && direct_profile[1] == GET_PROFILE)
                    {
                        direct_profile_restore = true;
                    }
                    else
                    {
                        direct_profile.clear();
                        LOG_WARNING("[Lenovo Legion 9] could not read the profile before Direct. Quit will leave the static colors.");
                    }
                }

                controller->setLedsByGroup(profile_id, GetLedGroups());
            }
            else if(UsesGen10PacketFormat(controller->getPid()))
            {
                /*-----------------------------------------*\
                | Gen10 may ignore A1 updates unless D0 is  |
                | reasserted.                               |
                \*-----------------------------------------*/
                controller->setLedsDirectOn(profile_id);
                direct_enabled = true;
                controller->setLedsDirect(leds, colors);
            }
            else
            {
                if(!direct_enabled)
                {
                    controller->setLedsDirectOn(profile_id);
                    direct_enabled = true;
                }
                controller->setLedsDirect(leds, colors);
            }
        }
        else
        {
            controller->setLedsByGroup(profile_id, GetLedGroups());
        }
    }

    UpdateLightStrip();
    RememberColorEdits();
    use_frame_colors = false;
}

void LenovoRGBController_Gen7_8::BuildStripGroups()
{
    strip_groups.clear();

    if(light_strip == nullptr)
    {
        return;
    }

    std::vector<LampAttributes> lamps = light_strip->GetLamps();
    std::vector<LampAttributes> left;
    std::vector<LampAttributes> right;
    std::vector<LampAttributes> front;
    std::vector<unsigned short> logo;
    unsigned int matched_keys = 0;

    keyboard_lamps.clear();

    for(const LampAttributes& lamp : lamps)
    {
        if(lamp.LampPurposes & HID_LAMPARRAY_PURPOSE_ACCENT)
        {
            /* Left column is x=6000. Right column is x=394000, including its front corner. */
            if(lamp.PositionXInMicrometers < 15000 && lamp.PositionYInMicrometers < 296000)
            {
                left.push_back(lamp);
            }
            else if(lamp.PositionXInMicrometers >= 390000)
            {
                right.push_back(lamp);
            }
            else
            {
                front.push_back(lamp);
            }
        }
        else if(lamp.LampPurposes & HID_LAMPARRAY_PURPOSE_BRANDING)
        {
            logo.push_back(lamp.LampId);
        }
        else if(lamp.LampPurposes & HID_LAMPARRAY_PURPOSE_CONTROL)
        {
            legion9_key_lamp key_lamp;
            key_lamp.lamp_id = lamp.LampId;
            key_lamp.led_index = (size_t)-1;

            const char* key_name = HIDLampArrayKeyName(lamp.LampKey);

            /* Firmware labels a few keys with a different HID usage than the legend. */
            if(key_name == KEY_EN_BACK_SLASH)
            {
                key_name = KEY_EN_ANSI_BACK_SLASH;
            }
            else if(lamp.LampKey == 0xFE)
            {
                key_name = KEY_EN_LEFT_FUNCTION;
            }
            else if(lamp.LampKey == 0x72)
            {
                key_name = KEY_EN_RIGHT_CONTROL;
            }

            if(key_name != nullptr)
            {
                for(size_t led_index = 0; led_index < leds.size(); led_index++)
                {
                    if(leds[led_index].name == key_name)
                    {
                        key_lamp.led_index = led_index;
                        matched_keys++;
                        break;
                    }
                }
            }

            keyboard_lamps.push_back(key_lamp);
        }
    }

    std::sort(left.begin(), left.end(), [](const LampAttributes& a, const LampAttributes& b)
    {
        return(a.PositionYInMicrometers < b.PositionYInMicrometers);
    });
    std::sort(right.begin(), right.end(), [](const LampAttributes& a, const LampAttributes& b)
    {
        return(a.PositionYInMicrometers > b.PositionYInMicrometers);
    });
    std::sort(front.begin(), front.end(), [](const LampAttributes& a, const LampAttributes& b)
    {
        return(a.PositionXInMicrometers < b.PositionXInMicrometers);
    });

    /* Back half, then the half toward the front edge. An odd count keeps the extra lamp on the front half. */
    size_t left_mid = left.size() / 2;
    size_t right_mid = (right.size() + 1) / 2;
    strip_groups[0x01F5] = LampIdsInRange(left, 0, left_mid);
    strip_groups[0x01F6] = LampIdsInRange(left, left_mid, left.size());

    const uint16_t front_addresses[6] =
    {
        0x01F7, 0x01F8, 0x01F9, 0x01FA, 0x01FB, 0x01FC
    };

    for(int part = 0; part < 6; part++)
    {
        size_t begin = front.size() * part / 6;
        size_t end = front.size() * (part + 1) / 6;
        strip_groups[front_addresses[part]] = LampIdsInRange(front, begin, end);
    }

    strip_groups[0x01FD] = LampIdsInRange(right, 0, right_mid);
    strip_groups[0x01FE] = LampIdsInRange(right, right_mid, right.size());
    strip_groups[0x05DD] = logo;

    LOG_DEBUG("[Lenovo Legion 9] strip groups left=%u lower-left=%u front=%u right=%u logo=%u key lamps=%u matched=%u",
              (unsigned int)strip_groups[0x01F5].size(),
              (unsigned int)strip_groups[0x01F6].size(),
              (unsigned int)front.size(),
              (unsigned int)(strip_groups[0x01FD].size() + strip_groups[0x01FE].size()),
              (unsigned int)logo.size(),
              (unsigned int)keyboard_lamps.size(),
              matched_keys);
}

void LenovoRGBController_Gen7_8::UpdateLightStrip()
{
    if(light_strip == nullptr || strip_groups.empty())
    {
        return;
    }

    bool per_led = (modes[active_mode].color_mode == MODE_COLORS_PER_LED) &&
                   (modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT ||
                    modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_STATIC);

    if(!per_led)
    {
        light_strip->SetLampArrayControlReport(1);
        return;
    }

    /* No key colors yet. Taking the lamp array over now drops the key lamps back to blue. */
    if(!Legion9SeizeLampArray(preserve_key_lamps ? 1 : 0))
    {
        return;
    }

    std::vector<unsigned short> lamp_ids;
    std::vector<RGBColor>       lamp_colors;
    const std::vector<RGBColor>& active_colors = (frame_colors.size() == colors.size()) ? frame_colors : colors;

    for(std::unordered_map<uint16_t, std::vector<unsigned short>>::iterator group = strip_groups.begin(); group != strip_groups.end(); group++)
    {
        std::unordered_map<unsigned int, size_t>::iterator led = led_id_to_index.find(group->first);

        if(led == led_id_to_index.end() || led->second >= active_colors.size())
        {
            continue;
        }

        for(unsigned int lamp_idx = 0; lamp_idx < group->second.size(); lamp_idx++)
        {
            lamp_ids.push_back(group->second[lamp_idx]);
            lamp_colors.push_back(active_colors[led->second]);
        }
    }

    /*-----------------------------------------------------*\
    | Host control applies to every lamp on this device.    |
    | Key lamps left out of the update fall back to blue.   |
    \*-----------------------------------------------------*/
    if(preserve_key_lamps)
    {
        for(const legion9_key_lamp& key_lamp : keyboard_lamps)
        {
            if(key_lamp.led_index == (size_t)-1 || key_lamp.led_index >= active_colors.size())
            {
                continue;
            }

            lamp_ids.push_back(key_lamp.lamp_id);
            lamp_colors.push_back(active_colors[key_lamp.led_index]);
        }
    }

    if(lamp_ids.empty())
    {
        return;
    }

    light_strip->SetLampArrayControlReport(0);

    size_t sent = 0;

    while(sent < lamp_ids.size())
    {
        unsigned int    count = LAMP_MULTI_UPDATE_LAMP_COUNT;
        unsigned short  ids[LAMP_MULTI_UPDATE_LAMP_COUNT];
        LampArrayColor  update_colors[LAMP_MULTI_UPDATE_LAMP_COUNT];
        unsigned char   flags = 0;

        if(lamp_ids.size() - sent < count)
        {
            count = (unsigned int)(lamp_ids.size() - sent);
        }

        for(unsigned int index = 0; index < count; index++)
        {
            ids[index] = lamp_ids[sent + index];
            update_colors[index].RedChannel = RGBGetRValue(lamp_colors[sent + index]);
            update_colors[index].GreenChannel = RGBGetGValue(lamp_colors[sent + index]);
            update_colors[index].BlueChannel = RGBGetBValue(lamp_colors[sent + index]);
            update_colors[index].IntensityChannel = 255;
        }

        sent += count;

        if(sent >= lamp_ids.size())
        {
            flags = LAMP_UPDATE_FLAG_UPDATE_COMPLETE;
        }

        light_strip->SetLampMultiUpdateReport((unsigned char)count, flags, ids, update_colors);
    }
}

void LenovoRGBController_Gen7_8::ReadDeviceSettings()
{
    vector<led_group> current_settings = controller->getProfileSettings(profile_id);
    if(current_settings.size() > 0)
    {
        for(unsigned int i = 0; i < modes.size(); i++)
        {
            if(modes[i].value == current_settings[0].mode)
            {
                switch(current_settings[0].color_mode)
                {
                    case 0x02:
                        if(modes[i].flags & MODE_FLAG_HAS_PER_LED_COLOR)
                        {
                            modes[i].color_mode = MODE_COLORS_PER_LED;
                        }
                        else
                        {
                            modes[i].color_mode = MODE_COLORS_MODE_SPECIFIC;
                        }
                        break;

                    case 0x01:
                        modes[i].color_mode = MODE_COLORS_RANDOM;
                        break;

                    default:
                        modes[i].color_mode = MODE_COLORS_NONE;
                }

                switch(modes[i].color_mode)
                {
                    case MODE_COLORS_PER_LED:
                        for(size_t j=0; j < colors.size(); j++)
                        {
                            colors[j]=0x00;
                        }
                        for(const led_group &lg : current_settings)
                        {
                            if(lg.colors.size() > 0)
                            {
                                for(uint16_t led_id : lg.leds)
                                {
                                    if(auto search = led_id_to_index.find(led_id); search != led_id_to_index.end())
                                    {
                                        colors[search->second] = lg.colors[0];
                                    }
                                }
                            }
                        }
                        break;

                    case MODE_COLORS_MODE_SPECIFIC:
                        for(size_t j=0; j < modes[i].colors.size(); j++)
                        {
                            if(j < current_settings[0].colors.size())
                            {
                                modes[i].colors[j] = current_settings[0].colors[j];
                            }
                            else
                            {
                                modes[i].colors[j] = 0x00;
                            }
                        }
                }

                switch(current_settings[0].direction)
                {
                    case 0x01:
                        modes[i].direction = MODE_DIRECTION_UP;
                        break;
                    case 0x02:
                        modes[i].direction = MODE_DIRECTION_DOWN;
                        break;
                    case 0x03:
                        modes[i].direction = MODE_DIRECTION_LEFT;
                        break;
                    case 0x04:
                        modes[i].direction = MODE_DIRECTION_RIGHT;
                        break;
                }

                switch(current_settings[0].spin)
                {
                    case 0x01:
                        modes[i].direction = MODE_DIRECTION_RIGHT;
                        break;
                    case 0x02:
                        modes[i].direction = MODE_DIRECTION_LEFT;
                        break;
                }

                active_mode = i;
                break; //stop for loop
            }
        }
    }
}

std::vector<led_group> LenovoRGBController_Gen7_8::GetLedGroups()
{
    if(Is24ZoneDevice(controller->getPid()) &&
       modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_STATIC &&
       modes[active_mode].color_mode == MODE_COLORS_RANDOM)
    {
        vector<led_group> led_groups;
        led_groups.reserve(leds.size());

        for(size_t i = 0; i < leds.size(); i++)
        {
            led_group group;
            group.mode = modes[active_mode].value;
            group.speed = modes[active_mode].speed;
            group.spin = 0x00;
            group.direction = 0x00;
            group.color_mode = 0x02;
            group.colors.push_back(legion_5gen10_zone_visualization_colors[i % 24]);
            group.leds.push_back(leds[i].value);
            led_groups.push_back(group);
        }

        return led_groups;
    }

    std::unordered_map<RGBColor, vector<uint16_t>> led_map;
    const std::vector<RGBColor>& active_colors = (use_frame_colors && frame_colors.size() == colors.size()) ? frame_colors : colors;

    /*-----------------------------------------------------*\
    | Legion 9 Direct is sent as per-key static, so it     |
    | has to be grouped by color like Static. Other Direct |
    | modes keep the single uncolored group used with A1.  |
    | Strip and logo addresses are not keyboard keys.      |
    \*-----------------------------------------------------*/
    if(modes[active_mode].color_mode == MODE_COLORS_PER_LED &&
            (modes[active_mode].value != LENOVO_LEGION_GEN7_8_MODE_DIRECT ||
             DirectUsesStaticProfile(controller->getPid())))
    {
        for(size_t i = 0; i < leds.size() && i < active_colors.size(); i++)
        {
            if(IsChassisLed(leds[i].value))
            {
                continue;
            }

            led_map[active_colors[i]].push_back(leds[i].value);
        }
    }
    else
    {
        size_t start = 0;
        size_t end = leds.size();

        /*-------------------------------------------------*\
        | Riplle and Type only apply to keyboard            |
        \*-------------------------------------------------*/
        if(modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_RIPPLE ||
                modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_TYPE)
        {
            for(const zone &z : zones)
            {
                if(z.name == "Keyboard")
                {
                    start = z.start_idx;
                    end = start + z.leds_count;
                }
            }

        }

        for(size_t i = start; i < end; i++)
        {
            if(IsChassisLed(leds[i].value))
            {
                continue;
            }

            led_map[0x00].push_back(leds[i].value);
        }
    }

    vector<led_group> led_groups;
    for(const auto &pair : led_map)
    {
        /*-------------------------------------------------*\
        | Dropping black keys makes this firmware paint    |
        | them its default blue. Keep them on Legion 9.    |
        \*-------------------------------------------------*/
        if(!Legion9KeepColorGroup(DirectUsesStaticProfile(controller->getPid()) ? 1 : 0,
                                  HostPerLedColorMode() ? 1 : 0,
                                  led_map.size() == 1 ? 1 : 0,
                                  pair.first))
        {
            continue;
        }

        led_group group;
        if(DirectUsesStaticProfile(controller->getPid()) &&
           modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_DIRECT)
        {
            group.mode = LENOVO_LEGION_GEN7_8_MODE_STATIC;
        }
        else
        {
            group.mode = modes[active_mode].value;
        }
        group.speed = modes[active_mode].speed;
        group.spin = 0x00;
        group.direction = 0x00;

        switch(modes[active_mode].direction)
        {
            case MODE_DIRECTION_UP:
                group.direction = 0x01;
                break;

            case MODE_DIRECTION_DOWN:
                group.direction = 0x02;
                break;

            case MODE_DIRECTION_LEFT:
                if(modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_SCREW_RAINBOW)
                {
                    group.spin      = 0x02;
                }
                else
                {
                    group.direction = 0x03;
                }
                break;

            case MODE_DIRECTION_RIGHT:
                if(modes[active_mode].value == LENOVO_LEGION_GEN7_8_MODE_SCREW_RAINBOW)
                {
                    group.spin      = 0x01;
                }
                else
                {
                    group.direction = 0x04;
                }
                break;
        }

        switch(modes[active_mode].color_mode)
        {
            default:
            case MODE_COLORS_NONE:
                group.color_mode = 0x00;
                break;

            case MODE_COLORS_RANDOM:
                group.color_mode = 0x01;
                break;

            case MODE_COLORS_MODE_SPECIFIC:
                group.color_mode = 0x02;
                for(RGBColor c : modes[active_mode].colors)
                {
                    if(c)
                    {
                        group.colors.push_back(c);
                    }
                }

                if(group.colors.size() == 0)
                {
                    group.colors.push_back(0xFFF500);
                }
                break;

            case MODE_COLORS_PER_LED:
                group.color_mode = 0x02;
                group.colors.push_back(pair.first);
                break;
        }

        group.leds = pair.second;

        led_groups.push_back(group);
    }

    return led_groups;
}
