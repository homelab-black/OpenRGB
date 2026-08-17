/*---------------------------------------------------------*\
| RGBController_DrevoKeyboard.cpp                           |
|                                                           |
|   RGBController for Drevo keyboards                       |
|                                                           |
|   Dennis Blokland                             02 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "KeyboardLayoutManager.h"
#include "RGBController_DrevoKeyboard.h"

/**------------------------------------------------------------------*\
    @name Drevo Tyrfing V2
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectDrevoKeyboard
    @comment
\*-------------------------------------------------------------------*/

/*---------------------------------------------------------*\
| The Tyrfing V2 addresses each key by its HID usage code.  |
|   The values below are listed in Keyboard Layout Manager  |
|   order, the two ISO only keys are placeholders which are |
|   removed again by the ANSI QWERTY layout.                |
\*---------------------------------------------------------*/
static const std::vector<unsigned int> drevo_tyrfing_v2_values =
{
    /* ESC          F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12   PRSC  SCLK  PSBK */
       0x29,       0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    /* BKTK    1     2     3     4     5     6     7     8     9     0     -     =   BSPC   INS  HOME  PGUP */
       0x35, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x2A, 0x49, 0x4A, 0x4B,
    /* TAB     Q     W     E     R     T     Y     U     I     O     P     [     ]     \    DEL   END  PGDN */
       0x2B, 0x14, 0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, 0x31, 0x4C, 0x4D, 0x4E,
    /* CPLK    A     S     D     F     G     H     J     K     L     ;     "     #  ENTR                    */
       0x39, 0x04, 0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34, 0x00, 0x28,
    /* LSFT  ISO\    Z     X     C     V     B     N     M     ,     .     /  RSFT  ARWU                    */
       0xE1, 0x00, 0x1D, 0x1B, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0xE5, 0x52,
    /* LCTL  LWIN  LALT               SPC              RALT  RFNC  RMNU  RCTL  ARWL  ARWD  ARWR            */
       0xE0, 0xE3, 0xE2,             0x2C,             0xE6, 0xED, 0x65, 0xE4, 0x50, 0x51, 0x4F,
};

static const layout_values drevo_tyrfing_v2_layout =
{
    drevo_tyrfing_v2_values,
    {
        /* Add regional layout fixes here */
    }
};

RGBController_DrevoKeyboard::RGBController_DrevoKeyboard(DrevoKeyboardController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "Drevo";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Drevo Keyboard Device";
    location    = controller->GetDeviceLocation();

    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = 0xFFFF;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_DrevoKeyboard::~RGBController_DrevoKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_DrevoKeyboard::SetupZones()
{
    /*---------------------------------------------------------*\
    | Create the keyboard zone using Keyboard Layout Manager    |
    \*---------------------------------------------------------*/
    KeyboardLayoutManager new_kb(KEYBOARD_LAYOUT_ANSI_QWERTY, KEYBOARD_SIZE_TKL, drevo_tyrfing_v2_layout);

    zone new_zone;

    new_zone.name               = ZONE_EN_KEYBOARD;
    new_zone.type               = ZONE_TYPE_MATRIX;
    new_zone.leds_count         = new_kb.GetKeyCount();
    new_zone.leds_min           = new_zone.leds_count;
    new_zone.leds_max           = new_zone.leds_count;
    new_zone.matrix_map         = new_kb.GetKeyMap(KEYBOARD_MAP_FILL_TYPE_COUNT);

    zones.push_back(new_zone);

    /*---------------------------------------------------------*\
    | Create LEDs for the Matrix zone                           |
    \*---------------------------------------------------------*/
    for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;

        new_led.name            = new_kb.GetKeyNameAt(led_idx);
        new_led.value           = new_kb.GetKeyValueAt(led_idx);

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_DrevoKeyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_DrevoKeyboard::DeviceUpdateLEDs()
{
    controller->SendDirect(colors, leds);
}

void RGBController_DrevoKeyboard::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_DrevoKeyboard::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_DrevoKeyboard::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}
