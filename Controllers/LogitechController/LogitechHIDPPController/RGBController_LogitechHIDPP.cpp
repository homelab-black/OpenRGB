/*---------------------------------------------------------*\
| RGBController_LogitechHIDPP.cpp                           |
|                                                           |
|   RGBController for unified Logitech HID++ 2.0 devices   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <map>
#include "RGBController_LogitechHIDPP.h"
#include "RGBControllerKeyNames.h"
#include "KeyboardLayoutManager.h"
#include "LogManager.h"

/**------------------------------------------------------------------*\
    @name Logitech HID++ 2.0
    @category Keyboard,Mouse,Headset
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectLogitechHIDPP
    @comment
        Unified HID++ 2.0 controller that dynamically discovers device
        capabilities via feature probing. Supports per-key lighting
        (0x8081/0x8080) and zone-based effects (0x8071/0x8070).
\*-------------------------------------------------------------------*/

static const char* zone_location_name(uint16_t location)
{
    switch(location)
    {
        case 0x0001: return "All";
        case 0x0002: return "Primary";
        case 0x0003: return "Combined";
        case 0x0004: return "Logo";
        case 0x0005: return "Left";
        case 0x0006: return "Right";
        case 0x0007: return "Group 1";
        case 0x0008: return "Group 2";
        case 0x0009: return "Group 3";
        case 0x000A: return "Group 4";
        case 0x000B: return "Group 5";
        case 0x2000: return "Top";
        case 0x4000: return "Bottom";
        default:
        {
            static char buf[16];
            snprintf(buf, sizeof(buf), "Zone 0x%04X", location);
            return buf;
        }
    }
}

