/* Das Keyboard X50 RGBController - SPDX-License-Identifier: GPL-2.0-or-later */
#include "RGBController_DasKeyboardX50.h"

#define NA 0xFFFFFFFF

static unsigned int x50_matrix[6][23] =
{
    {34,NA,23,15,7,12,0,5,69,74,58,46,51,40,NA,43,76,81,NA,86,90,95,NA},
    {35,30,24,16,8,13,1,6,70,75,61,54,47,44,NA,45,82,83,79,85,94,89,NA},
    {36,31,25,17,9,14,2,71,62,67,55,48,52,41,NA,78,80,77,106,88,98,92,NA},
    {37,32,26,18,22,10,3,72,63,64,56,49,42,NA,NA,NA,NA,NA,103,87,96,92,NA},
    {38,27,29,21,19,11,4,73,65,66,57,53,NA,NA,NA,NA,99,NA,100,84,93,91,NA},
    {39,33,28,NA,NA,20,NA,NA,68,60,59,50,NA,NA,NA,104,101,102,105,NA,97,91,NA}
};

/**------------------------------------------------------------------*\
    @name Das Keyboard X50 RGB
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectDasKeyboardX50
    @comment
\*-------------------------------------------------------------------*/

RGBController_DasKeyboardX50::RGBController_DasKeyboardX50(DasKeyboardX50Controller* controller_ptr)
    : controller(controller_ptr)
{
    name = "Das Keyboard X50 RGB"; vendor = "Das Keyboard"; type = DEVICE_TYPE_KEYBOARD;
    description = "Das Keyboard X50 RGB Mechanical Keyboard";
    location = controller->GetLocationString(); serial = controller->GetSerialString();
    mode direct;
    direct.name           = "Direct";
    direct.value          = 0;
    direct.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    direct.color_mode     = MODE_COLORS_PER_LED;
    direct.brightness_min = 0;
    direct.brightness_max = 100;
    direct.brightness     = 100;
    modes.push_back(direct);
    SetupZones();
}

RGBController_DasKeyboardX50::~RGBController_DasKeyboardX50() { Shutdown(); delete controller; }

void RGBController_DasKeyboardX50::SetupZones()
{
    zone keyboard; keyboard.name = "Keyboard"; keyboard.type = ZONE_TYPE_MATRIX; keyboard.leds_count = 107; keyboard.leds_min = 107; keyboard.leds_max = 107; keyboard.matrix_map.Set(6,23,(unsigned int*)&x50_matrix); zones.push_back(keyboard);
    zone ring; ring.name = "Volume Ring"; ring.type = ZONE_TYPE_LINEAR; ring.leds_count = 4; ring.leds_min = 4; ring.leds_max = 4; zones.push_back(ring);
    const DasKeyboardX50Led* table = controller->LedTable();
    for(unsigned int i = 0; i < controller->LedCount(); i++) { led item; item.name = table[i].name; leds.push_back(item); }
    SetupColors();
}

void RGBController_DasKeyboardX50::DeviceUpdateLEDs()
{
    const unsigned int brightness = modes[active_mode].brightness;
    std::vector<RGBColor> scaled_colors;
    scaled_colors.reserve(colors.size());

    for(const RGBColor color : colors)
    {
        const unsigned char red   = static_cast<unsigned char>((RGBGetRValue(color) * brightness + 50) / 100);
        const unsigned char green = static_cast<unsigned char>((RGBGetGValue(color) * brightness + 50) / 100);
        const unsigned char blue  = static_cast<unsigned char>((RGBGetBValue(color) * brightness + 50) / 100);
        scaled_colors.push_back(ToRGBColor(red, green, blue));
    }

    controller->SendColors(scaled_colors);
}
void RGBController_DasKeyboardX50::DeviceUpdateZoneLEDs(int) { DeviceUpdateLEDs(); }
void RGBController_DasKeyboardX50::DeviceUpdateSingleLED(int) { DeviceUpdateLEDs(); }
void RGBController_DasKeyboardX50::DeviceUpdateMode() { }
