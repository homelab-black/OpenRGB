/*---------------------------------------------------------*\
| RGBController_Sinowealth.cpp                              |
|                                                           |
|   RGBController for Sinowealth mice, including Glorious   |
|                                                           |
|   Niels Westphal (crashniels)                 20 May 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_Sinowealth.h"

#include "StringUtils.h"
#include <sstream>
#include "LogManager.h"

/**------------------------------------------------------------------*\
    @name Sinowealth Mice
    @category Mouse
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectSinowealthMouse
    @comment
\*-------------------------------------------------------------------*/

static std::string SinowealthDpiOption[] =
{
    "200", // code 0x01
    "300", // code 0x02
    "400", // code 0x03
    "500", // code 0x04
    "600", // code 0x05
    "700", // code 0x06
    "800", // code 0x07
    "900", // code 0x08
    "1000", // code 0x09
    "1100", // code 0x0A
    "1200", // code 0x0B
    "1300", // code 0x0C
    "1400", // code 0x0D
    "1500", // code 0x0E
    "1600", // code 0x0F
    "1700", // code 0x10
    "1800", // code 0x11
    "1900", // code 0x12
    "2000", // code 0x13
    "2100", // code 0x14
    "2200", // code 0x15
    "2300", // code 0x16
    "2400", // code 0x17
    "2500", // code 0x18
    "2600", // code 0x19
    "2700", // code 0x1A
    "2800", // code 0x1B
    "2900", // code 0x1C
    "3000", // code 0x1D
    "3100", // code 0x1E
    "3200", // code 0x1F
    "3300", // code 0x20
    "3400", // code 0x21
    "3500", // code 0x22
    "3600", // code 0x23
    "3700", // code 0x24
    "3800", // code 0x25
    "3900", // code 0x26
    "4000", // code 0x27
    "4500", // code 0x28
    "5000", // code 0x29
    "5100", // code 0x2A
    "5200", // code 0x2B
    "5300", // code 0x2C
    "5400", // code 0x2D
    "5500", // code 0x2E
    "5600", // code 0x2F
    "5700", // code 0x30
    "5800", // code 0x31
    "5900", // code 0x31
    "6000", // code 0x33
    "6100", // code 0x34
    "6200", // code 0x35
    "6300", // code 0x36
    "6400", // code 0x37
    "6500", // code 0x38
    "7000", // code 0x39
    "7500", // code 0x3A
    "8000", // code 0x3B
    "8500", // code 0x3C
    "9000", // code 0x3D
    "10000", // code 0x3E
    "11000", // code 0x3F
    "12000", // code 0x40
    "12400", // code 0x49
};

