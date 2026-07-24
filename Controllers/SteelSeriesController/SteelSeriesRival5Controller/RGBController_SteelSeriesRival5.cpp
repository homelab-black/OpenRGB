#include "RGBController_SteelSeriesRival5.h"

/*RGBController_SteelSeriesRival5::RGBController_SteelSeriesRival5(SteelSeriesMouseController* controller_ptr)
{
    controller = controller_ptr;
}
*/

RGBController_SteelSeriesRival5::RGBController_SteelSeriesRival5(SteelSeriesMouseController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetNameString();
    vendor                  = "SteelSeries";
    type                    = DEVICE_TYPE_MOUSE;
    description             = "SteelSeries Mouse Device";
    location                = controller->GetDeviceLocation();
    serial                  = controller->GetSerialString();
    version                 = controller->GetFirmwareVersion();

    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = STEELSERIES_MOUSE_EFFECT_DIRECT;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    Direct.brightness_min   = 0x00;
    Direct.brightness_max   = STEELSERIES_MOUSE_BRIGHTNESS_MAX;
    Direct.brightness       = STEELSERIES_MOUSE_BRIGHTNESS_MAX;


    steelseries_mouse mouse = controller->GetMouse();

    for(const uint8_t i: mouse.modes)
    {
        switch(i)
        {
            case STEELSERIES_MOUSE_EFFECT_DIRECT:
                modes.push_back(Direct);
                break;
        }
    }

    SetupZones();
}

RGBController_SteelSeriesRival5::~RGBController_SteelSeriesRival5()
{
    Shutdown();

    delete controller;
}


void RGBController_SteelSeriesRival5::SetupZones()
{
    steelseries_mouse mouse = controller->GetMouse();

    zone lighting_zone;

    lighting_zone.name       = "Whole Mouse";
    lighting_zone.type       = ZONE_TYPE_LINEAR;
    lighting_zone.leds_min   = static_cast<unsigned int>(mouse.leds.size());
    lighting_zone.leds_max   = static_cast<unsigned int>(mouse.leds.size());
    lighting_zone.leds_count = static_cast<unsigned int>(mouse.leds.size());

    zones.push_back(lighting_zone);

    for(const led_info& info : mouse.leds)
    {
        led mouse_led;

        mouse_led.name  = info.name;
        mouse_led.value = info.value;

        leds.push_back(mouse_led);
    }

    SetupColors();
}

void RGBController_SteelSeriesRival5::DeviceUpdateLEDs()
{
    printf("DeviceUpdateLEDs()\n");

    static_cast<SteelSeriesRival5Controller*>(controller)
        ->UpdateLEDs(colors);
}

void RGBController_SteelSeriesRival5::DeviceUpdateZoneLEDs(int)
{
    printf("DeviceUpdateZoneLEDs()\n");

    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesRival5::DeviceUpdateSingleLED(int)
{
    printf("DeviceUpdateSingleLED()\n");

    DeviceUpdateLEDs();
}
void RGBController_SteelSeriesRival5::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesRival5::DeviceSaveMode()
{
}