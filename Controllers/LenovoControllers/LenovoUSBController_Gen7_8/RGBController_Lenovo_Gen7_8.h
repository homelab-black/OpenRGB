/*---------------------------------------------------------*\
| RGBController_Lenovo_Gen7_8.h                             |
|                                                           |
|   RGBController for Lenovo Gen7 and Gen8 devices          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <unordered_map>
#include <vector>
#include "RGBController.h"
#include "LenovoUSBController_Gen7_8.h"

class HIDLampArrayController;

enum
{
    LENOVO_LEGION_GEN7_8_MODE_SCREW_RAINBOW                  = 0x01,
    LENOVO_LEGION_GEN7_8_MODE_RAINBOW_WAVE                   = 0x02,
    LENOVO_LEGION_GEN7_8_MODE_COLOR_CHANGE                   = 0x03,
    LENOVO_LEGION_GEN7_8_MODE_COLOR_PULSE                    = 0x04,
    LENOVO_LEGION_GEN7_8_MODE_COLOR_WAVE                     = 0x05,
    LENOVO_LEGION_GEN7_8_MODE_SMOOTH                         = 0x06,
    LENOVO_LEGION_GEN7_8_MODE_RAIN                           = 0x07,
    LENOVO_LEGION_GEN7_8_MODE_RIPPLE                         = 0x08,
    LENOVO_LEGION_GEN7_8_MODE_AUDIO_BOUNCE                   = 0x09,
    LENOVO_LEGION_GEN7_8_MODE_AUDIO_RIPPLE                   = 0x0A,
    LENOVO_LEGION_GEN7_8_MODE_STATIC                         = 0x0B,
    LENOVO_LEGION_GEN7_8_MODE_TYPE                           = 0x0C,
    LENOVO_LEGION_GEN7_8_MODE_DIRECT                         = 0x0D,
};

class LenovoRGBController_Gen7_8 : public RGBController
{
public:
    LenovoRGBController_Gen7_8(LenovoGen7And8USBController* controller_ptr);
    ~LenovoRGBController_Gen7_8();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    struct legion9_key_lamp
    {
        unsigned short lamp_id;
        size_t         led_index;
    };

    LenovoGen7And8USBController* controller;
    HIDLampArrayController* light_strip;
    /* Spectrum address -> LampArray lamp ids owned by that square. */
    std::unordered_map<uint16_t, std::vector<unsigned short>> strip_groups;
    /* Keyboard lamps on the strip controller. led_index is npos when unmatched. */
    std::vector<legion9_key_lamp> keyboard_lamps;
    std::vector<RGBColor> frame_colors;
    std::vector<RGBColor> committed_key_colors;
    std::vector<uint8_t>  key_touched;
    std::vector<RGBColor> chassis_hold;
    std::vector<uint8_t>  chassis_held;
    bool use_frame_colors;
    bool preserve_key_lamps;
    /* GET_PROFILE captured before the first Direct save. Restored on exit while Direct is still active. */
    std::vector<uint8_t> direct_profile;
    bool direct_profile_restore;
    std::vector<led_group> GetLedGroups();
    void ReadDeviceSettings();
    void BuildStripGroups();
    void UpdateLightStrip();
    void PrepareFrameColors();
    void RememberColorEdits();
    bool HostPerLedColorMode() const;
    bool IsChassisLed(unsigned int led_value) const;
    bool LedColorEdited(size_t led_index) const;
    std::unordered_map<unsigned int, size_t> led_id_to_index;
    int last_mode = 0;
    bool direct_enabled = false;
    uint8_t brightness = 0x00;
    uint8_t profile_id = 0x01;
};