RGBController_Sinowealth::RGBController_Sinowealth(SinowealthController* controller_ptr, std::function<void ()> callback)
{
    controller                      = controller_ptr;
    shutdown_callback               = callback;

    name                            = controller->GetName();
    type                            = DEVICE_TYPE_MOUSE;
    description                     = "Sinowealth Device";
    location                        = controller->GetLocation();
    serial                          = controller->GetSerialString();
    version                         = controller->GetFirmwareVersion();
    flags                           |= CONTROLLER_FLAG_MANUALLY_CONFIGURABLE_DEVICE_SPECIFIC;

    last_mode = -1;
    dpi.resize(controller->GetPresetCount(), 1);

    mode Static;
    Static.name                     = "Static";
    Static.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.brightness_min           = GLORIOUS_BRIGHTNESS_LOW;
    Static.brightness               = GLORIOUS_BRIGHTNESS_NORMAL;
    Static.brightness_max           = GLORIOUS_BRIGHTNESS_HIGH;
    Static.color_mode               = MODE_COLORS_PER_LED;
    Static.value                    = GLORIOUS_MODE_STATIC;
    Static.colors_min               = 4;//controller->GetLEDCount();
    Static.colors_max               = 4;//controller->GetLEDCount();
    Static.colors.resize(4);//controller->GetLEDCount());
    modes.push_back(Static);

    mode Single;
    Single.name                     = "Single color";
    Single.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Single.brightness_min           = GLORIOUS_BRIGHTNESS_LOW;
    Single.brightness               = GLORIOUS_BRIGHTNESS_NORMAL;
    Single.brightness_max           = GLORIOUS_BRIGHTNESS_HIGH;
    Single.color_mode               = MODE_COLORS_PER_LED;
    Single.value                    = GLORIOUS_MODE_STATIC_SINGLE;
    Single.colors_min               = 1;
    Single.colors_max               = 1;
    Single.colors.resize(1);
    modes.push_back(Single);

    mode Off;
    Off.name                        = "Off";
    Off.flags                       = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Off.color_mode                  = MODE_COLORS_PER_LED;
    Off.value                       = GLORIOUS_MODE_OFF;
    modes.push_back(Off);

    mode Rainbow;
    Rainbow.name                    = "Rainbow";
    Rainbow.flags                   = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_AUTOMATIC_SAVE;
    Rainbow.speed_min               = GLORIOUS_SPEED_SLOW;
    Rainbow.speed                   = GLORIOUS_SPEED_NORMAL;
    Rainbow.speed_max               = GLORIOUS_SPEED_FAST;
    Rainbow.direction               = MODE_DIRECTION_UP;
    Rainbow.color_mode              = MODE_COLORS_PER_LED;
    Rainbow.value                   = GLORIOUS_MODE_RAINBOW;
    modes.push_back(Rainbow);

    mode SpectrumBreathing;
    SpectrumBreathing.name          = "Multicolor Breathing";
    SpectrumBreathing.flags         = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_AUTOMATIC_SAVE;
    SpectrumBreathing.speed_min     = GLORIOUS_SPEED_SLOW;
    SpectrumBreathing.speed         = GLORIOUS_SPEED_NORMAL;
    SpectrumBreathing.speed_max     = GLORIOUS_SPEED_FAST;
    SpectrumBreathing.colors_min    = controller->getBreathingLimit();
    SpectrumBreathing.colors_max    = controller->getBreathingLimit();
    SpectrumBreathing.color_mode    = MODE_COLORS_PER_LED;
    SpectrumBreathing.value         = GLORIOUS_MODE_SPECTRUM_BREATHING;
    SpectrumBreathing.colors.resize(controller->getBreathingLimit());
    modes.push_back(SpectrumBreathing);

    mode Chase;
    Chase.name                      = "Tail";
    Chase.flags                     = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Chase.speed_min                 = GLORIOUS_SPEED_SLOW;
    Chase.speed                     = GLORIOUS_SPEED_NORMAL;
    Chase.speed_max                 = GLORIOUS_SPEED_FAST;
    Chase.brightness_min            = GLORIOUS_BRIGHTNESS_LOW;
    Chase.brightness                = GLORIOUS_BRIGHTNESS_NORMAL;
    Chase.brightness_max            = GLORIOUS_BRIGHTNESS_HIGH;
    Chase.color_mode                = MODE_COLORS_PER_LED;;
    Chase.value                     = GLORIOUS_MODE_TAIL;
    modes.push_back(Chase);

    mode SpectrumCycle;
    SpectrumCycle.name              = "Spectrum Cycle";
    SpectrumCycle.flags             = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_AUTOMATIC_SAVE;
    SpectrumCycle.speed_min         = GLORIOUS_SPEED_SLOW;
    SpectrumCycle.speed             = GLORIOUS_SPEED_NORMAL;
    SpectrumCycle.speed_max         = GLORIOUS_SPEED_FAST;
    SpectrumCycle.color_mode        = MODE_COLORS_PER_LED;
    SpectrumCycle.value             = GLORIOUS_MODE_SPECTRUM_CYCLE;
    modes.push_back(SpectrumCycle);

    mode Flashing;
    Flashing.name                   = "Rave";
    Flashing.flags                  = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Flashing.speed_min              = GLORIOUS_SPEED_SLOW;
    Flashing.speed                  = GLORIOUS_SPEED_NORMAL;
    Flashing.speed_max              = GLORIOUS_SPEED_FAST;
    Flashing.brightness_min         = GLORIOUS_BRIGHTNESS_LOW;
    Flashing.brightness             = GLORIOUS_BRIGHTNESS_NORMAL;
    Flashing.brightness_max         = GLORIOUS_BRIGHTNESS_HIGH;
    Flashing.colors_min             = 2;
    Flashing.colors_max             = 2;
    Flashing.color_mode             = MODE_COLORS_PER_LED;
    Flashing.value                  = GLORIOUS_MODE_RAVE;
    Flashing.colors.resize(2);
    modes.push_back(Flashing);

    mode Epilepsy;
    Epilepsy.name                   = "Epilepsy";
    Epilepsy.flags                  = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Epilepsy.color_mode             = MODE_COLORS_PER_LED;
    Epilepsy.value                  = GLORIOUS_MODE_EPILEPSY;
    modes.push_back(Epilepsy);

    mode Wave;
    Wave.name                       = "Wave";
    Wave.flags                      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Wave.speed_min                  = GLORIOUS_SPEED_SLOW;
    Wave.speed                      = GLORIOUS_SPEED_NORMAL;
    Wave.speed_max                  = GLORIOUS_SPEED_FAST;
    Wave.brightness_min             = GLORIOUS_BRIGHTNESS_LOW;
    Wave.brightness                 = GLORIOUS_BRIGHTNESS_NORMAL;
    Wave.brightness_max             = GLORIOUS_BRIGHTNESS_HIGH;
    Wave.color_mode                 = MODE_COLORS_PER_LED;
    Wave.value                      = GLORIOUS_MODE_WAVE;
    modes.push_back(Wave);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.flags                 = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.speed_min             = GLORIOUS_SPEED_SLOW;
    Breathing.speed                 = GLORIOUS_SPEED_NORMAL;
    Breathing.speed_max             = GLORIOUS_SPEED_FAST;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.color_mode            = MODE_COLORS_PER_LED;
    Breathing.value                 = GLORIOUS_MODE_BREATHING;
    Breathing.colors.resize(1);
    modes.push_back(Breathing);

    SetupZones();

    setColorOrder("RBG"); // Default is "RBG", Panteon PS150 uses RGB

    loadDeviceConfig(); // Note: color order may change after constructor exits, and the values will have been loaded with wrong order; may need to re-interpret them when color order is applied!
}

