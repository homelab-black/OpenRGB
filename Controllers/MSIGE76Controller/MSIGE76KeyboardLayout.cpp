/*---------------------------------------------------------*\
| MSIGE76KeyboardLayout.cpp                                 |
|                                                           |
|   Hand-authored 6×20 matrix coordinates for GE76 keyboard view.          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "MSIGE76KeyboardLayout.h"
#include "RGBControllerKeyNames.h"

#define NA 0xFFFFFFFF

/*
 * 6 × 20 visual matrix — every row sums to exactly 20 columns.
 * Wide / tall keys are represented by repeating the LED index in
 * adjacent cells (OpenRGB renders them as proportionally wider/taller).
 *
 * Key widths (columns occupied):
 *   Row 1: NUM_- (LED 37) × 3   → 3-unit wide numpad minus
 *   Row 2: NUM_+ (LED 55) × 3   → tall numpad plus, top half
 *   Row 3: ENTER (LED 68) × 2   → ISO enter body
 *          NUM_+ (LED 55) × 3   → tall numpad plus, bottom half
 *   Row 4: R-SHIFT (LED 83) × 2 → wide right shift
 *          NUM_EN (LED 101) × 3 → tall numpad enter, top half
 *   Row 5: SPACE  (LED 91) × 4  → wide space bar
 *          NUM_0  (LED 99) × 2  → wide numpad zero
 *          NUM_EN (LED 101) × 3 → tall numpad enter, bottom half
 */
const unsigned int MSIGE76_KLC_MATRIX_MAP[MSIGE76_KLC_MATRIX_ROWS * MSIGE76_KLC_MATRIX_COLS] =
{
    /* row 0 — Fn / media row: all 1-unit wide */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,

    /* row 1 — number row: NUM_- spans cols 17-19 */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 37, 37,

    /* row 2 — QWERTY row: NUM_+ spans cols 17-19 */
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 55,

    /* row 3 — ASDF row: ENTER spans 12-13; NUM_+ spans 17-19 */
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 68, 69, 70, 71, 55, 55, 55,

    /* row 4 — ZXCV row: R-SHIFT spans 11-12; NUM_EN spans 17-19 */
    72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 83, 84, 85, 86, 87, 101, 101, 101,

    /* row 5 — modifier row: SPACE spans 3-6; NUM_0 spans 14-15; NUM_EN spans 17-19 */
    88, 89, 90, 91, 91, 91, 91, 92, 93, 94, 95, 96, 97, 98, 99, 99, 100, 101, 101, 101,
};

const uint8_t MSIGE76_KLC_LED_KEYCODES[MSIGE76_KLC_LED_COUNT] =
{
    41, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 73, 76, 70, 75, 78, 102,
    53, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 42, 83, 84, 85, 86, 43, 20,
    26, 8,  21, 23, 28, 24, 12, 18, 19, 47, 48, 49, 95, 96, 97, 87, 57, 4,  22, 7,
    9,  10, 11, 13, 14, 15, 51, 52, 40, 92, 93, 94, 225, 29, 27, 6,  25, 5,  17, 16,
    54, 55, 56, 229, 82, 89, 90, 91, 224, 227, 226, 44, 230, 100, 240, 228, 80, 81, 79, 98,
    99, 88
};

const char* const MSIGE76_KLC_LED_NAMES[MSIGE76_KLC_LED_COUNT] =
{
    KEY_EN_ESCAPE, KEY_EN_F1, KEY_EN_F2, KEY_EN_F3,
    KEY_EN_F4, KEY_EN_F5, KEY_EN_F6, KEY_EN_F7,
    KEY_EN_F8, KEY_EN_F9, KEY_EN_F10, KEY_EN_F11,
    KEY_EN_F12, KEY_EN_SCROLL_LOCK, KEY_EN_INSERT, KEY_EN_DELETE,
    KEY_EN_PRINT_SCREEN, KEY_EN_PAGE_UP, KEY_EN_PAGE_DOWN, "PWR",
    "Key: Degrees", KEY_EN_1, KEY_EN_2, KEY_EN_3,
    KEY_EN_4, KEY_EN_5, KEY_EN_6, KEY_EN_7,
    KEY_EN_8, KEY_EN_9, KEY_EN_0, "Key: Apostrophe",
    "Key: Invert", KEY_EN_BACKSPACE, KEY_EN_NUMPAD_LOCK, KEY_EN_NUMPAD_DIVIDE,
    KEY_EN_NUMPAD_TIMES, KEY_EN_NUMPAD_MINUS, KEY_EN_TAB, KEY_EN_Q,
    KEY_EN_W, KEY_EN_E, KEY_EN_R, KEY_EN_T,
    KEY_EN_Y, KEY_EN_U, KEY_EN_I, KEY_EN_O,
    KEY_EN_P, KEY_EN_BACK_TICK, KEY_EN_PLUS, "Key: C Cedilla",
    KEY_EN_NUMPAD_7, KEY_EN_NUMPAD_8, KEY_EN_NUMPAD_9, KEY_EN_NUMPAD_PLUS,
    KEY_EN_CAPS_LOCK, KEY_EN_A, KEY_EN_S, KEY_EN_D,
    KEY_EN_F, KEY_EN_G, KEY_EN_H, KEY_EN_J,
    KEY_EN_K, KEY_EN_L, "Key: N Tilde", "Key: Acute",
    KEY_EN_ANSI_ENTER, KEY_EN_NUMPAD_4, KEY_EN_NUMPAD_5, KEY_EN_NUMPAD_6,
    KEY_EN_LEFT_SHIFT, KEY_EN_Z, KEY_EN_X, KEY_EN_C,
    KEY_EN_V, KEY_EN_B, KEY_EN_N, KEY_EN_M,
    KEY_EN_COMMA, KEY_EN_PERIOD, KEY_EN_MINUS, KEY_EN_RIGHT_SHIFT,
    KEY_EN_UP_ARROW, KEY_EN_NUMPAD_1, KEY_EN_NUMPAD_2, KEY_EN_NUMPAD_3,
    KEY_EN_LEFT_CONTROL, KEY_EN_LEFT_WINDOWS, KEY_EN_LEFT_ALT, KEY_EN_SPACE,
    KEY_EN_RIGHT_ALT, "Key: Angle Bracket", "Key: Fn", KEY_EN_RIGHT_CONTROL,
    KEY_EN_LEFT_ARROW, KEY_EN_DOWN_ARROW, KEY_EN_RIGHT_ARROW, KEY_EN_NUMPAD_0,
    KEY_EN_NUMPAD_PERIOD, KEY_EN_NUMPAD_ENTER,
};