/*---------------------------------------------------------*\
| HID++ per-key zone ID to OpenRGB key name mapping         |
| Zone IDs follow Solaar's KEYCODES (special_keys.py)       |
| Used to look up zone IDs by key name after KLM builds     |
| the keymap in its own sorted order.                       |
\*---------------------------------------------------------*/
static const std::map<std::string, unsigned int> hidpp_key_name_to_zone =
{
    { KEY_EN_A,                1  },
    { KEY_EN_B,                2  },
    { KEY_EN_C,                3  },
    { KEY_EN_D,                4  },
    { KEY_EN_E,                5  },
    { KEY_EN_F,                6  },
    { KEY_EN_G,                7  },
    { KEY_EN_H,                8  },
    { KEY_EN_I,                9  },
    { KEY_EN_J,               10  },
    { KEY_EN_K,               11  },
    { KEY_EN_L,               12  },
    { KEY_EN_M,               13  },
    { KEY_EN_N,               14  },
    { KEY_EN_O,               15  },
    { KEY_EN_P,               16  },
    { KEY_EN_Q,               17  },
    { KEY_EN_R,               18  },
    { KEY_EN_S,               19  },
    { KEY_EN_T,               20  },
    { KEY_EN_U,               21  },
    { KEY_EN_V,               22  },
    { KEY_EN_W,               23  },
    { KEY_EN_X,               24  },
    { KEY_EN_Y,               25  },
    { KEY_EN_Z,               26  },
    { KEY_EN_1,               27  },
    { KEY_EN_2,               28  },
    { KEY_EN_3,               29  },
    { KEY_EN_4,               30  },
    { KEY_EN_5,               31  },
    { KEY_EN_6,               32  },
    { KEY_EN_7,               33  },
    { KEY_EN_8,               34  },
    { KEY_EN_9,               35  },
    { KEY_EN_0,               36  },
    { KEY_EN_ANSI_ENTER,      37  },
    { KEY_EN_ESCAPE,          38  },
    { KEY_EN_BACKSPACE,       39  },
    { KEY_EN_TAB,             40  },
    { KEY_EN_SPACE,           41  },
    { KEY_EN_MINUS,           42  },
    { KEY_EN_EQUALS,          43  },
    { KEY_EN_LEFT_BRACKET,    44  },
    { KEY_EN_RIGHT_BRACKET,   45  },
    { KEY_EN_ANSI_BACK_SLASH, 46  },
    { KEY_EN_SEMICOLON,       48  },
    { KEY_EN_QUOTE,           49  },
    { KEY_EN_BACK_TICK,       50  },
    { KEY_EN_COMMA,           51  },
    { KEY_EN_PERIOD,          52  },
    { KEY_EN_FORWARD_SLASH,   53  },
    { KEY_EN_CAPS_LOCK,       54  },
    { KEY_EN_F1,              55  },
    { KEY_EN_F2,              56  },
    { KEY_EN_F3,              57  },
    { KEY_EN_F4,              58  },
    { KEY_EN_F5,              59  },
    { KEY_EN_F6,              60  },
    { KEY_EN_F7,              61  },
    { KEY_EN_F8,              62  },
    { KEY_EN_F9,              63  },
    { KEY_EN_F10,             64  },
    { KEY_EN_F11,             65  },
    { KEY_EN_F12,             66  },
    { KEY_EN_PRINT_SCREEN,    67  },
    { KEY_EN_SCROLL_LOCK,     68  },
    { KEY_EN_PAUSE_BREAK,     69  },
    { KEY_EN_INSERT,          70  },
    { KEY_EN_HOME,            71  },
    { KEY_EN_PAGE_UP,         72  },
    { KEY_EN_DELETE,          73  },
    { KEY_EN_END,             74  },
    { KEY_EN_PAGE_DOWN,       75  },
    { KEY_EN_RIGHT_ARROW,     76  },
    { KEY_EN_LEFT_ARROW,      77  },
    { KEY_EN_DOWN_ARROW,      78  },
    { KEY_EN_UP_ARROW,        79  },
    { KEY_EN_RIGHT_FUNCTION, 111  },
    { KEY_EN_MENU,            98  },
    { KEY_EN_LEFT_CONTROL,   104  },
    { KEY_EN_LEFT_SHIFT,     105  },
    { KEY_EN_LEFT_ALT,       106  },
    { KEY_EN_LEFT_WINDOWS,   107  },
    { KEY_EN_RIGHT_CONTROL,  108  },
    { KEY_EN_RIGHT_SHIFT,    109  },
    { KEY_EN_RIGHT_ALT,      110  },
    { KEY_EN_RIGHT_WINDOWS,  111  },
};

