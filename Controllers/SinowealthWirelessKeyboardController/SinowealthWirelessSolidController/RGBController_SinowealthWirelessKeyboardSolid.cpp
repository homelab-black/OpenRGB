#include "RGBController_SinowealthWirelessKeyboardSolid.h"
#include <LogManager.h>

using namespace std::chrono_literals;

RGBController_SinowealthWirelessKeyboardSolid::RGBController_SinowealthWirelessKeyboardSolid(
    SinowealthWirelessKeyboardController* controller)
    : RGBController_SinowealthWirelessKeyboard(controller), controller(controller)
{
    mode Direct;
    Direct.name       = "Direct";
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode = 0;

    SetupZones();
}

void RGBController_SinowealthWirelessKeyboardSolid::SetupZones()
{
    zone Keyboard;
    Keyboard.name       = "Keyboard";
    Keyboard.type       = ZONE_TYPE_SINGLE;
    Keyboard.leds_min   = 1;
    Keyboard.leds_max   = 1;
    Keyboard.leds_count = 1;
    Keyboard.matrix_map = NULL;
    zones.push_back(Keyboard);

    led Keyboard_LED;
    Keyboard_LED.name  = "Keyboard";
    Keyboard_LED.value = 0x01;
    leds.push_back(Keyboard_LED);

    SetupColors();
}

void RGBController_SinowealthWirelessKeyboardSolid::SendColor()
{
    controller->SendSolidColor(colors[0]);
}