RGBController_Sinowealth::~RGBController_Sinowealth()
{
    Shutdown();

    delete controller;

    if(shutdown_callback)
    {
        shutdown_callback();
    }
}


void RGBController_Sinowealth::SetupZones()
{
    /*---------------------------------------------------------*\
    | Create a single zone                                      |
    \*---------------------------------------------------------*/
    zone logo_zone;
    logo_zone.name       = "DPI presets(Logo + Wheel)";
    logo_zone.type       = ZONE_TYPE_SINGLE;
    logo_zone.leds_min   = controller->GetPresetCount();
    logo_zone.leds_max   = controller->GetPresetCount();
    logo_zone.leds_count = controller->GetPresetCount();
    zones.push_back(logo_zone);

    zone main_zone;
    main_zone.name       = "Main zone";
    main_zone.type       = ZONE_TYPE_LINEAR;
    // NOTE: LED count is temporary and will change the moment actual modes load in
    main_zone.leds_min   = 1;
    main_zone.leds_max   = 1;
    main_zone.leds_count = 1;
    zones.push_back(main_zone);

    setupLeds();
}

void RGBController_Sinowealth::setupLeds()
{
    /*---------------------------------------------------------*\
    | Set up LEDs vector and assign zone data                   |
    \*---------------------------------------------------------*/

    int total_led_count = zones[0].leds_count + zones[1].leds_count;
    int old_size = leds.size();
    leds.resize(total_led_count);
    //led_display_names.resize(total_led_count);
    SetupColors();

    /*---------------------------------------------------------*\
    | Set LED names                                             |
    | Use DPI values for DPI preset LEDs                        |
    \*---------------------------------------------------------*/
    if(active_dpi_preset > 0 && active_dpi_preset <= 6)
    {
        for(std::size_t led_idx = 0; led_idx < zones[0].leds_count; led_idx++)
        {
            std::ostringstream ss;
            ss << "DPI preset " << led_idx << " color";
            zones[0].leds[led_idx].name = ss.str();
/*
            unsigned int dpi_idx = dpi[active_dpi_preset - 1];
            if(dpi_idx > 0)
            {
                led_display_names[led_idx] = SinowealthDpiOption[dpi_idx - 1];
            }
            else
            {
                led_display_names[led_idx] = "";
            }
*/
        }
    }

    if(old_size != total_led_count)
    {
        SignalUpdate(RGBCONTROLLER_UPDATE_REASON_CONFIGUREZONE);
    }

    zones[1].flags = (zones[1].leds_min == zones[1].leds_max) ? 0 : (ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE | ZONE_FLAG_MANUALLY_CONFIGURED_SIZE);
    // zones[1].leds[0].name = "Mouse LED";
}