RGBController_LogitechHIDPP::RGBController_LogitechHIDPP(LogitechHIDPPController* controller_ptr)
{
    controller = controller_ptr;

    const HIDPPDeviceCapabilities& caps = controller->GetCapabilities();

    name        = caps.device_name;
    vendor      = "Logitech";
    description = "Logitech HID++ 2.0 Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    switch(caps.device_type)
    {
        case LOGITECH_DEVICE_TYPE_KEYBOARD:
            type = DEVICE_TYPE_KEYBOARD;
            break;
        case LOGITECH_DEVICE_TYPE_MOUSE:
        case LOGITECH_DEVICE_TYPE_TRACKBALL:
            type = DEVICE_TYPE_MOUSE;
            break;
        case LOGITECH_DEVICE_TYPE_HEADSET:
            type = DEVICE_TYPE_HEADSET;
            break;
        case LOGITECH_DEVICE_TYPE_MOUSEPAD:
            type = DEVICE_TYPE_MOUSEMAT;
            break;
        default:
            type = DEVICE_TYPE_UNKNOWN;
            break;
    }

    /*---------------------------------------------------------*\
    | Build mode list from discovered capabilities               |
    \*---------------------------------------------------------*/

    /*---------------------------------------------------------*\
    | Direct mode: per-key control via 0x8081                    |
    \*---------------------------------------------------------*/
    if(caps.has_perkey)
    {
        mode Direct;
        Direct.name       = "Direct";
        Direct.value      = 0;
        Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
        Direct.color_mode = MODE_COLORS_PER_LED;
        modes.push_back(Direct);
    }

    /*---------------------------------------------------------*\
    | Off mode: always available                                 |
    \*---------------------------------------------------------*/
    {
        mode Off;
        Off.name       = "Off";
        Off.value      = 0xFF;
        Off.flags      = 0;
        Off.color_mode = MODE_COLORS_NONE;
        modes.push_back(Off);
    }

    /*---------------------------------------------------------*\
    | Effect modes from zone cluster discovery                   |
    | Scan effects from the first cluster (effects are usually  |
    | the same across clusters)                                  |
    \*---------------------------------------------------------*/
    if(caps.has_zone_effects && !caps.zone_clusters.empty())
    {
        const HIDPPZoneCluster& cluster = caps.zone_clusters[0];

        for(size_t i = 0; i < cluster.effects.size(); i++)
        {
            const HIDPPEffect& fx = cluster.effects[i];

            switch(fx.effect_id)
            {
                case 0x0001:    // Static / Fixed Color
                {
                    mode Static;
                    Static.name       = "Static";
                    Static.value      = fx.index;
                    Static.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
                    Static.colors_min = 1;
                    Static.colors_max = 1;
                    Static.color_mode = MODE_COLORS_MODE_SPECIFIC;
                    Static.colors.resize(1);
                    modes.push_back(Static);
                    break;
                }

                case 0x0003:    // Color Cycle / Spectrum
                {
                    mode Cycle;
                    Cycle.name       = "Spectrum Cycle";
                    Cycle.value      = fx.index;
                    Cycle.flags      = MODE_FLAG_HAS_SPEED;
                    Cycle.speed_min  = 1;
                    Cycle.speed_max  = 255;
                    Cycle.speed      = 100;
                    Cycle.color_mode = MODE_COLORS_NONE;
                    modes.push_back(Cycle);
                    break;
                }

                case 0x000A:    // Breathing
                {
                    mode Breathing;
                    Breathing.name       = "Breathing";
                    Breathing.value      = fx.index;
                    Breathing.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED;
                    Breathing.speed_min  = 1;
                    Breathing.speed_max  = 255;
                    Breathing.speed      = 100;
                    Breathing.colors_min = 1;
                    Breathing.colors_max = 1;
                    Breathing.color_mode = MODE_COLORS_MODE_SPECIFIC;
                    Breathing.colors.resize(1);
                    modes.push_back(Breathing);
                    break;
                }

                case 0x0004:    // Color Wave
                {
                    mode Wave;
                    Wave.name       = "Color Wave";
                    Wave.value      = fx.index;
                    Wave.flags      = MODE_FLAG_HAS_SPEED;
                    Wave.speed_min  = 1;
                    Wave.speed_max  = 255;
                    Wave.speed      = 100;
                    Wave.color_mode = MODE_COLORS_NONE;
                    modes.push_back(Wave);
                    break;
                }

                case 0x000B:    // Ripple
                {
                    mode Ripple;
                    Ripple.name       = "Ripple";
                    Ripple.value      = fx.index;
                    Ripple.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED;
                    Ripple.speed_min  = 1;
                    Ripple.speed_max  = 255;
                    Ripple.speed      = 100;
                    Ripple.colors_min = 1;
                    Ripple.colors_max = 1;
                    Ripple.color_mode = MODE_COLORS_MODE_SPECIFIC;
                    Ripple.colors.resize(1);
                    modes.push_back(Ripple);
                    break;
                }

                default:
                    break;
            }
        }
    }

    SetupZones();
}

RGBController_LogitechHIDPP::~RGBController_LogitechHIDPP()
{
    delete controller;
}

