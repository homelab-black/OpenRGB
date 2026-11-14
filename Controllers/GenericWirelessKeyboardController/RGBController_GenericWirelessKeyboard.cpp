#include "RGBController_GenericWirelessKeyboard.h"

using namespace std::chrono_literals;

RGBController_GenericWirelessKeyboard::RGBController_GenericWirelessKeyboard(GenericWirelessKeyboardController *controller)
    : controller(controller)
{
    name = controller->GetName();
    type = DEVICE_TYPE_KEYBOARD;
    description = "Generic Wireless Keyboard";
    location = controller->GetLocation();

    mode Direct;
    Direct.name = "Direct";
    Direct.flags = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode = 0;

    SetupZones();

    /*------------------------------------------------------*\
    | The Generic Wireless Keyboard requires a steady stream |
    | of packets in order to not revert out of direct mode.  |
    | Start a thread to continuously refresh the device      |
    \*------------------------------------------------------*/
    keepalive_thread_run = true;
    keepalive_thread = new std::thread(&RGBController_GenericWirelessKeyboard::KeepaliveThreadFunction, this);
}

RGBController_GenericWirelessKeyboard::~RGBController_GenericWirelessKeyboard()
{
    keepalive_thread_run = false;
    keepalive_thread->join();
    delete keepalive_thread;
}

void RGBController_GenericWirelessKeyboard::SetupZones()
{
    zone Keyboard;
    Keyboard.name = "Keyboard";
    Keyboard.type = ZONE_TYPE_SINGLE;
    Keyboard.leds_min = 1;
    Keyboard.leds_max = 1;
    Keyboard.leds_count = 1;
    Keyboard.matrix_map = NULL;
    zones.push_back(Keyboard);

    led Keyboard_LED;
    Keyboard_LED.name = "Keyboard";
    Keyboard_LED.value = 0x01;
    leds.push_back(Keyboard_LED);

    SetupColors();
}

void RGBController_GenericWirelessKeyboard::UpdateLEDs()
{
    controller->SendColors(colors);
}

void RGBController_GenericWirelessKeyboard::SetCustomMode()
{
}

void RGBController_GenericWirelessKeyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
}

void RGBController_GenericWirelessKeyboard::DeviceUpdateLEDs()
{
    UpdateLEDs();
}

void RGBController_GenericWirelessKeyboard::UpdateZoneLEDs(int /*zone*/)
{
    UpdateLEDs();
}

void RGBController_GenericWirelessKeyboard::UpdateSingleLED(int /*led*/)
{
    UpdateLEDs();
}

void RGBController_GenericWirelessKeyboard::DeviceUpdateMode()
{
}

void RGBController_GenericWirelessKeyboard::KeepaliveThreadFunction()
{
    while (keepalive_thread_run)
    {
        if (active_mode == 0)
        {
            if ((std::chrono::steady_clock::now() - last_update_time) > std::chrono::milliseconds(500))
            {
                UpdateLEDs();
            }
        }
        std::this_thread::sleep_for(100ms);
    }
}