void RGBController_Sinowealth::DeviceUpdateLEDs()
{
    if(active_mode < int(modes.size()) && (zones[1].leds_count > 0))
    {
        // Copy mode colors from the main zone to active mode
        modes[active_mode].colors.resize(zones[1].leds_count);
        memcpy(&modes[active_mode].colors[0], zones[1].colors, sizeof(RGBColor) * modes[active_mode].colors.size());
    }

    DeviceUpdateMode();
}

void RGBController_Sinowealth::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Sinowealth::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Sinowealth::DeviceUpdateMode()
{
    if(active_mode != last_mode)
    {
        // Validate to prevent going out of bounds due to a faulty saved profile!
        unsigned int led_count = modes[active_mode].colors.size();
        if(led_count < modes[active_mode].colors_min)
            led_count = modes[active_mode].colors_min;
        if(led_count > modes[active_mode].colors_max)
            led_count = modes[active_mode].colors_max;
        modes[active_mode].colors.resize(led_count);

        zones[1].leds_count = led_count;
        zones[1].leds_min   = modes[active_mode].colors_min;
        zones[1].leds_max   = modes[active_mode].colors_max;
        setupLeds(); // Force the zone to resize

        // Copy mode colors to the main zone
        if(!modes[active_mode].colors.empty())
        {
            memcpy(zones[1].colors, &modes[active_mode].colors[0], sizeof(RGBColor) * modes[active_mode].colors.size());
        }

        last_mode = active_mode;
    }

    int speed      = (modes[active_mode].flags & MODE_FLAG_HAS_SPEED)      ? modes[active_mode].speed      : 4;
    int brightness = (modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS) ? modes[active_mode].brightness : 4;
    int direction  = (modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_HV)
                  || (modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_UD)
                  || (modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_LR)
                    ? modes[active_mode].brightness : 4;


    controller->requestProfile();
    controller->setDpiData(active_dpi_preset, dpi_mask, &dpi[0], zones[0].colors);
    controller->setMode(modes[active_mode].value, speed, brightness, direction, zones[1].colors);
    controller->sendProfile();
}

static unsigned char DefaultDpi[6] =
{
    0x01, 0x07, 0x0f, 0x1f, 0x27, 0x3d
};

