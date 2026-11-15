/*---------------------------------------------------------*\
| MSIGE76Controller.cpp                                     |
|                                                           |
|   Driver for MSI GE76 Raider SteelSeries KLC/ALC         |
|                                                           |
|   Protocol reverse-engineered from msi-perkeyrgb          |
|     https://github.com/Askannz/msi-perkeyrgb             |
|                                                           |
|   Colour packet: hid_send_feature_report, 524 bytes       |
|     buf[0] = 0x0e (cmd byte, NOT a report-ID prefix)     |
|     buf[1] = 0x00                                         |
|     buf[2] = region_id                                    |
|     buf[3] = 0x00                                         |
|     buf[4 .. 507]: 42 × 12-byte key entries              |
|       [R, G, B, 0, 0, 0, 0, 0, 0, 0x01, 0, keycode]     |
|     buf[508..521]: 14 × 0x00                             |
|     buf[522] = 0x08, buf[523] = 0x39                     |
|                                                           |
|   Commit packet: hid_write, 64 bytes, buf[0] = 0x09      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include "MSIGE76Controller.h"
#include "MSIGE76HardwarePresets.h"
#include "MSIGE76KeyboardLayout.h"
#include "StringUtils.h"

namespace
{
/* Pause between preset phases (black / default / target). Slower updates avoid
 * HID/firmware dropping or merging packets on the ALC and KLC. */
/* Shorter than before: KLC preset path now commits per feature packet. */
constexpr int MSIGE76_PRESET_STEP_DELAY_MS = 32;

/* After hardware presets, one commit for four KLC regions often only sticks to
 * part of the keyboard; commit per region with a short gap batches reliably. */
constexpr int MSIGE76_KLC_REGION_STEP_MS = 14;

/* Light bar: second identical numpad packet helps after effect mode (optional). */
constexpr int MSIGE76_ALC_DIRECT_REPEAT_MS = 14;

/* KLC: commit after each 524-byte preset packet so all regions/layers latch. */
constexpr int MSIGE76_PRESET_FEATURE_STEP_MS = 7;

/* Region index 1 = ENTER (0x0b), middle cluster that often sticks on GE76. */
constexpr int MSIGE76_KLC_ENTER_REGION_INDEX = 1;

/* ALC hardware presets reuse keyboard numpad (0x24) captures with real numpad HID
 * usage IDs (e.g. 64–99). Direct bar addressing uses slots 0–28 with KC 1–29 — if
 * we only shift slot data, RGB may sit on keycodes the bar firmware ignores (LEDs 1–2
 * black, pattern biased right). Fix:
 *   (1) Copy from original slot (s + shift) to pull the pattern toward the left.
 *   (2) Force keycode byte dst[11] = slot+1 for bar slots (same as SetColorsALC).
 *   (3) Clear packet slots >= ALC_LED_COUNT. Tune shift (3) if still skewed. */
constexpr int MSIGE76_ALC_PRESET_NUMPAD_SLOT_SHIFT = 3;

void PatchAlcNumpadPresetPacket(uint8_t* buf)
{
    if(buf[0] != MSIGE76_CMD_COLOR || buf[2] != MSIGE76_REGION_NUMPAD)
    {
        return;
    }

    if(MSIGE76_ALC_PRESET_NUMPAD_SLOT_SHIFT == 0)
    {
        return;
    }

    const int shift = MSIGE76_ALC_PRESET_NUMPAD_SLOT_SHIFT;

    uint8_t scratch[MSIGE76_KEYS_PER_REGION * MSIGE76_KEY_ENTRY_SIZE];
    memcpy(scratch, buf + 4, sizeof(scratch));

    for(int slot = 0; slot < MSIGE76_KEYS_PER_REGION; slot++)
    {
        uint8_t* dst = buf + 4 + slot * MSIGE76_KEY_ENTRY_SIZE;
        int       src_slot = slot + shift;

        if(src_slot >= 0 && src_slot < MSIGE76_KEYS_PER_REGION)
        {
            memcpy(dst, scratch + src_slot * MSIGE76_KEY_ENTRY_SIZE, MSIGE76_KEY_ENTRY_SIZE);
        }
        else
        {
            memset(dst, 0, MSIGE76_KEY_ENTRY_SIZE);
            dst[9] = MSIGE76_MODE_STATIC;
        }

        if(slot < (int)MSIGE76_ALC_LED_COUNT)
        {
            dst[11] = (uint8_t)(slot + 1);
        }
        else
        {
            memset(dst, 0, MSIGE76_KEY_ENTRY_SIZE);
            dst[9] = MSIGE76_MODE_STATIC;
        }
    }
}
}

