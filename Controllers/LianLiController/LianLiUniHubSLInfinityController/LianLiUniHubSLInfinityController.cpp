/*---------------------------------------------------------*\
| LianLiUniHubSLInfinityController.cpp                      |
|                                                           |
|   Driver for Lian Li SL Infinity Uni Hub                  |
|                                                           |
|   Pau Verdeguer                               13 May 2026 |
|   Simon McKenna                               21 Oct 2023 |
|   Will Kennedy                                17 Jan 2023 |
|   Oliver P                                    26 Apr 2022 |
|   Credit to Luca Lovisa for original work.                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string.h>
#include <thread>
#include "LianLiUniHubSLInfinityController.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| The hub drops an effect packet that comes too soon after  |
| the previous one on the same channel, static or animated  |
| (10 to 20 ms fails, 30 ms works).                         |
\*---------------------------------------------------------*/
#define UNIHUB_SLINF_CHANNEL_SPACING_MS     35

/*---------------------------------------------------------*\
| The hub also loses reports sent while it is still         |
| handling an effect packet on any channel (5 to 10 ms      |
| fails, 15 ms works).                                      |
\*---------------------------------------------------------*/
#define UNIHUB_SLINF_REPORT_SPACING_MS      15

/*---------------------------------------------------------*\
| An animated effect packet that comes too soon after a     |
| static one on the same channel can be dropped too (35 to  |
| 45 ms fails, 60 ms works), seen on outer rings coming out |
| of Merged mode.                                           |
\*---------------------------------------------------------*/
#define UNIHUB_SLINF_STATIC_SETTLE_MS       100

/*---------------------------------------------------------*\
| Channels sent right after entering Merged mode stay stuck |
| in it (0 ms fails, 50 ms works).                          |
\*---------------------------------------------------------*/
#define UNIHUB_SLINF_MERGE_SETTLE_MS        100

static const unsigned int brightness_codes[5] =
{
    UNIHUB_SLINF_LED_BRIGHTNESS_000,
    UNIHUB_SLINF_LED_BRIGHTNESS_025,
    UNIHUB_SLINF_LED_BRIGHTNESS_050,
    UNIHUB_SLINF_LED_BRIGHTNESS_075,
    UNIHUB_SLINF_LED_BRIGHTNESS_100,
};

static const unsigned char speed_codes[5] =
{
    UNIHUB_SLINF_LED_SPEED_000,
    UNIHUB_SLINF_LED_SPEED_025,
    UNIHUB_SLINF_LED_SPEED_050,
    UNIHUB_SLINF_LED_SPEED_075,
    UNIHUB_SLINF_LED_SPEED_100,
};

static unsigned int BrightnessCode(const mode& m)
{
    return(brightness_codes[(m.brightness < 5) ? m.brightness : 4]);
}

static unsigned char SpeedCode(const mode& m)
{
    return(speed_codes[(m.speed < 5) ? m.speed : 2]);
}

/*---------------------------------------------------------*\
| OpenRGB uses Left = 0 and Right = 1, the hub uses the     |
| opposite.                                                 |
\*---------------------------------------------------------*/
static unsigned int DirectionCode(const mode& m)
{
    if(m.direction == MODE_DIRECTION_LEFT)
    {
        return(UNIHUB_SLINF_LED_DIRECTION_LEFT);
    }

    return(UNIHUB_SLINF_LED_DIRECTION_RIGHT);
}

/*---------------------------------------------------------*\
| Keeps R + G + B of a single LED under 460 (out of 765) to |
| avoid overdriving it, scaling the color down so the hue   |
| stays the same.                                           |
\*---------------------------------------------------------*/
static float CalcBrightnessScale(RGBColor color)
{
    unsigned int sum = RGBGetRValue(color) + RGBGetGValue(color) + RGBGetBValue(color);

    return((sum > 460) ? 460.0f / (float)sum : 1.0f);
}