void RGBController_Sinowealth::loadDeviceConfig()
{
    if (controller->requestProfile() < SINOWEALTH_CONFIG_SIZE_MIN)
    {
        return;
    }
    std::vector<unsigned char> raw_config = controller->GetRawConfig();

    active_mode = 0;

    /*----------------------------------------*\
    | Perform certain sanity checks            |
    \*----------------------------------------*/
    unsigned int profile_id   = raw_config[0x02];
    unsigned int preset_const = raw_config[0x0B] & 0x0F;
    sensor_id                 = raw_config[0x0E];

    LOG_DEBUG("[Sinowealth controller]: Performed value load, profile ID=0x%02X (expected 0x11 or 0x21), preset const = 0x%02x (expected 0x06), sensor ID = 0x%02x (sensor model %s)", profile_id, preset_const, (unsigned int)(sensor_id), sensor_model.c_str());

    /*----------------------------------------*\
    | Load in DPI presets                      |
    \*----------------------------------------*/
    dpi_mask = raw_config[0x0C];
    dpi.resize(controller->GetPresetCount());
    if(colors.size() < 6)
    {
        colors.resize(6);
    }

    for(int i = 0; i < 6; ++i)
    {
        dpi[i] = raw_config[0x0D + i];
        unsigned int raw_color = (raw_config[i]) ;
        int offset = 0x1C + i * 3;
        raw_color = raw_config[offset]
                  | raw_config[offset + 1] << 8
                  | raw_config[offset + 2] << 16;
        colors[i] = reorderColorReverse(raw_color);
    }

    // 0x13 through 0x29 are unknown

    /*-----------------------------------------------*\
    | VALIDATE DPI!!! In case wrong data was received |
    | Mostly to ensure we don't corrupt the mouse     |
    \*-----------------------------------------------*/

    // Parse bare minimum values
    bool faultyDpi = false;

    // The byte is typically 0x16, 0x26, 0x36...
    active_dpi_preset = (raw_config[0x0B] >> 4) & 0x0F;
    if(active_dpi_preset == 0 || active_dpi_preset > 6)
    {
        faultyDpi = true;
        active_dpi_preset = 1; // Fallback value to re-enable mouse control
    }

    if((dpi_mask | 0xC0) == 0xFF)
    {
        faultyDpi = true;
    }

    unsigned int preset_count = controller->GetPresetCount();
    for(unsigned int i = 0; i < preset_count; ++i)
    {
        dpi[i] = raw_config[0x0D + i];
        if(dpi[i] < 1 || dpi[i] > 0x49)
        {
            faultyDpi = true;
        }
    }

    if(faultyDpi)
    {
        LOG_WARNING("[SinowealthController] Faulty DPI value detected! Replacing values!");
        dpi_mask = 0xC0;
        for(int i = 0; i < 6; ++i)
        {
            dpi[i] = DefaultDpi[i];
        }
        //SetDpi(DefaultDpi, 6, true);
    }

    /*----------------------------------------*\
    | Load in Modes & color data for each mode |
    \*----------------------------------------*/
    for(size_t i = 0; i < modes.size(); ++i)
    {
        if(modes[i].value == raw_config[0x35])
        {
            active_mode = i;
        }

        switch (modes[i].value)
        {
            case GLORIOUS_MODE_RAINBOW:
                modes[i].brightness = (raw_config[0x36] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x36]) & 0xF;
                modes[i].direction  = raw_config[0x37];
                break;
            case GLORIOUS_MODE_STATIC:
            {
                modes[i].brightness = (raw_config[0x36] >> 4) & 0xF;
                modes[i].colors.resize(1);
                RGBColor out_color = ToRGBColor(raw_config[0x39], raw_config[0x3A], raw_config[0x3B]); // Note: might be wrong G-B order
                modes[i].colors[0] = out_color;
                break;
            }
            case GLORIOUS_MODE_SPECTRUM_BREATHING:
            {
                //colours not yet researched
                modes[i].brightness = (raw_config[0x3C] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x3C]) & 0xF;
                modes[i].colors.resize(7);

                for(int color_index = 0; color_index < 7; ++color_index)
                {
                    int offset = 0x3E + color_index * 3;
                    RGBColor out_color = ToRGBColor(raw_config[offset], raw_config[offset + 1], raw_config[offset + 2]); // Note: might be wrong G-B order
                    modes[i].colors[color_index] = out_color;
                }
                break;
            }
            case GLORIOUS_MODE_TAIL:
                modes[i].brightness = (raw_config[0x53] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x53]) & 0xF;
                break;
            case GLORIOUS_MODE_SPECTRUM_CYCLE:
                modes[i].brightness = (raw_config[0x54] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x54]) & 0xF;
                break;
            case GLORIOUS_MODE_RAVE:
            {
                modes[i].brightness = (raw_config[0x74] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x74]) & 0xF;
                for(int color_index = 0; color_index < 2; ++color_index)
                {
                    int offset = 0x75 + color_index * 3;
                    RGBColor out_color = ToRGBColor(raw_config[offset], raw_config[offset + 1], raw_config[offset + 2]); // Note: might be wrong G-B order
                    modes[i].colors[color_index] = out_color;
                }
                break;
            }
            case GLORIOUS_MODE_WAVE:
                modes[i].brightness = (raw_config[0x7C] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x7C]) & 0xF;
                break;
            case GLORIOUS_MODE_BREATHING:
            {
                modes[i].brightness = (raw_config[0x7D] >> 4) & 0xF;
                modes[i].speed      = (raw_config[0x7D]) & 0xF;
                modes[i].colors.resize(1);
                RGBColor out_color = ToRGBColor(raw_config[0x7E], raw_config[0x7F], raw_config[0x80]); // Note: might be wrong G-B order
                modes[i].colors[0] = out_color;
            }
            case GLORIOUS_MODE_OFF:
                break;
            default:
                break;
        }
    }

    if(active_mode != last_mode)
    {
        zones[1].leds_count = modes[active_mode].colors.size();
        zones[1].leds_min   = modes[active_mode].colors_min;
        zones[1].leds_max   = modes[active_mode].colors_max;
        setupLeds(); // Force the zone to resize

        last_mode = active_mode;
    }

    // We just loaded colors for each mode - now load zone colors from the active mode
    if(!modes[active_mode].colors.empty())
    {
        memcpy(zones[1].colors, &modes[active_mode].colors[0], sizeof(RGBColor) * modes[active_mode].colors.size());
    }

    // Propagate the "configuration" string with updated schema
    initDeviceSpecificConfiguration();
}