/*---------------------------------------------------------*\
| Region slot tables (msi-perkeyrgb REGION_KEYCODES)         |
\*---------------------------------------------------------*/
static const uint8_t REGION_ALPHANUM[MSIGE76_KEYS_PER_REGION] =
{
    4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 58, 59, 60, 61,
    62, 63
};

/* Key 102 (PWR) is in NUMPAD region after 99 on this firmware — see ge76_discover.py */
static const uint8_t REGION_ENTER[MSIGE76_KEYS_PER_REGION] =
{
    40, 49, 50, 100, 135, 136, 137, 138, 139, 144,
    145,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0
};

static const uint8_t REGION_MODIFIERS[MSIGE76_KEYS_PER_REGION] =
{
    41, 42, 43, 44, 45, 46, 47, 48, 51, 52,
    53, 54, 55, 56, 57, 101, 224, 225, 226, 227,
    228, 229, 230, 240,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0
};

static const uint8_t REGION_NUMPAD[MSIGE76_KEYS_PER_REGION] =
{
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
    84, 85, 86, 87, 88, 89, 90, 91, 92, 93,
    94, 95, 96, 97, 98, 99, 102,  0,  0,  0,
     0,  0
};

static const uint8_t REGION_IDS[4] =
{
    MSIGE76_REGION_ALPHANUM,
    MSIGE76_REGION_ENTER,
    MSIGE76_REGION_MODIFIERS,
    MSIGE76_REGION_NUMPAD,
};

static const uint8_t* region_tables[4] =
{
    REGION_ALPHANUM,
    REGION_ENTER,
    REGION_MODIFIERS,
    REGION_NUMPAD,
};

MSIGE76Controller::MSIGE76Controller
(
    hid_device*         dev_handle,
    const char*         path,
    const std::string&  dev_name,
    unsigned int        led_count
)
{
    dev                     = dev_handle;
    location                = path;
    name                    = dev_name;
    num_leds                = led_count;
}

MSIGE76Controller::~MSIGE76Controller()
{
    hid_close(dev);
}

std::string MSIGE76Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string MSIGE76Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string MSIGE76Controller::GetDeviceName()
{
    return(name);
}

unsigned int MSIGE76Controller::GetLEDCount()
{
    return(num_leds);
}

void MSIGE76Controller::SetAllColor(RGBColor color)
{
    SetColors(std::vector<RGBColor>(1, color), MSIGE76_BRIGHTNESS_MAX);
}

void MSIGE76Controller::SetColors(const std::vector<RGBColor>& colors, uint8_t /*brightness*/)
{
    if(colors.empty())
    {
        return;
    }

    /* Some callers send one color for "all LEDs". Expand it here so KLC/ALC
     * do not leave stale hardware-preset pixels on untouched keycodes. */
    if(colors.size() == 1)
    {
        if(num_leds == MSIGE76_KLC_LED_COUNT)
        {
            SetColorsKLC(std::vector<RGBColor>(MSIGE76_KLC_LED_COUNT, colors[0]));
            return;
        }
        if(num_leds == MSIGE76_ALC_LED_COUNT)
        {
            SetColorsALC(std::vector<RGBColor>(MSIGE76_ALC_LED_COUNT, colors[0]));
            return;
        }
        SetColorsSolidSweep(colors[0]);
        return;
    }

    if(num_leds == MSIGE76_KLC_LED_COUNT)
    {
        SetColorsKLC(colors);
    }
    else if(num_leds == MSIGE76_ALC_LED_COUNT)
    {
        SetColorsALC(colors);
    }
    else
    {
        SetColorsSolidSweep(colors[0]);
    }
}