/*---------------------------------------------------------*\
| Effects take 16 colors: the 4 effect colors repeated 4    |
| times, in R, B, G order. Unused colors are left black.    |
\*---------------------------------------------------------*/
static void BuildModeLEDData(unsigned char* out, const std::vector<RGBColor>& src_colors)
{
    for(unsigned int color_idx = 0; color_idx < 4; color_idx++)
    {
        RGBColor color = (color_idx < src_colors.size()) ? src_colors[color_idx] : 0;
        float    scale = CalcBrightnessScale(color);

        for(unsigned int block_idx = 0; block_idx < 4; block_idx++)
        {
            unsigned int pos = (block_idx * 12) + (color_idx * 3);

            out[pos + 0] = (unsigned char)(RGBGetRValue(color) * scale);
            out[pos + 1] = (unsigned char)(RGBGetBValue(color) * scale);
            out[pos + 2] = (unsigned char)(RGBGetGValue(color) * scale);
        }
    }
}

static void WaitUntil(std::chrono::steady_clock::time_point time)
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    if(now < time)
    {
        std::this_thread::sleep_for(time - now);
    }
}

LianLiUniHubSLInfinityController::LianLiUniHubSLInfinityController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev       = dev_handle;
    location  = path;
    name      = dev_name;
    is_merged = false;

    /*-----------------------------------------------------*\
    | What the hub shows at start is unknown, so every      |
    | channel is sent on the first update.                  |
    \*-----------------------------------------------------*/
    memset(zone_color_cache, 0x00, sizeof(zone_color_cache));
    memset(zone_effect_cache, 0x00, sizeof(zone_effect_cache));

    for(unsigned int channel = 0; channel < UNIHUB_SLINF_CHANNEL_COUNT; channel++)
    {
        zone_color_count[channel]   = 0;
        zone_stale[channel]         = true;
        last_effect_static[channel] = false;
    }

    /*-----------------------------------------------------*\
    | Turn off motherboard ARGB sync, since the hub ignores |
    | lighting commands while it is on, and set the default |
    | merge order like L-Connect does.                      |
    \*-----------------------------------------------------*/
    unsigned char usb_buf[UNIHUB_SLINF_FEATURE_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
    usb_buf[0x01] = UNIHUB_SLINF_CMD_EFFECT;
    usb_buf[0x02] = UNIHUB_SLINF_SUBCMD_MB_ARGB_SYNC;
    usb_buf[0x03] = 0x00;
    SendFeatureReport(usb_buf);

    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
    usb_buf[0x01] = UNIHUB_SLINF_CMD_EFFECT;
    usb_buf[0x02] = UNIHUB_SLINF_SUBCMD_MERGE_ORDER;
    usb_buf[0x03] = 0x00;
    usb_buf[0x04] = 0x01;
    usb_buf[0x05] = 0x02;
    usb_buf[0x06] = 0x03;
    SendFeatureReport(usb_buf);
}

LianLiUniHubSLInfinityController::~LianLiUniHubSLInfinityController()
{
    hid_close(dev);
}