void RGBController_LogitechHIDPP::SetupZones()
{
    const HIDPPDeviceCapabilities& caps = controller->GetCapabilities();

    led_to_zone_id.clear();

    if(caps.has_perkey)
    {
        if(caps.device_type == LOGITECH_DEVICE_TYPE_KEYBOARD)
        {
            /*-------------------------------------------------*\
            | Keyboard: use KeyboardLayoutManager for matrix     |
            | layout. Derive size from numpad presence, layout   |
            | from 0x4540 KeyboardLayout feature.                |
            \*-------------------------------------------------*/
            KEYBOARD_SIZE kb_size = caps.has_numpad
                                  ? KEYBOARD_SIZE_FULL
                                  : KEYBOARD_SIZE_TKL;

            KEYBOARD_LAYOUT kb_layout;

            switch(caps.keyboard_layout_code)
            {
                case 3:     // German
                case 7:     // Swiss
                    kb_layout = KEYBOARD_LAYOUT_ISO_QWERTZ;
                    break;

                case 4:     // French
                    kb_layout = KEYBOARD_LAYOUT_ISO_AZERTY;
                    break;

                case 2:     // UK
                case 5:     // Spanish
                case 0x0B:  // Italian
                case 0x0D:  // Portuguese
                case 0x0E:  // Belgian
                case 0x0F:  // Scandinavian
                case 8:     // Nordic
                case 0x16:  // Nordic
                case 0x1D:  // Nordic
                case 0x21:  // Nordic
                case 0x24:  // Belgian
                    kb_layout = KEYBOARD_LAYOUT_ISO_QWERTY;
                    break;

                case 9:     // Japanese
                case 0x3E:  // Japanese
                    kb_layout = KEYBOARD_LAYOUT_JIS;
                    break;

                case 1:     // US
                default:
                    kb_layout = KEYBOARD_LAYOUT_ANSI_QWERTY;
                    break;
            }

            KeyboardLayoutManager klm(kb_layout, kb_size);

            zone perkey_zone;
            perkey_zone.name       = ZONE_EN_KEYBOARD;
            perkey_zone.type       = ZONE_TYPE_MATRIX;
            perkey_zone.leds_min   = klm.GetKeyCount();
            perkey_zone.leds_max   = klm.GetKeyCount();
            perkey_zone.leds_count = klm.GetKeyCount();

            matrix_map_type* new_map = new matrix_map_type;
            new_map->height          = klm.GetRowCount();
            new_map->width           = klm.GetColumnCount();
            new_map->map             = new unsigned int[new_map->height * new_map->width];
            klm.GetKeyMap(new_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT,
                          new_map->height, new_map->width);
            perkey_zone.matrix_map = new_map;
            zones.push_back(perkey_zone);

            for(unsigned int i = 0; i < klm.GetKeyCount(); i++)
            {
                led new_led;
                std::string key_name = klm.GetKeyNameAt(i);
                new_led.name = key_name;

                /*---------------------------------------------*\
                | Look up zone ID by key name                   |
                \*---------------------------------------------*/
                auto it = hidpp_key_name_to_zone.find(key_name);
                unsigned int zone_id = (it != hidpp_key_name_to_zone.end()) ? it->second : 0;
                new_led.value = zone_id;
                leds.push_back(new_led);

                led_to_zone_id.push_back((uint16_t)zone_id);
            }
        }
        else
        {
            /*-------------------------------------------------*\
            | Mouse/other: linear zone with auto-named LEDs     |
            \*-------------------------------------------------*/
            zone perkey_zone;
            perkey_zone.name       = "LEDs";
            perkey_zone.type       = ZONE_TYPE_LINEAR;
            perkey_zone.leds_min   = (unsigned int)caps.perkey_zone_ids.size();
            perkey_zone.leds_max   = (unsigned int)caps.perkey_zone_ids.size();
            perkey_zone.leds_count = (unsigned int)caps.perkey_zone_ids.size();
            perkey_zone.matrix_map = nullptr;
            zones.push_back(perkey_zone);

            for(size_t i = 0; i < caps.perkey_zone_ids.size(); i++)
            {
                led new_led;
                new_led.name  = "LED " + std::to_string(caps.perkey_zone_ids[i]);
                new_led.value = caps.perkey_zone_ids[i];
                leds.push_back(new_led);

                led_to_zone_id.push_back(caps.perkey_zone_ids[i]);
            }
        }
    }
    else if(caps.has_zone_effects)
    {
        /*-----------------------------------------------------*\
        | No per-key: create one zone per cluster                |
        \*-----------------------------------------------------*/
        for(size_t i = 0; i < caps.zone_clusters.size(); i++)
        {
            const HIDPPZoneCluster& cluster = caps.zone_clusters[i];

            zone new_zone;
            new_zone.name       = zone_location_name(cluster.location);
            new_zone.type       = ZONE_TYPE_SINGLE;
            new_zone.leds_min   = 1;
            new_zone.leds_max   = 1;
            new_zone.leds_count = 1;
            new_zone.matrix_map = nullptr;
            zones.push_back(new_zone);

            led new_led;
            new_led.name  = new_zone.name;
            new_led.value = cluster.index;
            leds.push_back(new_led);

            led_to_zone_id.push_back(cluster.index);
        }
    }

    SetupColors();
}

