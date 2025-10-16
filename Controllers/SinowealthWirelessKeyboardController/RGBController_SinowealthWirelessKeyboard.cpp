#include "RGBController_SinowealthWirelessKeyboard.h"
#include <LogManager.h>

using namespace std::chrono_literals;

RGBController_SinowealthWirelessKeyboard::RGBController_SinowealthWirelessKeyboard(
    SinowealthWirelessKeyboardController* controller)
    : controller(controller)
{
    name        = controller->GetName();
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Sinowealth Wireless Keyboard";
    location    = controller->GetLocation();

    /*---------------------------------------------------------*\
    | The Sinowealth Wireless Keyboard requires a steady stream |
    | of packets in order to not revert out of direct mode.     |
    | Start a thread to continuously refresh the device         |
    \*---------------------------------------------------------*/
    keepalive_thread_run = true;
    keepalive_thread     = new std::thread(&RGBController_SinowealthWirelessKeyboard::KeepaliveThreadFunction, this);
}

RGBController_SinowealthWirelessKeyboard::~RGBController_SinowealthWirelessKeyboard()
{
    keepalive_thread_run = false;
    keepalive_thread->join();
    delete keepalive_thread;
}

void RGBController_SinowealthWirelessKeyboard::UpdateLEDs()
{
}

void RGBController_SinowealthWirelessKeyboard::SetCustomMode()
{
}

void RGBController_SinowealthWirelessKeyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
}

void RGBController_SinowealthWirelessKeyboard::DeviceUpdateLEDs()
{
    UpdateLEDs();
}

void RGBController_SinowealthWirelessKeyboard::UpdateZoneLEDs(int /*zone*/)
{
    UpdateLEDs();
}

void RGBController_SinowealthWirelessKeyboard::UpdateSingleLED(int /*led*/)
{
    UpdateLEDs();
}

void RGBController_SinowealthWirelessKeyboard::DeviceUpdateMode()
{
}

void RGBController_SinowealthWirelessKeyboard::KeepaliveThreadFunction()
{
    while(keepalive_thread_run)
    {
        if(active_mode == 0)
        {
            if((std::chrono::steady_clock::now() - last_update_time) > (color_changed ? 5ms : 100ms))
            {
                SendColor();

                last_update_time = std::chrono::steady_clock::now();
                color_changed    = false;
            }
        }
        std::this_thread::sleep_for(1ms);
    }
}
