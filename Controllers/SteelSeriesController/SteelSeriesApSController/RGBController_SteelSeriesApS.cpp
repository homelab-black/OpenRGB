/*-----------------------------------------*\
|  RGBController_SteelSeriesApS.cpp         |
|                                           |
|  Generic RGB Interface for SteelSeries    |
|  ApS ALC LED Strip                        |
|                                           |
|  Morgan Guimard (morg) 1/25/2024          |
\*-----------------------------------------*/

#include "RGBController_SteelSeriesApS.h"
#include "SteelSeriesApSDevices.h"

/**------------------------------------------------------------------*\
    @name SteelSeries ApS
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :x:
    @detectors DetectSteelSeriesApS
    @comment This device equips most of the MSI GE laptops.
\*-------------------------------------------------------------------*/
RGBController_SteelSeriesApS::RGBController_SteelSeriesApS(SteelSeriesApSController* controller_ptr, unsigned short pid, std::string full_name)
{
    this->pid   = pid;

    controller  = controller_ptr;
    vendor      = "SteelSeries";    
    description = "SteelSeries ApS RGB Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();
    version     = controller->GetFirmwareVersion();
    name        = full_name;

    mode Custom;
    Custom.name       = "Custom";
    Custom.value      = 0x00;
    Custom.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Custom.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    RGBController_SteelSeriesApS::SetupZones();
}

RGBController_SteelSeriesApS::~RGBController_SteelSeriesApS()
{
    /*---------------------------------------------------------*\
    | Delete the matrix map                                     |
    \*---------------------------------------------------------*/
    /*
    if(type == DEVICE_TYPE_KEYBOARD)
    {

        for(unsigned int zone_index = 0; zone_index < zones.size(); zone_index++)
        {
            if(zones[zone_index].matrix_map != NULL)
            {
                free(zones[zone_index].matrix_map.map);
                delete zones[zone_index].matrix_map;
            }
        }
    }
    */
    delete controller;
}

void RGBController_SteelSeriesApS::SetupZones()
{
    /*---------------------------------------------------------*\
    | Check for KLC device                                      |
    \*---------------------------------------------------------*/
    if(steelseries_aps_klc_devices.count(pid))
    {

// TODO: remove
#ifdef STEELSERIES_APS_KLC_DUMMY_PID
        uint8_t layout = 0xFF;
#else
        uint8_t layout =  controller->GetLayout();
#endif
// -- TODO
        steelseries_aps_klc_device_layouts_map layouts_map = steelseries_aps_klc_devices.at(pid);

        if(layouts_map.count(layout))
        {
            SetupKLCZone(layouts_map.at(layout));
        }
        else
        {
            SetupALCZone();
        }
    }
    /*---------------------------------------------------------*\
    | Consider it is an ALC device                              |
    \*---------------------------------------------------------*/
    else
    {
        SetupALCZone();
    }

    SetupColors();
}

void RGBController_SteelSeriesApS::SetupALCZone()
{
    /*---------------------------------------------------------*\
    | ALC are considered as ledstrips                           |
    \*---------------------------------------------------------*/
    type                        = DEVICE_TYPE_LEDSTRIP;

    /*---------------------------------------------------------*\
    | Set up zone                                               |
    \*---------------------------------------------------------*/
    zone new_zone;
    new_zone.name               = "Led Strip";
    new_zone.type               = ZONE_TYPE_LINEAR;

    /*---------------------------------------------------------*\
    | Use configured sizes or fallback to unknown size (255)    |
    \*---------------------------------------------------------*/
    if(steelseries_aps_alc_devices.count(pid))
    {
        new_zone.leds_count = steelseries_aps_alc_devices.at(pid);
    }
    else
    {
        new_zone.leds_count = STEELSERIES_APS_ALC_UNKNOWN_SIZE;
    }

    /*---------------------------------------------------------*\
    | Fill in leds array                                        |
    \*---------------------------------------------------------*/
    for(unsigned int i = 0; i < new_zone.leds_count; i++)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    zones.push_back(new_zone);
}

void RGBController_SteelSeriesApS::SetupKLCZone(const steelseries_aps_klc_device& device)
{
    /*---------------------------------------------------------*\
    | KLC are considered as keyboards                           |
    \*---------------------------------------------------------*/
    type                        = DEVICE_TYPE_KEYBOARD;
    this->device                = device;

    /*---------------------------------------------------------*\
    | Set up zone                                               |
    \*---------------------------------------------------------*/
    zone new_zone;
    new_zone.name               = ZONE_EN_KEYBOARD;
    new_zone.type               = ZONE_TYPE_MATRIX;

    const keyboard_keymap_overlay_values& overlay_values = std::get<0>(device);
    const KEYBOARD_LAYOUT& keyboard_layout = std::get<1>(device);

    KeyboardLayoutManager new_kb(keyboard_layout, overlay_values.base_size);

    /*---------------------------------------------------------*\
    | Minor adjustments to keyboard layout                      |
    \*---------------------------------------------------------*/
    new_kb.ChangeKeys(overlay_values);

    new_zone.leds_count         = new_kb.GetKeyCount();
    new_zone.leds_min           = new_zone.leds_count;
    new_zone.leds_max           = new_zone.leds_count;

    new_zone.matrix_map         = new_kb.GetKeyMap(KEYBOARD_MAP_FILL_TYPE_COUNT, new_kb.GetRowCount(), new_kb.GetColumnCount());
    /*
    matrix_map_type * new_map   = new matrix_map_type;
    new_zone.matrix_map         = new_map;
    new_zone.matrix_map->height = new_kb.GetRowCount();
    new_zone.matrix_map->width  = new_kb.GetColumnCount();
    new_zone.matrix_map->map    = new unsigned int[new_map->height * new_map->width];
    new_kb.GetKeyMap(new_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT, new_map->height, new_map->width);
    */
    for(size_t led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;
        new_led.name                = new_kb.GetKeyNameAt(led_idx);
        new_led.value               = new_kb.GetKeyValueAt(led_idx);
        leds.push_back(new_led);
    }

    zones.push_back(new_zone);
}

void RGBController_SteelSeriesApS::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_SteelSeriesApS::DeviceUpdateLEDs()
{
    if(type == DEVICE_TYPE_KEYBOARD)
    {
        const keyboard_keymap_overlay_values& overlay_values = std::get<0>(device);
        controller->SetColors(colors, true, overlay_values.key_values.default_values);
    }
    else
    {
        controller->SetColors(colors);
    }
}

void RGBController_SteelSeriesApS::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesApS::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesApS::DeviceUpdateMode()
{

}
