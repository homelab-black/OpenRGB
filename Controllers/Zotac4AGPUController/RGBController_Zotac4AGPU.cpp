/*---------------------------------------------------------*\
| RGBController_Zotac4AGPU.cpp                              |
|                                                           |
|   RGBController for Zotac 4A GPU                          |
|                                                           |
|   Peter Berendi                               29 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_Zotac4AGPU.h"

/**------------------------------------------------------------------*\
    @name ZOTAC 30/40 series GPU
    @category GPU
    @type I2C
    @save :robot:
    @direct :x:
    @effects :tools:
    @detectors DetectZotacV2GPUControllers
    @comment

        This controller saves everything in flash, so only built-in effects are supported.

        Controller version is identified by polling the controller on address
        0x4A at register 0x5A. The first ten bytes read from that address are the version.

        The polling and reading is done by the ZOTAC Firestorm app on startup
        and can be monitored using NvAPIspy on Windows. The Firestorm app is 32 bit (as of Jan 2025)
        so you'd need the 32 bit NvAPISpy DLL installed in %WINDIR%\SysWOW64

        For RTX 4070 Ti Super Trinity OC the relevant NvAPISpy log entries are:

        > ```
        > NvAPI_I2CRead from device 0x4A
        > handle               0x00000100
        > version         [00] 0x0003002c
        > display_mask    [04] 0x00000001
        > is_ddc_port     [08] 0x01
        > i2c_dev_address [09] 0x94
        > i2c_reg_address [12] 0x00CF69EC: 5A
        > reg_addr_size   [16] 0x00000001
        > data            [20] 0x00CFFE10: { 0x4E, 0x37, 0x30, 0x32, 0x45, 0x2D, 0x31, 0x30, 0x30, 0x32, 0x01, 0x00, 0x00, 0xFF, 0x00, 0x2D, 0x0A, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
        > size            [24] 0x00000020
        > i2c_speed       [28] 0x0000ffff
        > i2c_speed_khz   [32] 0x00000002
        > port_id         [36] 0x00000001
        > is_port_id_set  [40] 0x00000001
        > NvAPI_I2CRead returned 0
        > ```

        The read data `0x4E, 0x37, 0x30, 0x32, 0x45, 0x2D, 0x31, 0x30, 0x30, 0x32` converts
        to Unicode `N702E-1002`

        If the device address is 0x49 instead of 0x4A then you'd need the ZotacV2GPUController driver instead.

        Cards with correct entries in `pci_ids/pci_ids.h` and `Controllers/Zotac4AGPUController/Zotac4AGPUControllerDetect.cpp`
        but with a different controller ID should produce a log entry with the controller version read from the card.

\*-------------------------------------------------------------------*/

RGBController_Zotac4AGPU::RGBController_Zotac4AGPU(Zotac4AGPUController* controller_ptr)
{
    controller  = controller_ptr;
    lastupdatetime = std::chrono::steady_clock::now();

    name        = "ZOTAC GPU";
    vendor      = "ZOTAC";
    description = "ZOTAC 40/50 series RGB GPU Device (" + controller->GetVersion() + ") protocol 4A";
    location    = controller->GetDeviceLocation();
    type        = DEVICE_TYPE_GPU;

    mode STATIC;
    STATIC.name       = "Static";
    STATIC.value      = ZOTAC_4A_GPU_MODE_STATIC;
    STATIC.flags      = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    STATIC.color_mode = MODE_COLORS_MODE_SPECIFIC;
    STATIC.colors_min = 1;
    STATIC.colors_max = 1;
    STATIC.colors.resize(1);
    STATIC.colors[0] = ToRGBColor(0, 0, 255);
    modes.push_back(STATIC);

    mode BREATHE;
    BREATHE.name       = "Breathing";
    BREATHE.value      = ZOTAC_4A_GPU_MODE_BREATHE;
    BREATHE.flags      = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED;
    BREATHE.speed_min  = 0;
    BREATHE.speed_max  = ZOTAC_4A_RGB_MAXSPEED;
    BREATHE.speed      = ZOTAC_4A_RGB_MAXSPEED - 20;
    BREATHE.color_mode = MODE_COLORS_MODE_SPECIFIC;
    BREATHE.colors_min = 1;
    BREATHE.colors_max = 1;
    BREATHE.colors.resize(1);
    BREATHE.colors[0]  = ToRGBColor(0, 0, 255);
    modes.push_back(BREATHE);

    mode FADE;
    FADE.name          = "Spectrum Cycle";
    FADE.value         = ZOTAC_4A_GPU_MODE_FADE;
    FADE.flags         = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED;
    FADE.speed_min     = 0;
    FADE.speed_max     = ZOTAC_4A_RGB_MAXSPEED;
    FADE.speed         = ZOTAC_4A_RGB_MAXSPEED - 20;
    FADE.color_mode    = MODE_COLORS_NONE;
    modes.push_back(FADE);

    mode WINK;
    WINK.name            = "Flashing";
    WINK.value           = ZOTAC_4A_GPU_MODE_WINK;
    WINK.flags           = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    WINK.speed_min       = 0;
    WINK.speed_max       = ZOTAC_4A_RGB_MAXSPEED;
    WINK.speed           = ZOTAC_4A_RGB_MAXSPEED - 10;
    WINK.brightness_min  = 0;
    WINK.brightness_max  = ZOTAC_4A_RGB_MAXSPEED;
    WINK.brightness      = ZOTAC_4A_RGB_MAXSPEED - 45;
    WINK.color_mode      = MODE_COLORS_MODE_SPECIFIC;
    WINK.colors_min      = 1;
    WINK.colors_max      = 1;
    WINK.colors.resize(1);
    WINK.colors[0]       = ToRGBColor(0, 0, 255);
    modes.push_back(WINK);

    mode SPECTRUMBREATHE;
    SPECTRUMBREATHE.name            = "Spectrum Breathing";
    SPECTRUMBREATHE.value           = ZOTAC_4A_GPU_MODE_SPECTRUMBREATHE;
    SPECTRUMBREATHE.flags           = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED;
    SPECTRUMBREATHE.speed_min       = 0;
    SPECTRUMBREATHE.speed_max       = ZOTAC_4A_RGB_MAXSPEED;
    SPECTRUMBREATHE.speed           = ZOTAC_4A_RGB_MAXSPEED - 20;
    SPECTRUMBREATHE.color_mode      = MODE_COLORS_NONE;
    modes.push_back(SPECTRUMBREATHE);

    mode RANDOM;
    RANDOM.name            = "Random Flashing";
    RANDOM.value           = ZOTAC_4A_GPU_MODE_RANDOM;
    RANDOM.flags           = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    RANDOM.speed_min       = 0;
    RANDOM.speed_max       = ZOTAC_4A_RGB_MAXSPEED;
    RANDOM.speed           = ZOTAC_4A_RGB_MAXSPEED - 10;
    RANDOM.brightness_min  = 0;
    RANDOM.brightness_max  = ZOTAC_4A_RGB_MAXSPEED;
    RANDOM.brightness      = ZOTAC_4A_RGB_MAXSPEED - 45;
    RANDOM.color_mode      = MODE_COLORS_NONE;
    modes.push_back(RANDOM);

    SetupZones();
}

