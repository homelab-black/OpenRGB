/*---------------------------------------------------------*\
| EVisionKeyboardLayouts.h                                  |
|                                                           |
|   Per-model key layouts for EVision keyboards whose LED   |
|   arrangement differs from the generic EVision matrix     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBControllerKeyNames.h"

/*---------------------------------------------------------*\
| One physical key: matrix position, index into the device  |
| color buffer (3 bytes per index), and key name            |
\*---------------------------------------------------------*/
typedef struct
{
    unsigned char       row;
    unsigned char       col;
    unsigned char       index;
    const char*         name;
} evision_keyboard_key;

typedef struct
{
    const char*                 vendor;
    const char*                 description;
    unsigned int                rows;
    unsigned int                cols;
    unsigned int                key_count;
    const evision_keyboard_key* keys;
    /*-----------------------------------------------------*\
    | Follow the vendor software: wrap every transfer in    |
    | Begin (0x01) / End (0x02) packets, check each reply   |
    | and retry packets the keyboard did not acknowledge    |
    \*-----------------------------------------------------*/
    bool                        use_begin_end;
    /*-----------------------------------------------------*\
    | Highest brightness value, 0 = EVision default         |
    \*-----------------------------------------------------*/
    unsigned int                brightness_max;
} evision_keyboard_layout;

