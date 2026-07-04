#include "LogManager.h"
#include "RGBController_NZXTFunctionKeyboard.h"
#include "KeyboardLayoutManager.h"


RGBController_NZXTFunctionKeyboard::RGBController_NZXTFunctionKeyboard(NZXTFunctionKeyboardController* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = "NZXT Function Keyboard";
    vendor                          = "NZXT";
    type                            = DEVICE_TYPE_KEYBOARD;
    description                     = "NZXT Function Keyboard";
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();
    version                         = controller->GetFirmwareVersion();

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = NZXT_FUNCTION_KEYBOARD_MODE_DIRECT;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.brightness_min           = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Direct.brightness_max           = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Direct.brightness               = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Rainbow;
    Rainbow.name                    = "Rainbow";
    Rainbow.value                   = NZXT_FUNCTION_KEYBOARD_MODE_RAINBOW;
    Rainbow.flags                   = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Rainbow.speed_min               = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Rainbow.speed_max               = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Rainbow.speed                   = 0x05;
    Rainbow.brightness_min          = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Rainbow.brightness_max          = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Rainbow.brightness              = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Rainbow.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Rainbow);

    mode Wave;
    Wave.name                       = "Wave";
    Wave.value                      = NZXT_FUNCTION_KEYBOARD_MODE_WAVE;
    Wave.flags                      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Wave.speed_min                  = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Wave.speed_max                  = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Wave.speed                      = 0x05;
    Wave.brightness_min             = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Wave.brightness_max             = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Wave.brightness                 = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Wave.direction                  = MODE_DIRECTION_LEFT;
    Wave.colors_min                 = 1;
    Wave.colors_max                 = 1;
    Wave.colors.resize(1);
    Wave.color_mode                 = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Wave);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.value                 = NZXT_FUNCTION_KEYBOARD_MODE_BREATH;
    Breathing.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Breathing.speed_min             = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Breathing.speed_max             = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Breathing.speed                 = 0x05;
    Breathing.brightness_min        = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Breathing.brightness_max        = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Breathing.brightness            = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.colors.resize(1);
    Breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Breathing);

    mode Reactive;
    Reactive.name                   = "Reactive";
    Reactive.value                  = NZXT_FUNCTION_KEYBOARD_MODE_REACTIVE;
    Reactive.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Reactive.speed_min              = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Reactive.speed_max              = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Reactive.speed                  = 0x05;
    Reactive.brightness_min         = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Reactive.brightness_max         = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Reactive.brightness             = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Reactive.colors_min             = 1;
    Reactive.colors_max             = 1;
    Reactive.colors.resize(1);
    Reactive.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Reactive);

    mode Ripple;
    Ripple.name                     = "Ripple";
    Ripple.value                    = NZXT_FUNCTION_KEYBOARD_MODE_RIPPLE;
    Ripple.flags                    = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Ripple.speed_min                = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Ripple.speed_max                = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Ripple.speed                    = 0x05;
    Ripple.brightness_min           = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Ripple.brightness_max           = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Ripple.brightness               = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Ripple.colors_min               = 1;
    Ripple.colors_max               = 1;
    Ripple.colors.resize(1);
    Ripple.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Ripple);

    mode Marquee;
    Marquee.name                    = "Marquee";
    Marquee.value                   = NZXT_FUNCTION_KEYBOARD_MODE_MARQUEE;
    Marquee.flags                   = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Marquee.speed_min               = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Marquee.speed_max               = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Marquee.speed                   = 0x05;
    Marquee.brightness_min          = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Marquee.brightness_max          = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Marquee.brightness              = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Marquee.colors_min              = 1;
    Marquee.colors_max              = 1;
    Marquee.colors.resize(1);
    Marquee.color_mode              = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Marquee);

    mode Raindrop;
    Raindrop.name                   = "Raindrop";
    Raindrop.value                  = NZXT_FUNCTION_KEYBOARD_MODE_RAINDROP;
    Raindrop.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Raindrop.speed_min              = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Raindrop.speed_max              = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Raindrop.speed                  = 0x05;
    Raindrop.brightness_min         = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Raindrop.brightness_max         = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Raindrop.brightness             = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Raindrop.colors_min             = 1;
    Raindrop.colors_max             = 1;
    Raindrop.colors.resize(1);
    Raindrop.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Raindrop);

    mode Aurora;
    Aurora.name                     = "Aurora";
    Aurora.value                    = NZXT_FUNCTION_KEYBOARD_MODE_AURORA;
    Aurora.flags                    = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Aurora.speed_min                = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Aurora.speed_max                = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Aurora.speed                    = 0x05;
    Aurora.brightness_min           = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Aurora.brightness_max           = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Aurora.brightness               = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Aurora.colors_min               = 1;
    Aurora.colors_max               = 1;
    Aurora.colors.resize(1);
    Aurora.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Aurora);

    mode Spark;
    Spark.name                      = "Spark";
    Spark.value                     = NZXT_FUNCTION_KEYBOARD_MODE_SPARK;
    Spark.flags                     = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Spark.speed_min                 = NZXT_FUNCTION_KEYBOARD_SPEED_MAX;
    Spark.speed_max                 = NZXT_FUNCTION_KEYBOARD_SPEED_MIN;
    Spark.speed                     = 0x05;
    Spark.brightness_min            = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN;
    Spark.brightness_max            = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Spark.brightness                = NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX;
    Spark.colors_min                = 1;
    Spark.colors_max                = 1;
    Spark.colors.resize(1);
    Spark.color_mode                = MODE_COLORS_MODE_SPECIFIC;
    modes.push_back(Spark);

    mode Off;
    Off.name                        = "Off";
    Off.value                       = 0xFF;
    Off.flags                       = 0;
    Off.color_mode                  = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();

}

