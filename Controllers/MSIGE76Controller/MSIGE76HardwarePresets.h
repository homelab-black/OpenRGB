/*---------------------------------------------------------*\
| MSIGE76HardwarePresets.h                                  |
|                                                           |
|   Firmware effect presets (msi-perkeyrgb captures)      |
|   See MSIGE76HardwarePresets.cpp and scripts/             |
|   gen_hardware_preset_cpp.py                              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later             |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>

#define MSIGE76_HARDWARE_PRESET_PACKET_SIZE   524

struct MSIGE76HardwarePresetDesc
{
    const char*     name;
    const uint8_t (*packets)[MSIGE76_HARDWARE_PRESET_PACKET_SIZE];
    unsigned int    packet_count;
    /** If non-null: 64-byte output report (hid_write) before Commit (0x09). */
    const uint8_t*  tail_output;
};

extern const MSIGE76HardwarePresetDesc MSIGE76_HARDWARE_PRESETS[];
extern const unsigned int MSIGE76_HARDWARE_PRESET_COUNT;
/** Table index of the vendor \"default\" capture — resets effect layers before other presets. */
extern const unsigned int MSIGE76_HARDWARE_PRESET_INDEX_DEFAULT;
