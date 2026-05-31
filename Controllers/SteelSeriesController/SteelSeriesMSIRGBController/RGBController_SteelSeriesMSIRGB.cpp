/*---------------------------------------------------------*\
| RGBController_SteelSeriesMSIRGB.cpp                       |
|                                                           |
|   RGBController for MSI laptop SteelSeries RGB devices    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_SteelSeriesMSIRGB.h"

#define NA                                      0xFFFFFFFF
#define STEELSERIES_MSI_RGB_ARRAY_SIZE(x)      (sizeof(x) / sizeof((x)[0]))
#define STEELSERIES_MSI_RGB_KLC_MATRIX_HEIGHT  6
#define STEELSERIES_MSI_RGB_KLC_MATRIX_WIDTH   23
#define STEELSERIES_MSI_RGB_ALC_LIGHTBAR_LEDS  5

typedef struct
{
    const char*     name;
    unsigned char   id;

} steelseries_msi_rgb_led;

static const steelseries_msi_rgb_led klc_leds[] =
{
    { KEY_EN_A,                 0x04 },
    { KEY_EN_B,                 0x05 },
    { KEY_EN_C,                 0x06 },
    { KEY_EN_D,                 0x07 },
    { KEY_EN_E,                 0x08 },
    { KEY_EN_F,                 0x09 },
    { KEY_EN_G,                 0x0A },
    { KEY_EN_H,                 0x0B },
    { KEY_EN_I,                 0x0C },
    { KEY_EN_J,                 0x0D },
    { KEY_EN_K,                 0x0E },
    { KEY_EN_L,                 0x0F },
    { KEY_EN_M,                 0x10 },
    { KEY_EN_N,                 0x11 },
    { KEY_EN_O,                 0x12 },
    { KEY_EN_P,                 0x13 },
    { KEY_EN_Q,                 0x14 },
    { KEY_EN_R,                 0x15 },
    { KEY_EN_S,                 0x16 },
    { KEY_EN_T,                 0x17 },
    { KEY_EN_U,                 0x18 },
    { KEY_EN_V,                 0x19 },
    { KEY_EN_W,                 0x1A },
    { KEY_EN_X,                 0x1B },
    { KEY_EN_Y,                 0x1C },
    { KEY_EN_Z,                 0x1D },
    { KEY_EN_1,                 0x1E },
    { KEY_EN_2,                 0x1F },
    { KEY_EN_3,                 0x20 },
    { KEY_EN_4,                 0x21 },
    { KEY_EN_5,                 0x22 },
    { KEY_EN_6,                 0x23 },
    { KEY_EN_7,                 0x24 },
    { KEY_EN_8,                 0x25 },
    { KEY_EN_9,                 0x26 },
    { KEY_EN_0,                 0x27 },
    { KEY_EN_ESCAPE,            0x29 },
    { KEY_EN_TAB,               0x2B },
    { KEY_EN_SPACE,             0x2C },
    { KEY_EN_MINUS,             0x2D },
    { KEY_EN_EQUALS,            0x2E },
    { KEY_EN_LEFT_BRACKET,      0x2F },
    { KEY_EN_RIGHT_BRACKET,     0x30 },
    { KEY_EN_SEMICOLON,         0x33 },
    { KEY_EN_QUOTE,             0x34 },
    { KEY_EN_BACK_TICK,         0x35 },
    { KEY_EN_COMMA,             0x36 },
    { KEY_EN_PERIOD,            0x37 },
    { KEY_EN_FORWARD_SLASH,     0x38 },
    { KEY_EN_CAPS_LOCK,         0x39 },
    { KEY_EN_F1,                0x3A },
    { KEY_EN_F2,                0x3B },
    { KEY_EN_F3,                0x3C },
    { KEY_EN_F4,                0x3D },
    { KEY_EN_F5,                0x3E },
    { KEY_EN_F6,                0x3F },
    { KEY_EN_F7,                0x40 },
    { KEY_EN_F8,                0x41 },
    { KEY_EN_F9,                0x42 },
    { KEY_EN_F10,               0x43 },
    { KEY_EN_F11,               0x44 },
    { KEY_EN_F12,               0x45 },
    { KEY_EN_PRINT_SCREEN,      0x46 },
    { KEY_EN_SCROLL_LOCK,       0x47 },
    { KEY_EN_INSERT,            0x49 },
    { "Home/Page Up",           0x4B },
    { KEY_EN_DELETE,            0x4C },
    { KEY_EN_PAGE_DOWN,         0x4E },
    { KEY_EN_RIGHT_ARROW,       0x4F },
    { KEY_EN_LEFT_ARROW,        0x50 },
    { KEY_EN_DOWN_ARROW,        0x51 },
    { KEY_EN_UP_ARROW,          0x52 },
    { KEY_EN_NUMPAD_LOCK,       0x53 },
    { KEY_EN_NUMPAD_DIVIDE,     0x54 },
    { KEY_EN_NUMPAD_TIMES,      0x55 },
    { KEY_EN_NUMPAD_MINUS,      0x56 },
    { KEY_EN_NUMPAD_PLUS,       0x57 },
    { KEY_EN_NUMPAD_ENTER,      0x58 },
    { KEY_EN_NUMPAD_1,          0x59 },
    { KEY_EN_NUMPAD_2,          0x5A },
    { KEY_EN_NUMPAD_3,          0x5B },
    { KEY_EN_NUMPAD_4,          0x5C },
    { KEY_EN_NUMPAD_5,          0x5D },
    { KEY_EN_NUMPAD_6,          0x5E },
    { KEY_EN_NUMPAD_7,          0x5F },
    { KEY_EN_NUMPAD_8,          0x60 },
    { KEY_EN_NUMPAD_9,          0x61 },
    { KEY_EN_NUMPAD_0,          0x62 },
    { KEY_EN_NUMPAD_PERIOD,     0x63 },
    { KEY_EN_POWER,             0x66 },
    { KEY_EN_LEFT_CONTROL,      0xE0 },
    { KEY_EN_LEFT_SHIFT,        0xE1 },
    { KEY_EN_LEFT_ALT,          0xE2 },
    { KEY_EN_LEFT_WINDOWS,      0xE3 },
    { KEY_EN_RIGHT_WINDOWS,     0xE4 },
    { KEY_EN_RIGHT_FUNCTION,    0xF0 },
    { KEY_EN_ANSI_ENTER,        0x28 },
    { KEY_EN_BACKSPACE,         0x2A },
    { KEY_EN_BACK_SLASH,        0x31 },
    { KEY_EN_ISO_BACK_SLASH,    0x64 },
    { KEY_EN_RIGHT_SHIFT,       0xE5 },
    { KEY_EN_RIGHT_ALT,         0xE6 },
};

static unsigned int klc_matrix_map[STEELSERIES_MSI_RGB_KLC_MATRIX_HEIGHT][STEELSERIES_MSI_RGB_KLC_MATRIX_WIDTH] =
{
    { 36, NA, 50, 51, 52, 53, NA, 54, 55, 56, 57, NA, 58, 59, 60, 61, 62, 63, NA, NA, NA, NA, 89 },
    { 45, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 39, 40, 97, NA, 64, 65, NA, 72, 73, 74, 75, NA },
    { 37, 16, 22,  4, 17, 19, 24, 20,  8, 14, 15, 41, 42, 98, NA, 66, 67, NA, 84, 85, 86, 76, NA },
    { 49,  0, 18,  3,  5,  6,  7,  9, 10, 11, 43, 44, 96, NA, NA, NA, NA, NA, 81, 82, 83, NA, NA },
    { 91, 25, 23,  2, 21,  1, 13, 12, 46, 47, 48,100, NA, NA, NA, NA, 71, NA, 78, 79, 80, 77, NA },
    { 90, 93, 92, NA, NA, NA, 38, NA, NA, NA,101, 94, 95, NA, 69, 70, 68, NA, 87, NA, 88, NA, NA },
};

static const steelseries_msi_rgb_led alc_leds[] =
{
    { "Lightbar 1", 0x00 },
    { "Lightbar 2", 0x01 },
    { "Lightbar 3", 0x02 },
    { "Lightbar 4", 0x03 },
    { "Lightbar 5", 0x04 },
    { "Logo",       0x05 },
};

/**------------------------------------------------------------------*\
    @name MSI Raider A18 HX A9WJG SteelSeries RGB
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectSteelSeriesMSIRGBKeyboard, DetectSteelSeriesMSIRGBLightbar
    @comment
\*-------------------------------------------------------------------*/