void RGBController_LogitechHIDPP::ResizeZone(int /*zone*/, int /*new_size*/)
{
}

void RGBController_LogitechHIDPP::DeviceUpdateLEDs()
{
    const HIDPPDeviceCapabilities& caps = controller->GetCapabilities();

    if(caps.has_perkey && (unsigned int)active_mode < modes.size() &&
       modes[active_mode].color_mode == MODE_COLORS_PER_LED)
    {
        /*-----------------------------------------------------*\
        | Group colors by value for efficient batching.          |
        | fn6 (SetRgbZonesSingleValue) sends 1 color + up to    |
        | 13 zone IDs per packet. fn1 (SetIndividualRgbZones)    |
        | sends 4 zone+color entries per packet.                 |
        | Matching Solaar's approach: bulk same-color groups      |
        | via fn6, then remaining via fn1.                       |
        \*-----------------------------------------------------*/
        std::map<RGBColor, std::vector<uint8_t>> color_to_zones;

        for(size_t i = 0; i < colors.size() && i < led_to_zone_id.size(); i++)
        {
            if(led_to_zone_id[i] > 255)
            {
                continue;
            }

            color_to_zones[colors[i]].push_back((uint8_t)led_to_zone_id[i]);
        }

        /*-----------------------------------------------------*\
        | Send bulk groups via fn6, individuals via fn1           |
        \*-----------------------------------------------------*/
        std::vector<std::pair<uint16_t, RGBColor>> individual_pairs;

        for(auto& entry : color_to_zones)
        {
            RGBColor color = entry.first;
            std::vector<uint8_t>& zone_ids = entry.second;

            if(zone_ids.size() > 3)
            {
                /*-------------------------------------------------*\
                | Bulk: fn6 (SetRgbZonesSingleValue), 13 per packet  |
                \*-------------------------------------------------*/
                uint8_t r = RGBGetRValue(color);
                uint8_t g = RGBGetGValue(color);
                uint8_t b = RGBGetBValue(color);

                uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

                for(size_t i = 0; i < zone_ids.size(); i += 13)
                {
                    uint8_t data[16] = {};
                    data[0] = r;
                    data[1] = g;
                    data[2] = b;

                    size_t batch = zone_ids.size() - i;

                    if(batch > 13)
                    {
                        batch = 13;
                    }

                    for(size_t j = 0; j < batch; j++)
                    {
                        data[3 + j] = zone_ids[i + j];
                    }

                    controller->SendPerKeyData(perkey_idx, 0x60, data, 3 + batch);
                }
            }
            else
            {
                /*-------------------------------------------------*\
                | Individual: accumulate for fn1                     |
                \*-------------------------------------------------*/
                for(uint8_t zid : zone_ids)
                {
                    individual_pairs.push_back({zid, color});
                }
            }
        }

        if(!individual_pairs.empty())
        {
            controller->SetPerKeyColors(individual_pairs);
        }

        controller->PerKeyFrameEnd();
    }
}

