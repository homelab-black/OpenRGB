#pragma once

#include <cstdint>
#include <string>
#include <vector>

typedef unsigned int RGBColor;
typedef unsigned int controller_flags;

#define RGBGetRValue(rgb)   (rgb & 0x000000FF)
#define RGBGetGValue(rgb)  ((rgb & 0x0000FF00) >> 8)
#define RGBGetBValue(rgb)  ((rgb & 0x00FF0000) >> 16)
#define ToRGBColor(r, g, b) ((RGBColor)(((b) << 16) | ((g) << 8) | (r)))

enum
{
    DEVICE_TYPE_KEYBOARD = 6,
    CONTROLLER_FLAG_NO_PROFILE_APPLY = (1 << 9),
    MODE_FLAG_HAS_MODE_SPECIFIC_COLOR = (1 << 10),
    MODE_FLAG_HAS_PER_LED_COLOR = (1 << 11),
    MODE_COLORS_MODE_SPECIFIC = 1,
    MODE_COLORS_PER_LED = 2,
    ZONE_TYPE_MATRIX = 2
};

struct mode
{
    std::string name;
    int value = 0;
    unsigned int flags = 0;
    unsigned int color_mode = 0;
    unsigned int colors_min = 0;
    unsigned int colors_max = 0;
    std::vector<RGBColor> colors;
};

class matrix_map_type
{
public:
    void Set(unsigned int new_height, unsigned int new_width, unsigned int* new_map)
    {
        height = new_height;
        width  = new_width;

        if(new_map)
        {
            map.assign(new_map, new_map + (height * width));
        }
        else
        {
            map.assign(height * width, 0);
        }
    }

    unsigned int height = 0;
    unsigned int width  = 0;
    std::vector<unsigned int> map;
};

struct led
{
    std::string name;
    unsigned int value = 0;
};

struct zone
{
    std::string name;
    unsigned int type = 0;
    unsigned int leds_count = 0;
    unsigned int leds_min = 0;
    unsigned int leds_max = 0;
    unsigned int start_idx = 0;
    RGBColor* colors = nullptr;
    led* leds = nullptr;
    matrix_map_type matrix_map;
};

class RGBController
{
public:
    virtual ~RGBController() = default;

    void Shutdown() {}

    void SetupColors()
    {
        unsigned int count = 0;
        for(const zone& current_zone : zones)
        {
            count += current_zone.leds_count;
        }
        colors.assign(count, 0);
    }

    void UpdateLEDs()                 { DeviceUpdateLEDs(); }
    void UpdateZoneLEDs(int zone_idx) { DeviceUpdateZoneLEDs(zone_idx); }
    void UpdateSingleLED(int led_idx) { DeviceUpdateSingleLED(led_idx); }
    void UpdateMode()                 { DeviceUpdateMode(); }
    void EnableUserLightingControl()  { DeviceEnableUserLightingControl(); }

    void SetActiveMode(int mode_idx)
    {
        active_mode = mode_idx;
        UpdateMode();
    }

    void SetCustomMode()
    {
        active_mode = 1;
    }

    void SetModeColor(unsigned int mode_idx, unsigned int color_idx, RGBColor color)
    {
        modes[mode_idx].colors[color_idx] = color;
    }

    virtual void DeviceUpdateLEDs() {}
    virtual void DeviceUpdateZoneLEDs(int) {}
    virtual void DeviceUpdateSingleLED(int) {}
    virtual void DeviceUpdateMode() {}
    virtual void DeviceEnableUserLightingControl() {}

    std::string name;
    std::string vendor;
    std::string description;
    std::string location;
    unsigned int type = 0;
    controller_flags flags = 0;
    int active_mode = 0;
    std::vector<mode> modes;
    std::vector<zone> zones;
    std::vector<led> leds;
    std::vector<RGBColor> colors;
};
