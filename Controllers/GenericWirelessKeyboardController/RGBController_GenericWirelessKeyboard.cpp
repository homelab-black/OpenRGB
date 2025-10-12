#include "RGBController_GenericWirelessKeyboard.h"

using namespace std::chrono_literals;

#define NA 0xFFFFFFFF
#define WIDTH 19
#define HEIGHT 6

static unsigned int matrix_map[HEIGHT][WIDTH] =
    {{0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 90, 96, 102, 108},
     {1, 7, 13, 19, 25, 31, 37, 43, 49, 55, 61, 67, 73, 79, 91, 97, 103, 109},
     {2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 92, 98, 104, 110},
     {3, 9, 15, 21, 27, 33, 39, 45, 51, 57, 63, 69, NA, 81, 93, 99, 105, NA},
     {4, 10, 16, 22, 28, 34, 40, 46, 52, 58, 64, NA, 70, 82, 88, 94, 100, 112},
     {5, 11, 17, NA, NA, 35, NA, NA, 53, 59, 65, NA, 83, 89, 95, 101, 107, NA}};

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

    // TODO: dynamically get layout
    zone Keyboard;
    Keyboard.name = "Keyboard";
    Keyboard.type = ZONE_TYPE_MATRIX;
    Keyboard.leds_min = 99;
    Keyboard.leds_max = 99;
    Keyboard.leds_count = 99;
    Keyboard.matrix_map = NULL;
    zones.push_back(Keyboard);

    for (size_t i = 0; i < 99; i++)
    {
        led Keyboard_LED;
        Keyboard_LED.name = "LED " + std::to_string(i);
        Keyboard_LED.value = 0x01;
        leds.push_back(Keyboard_LED);
    }

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
