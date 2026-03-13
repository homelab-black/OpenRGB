/*---------------------------------------------------------*\
| RGBController_RealtekARGB.cpp                             |
|                                                           |
|   RGBController for Realtek USB ARGB ICs                  |
|                                                           |
|   Jerry Fan (JerryFan0612)                    13 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#ifndef __FreeBSD__

#include "RGBController_RealtekARGB.h"

/**------------------------------------------------------------------*\
    @name Realtek ARGB Device
    @category LEDStrip
    @type USB
    @save :x:
    @direct :rotating_light:
    @effects :white_check_mark:
    @detectors RealtekARGBControllerDetect
    @comment
\*-------------------------------------------------------------------*/

RGBController_RealtekARGB::RGBController_RealtekARGB(RealtekARGBController* controller_ptr)
{
    controller  = controller_ptr;
    name        = controller_ptr->get_dev_name();
    vendor      = controller_ptr->get_manu_name();
    location    = controller_ptr->get_dev_loc();
    serial      = controller_ptr->get_sn();
    version     = controller_ptr->get_fw_ver();
    description = vendor + "ARGB Device";
    type        = DEVICE_TYPE_LEDSTRIP;

    SetupStrip();
    SetupZones();
}

void RGBController_RealtekARGB::SetupStrip()
{
    int brightness = controller->get_argb_brightness(0) >> 8;

    mode Direct;
    Direct.name                   = "Direct";
    Direct.value                  = REALTEK_ARGB_EFF_NULL;
    Direct.flags                  = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode             = MODE_COLORS_PER_LED;
    Direct.brightness_min         = 0;
    Direct.brightness_max         = 255;
    Direct.brightness             = brightness;
    modes.push_back(Direct);

    mode Static;
    Static.name                   = "Static";
    Static.value                  = REALTEK_ARGB_EFF_ALWAYS_ON;
    Static.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.colors_min             = 1;
    Static.colors_max             = 1;
    Static.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Static.colors.resize(1);
    Static.brightness_min         = 0;
    Static.brightness_max         = 255;
    Static.brightness             = brightness;
    modes.push_back(Static);

    mode Blink;
    Blink.name                    = "Blink";
    Blink.value                   = REALTEK_ARGB_EFF_BLINK;
    Blink.flags                   = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Blink.speed_min               = REALTEK_ARGB_SPEED_MIN;
    Blink.speed_max               = REALTEK_ARGB_SPEED_MAX;
    Blink.speed                   = REALTEK_ARGB_SPEED_NORMAL;
    Blink.colors_min              = 1;
    Blink.colors_max              = 2;
    Blink.color_mode              = MODE_COLORS_MODE_SPECIFIC;
    Blink.colors.resize(2);
    Blink.brightness_min          = 0;
    Blink.brightness_max          = 255;
    Blink.brightness              = brightness;
    modes.push_back(Blink);

    mode Breathing;
    Breathing.name                = "Breathing";
    Breathing.value               = REALTEK_ARGB_EFF_BREATH;
    Breathing.flags               = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Breathing.speed_min           = REALTEK_ARGB_SPEED_MIN;
    Breathing.speed_max           = REALTEK_ARGB_SPEED_MAX;
    Breathing.speed               = REALTEK_ARGB_SPEED_NORMAL;
    Breathing.colors_min          = 1;
    Breathing.colors_max          = 2;
    Breathing.color_mode          = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors.resize(2);
    Breathing.brightness_min      = 0;
    Breathing.brightness_max      = 255;
    Breathing.brightness          = brightness;
    modes.push_back(Breathing);

    mode Spectrum;
    Spectrum.name                 = "Spectrum";
    Spectrum.value                = REALTEK_ARGB_EFF_SPECTRUM;
    Spectrum.flags                = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Spectrum.speed_min            = REALTEK_ARGB_SPEED_MIN;
    Spectrum.speed_max            = REALTEK_ARGB_SPEED_MAX;
    Spectrum.speed                = REALTEK_ARGB_SPEED_NORMAL;
    Spectrum.color_mode           = MODE_COLORS_NONE;
    Spectrum.brightness_min       = 0;
    Spectrum.brightness_max       = 255;
    Spectrum.brightness           = brightness;
    modes.push_back(Spectrum);

    mode Scroll;
    Scroll.name                   = "Scroll";
    Scroll.value                  = REALTEK_ARGB_EFF_SCROLL;
    Scroll.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Scroll.speed_min              = REALTEK_ARGB_SPEED_MIN;
    Scroll.speed_max              = REALTEK_ARGB_SPEED_MAX;
    Scroll.speed                  = REALTEK_ARGB_SPEED_NORMAL;
    Scroll.colors_min             = 1;
    Scroll.colors_max             = 2;
    Scroll.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Scroll.colors.resize(2);
    Scroll.brightness_min         = 0;
    Scroll.brightness_max         = 255;
    Scroll.brightness             = brightness;
    modes.push_back(Scroll);

    mode RainbowScroll;
    RainbowScroll.name            = "RainbowScroll";
    RainbowScroll.value           = REALTEK_ARGB_EFF_RAINBOW_SCROLL;
    RainbowScroll.flags           = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    RainbowScroll.speed_min       = REALTEK_ARGB_SPEED_MIN;
    RainbowScroll.speed_max       = REALTEK_ARGB_SPEED_MAX;
    RainbowScroll.speed           = REALTEK_ARGB_SPEED_NORMAL;
    RainbowScroll.color_mode      = MODE_COLORS_NONE;
    RainbowScroll.brightness_min  = 0;
    RainbowScroll.brightness_max  = 255;
    RainbowScroll.brightness      = brightness;
    modes.push_back(RainbowScroll);

    mode RunningWater;
    RunningWater.name             = "RunningWater";
    RunningWater.value            = REALTEK_ARGB_EFF_RUNNING_WATER;
    RunningWater.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    RunningWater.speed_min        = REALTEK_ARGB_SPEED_MIN;
    RunningWater.speed_max        = REALTEK_ARGB_SPEED_MAX;
    RunningWater.speed            = REALTEK_ARGB_SPEED_NORMAL;
    RunningWater.colors_min       = 1;
    RunningWater.colors_max       = 2;
    RunningWater.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    RunningWater.colors.resize(2);
    RunningWater.brightness_min   = 0;
    RunningWater.brightness_max   = 255;
    RunningWater.brightness       = brightness;
    modes.push_back(RunningWater);

    mode Sliding;
    Sliding.name                   = "Sliding";
    Sliding.value                  = REALTEK_ARGB_EFF_SLIDING;
    Sliding.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Sliding.speed_min              = REALTEK_ARGB_SPEED_MIN;
    Sliding.speed_max              = REALTEK_ARGB_SPEED_MAX;
    Sliding.speed                  = REALTEK_ARGB_SPEED_NORMAL;
    Sliding.colors_min             = 1;
    Sliding.colors_max             = 2;
    Sliding.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Sliding.colors.resize(2);
    Sliding.brightness_min         = 0;
    Sliding.brightness_max         = 255;
    Sliding.brightness             = brightness;
    modes.push_back(Sliding);

    mode NewtonCradle;
    NewtonCradle.name             = "NewtonCradle";
    NewtonCradle.value            = REALTEK_ARGB_EFF_NEWTON_CRADLE;
    NewtonCradle.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    NewtonCradle.speed_min        = REALTEK_ARGB_SPEED_MIN;
    NewtonCradle.speed_max        = REALTEK_ARGB_SPEED_MAX;
    NewtonCradle.speed            = REALTEK_ARGB_SPEED_NORMAL;
    NewtonCradle.colors_min       = 1;
    NewtonCradle.colors_max       = 2;
    NewtonCradle.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    NewtonCradle.colors.resize(2);
    NewtonCradle.brightness_min   = 0;
    NewtonCradle.brightness_max   = 255;
    NewtonCradle.brightness       = brightness;
    modes.push_back(NewtonCradle);

    mode Meteor;
    Meteor.name                   = "Meteor";
    Meteor.value                  = REALTEK_ARGB_EFF_METEOR;
    Meteor.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Meteor.speed_min              = REALTEK_ARGB_SPEED_MIN;
    Meteor.speed_max              = REALTEK_ARGB_SPEED_MAX;
    Meteor.speed                  = REALTEK_ARGB_SPEED_NORMAL;
    Meteor.colors_min             = 1;
    Meteor.colors_max             = 2;
    Meteor.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Meteor.colors.resize(2);
    Meteor.brightness_min         = 0;
    Meteor.brightness_max         = 255;
    Meteor.brightness             = brightness;
    modes.push_back(Meteor);
}