nlohmann::json RGBController_Sinowealth::GetDeviceSpecificConfigurationSchema()
{
    nlohmann::json schema;
    //std::vector<unsigned char> dpis = controller->GetDpi();
    unsigned int preset_count = controller->GetPresetCount();
    if(preset_count < 1)
    {
        preset_count = 1;
    }

    schema["color_order"]["title"]       = "RGB Color order";
    schema["color_order"]["description"] = "Change if you get wrong colors (e.g. green turns blue and blue turns green)";
    schema["color_order"]["type"]        = "string";
    schema["color_order"]["default"]     = "RBG";
    schema["color_order"]["enum"]        = {"RGB", "RBG", "GRB", "GBR", "BRG", "BGR"};

    schema["dpi_presets"]["title"]       = "DPI Presets";
    schema["dpi_presets"]["description"] = "DPI Presets available to scroll through on the mouse\nNumber of presets can be reduced by resizing \"Presets\" zone";
    schema["dpi_presets"]["type"]        = "object";
    nlohmann::json& properties           = (schema["dpi_presets"]["properties"] = nlohmann::json::object({}));

    for(unsigned int i = 1; i <= preset_count; ++i)
    {
        std::ostringstream ss_index, ss_title, ss_index_use, ss_title_use;
        ss_index     << "dpi_" << i;
        ss_index_use << "dpi_use_" << i;
        ss_title     << "DPI preset " << i;
        ss_title_use << "Use DPI preset " << i;

        if(i == active_dpi_preset)
        {
            ss_title << " *";
        }

        nlohmann::json& preset_use = (properties[ss_index_use.str()] = nlohmann::json::object({}));
        preset_use["title"]        = ss_title_use.str();
        preset_use["description"]  = "Flag this DPI preset to be used (at least one preset has to be enabled)";
        preset_use["type"]         = "bool";
        preset_use["default"]      = true;
        preset_use["order"]        = i * 2;

        nlohmann::json& preset = (properties[ss_index.str()] = nlohmann::json::object({}));
        preset["title"]        = ss_title.str();
        preset["description"]  = "DPI value";
        preset["type"]         = "integer";
        preset["default"]      = SinowealthDpiOption[i];
        preset["enum"]         = nlohmann::json::array();
        preset["enumNames"]    = nlohmann::json::array();
        preset["order"]        = i * 2 + 1;

        for(size_t i = 0; i < sizeof(SinowealthDpiOption) / sizeof(SinowealthDpiOption[0]); ++i)
        {
            preset["enum"][i] = i + 1;
            preset["enumNames"][i] = SinowealthDpiOption[i];
        }

    }

    return schema;
}

