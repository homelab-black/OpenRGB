#include "RGBController_AlienwareMonitor.h"

RGBController_AlienwareMonitor::RGBController_AlienwareMonitor(AlienwareMonitorController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = "Alienware Monitor";
    description             = "Alienware Monitor";
    vendor                  = "Alienware";
    type                    = DEVICE_TYPE_ACCESSORY;
    location                = controller->GetLocation();
    serial                  = controller->GetSerialString();

    mode Direct;
    Direct.name             = "Direct";
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode             = 0;

    SetupZones();
}

RGBController_AlienwareMonitor::~RGBController_AlienwareMonitor()
{

}

void RGBController_AlienwareMonitor::SetupZones()
{
    zone Logo;
    Logo.name               = "Logo";
    Logo.type               = ZONE_TYPE_SINGLE;
    Logo.leds_min           = 1;
    Logo.leds_max           = 1;
    Logo.leds_count         = 1;
    Logo.matrix_map         = NULL;
    zones.push_back(Logo);

    led Logo_LED;
    Logo_LED.name           = "Logo";
    Logo_LED.value          = 0x01;
    leds.push_back(Logo_LED);

    zone Number;
    Number.name             = "Number";
    Number.type             = ZONE_TYPE_SINGLE;
    Number.leds_min         = 1;
    Number.leds_max         = 1;
    Number.leds_count       = 1;
    Number.matrix_map       = NULL;
    zones.push_back(Number);

    led Number_LED;
    Number_LED.name         = "Number";
    Number_LED.value        = 0x02;
    leds.push_back(Number_LED);

    zone PowerButton;
    PowerButton.name        = "Power Button";
    PowerButton.type        = ZONE_TYPE_SINGLE;
    PowerButton.leds_min    = 1;
    PowerButton.leds_max    = 1;
    PowerButton.leds_count  = 1;
    PowerButton.matrix_map  = NULL;
    zones.push_back(PowerButton);

    led PowerButton_LED;
    PowerButton_LED.name    = "Power Button";
    PowerButton_LED.value   = 0x08;
    leds.push_back(PowerButton_LED);

    SetupColors();
}

void RGBController_AlienwareMonitor::ResizeZone(int /*zone*/, int /*new_size*/)
{

}

void RGBController_AlienwareMonitor::DeviceUpdateLEDs()
{
    for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        UpdateSingleLED(led_idx);
    }
}

void RGBController_AlienwareMonitor::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AlienwareMonitor::UpdateSingleLED(int led)
{
    unsigned char red = RGBGetRValue(colors[led]);
    unsigned char grn = RGBGetGValue(colors[led]);
    unsigned char blu = RGBGetBValue(colors[led]);
    controller->SendColor(leds[led].value, red, grn, blu);
}

void RGBController_AlienwareMonitor::DeviceUpdateMode()
{

}