std::string LianLiUniHubSLInfinityController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string LianLiUniHubSLInfinityController::GetFirmwareVersionString()
{
    /*-----------------------------------------------------*\
    | Same query as L-Connect. The reply is E0 50 80 C4     |
    | plus a version byte, decoded like L-Connect does.     |
    \*-----------------------------------------------------*/
    unsigned char usb_buf[UNIHUB_SLINF_FEATURE_REPORT_SIZE];
    unsigned char in_buf[UNIHUB_SLINF_INPUT_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
    usb_buf[0x01] = UNIHUB_SLINF_CMD_QUERY;
    usb_buf[0x02] = 0x01;
    SendFeatureReport(usb_buf);

    memset(in_buf, 0x00, sizeof(in_buf));
    in_buf[0x00] = UNIHUB_SLINF_REPORT_ID;

    if(hid_get_input_report(dev, in_buf, sizeof(in_buf)) >= 6
    && in_buf[1] == 0xE0 && in_buf[2] == 0x50 && in_buf[3] == 0x80 && in_buf[4] == 0xC4)
    {
        unsigned int version_byte = in_buf[5];
        unsigned int high         = (version_byte >> 4) & 0x0F;
        unsigned int low          = version_byte & 0x0F;

        if(!(version_byte <= 3 || (version_byte >= 15 && version_byte <= 31) || (version_byte >= 32 && low > 9)))
        {
            if(version_byte == 13 || version_byte == 14)
            {
                low++;
            }

            unsigned int version = high * 10 + low;

            return(std::to_string(version / 10) + "." + std::to_string(version % 10));
        }
    }

    wchar_t product_string[40];

    if(hid_get_product_string(dev, product_string, 40) != 0)
    {
        return("");
    }

    std::string product = StringUtils::wstring_to_string(product_string);

    return(product.substr(product.find_last_of("-") + 1, 4));
}

std::string LianLiUniHubSLInfinityController::GetName()
{
    return(name);
}

std::string LianLiUniHubSLInfinityController::GetSerialString()
{
    wchar_t serial_string[128];

    if(hid_get_serial_number_string(dev, serial_string, 128) != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool LianLiUniHubSLInfinityController::IsMerged()
{
    return(is_merged);
}

void LianLiUniHubSLInfinityController::SendFeatureReport(unsigned char* data)
{
    WaitUntil(last_report_time + std::chrono::milliseconds(UNIHUB_SLINF_REPORT_SPACING_MS));

    hid_send_feature_report(dev, data, UNIHUB_SLINF_FEATURE_REPORT_SIZE);

    last_report_time = std::chrono::steady_clock::now();
}

/*---------------------------------------------------------*\
| Every color and effect packet goes through here, so that  |
| no path can skip the pacing.                              |
\*---------------------------------------------------------*/
void LianLiUniHubSLInfinityController::SendChannel(unsigned char channel, unsigned int num_leds,
                                                    unsigned char* led_data, unsigned char effect,
                                                    unsigned char speed, unsigned int direction,
                                                    unsigned int brightness)
{
    unsigned char color_buf[UNIHUB_SLINF_OUTPUT_REPORT_SIZE];
    unsigned char effect_buf[UNIHUB_SLINF_FEATURE_REPORT_SIZE];
    bool          sent = true;

    WaitUntil(last_effect_time[channel] + std::chrono::milliseconds(UNIHUB_SLINF_CHANNEL_SPACING_MS));
    WaitUntil(last_report_time + std::chrono::milliseconds(UNIHUB_SLINF_REPORT_SPACING_MS));

    if(effect != UNIHUB_SLINF_LED_MODE_STATIC_COLOR && last_effect_static[channel])
    {
        WaitUntil(last_effect_time[channel] + std::chrono::milliseconds(UNIHUB_SLINF_STATIC_SETTLE_MS));
    }

    /*-----------------------------------------------------*\
    | The hub copies the whole report into its RAM, so the  |
    | unused part has to be zeroed.                         |
    \*-----------------------------------------------------*/
    if(num_leds > 0)
    {
        memset(color_buf, 0x00, sizeof(color_buf));
        color_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
        color_buf[0x01] = UNIHUB_SLINF_CMD_COLOR | channel;
        memcpy(&color_buf[0x02], led_data, num_leds * 3);

        sent = (hid_write(dev, color_buf, sizeof(color_buf)) >= 0);
    }

    memset(effect_buf, 0x00, sizeof(effect_buf));
    effect_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
    effect_buf[0x01] = UNIHUB_SLINF_CMD_EFFECT | channel;
    effect_buf[0x02] = effect;
    effect_buf[0x03] = speed;
    effect_buf[0x04] = (unsigned char)direction;
    effect_buf[0x05] = (unsigned char)brightness;

    sent = (hid_send_feature_report(dev, effect_buf, sizeof(effect_buf)) >= 0) && sent;

    last_effect_time[channel]   = std::chrono::steady_clock::now();
    last_effect_static[channel] = (effect == UNIHUB_SLINF_LED_MODE_STATIC_COLOR);
    last_report_time            = last_effect_time[channel];

    if(!sent)
    {
        zone_stale[channel] = true;
    }
}

void LianLiUniHubSLInfinityController::SendGlobalTrigger()
{
    /*-----------------------------------------------------*\
    | L-Connect calls this SetFrame and always sends 1 as   |
    | the frame number. It syncs the animation of all       |
    | channels, colors show up without it.                  |
    \*-----------------------------------------------------*/
    unsigned char usb_buf[UNIHUB_SLINF_FEATURE_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
    usb_buf[0x01] = UNIHUB_SLINF_CMD_SET_FRAME;
    usb_buf[0x02] = 0x00;
    usb_buf[0x03] = 0x01;

    SendFeatureReport(usb_buf);
}

void LianLiUniHubSLInfinityController::SetFanQuantity(unsigned char group, unsigned char quantity)
{
    unsigned char usb_buf[UNIHUB_SLINF_FEATURE_REPORT_SIZE];

    if(group >= UNIHUB_SLINF_GROUP_COUNT)
    {
        return;
    }

    if(quantity > UNIHUB_SLINF_MAX_FAN_COUNT)
    {
        quantity = UNIHUB_SLINF_MAX_FAN_COUNT;
    }

    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00] = UNIHUB_SLINF_REPORT_ID;
    usb_buf[0x01] = UNIHUB_SLINF_CMD_EFFECT;
    usb_buf[0x02] = UNIHUB_SLINF_SUBCMD_FAN_QUANTITY;
    usb_buf[0x03] = group + 1;
    usb_buf[0x04] = quantity;

    SendFeatureReport(usb_buf);
}

bool LianLiUniHubSLInfinityController::IsCached(unsigned char channel, unsigned int num_leds,
                                                 unsigned char* led_data, unsigned char effect,
                                                 unsigned char speed, unsigned int direction,
                                                 unsigned int brightness)
{
    return(!zone_stale[channel]
        && zone_color_count[channel]     == num_leds
        && zone_effect_cache[channel][0] == effect
        && zone_effect_cache[channel][1] == speed
        && zone_effect_cache[channel][2] == direction
        && zone_effect_cache[channel][3] == brightness
        && memcmp(zone_color_cache[channel], led_data, num_leds * 3) == 0);
}

bool LianLiUniHubSLInfinityController::StageZone(unsigned char channel, unsigned int num_leds,
                                                  unsigned char* led_data, unsigned char effect,
                                                  unsigned char speed, unsigned int direction,
                                                  unsigned int brightness)
{
    if(IsCached(channel, num_leds, led_data, effect, speed, direction, brightness))
    {
        return(false);
    }

    memset(zone_color_cache[channel], 0x00, sizeof(zone_color_cache[channel]));
    memcpy(zone_color_cache[channel], led_data, num_leds * 3);

    zone_color_count[channel]     = num_leds;
    zone_effect_cache[channel][0] = effect;
    zone_effect_cache[channel][1] = speed;
    zone_effect_cache[channel][2] = (unsigned char)direction;
    zone_effect_cache[channel][3] = (unsigned char)brightness;
    zone_stale[channel]           = false;

    SendChannel(channel, num_leds, led_data, effect, speed, direction, brightness);

    return(true);
}

unsigned int LianLiUniHubSLInfinityController::BuildPerLEDData(unsigned char* led_data, RGBColor* colors,
                                                                unsigned int num_colors)
{
    if(num_colors > UNIHUB_SLINF_MAX_CHAN_LED_COUNT)
    {
        num_colors = UNIHUB_SLINF_MAX_CHAN_LED_COUNT;
    }

    for(unsigned int led_idx = 0; led_idx < num_colors; led_idx++)
    {
        float        scale = CalcBrightnessScale(colors[led_idx]);
        unsigned int pos   = led_idx * 3;

        led_data[pos + 0] = (unsigned char)(RGBGetRValue(colors[led_idx]) * scale);
        led_data[pos + 1] = (unsigned char)(RGBGetBValue(colors[led_idx]) * scale);
        led_data[pos + 2] = (unsigned char)(RGBGetGValue(colors[led_idx]) * scale);
    }

    return(num_colors);
}

bool LianLiUniHubSLInfinityController::SetChannelLEDs(unsigned char channel, RGBColor* colors,
                                                       unsigned int num_colors)
{
    /*-----------------------------------------------------*\
    | A color report followed by a static effect packet on  |
    | the same channel shows the colors right away.         |
    \*-----------------------------------------------------*/
    unsigned char led_data[UNIHUB_SLINF_MAX_CHAN_LED_COUNT * 3] = {};

    if(num_colors == 0)
    {
        return(false);
    }

    num_colors = BuildPerLEDData(led_data, colors, num_colors);

    return(StageZone(channel, num_colors, led_data,
                     UNIHUB_SLINF_LED_MODE_STATIC_COLOR,
                     UNIHUB_SLINF_LED_SPEED_050,
                     UNIHUB_SLINF_LED_DIRECTION_RIGHT,
                     UNIHUB_SLINF_LED_BRIGHTNESS_100));
}

/*---------------------------------------------------------*\
| The static effect ignores the brightness byte, so rings   |
| are turned off with black colors.                         |
\*---------------------------------------------------------*/
void LianLiUniHubSLInfinityController::SetChannelOff(unsigned char channel)
{
    RGBColor black[UNIHUB_SLINF_MAX_CHAN_LED_COUNT] = {};

    SetChannelLEDs(channel, black, UNIHUB_SLINF_MAX_CHAN_LED_COUNT);
}

bool LianLiUniHubSLInfinityController::SetChannelBreathing(unsigned char channel, RGBColor* colors,
                                                            unsigned int num_colors, const mode& m)
{
    unsigned char led_data[UNIHUB_SLINF_MAX_CHAN_LED_COUNT * 3] = {};

    if(num_colors == 0)
    {
        return(false);
    }

    num_colors = BuildPerLEDData(led_data, colors, num_colors);

    return(StageZone(channel, num_colors, led_data,
                     UNIHUB_SLINF_LED_MODE_BREATHING,
                     SpeedCode(m),
                     UNIHUB_SLINF_LED_DIRECTION_RIGHT,
                     BrightnessCode(m)));
}

bool LianLiUniHubSLInfinityController::SetChannelMode(unsigned char channel, const mode& m)
{
    unsigned char led_data[UNIHUB_SLINF_MAX_CHAN_LED_COUNT * 3] = {};

    BuildModeLEDData(led_data, m.colors);

    return(StageZone(channel, UNIHUB_SLINF_EFFECT_COLOR_SLOTS, led_data,
                     (unsigned char)m.value,
                     SpeedCode(m),
                     DirectionCode(m),
                     BrightnessCode(m)));
}

/*---------------------------------------------------------*\
| Sent to the inner channel only, the outer ring follows it |
| by itself.                                                |
\*---------------------------------------------------------*/
bool LianLiUniHubSLInfinityController::SetChannelModeSimultaneous(unsigned char inner_channel, const mode& m)
{
    if(!SetChannelMode(inner_channel, m))
    {
        return(false);
    }

    zone_stale[inner_channel + 1] = true;

    return(true);
}

void LianLiUniHubSLInfinityController::SetMergedMode(const mode& m)
{
    /*-----------------------------------------------------*\
    | Same sequence as L-Connect. Channels 7 to 1 only get  |
    | an effect packet (Simultaneous Rainbow, brightness    |
    | off), which makes them follow channel 0, and then     |
    | channel 0 gets the Merged effect.                     |
    \*-----------------------------------------------------*/
    unsigned char led_data[UNIHUB_SLINF_MAX_CHAN_LED_COUNT * 3] = {};
    unsigned char effect     = (unsigned char)m.value;
    unsigned char speed      = SpeedCode(m);
    unsigned int  direction  = DirectionCode(m);
    unsigned int  brightness = BrightnessCode(m);

    BuildModeLEDData(led_data, m.colors);

    if(is_merged && IsCached(0, UNIHUB_SLINF_EFFECT_COLOR_SLOTS, led_data, effect, speed, direction, brightness))
    {
        return;
    }

    for(int channel = UNIHUB_SLINF_CHANNEL_COUNT - 1; channel >= 1; channel--)
    {
        zone_stale[channel] = true;

        SendChannel((unsigned char)channel, 0, NULL,
                    UNIHUB_SLINF_LED_MODE_RAINBOW_SIM,
                    UNIHUB_SLINF_LED_SPEED_050,
                    UNIHUB_SLINF_LED_DIRECTION_RIGHT,
                    UNIHUB_SLINF_LED_BRIGHTNESS_000);
    }

    zone_stale[0] = true;

    StageZone(0, UNIHUB_SLINF_EFFECT_COLOR_SLOTS, led_data, effect, speed, direction, brightness);

    merge_time = std::chrono::steady_clock::now();
    is_merged  = true;
}

/*---------------------------------------------------------*\
| The other channels only leave Merged mode together with   |
| channel 0, so every channel is marked to be sent again.   |
\*---------------------------------------------------------*/
void LianLiUniHubSLInfinityController::LeaveMergedMode()
{
    WaitUntil(merge_time + std::chrono::milliseconds(UNIHUB_SLINF_MERGE_SETTLE_MS));

    MarkAllChannelsStale();

    is_merged = false;
}

void LianLiUniHubSLInfinityController::MarkAllChannelsStale()
{
    for(unsigned int channel = 0; channel < UNIHUB_SLINF_CHANNEL_COUNT; channel++)
    {
        zone_stale[channel] = true;
    }
}