void MSIGE76Controller::SendOneHardwarePreset(const MSIGE76HardwarePresetDesc& p)
{
    const bool klc = (num_leds == MSIGE76_KLC_LED_COUNT);
    const bool alc = (num_leds == MSIGE76_ALC_LED_COUNT);

    uint8_t alc_scratch[MSIGE76_COLOR_PKT_SIZE];

    for(unsigned int i = 0; i < p.packet_count; i++)
    {
        if(alc)
        {
            memcpy(alc_scratch, p.packets[i], MSIGE76_COLOR_PKT_SIZE);
            PatchAlcNumpadPresetPacket(alc_scratch);
            hid_send_feature_report(dev, alc_scratch, MSIGE76_COLOR_PKT_SIZE);
        }
        else
        {
            hid_send_feature_report(dev, p.packets[i], MSIGE76_COLOR_PKT_SIZE);
        }

        if(klc)
        {
            Commit();
            if(i + 1 < p.packet_count)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(MSIGE76_PRESET_FEATURE_STEP_MS));
            }
        }
    }

    if(p.tail_output != nullptr)
    {
        hid_write(dev, p.tail_output, MSIGE76_COMMIT_PKT_SIZE);
    }

    Commit();
}

void MSIGE76Controller::ApplyHardwarePreset(unsigned int preset_index)
{
    if((num_leds != MSIGE76_KLC_LED_COUNT && num_leds != MSIGE76_ALC_LED_COUNT)
       || preset_index >= MSIGE76_HARDWARE_PRESET_COUNT)
    {
        return;
    }

    auto flush_static_black = [this]()
    {
        if(num_leds == MSIGE76_KLC_LED_COUNT)
        {
            std::vector<RGBColor> black(MSIGE76_KLC_LED_COUNT, ToRGBColor(0, 0, 0));
            /* Single tail commit: avoids four stepped black flashes before preset. */
            SetColorsKLC(black, true);
        }
        else
        {
            std::vector<RGBColor> black(MSIGE76_ALC_LED_COUNT, ToRGBColor(0, 0, 0));
            SetColorsALC(black);
        }
    };

    /* One static clear, then longer gaps so the controller finishes each phase. */
    flush_static_black();
    std::this_thread::sleep_for(std::chrono::milliseconds(MSIGE76_PRESET_STEP_DELAY_MS));

    if(preset_index != MSIGE76_HARDWARE_PRESET_INDEX_DEFAULT)
    {
        SendOneHardwarePreset(MSIGE76_HARDWARE_PRESETS[MSIGE76_HARDWARE_PRESET_INDEX_DEFAULT]);
        std::this_thread::sleep_for(std::chrono::milliseconds(MSIGE76_PRESET_STEP_DELAY_MS));
    }

    SendOneHardwarePreset(MSIGE76_HARDWARE_PRESETS[preset_index]);
}

void MSIGE76Controller::SetColorsKLC(const std::vector<RGBColor>& colors, bool single_commit_flush)
{
    RGBColor by_kc[256];
    memset(by_kc, 0, sizeof(by_kc));

    unsigned int n = (unsigned int)colors.size();
    if(n > MSIGE76_KLC_LED_COUNT)
    {
        n = MSIGE76_KLC_LED_COUNT;
    }

    for(unsigned int i = 0; i < n; i++)
    {
        uint8_t kc = MSIGE76_KLC_LED_KEYCODES[i];
        by_kc[kc] = colors[i];
    }

    auto send_region = [&](int ri)
    {
        RGBColor slot_cols[MSIGE76_KEYS_PER_REGION];
        for(int s = 0; s < MSIGE76_KEYS_PER_REGION; s++)
        {
            uint8_t kc = region_tables[ri][s];
            slot_cols[s] = kc ? by_kc[kc] : ToRGBColor(0, 0, 0);
        }
        SendRegionPacket(REGION_IDS[ri], region_tables[ri], slot_cols);
    };

    if(single_commit_flush)
    {
        for(int ri = 0; ri < 4; ri++)
        {
            send_region(ri);
        }
        Commit();
        return;
    }

    /* Direct mode — clear → pause → set pattern:
     * 1. Flush all regions to black with a single commit so the firmware
     *    starts from a known-clean state.
     * 2. Wait 500 ms so the clear visibly latches on all regions including
     *    the numpad / F7+ section.
     * 3. Send the desired colours, committing each region individually and
     *    ending with NUMPAD last (a later commit for another region resets
     *    the numpad/navigation/F7+ section on this firmware). */

    /* Step 1: black flush */
    SetColorsKLC(std::vector<RGBColor>(MSIGE76_KLC_LED_COUNT, ToRGBColor(0, 0, 0)), true);

    /* Step 2: pause */
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    /* Step 3: desired colours — ALPHANUM → MODIFIERS → ENTER (×2) → NUMPAD (×2) */
    static const int direct_order[] = { 0, 2, 1, 1, 3, 3 };

    for(int step = 0; step < 6; step++)
    {
        send_region(direct_order[step]);
        Commit();
        if(step < 5)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(MSIGE76_KLC_REGION_STEP_MS));
        }
    }
}

