#ifndef STEELSERIESAPSKLCLAYOUTS_H
#define STEELSERIESAPSKLCLAYOUTS_H

#include "RGBControllerKeyNames.h"
#include "KeyboardLayoutManager.h"
#include <vector>

/*-----------------------------------------------------*\
| Values are the ids to be sent in buffers              |
\*-----------------------------------------------------*/
const std::vector<unsigned int> steelseries_aps_klc_1122_default_ids_order =
{
    41, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71, 72, 73, 76, 75, 78, 69,
    53, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 42, 83, 84, 85, 86, 43, 20,
    26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, 49, 95, 96, 97, 87, 57, 4, 22, 7, 9,
    10, 11, 13, 14, 15, 51, 52, 40, 92, 93, 94, 225, 29, 27, 6, 25, 5, 17, 16, 54, 55,
    56, 82, 229, 89, 90, 91, 88, 224, 240, 226, 44, 100, 230, 227, 80, 81, 228, 79,
    98, 99, 50, 74, 77
};

const keyboard_keymap_overlay_values steelseries_aps_klc_1122_default_layout
{
    KEYBOARD_SIZE::KEYBOARD_SIZE_FULL,
    {
        steelseries_aps_klc_1122_default_ids_order,
        {
        /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                                  |
        |   Zone,   Row,    Column,     Value,      Key,                          Alt name       OpCode,                             |
        \*--------------------------------------------------------------------------------------------------------------------------*/
        {0,      0,      1,           0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // shift fn keys
        {0,      0,      13,          0,         KEY_EN_PRINT_SCREEN,         KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      14,          0,         KEY_EN_SCROLL_LOCK,          KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      15,          0,         KEY_EN_PAUSE_BREAK,          KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      16,          0,         KEY_EN_INSERT,               KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      17,          0,         KEY_EN_DELETE,               KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      18,          0,         KEY_EN_HOME,                 KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      19,          0,         KEY_EN_END,                  KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      0,      20,          0,         KEY_EN_POWER,                KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // replace keys above numpad
        {0,      1,      1,           0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, }, // Shift 2nd row to the right
        {0,      1,      15,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      1,      16,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      2,      1,           0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, }, // Shift 3rd row to the right
        {0,      2,      15,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      2,      16,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      3,      1,           0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, }, // Shift 4th row to the right
        {0,      3,      15,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      3,      16,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      4,      16,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      5,      15,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      5,      16,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Remove extra space
        {0,      4,      1,           0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, }, // Shift 4th row to the right
        {0,      4,      13,          0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT, },  // Set arrow keys
        {0,      4,      15,          0,         KEY_EN_UP_ARROW,             KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set arrow keys
        {0,      5,      14,          0,         KEY_EN_LEFT_ARROW,           KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set arrow keys
        {0,      5,      15,          0,         KEY_EN_DOWN_ARROW,           KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set arrow keys
        {0,      5,      16,          0,         KEY_EN_RIGHT_ARROW,          KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, }, // Set arrow keys
        {0,      5,      1,           0,         KEY_EN_LEFT_FUNCTION,        KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set FN key
        {0,      5,      10,          0,         KEY_EN_BACK_SLASH,           KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set backslash key
        {0,      5,      11,          0,         KEY_EN_RIGHT_ALT,            KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set right alt key
        {0,      5,      12,          0,         KEY_EN_RIGHT_WINDOWS,        KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },          // Set right windows key
    }
};

/*-----------------------------------------------------*\
| Values are the ids to be sent in buffers              |
\*-----------------------------------------------------*/
const std::vector<unsigned int> steelseries_aps_klc_1122_0x21_values =
    {
    /*  0      1     2     3    4     5     6     7     8     9     10    11   12    13    14    15    16    17*/

    /* ESC          F1    F2    F3   F4          F5    F6    F7    F8    F9   F10   F11   F12   PRSC  SCLK  PSBK */
        41,         58,   59,   60,  61,         62,   63,   64,    65,   66,  67,   68,   69,   70,   71,   72,
     /* §     1     2     3     4    5     6     7     8     9     0     +    `           BSPC  INS   HOME  PGUP */
        53,   30,   31,   32,   33,  34,   35,   36,   37,   38,   39,   45,   46,         42,   73,   74,   75,
    /* TAB          Q     W     E    R     T     Y     U     I     O     P    Å     ^     *     DEL   END   PGDN */
        43,         20,   26,   8,   21,   23,   28,   24,   12,   18,   19,  47,   48,   49,   76,   77,   78,
    /* CPLK         A     S     D    F     G     H     J     K     L     Ø    Æ           ENTR                   */
        57,         4,    22,   7,   9,    10,   11,   13,   14,   15,   51,  52,         40,
    /* LSFT         Z     X     C    V     B     N     M     ;     :     _                RSFT        ARWU       */
        225,        29,   27,   6,   25,   5,    17,   16,   54,   55,   56,              229,         82,
    /* LCTL   LWIN  LALT                   SPC                      >    ALTGR      FN    RCTL  ARWL  ARWD  ARWR */
        224,  227,  226,                   44,                    100,   230,       240,  228,  80,  81,  79,
};

const keyboard_keymap_overlay_values steelseries_aps_klc_1122_0x21_layout =
{
    KEYBOARD_SIZE::KEYBOARD_SIZE_TKL,
    {
        steelseries_aps_klc_1122_0x21_values,
        {
        /* Add more regional layout fixes here */
        }
    },
    {
    /*--------------------------------------------------------------------------------------------------------------------------*\
    | Edit Keys                                                                                                                  |
    |   Zone,   Row,    Column,     Value,      Key,                          Alt name       OpCode,                             |
    \*--------------------------------------------------------------------------------------------------------------------------*/
        {0,      0,      6,            0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {0,      1,     13,            0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {0,      2,      1,            0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {0,      3,      1,            0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {0,      4,     15,            0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {0,      5,     12,            0,         KEY_EN_UNUSED,               KEY_EN_UNUSED, KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {0,      1,      0,           53,         KEY_NORD_HALF,               KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      1,     11,           45,         KEY_NORD_PLUS_QUESTION,      KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      1,     12,           46,         KEY_EN_BACK_TICK,            KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      2,     12,           47,         KEY_NORD_AAL,                KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      2,     13,           48,         KEY_JP_CHEVRON,              KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      2,     14,           49,         KEY_FR_ASTERIX,              KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      3,     11,           51,         KEY_NORD_OE_O,               KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      3,     12,           52,         KEY_NORD_AE_A,               KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      4,      9,           54,         KEY_EN_SEMICOLON,            KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      4,     10,           55,         KEY_JP_COLON,                KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      4,     11,           56,         KEY_FR_UNDERSCORE,           KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      5,     10,          100,         KEY_NORD_ANGLE_BRACKET,      KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      5,     11,          230,         KEY_EN_RIGHT_ALT,            KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
        {0,      5,     13,          240,         KEY_EN_RIGHT_FUNCTION,       KEY_EN_UNUSED, KEYBOARD_OPCODE_SWAP_ONLY, },
    }
};


#endif // STEELSERIESAPSKLCLAYOUTS_H