RGBController_Zotac4AGPU::~RGBController_Zotac4AGPU()
{
    delete controller;
}

void RGBController_Zotac4AGPU::SetupZones()
{
    led new_led;
    new_led.name           = "GPU LED";
    leds.push_back(new_led);

    zone new_zone;
    new_zone.name          = "GPU Zone";
    new_zone.type          = ZONE_TYPE_SINGLE;
    new_zone.leds_min      = 1;
    new_zone.leds_max      = 1;
    new_zone.leds_count    = 1;
    zones.push_back(new_zone);

    SetupColors();
    SetupInitialValues();
}

void RGBController_Zotac4AGPU::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_Zotac4AGPU::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_Zotac4AGPU::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateMode();
}

void RGBController_Zotac4AGPU::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateMode();
}

void RGBController_Zotac4AGPU::DeviceUpdateMode()
{
    /*-----------------------------------------------------------------------*\
    | Sending multiple commands with less than 113 ms between them fails.     |
    | Enforce 200 ms delay for good measure.                                  |
    | Doing it before accessing modes[active_mode] reduces the chance of      |
    | missing an update from the UI.                                          |
    \*-----------------------------------------------------------------------*/
    std::this_thread::sleep_until(lastupdatetime + ZOTAC_4A_GPU_WAIT_BETWEEN_REQUESTS);

    Zotac4AGPUZone zoneConfig;
    zoneConfig.mode = modes[active_mode].value;
    zoneConfig.color = modes[active_mode].colors.size() >= 1 ? modes[active_mode].colors[0] : ToRGBColor(0, 0, 0);
    zoneConfig.speed = modes[active_mode].speed;
    zoneConfig.on_time = modes[active_mode].brightness;
    controller->SetMode(zoneConfig);

    lastupdatetime = std::chrono::steady_clock::now();
}

void RGBController_Zotac4AGPU::SetupInitialValues()
{
    /*---------------------------------------------------------*\
    | Retrieve current values by reading the device             |
    \*---------------------------------------------------------*/

    bool on;
    Zotac4AGPUZone zoneConfig;

    if(!controller->GetMode(zoneConfig, on))
    {
        return;
    }

    for(unsigned int i = 0; i < modes.size(); ++i)
    {
        if(zoneConfig.mode != modes[i].value)
        {
            continue;
        }
        active_mode = i;
    }

    colors[0] = zoneConfig.color;
    if(modes[active_mode].colors.size() >= 1)
    {
        modes[active_mode].colors[0] = zoneConfig.color;
    }

    modes[active_mode].speed = zoneConfig.speed;
    modes[active_mode].brightness = zoneConfig.on_time;

    SignalUpdate(RGBCONTROLLER_UPDATE_REASON_UPDATEMODE);
}