RGBController_SteelSeriesMSIRGB::RGBController_SteelSeriesMSIRGB(SteelSeriesMSIRGBController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetDeviceName();
    vendor              = "SteelSeries";
    description         = "MSI Raider A18 HX A9WJG SteelSeries RGB Device";
    location            = controller->GetDeviceLocation();
    serial              = controller->GetSerialString();
    type                = (controller->GetDeviceType() == STEELSERIES_MSI_RGB_KLC) ? DEVICE_TYPE_KEYBOARD : DEVICE_TYPE_LEDSTRIP;

    mode Direct;
    Direct.name         = "Direct";
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_SteelSeriesMSIRGB::~RGBController_SteelSeriesMSIRGB()
{
    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].matrix_map != NULL)
        {
            delete zones[zone_idx].matrix_map;
        }
    }

    delete controller;
}

void RGBController_SteelSeriesMSIRGB::SetupZones()
{
    if(controller->GetDeviceType() == STEELSERIES_MSI_RGB_KLC)
    {
        zone keyboard_zone;

        keyboard_zone.name                   = ZONE_EN_KEYBOARD;
        keyboard_zone.type                   = ZONE_TYPE_MATRIX;
        keyboard_zone.leds_min               = STEELSERIES_MSI_RGB_ARRAY_SIZE(klc_leds);
        keyboard_zone.leds_max               = STEELSERIES_MSI_RGB_ARRAY_SIZE(klc_leds);
        keyboard_zone.leds_count             = STEELSERIES_MSI_RGB_ARRAY_SIZE(klc_leds);
        keyboard_zone.matrix_map             = new matrix_map_type;
        keyboard_zone.matrix_map->height     = STEELSERIES_MSI_RGB_KLC_MATRIX_HEIGHT;
        keyboard_zone.matrix_map->width      = STEELSERIES_MSI_RGB_KLC_MATRIX_WIDTH;
        keyboard_zone.matrix_map->map        = (unsigned int *)&klc_matrix_map;

        zones.push_back(keyboard_zone);

        for(unsigned int led_idx = 0; led_idx < STEELSERIES_MSI_RGB_ARRAY_SIZE(klc_leds); led_idx++)
        {
            led new_led;
            new_led.name  = klc_leds[led_idx].name;
            new_led.value = klc_leds[led_idx].id;
            leds.push_back(new_led);
        }
    }
    else
    {
        zone lightbar_zone;
        lightbar_zone.name       = "Lightbar";
        lightbar_zone.type       = ZONE_TYPE_LINEAR;
        lightbar_zone.start_idx  = 0;
        lightbar_zone.leds_min   = STEELSERIES_MSI_RGB_ALC_LIGHTBAR_LEDS;
        lightbar_zone.leds_max   = STEELSERIES_MSI_RGB_ALC_LIGHTBAR_LEDS;
        lightbar_zone.leds_count = STEELSERIES_MSI_RGB_ALC_LIGHTBAR_LEDS;
        lightbar_zone.matrix_map = NULL;
        zones.push_back(lightbar_zone);

        zone logo_zone;
        logo_zone.name           = "Logo";
        logo_zone.type           = ZONE_TYPE_SINGLE;
        logo_zone.start_idx      = STEELSERIES_MSI_RGB_ALC_LIGHTBAR_LEDS;
        logo_zone.leds_min       = 1;
        logo_zone.leds_max       = 1;
        logo_zone.leds_count     = 1;
        logo_zone.matrix_map     = NULL;
        zones.push_back(logo_zone);

        for(unsigned int led_idx = 0; led_idx < STEELSERIES_MSI_RGB_ARRAY_SIZE(alc_leds); led_idx++)
        {
            led new_led;
            new_led.name  = alc_leds[led_idx].name;
            new_led.value = alc_leds[led_idx].id;
            leds.push_back(new_led);
        }
    }

    SetupColors();
}

void RGBController_SteelSeriesMSIRGB::ResizeZone(int /*zone*/, int /*new_size*/)
{
}

void RGBController_SteelSeriesMSIRGB::DeviceUpdateLEDs()
{
    controller->SetLEDs(leds, colors);
}

void RGBController_SteelSeriesMSIRGB::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesMSIRGB::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesMSIRGB::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}