RGBController_RealtekARGB::~RGBController_RealtekARGB()
{
    delete controller;
}

void RGBController_RealtekARGB::SetupZones()
{
    int idx = 0, argb_num = 0;
    int fix_grps = 0, num_fixgrp = 0, argb_num_fixgrp = 0;

    leds.clear();
    colors.clear();
    zones.clear();
    valid_grp.clear();
    fix_grps = controller->get_fix_grps();

    for(int grp_num = 0; grp_num < REALTEK_ARGB_NUM_ARGB_GRP; grp_num++)
    {
        if(controller->get_argb_num(grp_num) > 0)
        {
            valid_grp.push_back(grp_num);
            if(fix_grps & (0x1 << grp_num))
            {
                num_fixgrp++;
                argb_num_fixgrp += controller->get_argb_num(grp_num);
            }
        }
    }

    zones.resize(valid_grp.size());
    for(int grp_num : valid_grp)
    {
        argb_num = controller->get_argb_num(grp_num);
        if(argb_num <= 0)
            continue;

        zones[idx].leds_count = argb_num;
        zones[idx].name       = "strip " + std::to_string(idx + 1);
        zones[idx].type       = ZONE_TYPE_LINEAR;
        if(fix_grps & (0x1 << grp_num))
        {
            zones[idx].leds_min   = argb_num;
            zones[idx].leds_max   = argb_num;
        }
        else
        {
            zones[idx].leds_min   = 1;
            zones[idx].leds_max   = (REALTEK_ARGB_MAX - argb_num_fixgrp) / (valid_grp.size() - num_fixgrp);
        }
        zones[idx].matrix_map = NULL;

        for(unsigned int led_idx = 0; led_idx < zones[idx].leds_count; led_idx++)
        {
            led myled;
            myled.name = zones[idx].name + "  led_";
            myled.name.append(std::to_string(led_idx + 1));
            leds.push_back(myled);
        }
        idx++;
    }
    SetupColors();
}

