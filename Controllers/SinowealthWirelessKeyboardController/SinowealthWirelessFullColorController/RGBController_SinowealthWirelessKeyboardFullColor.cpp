#include "RGBController_SinowealthWirelessKeyboardFullColor.h"
#include <LogManager.h>
#include "RGBControllerKeyNames.h"
#include "KeyboardLayoutManager.h"
#include "SinowealthWirelessFullColorDevices.h"

using namespace std::chrono_literals;

RGBController_SinowealthWirelessKeyboardFullColor::RGBController_SinowealthWirelessKeyboardFullColor(
    SinowealthWirelessKeyboardController* controller, unsigned char device_id)
    : RGBController_SinowealthWirelessKeyboard(controller), controller(controller), device_id(device_id)
{
    mode Direct;
    Direct.name       = "Direct";
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode = 0;

    SetupZones();
}

void RGBController_SinowealthWirelessKeyboardFullColor::SetupZones()
{
    /*---------------------------------------------------------*\
    | Create the keyboard zone usiung Keyboard Layout Manager   |
    \*---------------------------------------------------------*/
    zone new_zone;
    new_zone.name = ZONE_EN_KEYBOARD;
    new_zone.type = ZONE_TYPE_MATRIX;

    sinowealth_device device = sinowealth_wireless_keyboards.at(device_id);

    KeyboardLayoutManager new_kb(KEYBOARD_LAYOUT_ANSI_QWERTY, device.keyboard_layout.base_size,
                                 device.keyboard_layout.key_values);
    new_kb.ChangeKeys(device.keyboard_layout.edit_keys);

    matrix_map_type* new_map    = new matrix_map_type;
    new_zone.matrix_map         = new_map;
    new_zone.matrix_map->height = new_kb.GetRowCount();
    new_zone.matrix_map->width  = new_kb.GetColumnCount();

    new_zone.matrix_map->map = new unsigned int[new_map->height * new_map->width];
    new_zone.leds_count      = new_kb.GetKeyCount();
    new_zone.leds_min        = new_zone.leds_count;
    new_zone.leds_max        = new_zone.leds_count;
    new_kb.GetKeyMap(new_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT, new_map->height, new_map->width);

    for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;

        new_led.name  = new_kb.GetKeyNameAt(led_idx);
        new_led.value = new_kb.GetKeyValueAt(led_idx);
        leds.push_back(new_led);
    }

    zones.push_back(new_zone);

    SetupColors();
}

void RGBController_SinowealthWirelessKeyboardFullColor::SendColor()
{
    if(!led_color_map_lock.try_lock())
    {
        return;
    }

    if(led_color_map.size() == 1 && led_color_map.begin()->second.size() == leds.size())
    {
        controller->SendSolidColor(led_color_map.begin()->first);
    }
    else
    {
        controller->SendLEDColors(led_color_map);
    }

    led_color_map_lock.unlock();
}

void RGBController_SinowealthWirelessKeyboardFullColor::UpdateLEDs()
{
    std::unordered_map<RGBColor, std::vector<unsigned char>> new_color_map;

    for(unsigned char bits_to_discard = 0; bits_to_discard < 8; bits_to_discard++)
    {
        new_color_map.clear();
        int payload_size = 0;
        for(size_t i = 0; i < colors.size(); ++i)
        {
            if(colors[i] == 0)
            {
                continue;
            }
            RGBColor color = QuantizeColor(colors[i], bits_to_discard);
            // Each lit LED adds one byte
            payload_size += 1;
            new_color_map[color].push_back(leds[i].value);
        }

        // Each color adds 4 bytes
        payload_size += new_color_map.size() * 4;

        if(payload_size < 14 * 9)
        {
            break;
        }
    }

    if(new_color_map == led_color_map)
    {
        return;
    }

    color_changed = true;

    if(led_color_map_lock.try_lock())
    {
        led_color_map = std::move(new_color_map);
        led_color_map_lock.unlock();
    }
}

RGBColor RGBController_SinowealthWirelessKeyboardFullColor::QuantizeColor(RGBColor color, unsigned char bits_to_discard)
{
    // A value of 0 means no quantization.
    if(bits_to_discard <= 0)
    {
        return color;
    }
    // Don't discard all the bits.
    if(bits_to_discard >= 8)
    {
        bits_to_discard = 7;
    }

    unsigned char r = RGBGetRValue(color);
    unsigned char g = RGBGetGValue(color);
    unsigned char b = RGBGetBValue(color);

    // Reduce precision by shifting right. This gets the most significant bits.
    unsigned char r_reduced = r >> bits_to_discard;
    unsigned char g_reduced = g >> bits_to_discard;
    unsigned char b_reduced = b >> bits_to_discard;

    // Scale back up to the 0-255 range by copying the high bits into the low bits.
    int           bits_to_keep = 8 - bits_to_discard;
    unsigned char r_quantized  = (r_reduced << bits_to_discard) | (r_reduced >> (bits_to_keep - bits_to_discard));
    unsigned char g_quantized  = (g_reduced << bits_to_discard) | (g_reduced >> (bits_to_keep - bits_to_discard));
    unsigned char b_quantized  = (b_reduced << bits_to_discard) | (b_reduced >> (bits_to_keep - bits_to_discard));

    return ToRGBColor(r_quantized, g_quantized, b_quantized);
}