/*---------------------------------------------------------*\
| Sharkoon Skiller Mech SGK3 (0C45:8513), full size ISO     |
|                                                           |
|   LED indices were taken from the key-to-LED table in     |
|   the Sharkoon "SKILLER MECH SGK3" software (US key       |
|   identities) and cross-checked against its per-key       |
|   profile files, which mark the 21 unused slots below     |
|   126. Index 112 (Right Windows) is unused: the SGK3 has  |
|   a Fn key (110) in that position.                        |
\*---------------------------------------------------------*/
static const evision_keyboard_key sharkoon_sgk3_keys[] =
{
    /*-----------------------------------------------------*\
    | Row 0: Esc, F1-F12, PrtSc, ScrLk, Pause               |
    \*-----------------------------------------------------*/
    { 0,  0,   0, KEY_EN_ESCAPE             },
    { 0,  2,   1, KEY_EN_F1                 },
    { 0,  3,   2, KEY_EN_F2                 },
    { 0,  4,   3, KEY_EN_F3                 },
    { 0,  5,   4, KEY_EN_F4                 },
    { 0,  6,   5, KEY_EN_F5                 },
    { 0,  7,   6, KEY_EN_F6                 },
    { 0,  8,   7, KEY_EN_F7                 },
    { 0,  9,   8, KEY_EN_F8                 },
    { 0, 10,   9, KEY_EN_F9                 },
    { 0, 11,  10, KEY_EN_F10                },
    { 0, 12,  11, KEY_EN_F11                },
    { 0, 13,  12, KEY_EN_F12                },
    { 0, 14,  14, KEY_EN_PRINT_SCREEN       },
    { 0, 15,  15, KEY_EN_SCROLL_LOCK        },
    { 0, 16,  16, KEY_EN_PAUSE_BREAK        },

    /*-----------------------------------------------------*\
    | Row 1: number row, Ins/Home/PgUp, numpad top          |
    \*-----------------------------------------------------*/
    { 1,  0,  21, KEY_EN_BACK_TICK          },
    { 1,  1,  22, KEY_EN_1                  },
    { 1,  2,  23, KEY_EN_2                  },
    { 1,  3,  24, KEY_EN_3                  },
    { 1,  4,  25, KEY_EN_4                  },
    { 1,  5,  26, KEY_EN_5                  },
    { 1,  6,  27, KEY_EN_6                  },
    { 1,  7,  28, KEY_EN_7                  },
    { 1,  8,  29, KEY_EN_8                  },
    { 1,  9,  30, KEY_EN_9                  },
    { 1, 10,  31, KEY_EN_0                  },
    { 1, 11,  32, KEY_EN_MINUS              },
    { 1, 12,  33, KEY_EN_EQUALS             },
    { 1, 13,  34, KEY_EN_BACKSPACE          },
    { 1, 14,  35, KEY_EN_INSERT             },
    { 1, 15,  36, KEY_EN_HOME               },
    { 1, 16,  37, KEY_EN_PAGE_UP            },
    { 1, 17,  38, KEY_EN_NUMPAD_LOCK        },
    { 1, 18,  39, KEY_EN_NUMPAD_DIVIDE      },
    { 1, 19,  40, KEY_EN_NUMPAD_TIMES       },
    { 1, 20,  41, KEY_EN_NUMPAD_MINUS       },

    /*-----------------------------------------------------*\
    | Row 2: Tab row, ISO Enter, Del/End/PgDn, numpad 7-9 + |
    \*-----------------------------------------------------*/
    { 2,  0,  42, KEY_EN_TAB                },
    { 2,  1,  43, KEY_EN_Q                  },
    { 2,  2,  44, KEY_EN_W                  },
    { 2,  3,  45, KEY_EN_E                  },
    { 2,  4,  46, KEY_EN_R                  },
    { 2,  5,  47, KEY_EN_T                  },
    { 2,  6,  48, KEY_EN_Y                  },
    { 2,  7,  49, KEY_EN_U                  },
    { 2,  8,  50, KEY_EN_I                  },
    { 2,  9,  51, KEY_EN_O                  },
    { 2, 10,  52, KEY_EN_P                  },
    { 2, 11,  53, KEY_EN_LEFT_BRACKET       },
    { 2, 12,  54, KEY_EN_RIGHT_BRACKET      },
    { 2, 13,  76, KEY_EN_ISO_ENTER          },
    { 2, 14,  56, KEY_EN_DELETE             },
    { 2, 15,  57, KEY_EN_END                },
    { 2, 16,  58, KEY_EN_PAGE_DOWN          },
    { 2, 17,  59, KEY_EN_NUMPAD_7           },
    { 2, 18,  60, KEY_EN_NUMPAD_8           },
    { 2, 19,  61, KEY_EN_NUMPAD_9           },
    { 2, 20,  62, KEY_EN_NUMPAD_PLUS        },

    /*-----------------------------------------------------*\
    | Row 3: Caps row, ISO #, numpad 4-6                    |
    |   The key left of ISO Enter uses the ANSI backslash   |
    |   LED (55)                                            |
    \*-----------------------------------------------------*/
    { 3,  0,  63, KEY_EN_CAPS_LOCK          },
    { 3,  1,  64, KEY_EN_A                  },
    { 3,  2,  65, KEY_EN_S                  },
    { 3,  3,  66, KEY_EN_D                  },
    { 3,  4,  67, KEY_EN_F                  },
    { 3,  5,  68, KEY_EN_G                  },
    { 3,  6,  69, KEY_EN_H                  },
    { 3,  7,  70, KEY_EN_J                  },
    { 3,  8,  71, KEY_EN_K                  },
    { 3,  9,  72, KEY_EN_L                  },
    { 3, 10,  73, KEY_EN_SEMICOLON          },
    { 3, 11,  74, KEY_EN_QUOTE              },
    { 3, 12,  55, KEY_EN_POUND              },
    { 3, 17,  80, KEY_EN_NUMPAD_4           },
    { 3, 18,  81, KEY_EN_NUMPAD_5           },
    { 3, 19,  82, KEY_EN_NUMPAD_6           },

    /*-----------------------------------------------------*\
    | Row 4: Shift row with ISO backslash, Up, numpad 1-3   |
    \*-----------------------------------------------------*/
    { 4,  0,  84, KEY_EN_LEFT_SHIFT         },
    { 4,  1,  85, KEY_EN_ISO_BACK_SLASH     },
    { 4,  2,  86, KEY_EN_Z                  },
    { 4,  3,  87, KEY_EN_X                  },
    { 4,  4,  88, KEY_EN_C                  },
    { 4,  5,  89, KEY_EN_V                  },
    { 4,  6,  90, KEY_EN_B                  },
    { 4,  7,  91, KEY_EN_N                  },
    { 4,  8,  92, KEY_EN_M                  },
    { 4,  9,  93, KEY_EN_COMMA              },
    { 4, 10,  94, KEY_EN_PERIOD             },
    { 4, 11,  95, KEY_EN_FORWARD_SLASH      },
    { 4, 13,  97, KEY_EN_RIGHT_SHIFT        },
    { 4, 15,  99, KEY_EN_UP_ARROW           },
    { 4, 17, 101, KEY_EN_NUMPAD_1           },
    { 4, 18, 102, KEY_EN_NUMPAD_2           },
    { 4, 19, 103, KEY_EN_NUMPAD_3           },
    { 4, 20, 104, KEY_EN_NUMPAD_ENTER       },

    /*-----------------------------------------------------*\
    | Row 5: bottom row, arrows, numpad 0 and period        |
    \*-----------------------------------------------------*/
    { 5,  0, 105, KEY_EN_LEFT_CONTROL       },
    { 5,  1, 106, KEY_EN_LEFT_WINDOWS       },
    { 5,  2, 107, KEY_EN_LEFT_ALT           },
    { 5,  6, 108, KEY_EN_SPACE              },
    { 5, 10, 109, KEY_EN_RIGHT_ALT          },
    { 5, 11, 110, KEY_EN_RIGHT_FUNCTION     },
    { 5, 12, 111, KEY_EN_MENU               },
    { 5, 13, 113, KEY_EN_RIGHT_CONTROL      },
    { 5, 14, 119, KEY_EN_LEFT_ARROW         },
    { 5, 15, 120, KEY_EN_DOWN_ARROW         },
    { 5, 16, 121, KEY_EN_RIGHT_ARROW        },
    { 5, 17, 123, KEY_EN_NUMPAD_0           },
    { 5, 19, 124, KEY_EN_NUMPAD_PERIOD      },
};

static const evision_keyboard_layout sharkoon_sgk3_layout =
{
    "Sharkoon",
    "Sharkoon Skiller Mech SGK3",
    6,
    21,
    sizeof(sharkoon_sgk3_keys) / sizeof(sharkoon_sgk3_keys[0]),
    sharkoon_sgk3_keys,
    true,
    /*-----------------------------------------------------*\
    | The SGK3 stores brightness 5 in its default profiles  |
    \*-----------------------------------------------------*/
    5,
};