RGBController_NZXTFunctionKeyboard::~RGBController_NZXTFunctionKeyboard()
{
    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].matrix_map != nullptr)
        {
            delete[] zones[zone_idx].matrix_map->map;
            delete zones[zone_idx].matrix_map;
        }
    }

    delete controller;
}
#include <iostream>

void RGBController_NZXTFunctionKeyboard::SetupZones()
{
    /*---------------------------------------------------------*\
    | Create keyboard layout using KeyboardLayoutManager        |
    \*---------------------------------------------------------*/
    zone new_zone;
    KeyboardLayoutManager new_kb(KEYBOARD_LAYOUT_ISO_AZERTY, KEYBOARD_SIZE_FULL, nzxt_function_keyboard_offset_values);

    matrix_map_type * new_map   = new matrix_map_type;
    new_zone.matrix_map         = new_map;
    new_zone.matrix_map->height = new_kb.GetRowCount();
    new_zone.matrix_map->width  = new_kb.GetColumnCount();

    new_zone.matrix_map->map    = new unsigned int[new_map->height * new_map->width];
    new_zone.leds_count         = new_kb.GetKeyCount();
    new_zone.leds_min           = new_zone.leds_count;
    new_zone.leds_max           = new_zone.leds_count;

    /*---------------------------------------------------------*\
    | Matrix map still uses declared zone rows and columns      |
    |   as the packet structure depends on the matrix map       |
    \*---------------------------------------------------------*/
    new_kb.GetKeyMap(new_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT, new_map->height, new_map->width);

    /*---------------------------------------------------------*\
    | Create LEDs for the Matrix zone                           |
    |   Place keys in the layout to populate the matrix         |
    \*---------------------------------------------------------*/
    //for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;

        new_led.name                = new_kb.GetKeyNameAt(led_idx);
        new_led.value               = new_kb.GetKeyValueAt(led_idx);
        std::cout << new_led.name << ": " << new_led.value << std::endl;
        leds.push_back(std::move(new_led));
    }

    zones.push_back(new_zone);

    SetupColors();


    /*---------------------------------------------------------*\
    | Create buffer map to translate OpenRGB LED order to       |
    | hardware LED order. The hardware expects 126 color values |
    | indexed by LED position (0-125).                          |
    \*---------------------------------------------------------*/
    /*null_color = 0x00000000;
    buffer_map.resize(NZXT_FUNCTION_KEYBOARD_NUM_LEDS, &null_color);

    for(size_t led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        buffer_map[leds[led_idx].value] = &colors[led_idx];
    }*/
}

void RGBController_NZXTFunctionKeyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/

}

void RGBController_NZXTFunctionKeyboard::DeviceUpdateLEDs()
{
    /*---------------------------------------------------------*\
    | Build color data array using buffer map and send to device|
    | The buffer_map translates from hardware LED index to the  |
    | corresponding color pointer for that LED.                 |
    \*---------------------------------------------------------*/
    if(!controller)
    {
        LOG_ERROR("[NZXT] controller is NULL");
        return;
    }
    std::cout << "Update" << std::endl;

    if(colors.size() < NZXT_FUNCTION_KEYBOARD_NUM_LEDS)
    {
        LOG_ERROR("[NZXT] colors.size() = %d, attendu %d",
                  (int)colors.size(), NZXT_FUNCTION_KEYBOARD_NUM_LEDS);
        return;
    }

    std::map<std::tuple<int, int, int>, std::vector<unsigned int>> leds_states_by_color; // {(R,G,B): [indexes]}

    for(int i = 0; i < NZXT_FUNCTION_KEYBOARD_NUM_LEDS; i++)
    {
        auto rgb_tuple = std::tuple<int,int,int>(RGBGetRValue(colors[i]), RGBGetGValue(colors[i]),RGBGetBValue(colors[i]));
        if(rgb_tuple == std::tuple<int,int,int>(0, 0, 0)) {
            continue;
        }
        if(leds_states_by_color.find(rgb_tuple) == leds_states_by_color.end()) {
            // Append the color to the list
            leds_states_by_color[rgb_tuple] = std::vector<unsigned int>({leds[i].value});
        } else {
            // Append the led to this color
            leds_states_by_color[rgb_tuple].push_back(leds[i].value);
        }
    }

    controller->SendColors(leds_states_by_color, modes[active_mode].brightness);

}

void RGBController_NZXTFunctionKeyboard::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_NZXTFunctionKeyboard::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_NZXTFunctionKeyboard::DeviceUpdateMode()
{

}
