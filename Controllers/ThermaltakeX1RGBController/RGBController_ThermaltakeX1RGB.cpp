#include "RGBController_ThermaltakeX1RGB.h"

/* User-validated LED-index order. Do not sort this list by physical position.
 * Index 101 was not recorded; 102 was explicitly recorded as UNKNOWN.
 * Index 109 illuminates Next Track and the analog-output indicator together. */
static const char* const x1_key_names[] =
{
    "~", "1", "Tab", "Q", "Caps Lock", "A", "Left Shift", "Z",
    "2", "3", "W", "E", "S", "D", "X", "C",
    "4", "5", "R", "T", "F", "G", "V", "B",
    "6", "7", "Y", "U", "H", "J", "N", "M",
    "8", "9", "I", "O", "K", "L", "<", ">",
    "0", "-", "P", "{", ":", "Quote", "?", "Right Shift",
    "+", "Backspace", "}", "|", "Enter", "Right Ctrl",
    "Insert", "Home", "Delete", "End", "Menu",
    "Up", "Down", "Left", "Page Up", "Page Down",
    "F10", "F11", "F12", "Right",
    "Numpad 0", "Num Lock", "Numpad /", "Numpad 7", "Numpad 8",
    "Numpad 4", "Numpad 5", "Numpad 1", "Numpad 2", "Numpad *", "Numpad -",
    "Numpad 9", "Numpad +", "Numpad 6", "Numpad Enter", "Numpad 3", "Numpad .",
    "Win", "Left Ctrl", "Left Alt", "Space", "Right Alt", "Fn",
    "Esc", "F1", "F2", "F3", "Print Screen", "Scroll Lock",
    "F4", "F5", "F6", "Pause Break", "Unrecorded LED 101", "Unknown LED 102",
    "F7", "F8", "F9", "Media Stop", "Media Play/Pause", "Previous Track",
    "Next Track / Analog Output"
};
static_assert(sizeof(x1_key_names) / sizeof(x1_key_names[0]) == THERMALTAKE_X1_LED_COUNT,
              "X1 key names must preserve all 110 LED indices");

RGBController_ThermaltakeX1RGB::RGBController_ThermaltakeX1RGB(ThermaltakeX1RGBController* controller_ptr)
{
    controller  = controller_ptr;
    name        = "Thermaltake TT Premium X1 RGB";
    vendor      = "Thermaltake";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Thermaltake TT Premium X1 RGB Keyboard";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name       = "Direct";
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Static;
    Static.name       = "Static";
    Static.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min = 1;
    Static.colors_max = 1;
    Static.colors.resize(1);
    Static.brightness_min = 0;
    Static.brightness_max = 100;
    Static.brightness = 100;
    modes.push_back(Static);

    SetupZones();
}

RGBController_ThermaltakeX1RGB::~RGBController_ThermaltakeX1RGB()
{
    Shutdown();
    delete controller;
}

void RGBController_ThermaltakeX1RGB::SetupZones()
{
    zone keyboard;
    keyboard.name       = "Keyboard";
    keyboard.type       = ZONE_TYPE_LINEAR;
    keyboard.leds_min   = THERMALTAKE_X1_LED_COUNT;
    keyboard.leds_max   = THERMALTAKE_X1_LED_COUNT;
    keyboard.leds_count = THERMALTAKE_X1_LED_COUNT;
    zones.push_back(keyboard);

    for(unsigned int i = 0; i < THERMALTAKE_X1_LED_COUNT; i++)
    {
        led new_led;
        new_led.name = x1_key_names[i];
        leds.push_back(new_led);
    }
    SetupColors();
}

void RGBController_ThermaltakeX1RGB::DeviceUpdateLEDs()
{
    if(active_mode == 0)
    {
        controller->SetLEDs(colors);
        return;
    }
    RGBColor color = modes[active_mode].colors[0];
    unsigned int brightness = modes[active_mode].brightness;
    color = ToRGBColor(RGBGetRValue(color) * brightness / 100,
                       RGBGetGValue(color) * brightness / 100,
                       RGBGetBValue(color) * brightness / 100);
    controller->SetStatic(color);
}

void RGBController_ThermaltakeX1RGB::DeviceUpdateZoneLEDs(int) { DeviceUpdateLEDs(); }
void RGBController_ThermaltakeX1RGB::DeviceUpdateSingleLED(int) { DeviceUpdateLEDs(); }
void RGBController_ThermaltakeX1RGB::DeviceUpdateMode() { DeviceUpdateLEDs(); }