nlohmann::json RGBController_Sinowealth::GetDeviceSpecificConfiguration()
{
    nlohmann::json result;

    unsigned int preset_count = controller->GetPresetCount();

    if(dpi.size() < preset_count)
    {
        dpi.resize(preset_count);
    }

    for(unsigned int i = 0; i < preset_count; ++i)
    {
        std::ostringstream ss_index, ss_use_index;
        ss_index << "dpi_" << i + 1;
        ss_use_index << "dpi_use_" << i + 1;

        result["dpi_presets"][ss_index.str()] = int(dpi[i]);
        bool enable = !(dpi_mask & (1 << i));
        result["dpi_presets"][ss_use_index.str()] = enable;
    }

    return result;
}

void RGBController_Sinowealth::applyDeviceSpecificConfiguration(const nlohmann::json& config)
{
    std::vector<unsigned char> new_dpis;
    uint8_t new_dpi_mask = 0xC0;

    new_dpis.resize(controller->GetPresetCount(), 1);

    if(config.contains("dpi_presets"))
    {
        unsigned int preset_count = controller->GetPresetCount();
        unsigned int new_active_preset = 0;

        // Reload internal config so we are NOT changing any mouse settings other than what we just received
        loadDeviceConfig();

        // Note: a copy is made bc it CRASHES otherwise!
        const nlohmann::json& presets = config["dpi_presets"];
        if(presets.is_object())
        {
            LOG_DEBUG("Loading new DPIs from preset list: %s", presets.dump().c_str());
            for(unsigned int i = 0; i < preset_count; ++i)
            {
                std::ostringstream ss, ss_use;
                ss << "dpi_" << i + 1;
                ss_use << "dpi_use_" << i + 1;
                std::string key = ss.str();

                if(presets.contains(key))
                {
                    int value = int(presets[key]);
                    if(value < 1 || value >= 0x049)
                    {
                        // This area should not be reachable, but a failsafe is in place regardless
                        value = DefaultDpi[i];
                    }
                    new_dpis[i] = value;
                }
                key = ss_use.str();
                if(presets.contains(key))
                {
                    bool value = bool(presets[key]);
                    if(!value)
                    {
                        new_dpi_mask |= 1 << i;
                    }
                }
                else
                {
                    if(active_dpi_preset == i + 1 || !new_active_preset)
                    {
                        new_active_preset = i + 1;
                    }
                }
            }
            if(!new_active_preset)
            {
                LOG_DIALOG("At least one DPI preset has to be active!\nForcing last active to be enabled");
                new_dpi_mask &= ~(1 << (active_dpi_preset - 1));
            }

            LOG_DEBUG("Loaded %d DPIs, next active preset: %d", int(new_dpis.size()), new_active_preset);
            active_dpi_preset = new_active_preset;
            dpi_mask = new_dpi_mask;
            dpi.swap(new_dpis);
        }
    }
}

