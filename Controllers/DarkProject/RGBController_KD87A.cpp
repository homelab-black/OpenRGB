#include "RGBController_KD87A.h"
#include <string>

RGBController_KD87A::RGBController_KD87A(DarkProjectKD87AController* controller_ptr)
{
    controller = controller_ptr;

    name = "Dark Project KD87A Skeleton";
    vendor = "Dark Project";
    description = "Dark Project ONE KD87A Keyboard";
    type = DEVICE_TYPE_KEYBOARD;
    version = "v1";

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = 0x01;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    Direct.speed_min            = 0;
    Direct.speed_max            = 0;
    Direct.brightness_min       = 0;
    Direct.brightness_max       = 0;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_KD87A::~RGBController_KD87A()
{
    delete controller;
}

void RGBController_KD87A::SetupZones()
{
    // Одна зона для всей клавиатуры с 87 LED
    zone new_zone;
    new_zone.name = "Keyboard";
    new_zone.type = ZONE_TYPE_SINGLE;
    new_zone.leds_min = 87;
    new_zone.leds_max = 87;
    new_zone.leds_count = 87;
    zones.push_back(new_zone);

    // Создаём LED для каждой клавиши
    for (int i = 0; i < 87; i++)
    {
        led new_led;
        new_led.name = "Key " + std::to_string(i);
        new_led.value = i;
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_KD87A::ResizeZone(int zone, int new_size)
{
    (void)zone;
    (void)new_size;
}

void RGBController_KD87A::DeviceUpdateLEDs()
{
    // Пока ничего не делаем
}

void RGBController_KD87A::UpdateZoneLEDs(int zone)
{
    if (zone >= (int)zones.size())
    {
        return;
    }

    // Собираем все цвета из зоны
    std::vector<RGBColor> zone_colors;
    for (size_t i = 0; i < zones[zone].leds_count; i++)
    {
        zone_colors.push_back(zones[zone].colors[i]);
    }

    controller->SetLedsDirect(zone_colors);
}

void RGBController_KD87A::UpdateSingleLED(int led)
{
    if (led < 0 || led >= (int)colors.size())
    {
        return;
    }

    // Отправляем только один LED
    std::vector<RGBColor> single_color;
    single_color.push_back(colors[led]);
    
    controller->SetLedsDirect(single_color);
}