void RGBController_LogitechHIDPP::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechHIDPP::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechHIDPP::DeviceUpdateMode()
{
    const HIDPPDeviceCapabilities& caps = controller->GetCapabilities();

    if((unsigned int)active_mode >= modes.size())
    {
        return;
    }

    const mode& current = modes[active_mode];

    /*---------------------------------------------------------*\
    | Direct mode: send per-key colors                           |
    \*---------------------------------------------------------*/
    if(current.name == "Direct")
    {
        DeviceUpdateLEDs();
        return;
    }

    /*---------------------------------------------------------*\
    | Per-key completely masks zone effects and the per-key      |
    | buffer can only be cleared by a power cycle. So on         |
    | devices with per-key, implement all modes via per-key      |
    | writes. Zone effects are only used on devices without      |
    | per-key support.                                           |
    \*---------------------------------------------------------*/
    if(caps.has_perkey)
    {
        /*-----------------------------------------------------*\
        | Off mode via per-key: set all LEDs to black            |
        \*-----------------------------------------------------*/
        if(current.value == 0xFF)
        {
            controller->SetAllPerKeyColor(ToRGBColor(0, 0, 0));
            controller->PerKeyFrameEnd();
            return;
        }

        /*-----------------------------------------------------*\
        | Static mode via per-key: set all LEDs to mode color    |
        \*-----------------------------------------------------*/
        if(current.color_mode == MODE_COLORS_MODE_SPECIFIC && current.colors.size() > 0)
        {
            controller->SetAllPerKeyColor(current.colors[0]);
            controller->PerKeyFrameEnd();
            return;
        }

        /*-----------------------------------------------------*\
        | For animated effects (Breathing, Cycle, etc.) on       |
        | per-key devices: fall through to zone effects as a     |
        | best-effort. They'll only be visible if per-key        |
        | hasn't been written yet this session.                  |
        \*-----------------------------------------------------*/
    }

    /*---------------------------------------------------------*\
    | Zone effect modes (devices without per-key, or animated    |
    | effects that can't be done via per-key)                    |
    \*---------------------------------------------------------*/

    /*---------------------------------------------------------*\
    | Off mode: set static black on all clusters                 |
    \*---------------------------------------------------------*/
    if(current.value == 0xFF)
    {
        for(size_t i = 0; i < caps.zone_clusters.size(); i++)
        {
            for(size_t j = 0; j < caps.zone_clusters[i].effects.size(); j++)
            {
                if(caps.zone_clusters[i].effects[j].effect_id == 0x0001)
                {
                    controller->SetZoneEffect(
                        caps.zone_clusters[i].index,
                        caps.zone_clusters[i].effects[j].index,
                        0x0001, 0, 0, 0, 0, false);
                    break;
                }
            }
        }
        return;
    }

    /*---------------------------------------------------------*\
    | Effect mode: apply to all clusters                         |
    \*---------------------------------------------------------*/
    unsigned char r = 0, g = 0, b = 0;

    if(current.color_mode == MODE_COLORS_MODE_SPECIFIC && current.colors.size() > 0)
    {
        r = RGBGetRValue(current.colors[0]);
        g = RGBGetGValue(current.colors[0]);
        b = RGBGetBValue(current.colors[0]);
    }

    uint16_t period = (uint16_t)(current.speed * 100);

    for(size_t i = 0; i < caps.zone_clusters.size(); i++)
    {
        uint16_t eff_id = 0;

        for(const auto& fx : caps.zone_clusters[i].effects)
        {
            if(fx.index == (uint8_t)current.value)
            {
                eff_id = fx.effect_id;
                break;
            }
        }

        controller->SetZoneEffect(
            caps.zone_clusters[i].index,
            current.value,
            eff_id, r, g, b, period, false);
    }
}