void MSIGE76Controller::SetColorsALC(const std::vector<RGBColor>& colors)
{
    if(colors.empty())
    {
        return;
    }

    uint8_t kcs[MSIGE76_KEYS_PER_REGION];
    RGBColor slot_cols[MSIGE76_KEYS_PER_REGION];
    memset(kcs, 0, sizeof(kcs));
    memset(slot_cols, 0, sizeof(slot_cols));

    unsigned int n = (unsigned int)colors.size();
    if(n > MSIGE76_ALC_LED_COUNT)
    {
        n = MSIGE76_ALC_LED_COUNT;
    }

    /* 29 bar LEDs: slots 0..28, HID keycodes 1..29. */
    for(unsigned int i = 0; i < n; i++)
    {
        kcs[i]       = (uint8_t)(i + 1);
        slot_cols[i] = colors[i];
    }

    SendRegionPacket(MSIGE76_REGION_NUMPAD, kcs, slot_cols);
    Commit();
    std::this_thread::sleep_for(std::chrono::milliseconds(MSIGE76_ALC_DIRECT_REPEAT_MS));
    SendRegionPacket(MSIGE76_REGION_NUMPAD, kcs, slot_cols);
    Commit();
}

void MSIGE76Controller::SetColorsSolidSweep(RGBColor c)
{
    static const uint8_t regions[4] =
    {
        MSIGE76_REGION_MODIFIERS,
        MSIGE76_REGION_ALPHANUM,
        MSIGE76_REGION_ENTER,
        MSIGE76_REGION_NUMPAD,
    };

    for(int base = 0; base < 252; base += MSIGE76_KEYS_PER_REGION)
    {
        uint8_t keycodes[MSIGE76_KEYS_PER_REGION];
        for(int i = 0; i < MSIGE76_KEYS_PER_REGION; i++)
        {
            keycodes[i] = (uint8_t)(base + i);
        }

        RGBColor slot_cols[MSIGE76_KEYS_PER_REGION];
        for(int i = 0; i < MSIGE76_KEYS_PER_REGION; i++)
        {
            slot_cols[i] = c;
        }

        for(int ri = 0; ri < 4; ri++)
        {
            SendRegionPacket(regions[ri], keycodes, slot_cols);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    Commit();
}

void MSIGE76Controller::SendRegionPacket
(
    uint8_t        region_id,
    const uint8_t  keycodes[MSIGE76_KEYS_PER_REGION],
    const RGBColor slot_colors[MSIGE76_KEYS_PER_REGION]
)
{
    uint8_t buf[MSIGE76_COLOR_PKT_SIZE];
    memset(buf, 0x00, sizeof(buf));

    buf[0] = MSIGE76_CMD_COLOR;
    buf[1] = 0x00;
    buf[2] = region_id;
    buf[3] = 0x00;

    for(int i = 0; i < MSIGE76_KEYS_PER_REGION; i++)
    {
        int off         = 4 + i * MSIGE76_KEY_ENTRY_SIZE;
        buf[off + 0]    = RGBGetRValue(slot_colors[i]);
        buf[off + 1]    = RGBGetGValue(slot_colors[i]);
        buf[off + 2]    = RGBGetBValue(slot_colors[i]);
        buf[off + 3]    = 0x00;
        buf[off + 4]    = 0x00;
        buf[off + 5]    = 0x00;
        buf[off + 6]    = 0x00;
        buf[off + 7]    = 0x00;
        buf[off + 8]    = 0x00;
        buf[off + 9]    = MSIGE76_MODE_STATIC;
        buf[off + 10]   = 0x00;
        buf[off + 11]   = keycodes[i];
    }

    buf[522] = 0x08;
    buf[523] = 0x39;

    hid_send_feature_report(dev, buf, MSIGE76_COLOR_PKT_SIZE);
}

void MSIGE76Controller::Commit()
{
    uint8_t buf[MSIGE76_COMMIT_PKT_SIZE];
    memset(buf, 0x00, sizeof(buf));
    buf[0] = MSIGE76_CMD_COMMIT;
    hid_write(dev, buf, MSIGE76_COMMIT_PKT_SIZE);
}
