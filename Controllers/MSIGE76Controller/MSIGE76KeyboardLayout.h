/*---------------------------------------------------------*\
| MSIGE76KeyboardLayout.h                                   |
|                                                           |
|   Spanish MSI GE76 Raider matrix: hand-authored 6×20 coordinates.          |
|   from interactive discovery + PAGE_UP (75)               |
|                                                           |
|   Matrix map is edited manually in MSIGE76KeyboardLayout.cpp               |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>

#define MSIGE76_KLC_MATRIX_ROWS     6
#define MSIGE76_KLC_MATRIX_COLS     20
#define MSIGE76_KLC_LED_COUNT       102

#define MSIGE76_ALC_LED_COUNT       29

/*
 * Aurora bar: 29 physical zones. OpenRGB LED i -> NUMPAD slot i, HID KC i+1
 * (i = 0..28, KC 1..29). A 30th logical LED had no hardware mapping.
 */
#define MSIGE76_ALC_MATRIX_ROWS     1
#define MSIGE76_ALC_MATRIX_COLS     MSIGE76_ALC_LED_COUNT

extern const unsigned int MSIGE76_KLC_MATRIX_MAP[MSIGE76_KLC_MATRIX_ROWS * MSIGE76_KLC_MATRIX_COLS];
extern const uint8_t      MSIGE76_KLC_LED_KEYCODES[MSIGE76_KLC_LED_COUNT];
extern const char* const    MSIGE76_KLC_LED_NAMES[MSIGE76_KLC_LED_COUNT];