void RGBController_Sinowealth::DeviceUpdateDeviceSpecificZoneConfiguration(int zone_idx)
{
    if(zone_idx == 1)
    {
        // Effect LED count
        unsigned int color_count = zones[1].leds_count;
        if(color_count < zones[1].leds_min)
            color_count = zones[1].leds_min;
        if(color_count > zones[1].leds_max)
            color_count = zones[1].leds_max;
        zones[1].leds_count = color_count;
        modes[active_mode].colors.resize(color_count);
    }

    // Rearrange LEDs
    setupLeds();
    DeviceUpdateMode();
}

void RGBController_Sinowealth::initDeviceSpecificConfiguration()
{
    nlohmann::json con;
    con["configuration"] = GetDeviceSpecificConfiguration();
    con["schema"] = GetDeviceSpecificConfigurationSchema();
    configuration = con.dump();
}

void RGBController_Sinowealth::DeviceUpdateDeviceSpecificConfiguration()
{
    // The device can configure DPI presets (up to 6), KEY assignemts (up to 7), and RGB order for some models
    // So far only DPI preset setting is implemented
    nlohmann::json configuration_json;

    try
    {
        configuration_json = nlohmann::json::parse(configuration);
    }
    catch(...)
    {
        return;
    }

    if(!configuration_json.contains("configuration") || !configuration_json["configuration"].is_object())
    {
        return;
    }

    const nlohmann::json& config = configuration_json["configuration"];

    applyDeviceSpecificConfiguration(config);
}

void RGBController_Sinowealth::setColorOrder(const std::string& rgb_order)
{
    if(rgb_order.empty())
    {
        return;
    }

    /*---------------------------------------------------------------------*\
    | Convert to uppercase                                                  |
    \*---------------------------------------------------------------------*/
    std::string key = rgb_order;
    std::transform(key.begin(), key.end(), key.begin(),
                   [](unsigned char c){ return char(std::toupper(c)); });

    auto rp = key.find('R');
    auto gp = key.find('G');
    auto bp = key.find('B');

    if(rp >= 3 || gp >= 3 || bp >= 3)
    {
        return;
    }

    uint32_t order = rp | (gp << 8) | (bp << 16);

    color_order = order;
}

std::string RGBController_Sinowealth::getColorOrder() const
{
    std::string out = "OFF"; // Placeholder value

    uint8_t bo_b        =  color_order        & 0xFF;
    uint8_t bo_g        = (color_order >> 8 ) & 0xFF;
    uint8_t bo_r        = (color_order >> 16) & 0xFF;

    bool in_range       = (bo_r < 3 && bo_g < 3 && bo_b < 3);
    bool distinct       = (bo_r != bo_g && bo_r != bo_b && bo_g != bo_b);

    if(in_range && distinct)
    {
        out[bo_r] = 'R';
        out[bo_g] = 'G';
        out[bo_b] = 'B';
        return out;
    }

    return "INVALID";
}

uint32_t RGBController_Sinowealth::reorderColor(RGBColor in)
{
    uint8_t r = RGBGetRValue(in);
    uint8_t g = RGBGetGValue(in);
    uint8_t b = RGBGetBValue(in);

    uint8_t bo_r        =  color_order        & 0xFF;
    uint8_t bo_g        = (color_order >> 8 ) & 0xFF;
    uint8_t bo_b        = (color_order >> 16) & 0xFF;

    uint32_t result = r << (8 * bo_r)
                    | g << (8 * bo_g)
                    | b << (8 * bo_b);

    return result;
}


RGBColor RGBController_Sinowealth::reorderColorReverse(uint32_t in)
{
    uint8_t bo_r        =  color_order        & 0xFF;
    uint8_t bo_g        = (color_order >> 8 ) & 0xFF;
    uint8_t bo_b        = (color_order >> 16) & 0xFF;

    uint8_t r = (in >> (8 * bo_r)) & 0xFF;
    uint8_t g = (in >> (8 * bo_g)) & 0xFF;
    uint8_t b = (in >> (8 * bo_b)) & 0xFF;

    RGBColor result = r | (g << 8) | (b << 16);

    return result;
}