void RGBController_RealtekARGB::ResizeZone(int zone, int new_size)
{
    if((size_t) zone >= zones.size())
    {
        return;
    }

    if(((unsigned int)new_size >= zones[zone].leds_min) && ((unsigned int)new_size <= zones[zone].leds_max))
    {
        int orig_num = controller->get_argb_num(valid_grp[zone]);
        int total_argb_num = 0;
        std::vector<RGBColor> rtk_colors(orig_num, 0);
        for(int grp_num = 0; grp_num < REALTEK_ARGB_NUM_ARGB_GRP; grp_num++)
        {
            if(grp_num == valid_grp[zone])
                total_argb_num += new_size;
            else
                total_argb_num += controller->get_argb_num(grp_num);
        }
        if(total_argb_num > REALTEK_ARGB_MAX)
            return;

        controller->set_argb_direct(valid_grp[zone], rtk_colors, 0xFF);
        controller->set_argb_num(valid_grp[zone], new_size);
        controller->argb_reinit();

        SetupZones();
    }
}

void RGBController_RealtekARGB::DeviceUpdateLEDs()
{
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].leds_count > 0)
        {
           UpdateZoneLEDs(zone_idx);
        }
    }
}

void RGBController_RealtekARGB::UpdateZoneLEDs(int zone)
{
    unsigned short brightness = 0xFF;
    mode& curr_mode = modes[active_mode];
    std::vector<RGBColor> color_buf;

    if(curr_mode.color_mode == MODE_COLORS_PER_LED &&
       curr_mode.value == REALTEK_ARGB_EFF_NULL)       //direct mode
    {
        if(curr_mode.flags & MODE_FLAG_HAS_BRIGHTNESS)
            brightness = curr_mode.brightness;

        color_buf.resize(zones[zone].leds_count);
        for(int i = 0; i < zones[zone].leds_count; i++)
            color_buf[i] = zones[zone].colors[i];
        controller->set_argb_direct(valid_grp[zone], color_buf, brightness);
    }
    else
        UpdateSingleLED(zone);
}

void RGBController_RealtekARGB::UpdateSingleLED(int zone)
{
    unsigned char speed = REALTEK_ARGB_SPEED_NORMAL;
    unsigned char dir = 0;
    unsigned short brightness = 0xFF;
    mode& curr_mode = modes[active_mode];
    std::vector<RGBColor> rtk_colors = curr_mode.colors;

    if(curr_mode.flags & MODE_FLAG_HAS_SPEED)
    {
        speed = curr_mode.speed;
    }
    if(curr_mode.flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        brightness = curr_mode.brightness;
    }
    if(curr_mode.flags & MODE_FLAG_HAS_DIRECTION_LR)
    {
        if(curr_mode.direction == MODE_DIRECTION_RIGHT)
        {
            dir = 1;
        }
    }

    if(curr_mode.color_mode == MODE_COLORS_PER_LED)
    {
        rtk_colors = colors;
    }
    else if(curr_mode.color_mode == MODE_COLORS_NONE)
    {
        rtk_colors.clear();
    }

    controller->set_argb_effect(valid_grp[zone], curr_mode.value, rtk_colors, speed, brightness);
}

void RGBController_RealtekARGB::DeviceUpdateMode()
{
    if(modes[active_mode].value != REALTEK_ARGB_EFF_NULL)
    {
        DeviceUpdateLEDs();
    }
}

#endif
