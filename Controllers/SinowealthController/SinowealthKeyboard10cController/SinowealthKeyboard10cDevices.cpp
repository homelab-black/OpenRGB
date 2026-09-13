/*---------------------------------------------------------*\
| SinowealthKeyboard10cDevices.cpp                          |
|                                                           |
|   Device list for Sinowealth Keyboards with PID 010C      |
|                                                           |
|   Rodrigo Tavares                             27 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "SinowealthKeyboard10cDevices.h"
#include "KeyboardLayoutManager.h"
#include "RGBControllerKeyNames.h"

/*-------------------------------------------------------------------------*\
|  KEYMAPS                                                                  |
\*-------------------------------------------------------------------------*/
const keyboard_keymap_overlay_values aula_f99_layout
{
    KEYBOARD_SIZE_FULL,
    {
        {
         /* ESC          F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12         NULL  NULL  NULL    DEL HOME END PGUP PGDN */
            0,            6,   12,   18,   24,   30,   36,   42,   48,   54,   60,   66,   72,           0,   0,    0,
         /* BKTK          1     2     3     4     5     6     7     8     9     0     -     =   BSPC  NULL  NULL  NULL   NMLK  NMDV  NMTM  NMMI  */
            1,            7,   13,   19,   25,   31,   37,   43,   49,   55,   61,   67,   73,   79,     0,    0,    0,   91,   97,  103,  109,
         /* TAB           Q     W     E     R     T     Y     U     I     O     P     [     ]     \   NULL  NULL  NULL   NM7   NM8   NM9   NMPL  */
            2,            8,   14,   20,   26,   32,   38,   44,   50,   56,   62,   68,   74,   80,     0,    0,    0,   92,   98,  104,  110,
         /* CPLK          A     S     D     F     G     H     J     K     L     ;     "     #   ENTR                     NM4   NM5   NM6         */
            3,            9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,    0,   81,                      93,   99,  105,
         /* LSFT  ISO\    Z     X     C     V     B     N     M     ,     .     /   RSFT                    ARWU         NM1   NM2   NM3   NMER  */
            4,      0,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   82,                     88,          94,  100,  106,  112,
         /* LCTL       LWIN   LALT              SPC               RALT  RFNC  RMNU  RCTL              ARWL  ARWD  ARWR         NM0   NMPD        */
            5,           11,   17,               35,               53,   59,   0,    65,                83,  89,   95,         101,  107,
        },
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                            |
        |   Zone,   Row,    Column,    Value,    Key,                 Alternate Name,      OpCode,                             |
        \*--------------------------------------------------------------------------------------------------------------------*/
        {   0,      0,      14,         78,      KEY_EN_DELETE,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Replace Print w/ Delete
        {   0,      0,      15,         90,      KEY_EN_HOME,         KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Replace ScrLk w/ Home
        {   0,      0,      16,         96,      KEY_EN_END,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Replace Pause w/ End
        {   0,      0,      17,        102,      KEY_EN_PAGE_UP,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgUp
        {   0,      0,      18,        108,      KEY_EN_PAGE_DOWN,    KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgDn

        {   0,      1,      14,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Remove Insert
        {   0,      1,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Home
        {   0,      1,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove PgUp

        {   0,      2,       1,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Padding after Tab
        {   0,      2,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Delete
        {   0,      2,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove End
        {   0,      2,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove PgDn

        {   0,      3,       1,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Padding after CapsLk
        {   0,      3,      13,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove extra key
        {   0,      3,      14,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
        {   0,      3,      14,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
        {   0,      4,      12,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove extra key
        {   0,      4,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
        {   0,      5,      12,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Menu
        {   0,      5,      16,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
    }
};


const keyboard_keymap_overlay_values aula_f75_layout
{
    KEYBOARD_SIZE_SEVENTY_FIVE,
    {
        {
         /* ESC                F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12 */
            0,                 12,   18,   24,   30,   36,   42,   48,   54,   60,   66,   72,   78,
         /* BKTK          1     2     3     4     5     6     7     8     9     0     -     =   BSPC  DEL   */
            1,            7,   13,   19,   25,   31,   37,   43,   49,   55,   61,   67,   73,   79,
         /* TAB           Q     W     E     R     T     Y     U     I     O     P     [     ]     \   PGUP  */
            2,            8,   14,   20,   26,   32,   38,   44,   50,   56,   62,   68,   74,   80,
         /* CPLK          A     S     D     F     G     H     J     K     L     ;     "     #   ENTR  PGDN  */
            3,            9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,    0,   81,
         /* LSFT  ISO\    Z     X     C     V     B     N     M     ,     .     /   RSFT        ARWU  END   */
            4,      0,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   70,
         /* LCTL        LWIN  LALT              SPC         RALT  RFNC  RMNU  RCTL        ARWL  ARWD  ARWR  */
            5,           11,   17,               35,          0,   53,    0,   59,
        },
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                            |
        |   Zone,   Row,    Column,    Value,    Key,                 Alternate Name,      OpCode,                             |
        \*--------------------------------------------------------------------------------------------------------------------*/
        {   0,      0,       0,         0,       KEY_EN_ESCAPE,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY, },           // Insert ESC
        {   0,      1,      14,        85,       KEY_EN_DELETE,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Delete
        {   0,      2,      14,        86,       KEY_EN_PAGE_UP,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgUp
        {   0,      3,      14,        87,       KEY_EN_PAGE_DOWN,    KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgDn

        {   0,      4,      12,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove RShift gap
        {   0,      4,      13,        82,       KEY_EN_UP_ARROW,     KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert UpArrow
        {   0,      4,      14,        88,       KEY_EN_END,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert End

        {   0,      5,      10,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove RAlt
        {   0,      5,      11,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove RMenu

        {   0,      5,      12,        77,       KEY_EN_LEFT_ARROW,   KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert LeftArrow
        {   0,      5,      13,        83,       KEY_EN_DOWN_ARROW,   KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert DownArrow
        {   0,      5,      14,        89,       KEY_EN_RIGHT_ARROW,  KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert RightArrow

    }
};


const keyboard_keymap_overlay_values aula_f87_layout
{
    KEYBOARD_SIZE_TKL,
    {
        {
            /* ESC              F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12    PRNT  SCRL  PAUSE */
                0,              12,   18,   24,   30,   36,   42,   48,   54,   60,   66,   72,   78,    84,   90,   96,
            /* BKTK        1     2     3     4     5     6     7     8     9     0     -     =   BSPC   INSRT HOME  PGUP  */
                1,         7,   13,   19,   25,   31,   37,   43,   49,   55,   61,   67,   73,   79,    85,   91,   97,
            /* TAB         Q     W     E     R     T     Y     U     I     O     P     [     ]     \     DEL   END  PGDN  */
                2,         8,   14,   20,   26,   32,   38,   44,   50,   56,   62,   68,   74,   80,    86,   92,   98,
            /* CPLK        A     S     D     F     G     H     J     K     L     ;     "     #   ENTR                     */
                 3,        9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,    0,   81,
            /* LSFT ISO\   Z     X     C     V     B     N     M     ,     .     /               RSFT         ARWU        */
                 4,  0,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,               82,          94,
            /* LCTL       LWIN  LALT              SPC               RALT  RFNC  RMNU             RCTL   ARWL  ARWD  ARWR  */
                5,        11,   17,               35,               53,   59,   65,               83,    89,   95,  101
        },
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                            |
        |   Zone,   Row,    Column,    Value,    Key,                 Alternate Name,      OpCode,                             |
        \*--------------------------------------------------------------------------------------------------------------------*/
        {   0,      4,      12,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },   // Remove RShift gap
        {   0,      4,      14,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },   // Add gap after RShift
    }
};


const keyboard_keymap_overlay_values leobog_hi75c_pro_layout
{
    KEYBOARD_SIZE_SEVENTY_FIVE,
    {
        {
         /* ESC                F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12 */
            0,                 12,   18,   24,   30,   36,   42,   48,   54,   60,   66,   72,   78,
         /* BKTK          1     2     3     4     5     6     7     8     9     0     -     =   BSPC  DEL   */
            1,            7,   13,   19,   25,   31,   37,   43,   49,   55,   61,   67,   73,   79,
         /* TAB           Q     W     E     R     T     Y     U     I     O     P     [     ]     \   END   */
            2,            8,   14,   20,   26,   32,   38,   44,   50,   56,   62,   68,   74,   80,
         /* CPLK          A     S     D     F     G     H     J     K     L     ;     "     #   ENTR  PGUP  */
            3,            9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,    0,   81,
         /* LSFT  ISO\    Z     X     C     V     B     N     M     ,     .     /   RSFT        ARWU  PGDN  */
            4,      0,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   70,
         /* LCTL        LWIN  LALT              SPC         RALT  RFNC  RMNU  RCTL        ARWL  ARWD  ARWR  */
            5,           11,   17,               35,         53,   59,    0,   65,
        },
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                            |
        |   Zone,   Row,    Column,    Value,    Key,                 Alternate Name,      OpCode,                             |
        \*--------------------------------------------------------------------------------------------------------------------*/
        {   0,      0,       0,         0,       KEY_EN_ESCAPE,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY, },           // Insert ESC
        {   0,      1,      14,        85,       KEY_EN_DELETE,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Delete
        {   0,      2,      14,        86,       KEY_EN_END,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert End
        {   0,      3,      14,        87,       KEY_EN_PAGE_UP,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgUp
        {   0,      4,      14,        88,       KEY_EN_PAGE_DOWN,    KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgDn

        {   0,      4,      12,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove RShift gap
        {   0,      4,      13,        82,       KEY_EN_UP_ARROW,     KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert UpArrow

        {   0,      5,      12,         0,       KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove RMenu
        {   0,      5,      13,        77,       KEY_EN_LEFT_ARROW,   KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert LeftArrow
        {   0,      5,      14,        83,       KEY_EN_DOWN_ARROW,   KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert DownArrow
        {   0,      5,      15,        89,       KEY_EN_RIGHT_ARROW,  KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert RightArrow
    }
};


const keyboard_keymap_overlay_values redragon_k686_eisa_pro
{
    KEYBOARD_SIZE_FULL,
    {
        {
         /* ESC          F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12         NULL  NULL  NULL    DEL HOME END PGUP PGDN */
            0,            6,   12,   18,   24,   30,   36,   42,   48,   54,   60,   66,   72,           0,   0,    0,
         /* BKTK          1     2     3     4     5     6     7     8     9     0     -     =   BSPC  NULL  NULL  NULL   NMLK  NMDV  NMTM  NMMI  */
            1,            7,   13,   19,   25,   31,   37,   43,   49,   55,   61,   67,   73,   79,     0,    0,    0,   91,   97,  103,  109,
         /* TAB           Q     W     E     R     T     Y     U     I     O     P     [     ]     \   NULL  NULL  NULL   NM7   NM8   NM9   NMPL  */
            2,            8,   14,   20,   26,   32,   38,   44,   50,   56,   62,   68,   74,   80,     0,    0,    0,   92,   98,  104,  110,
         /* CPLK          A     S     D     F     G     H     J     K     L     ;     "     #   ENTR                     NM4   NM5   NM6         */
            3,            9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,    0,   81,                      93,   99,  105,
         /* LSFT  ISO\    Z     X     C     V     B     N     M     ,     .     /   RSFT                    ARWU         NM1   NM2   NM3   NMER  */
            4,      0,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   82,                     88,          94,  100,  106,  112,
         /* LCTL       LWIN   LALT              SPC               RALT  RFNC  RMNU  RCTL              ARWL  ARWD  ARWR         NM0   NMPD        */
            5,           11,   17,               35,               53,   59,   0,    65,                83,  89,   95,         101,  107,
        },
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                            |
        |   Zone,   Row,    Column,    Value,    Key,                 Alternate Name,      OpCode,                             |
        \*--------------------------------------------------------------------------------------------------------------------*/
        {   0,      0,      14,         78,      KEY_EN_DELETE,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Replace Print w/ Delete
        {   0,      0,      15,         90,      KEY_EN_INSERT,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Replace ScrLk w/ Insert
        {   0,      0,      16,         96,      KEY_EN_END,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Replace Pause w/ End
        {   0,      0,      17,        102,      KEY_EN_PAGE_UP,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgUp
        {   0,      0,      18,        108,      KEY_EN_PAGE_DOWN,    KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert PgDn
        {   0,      2,      13,         75,      KEY_EN_POUND,        KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Insert ] } Key
        {   0,      4,       1,         76,      KEY_EN_ISO_BACK_SLASH, KEY_EN_UNUSED,     KEYBOARD_OPCODE_SWAP_ONLY,          },  // Insert \ | Key

        {   0,      1,      14,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_SWAP_ONLY,          },  // Remove Insert
        {   0,      1,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Home
        {   0,      1,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove PgUp

        {   0,      2,       1,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Padding after Tab
        {   0,      2,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Delete
        {   0,      2,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove End
        {   0,      2,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove PgDn

        {   0,      3,       1,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Padding after CapsLk
        {   0,      3,      13,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove extra key
        {   0,      3,      14,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
        {   0,      3,      14,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
        {   0,      4,      12,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove extra key
        {   0,      4,      15,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
        {   0,      5,      12,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Menu
        {   0,      5,      16,          0,      KEY_EN_UNUSED,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove padding
    }
};


/*-------------------------------------------------------------------------*\
|  LEOBOG K21 / SOLAKAKA K21 21-key numpad                                  |
|                                                                           |
|  LED indexes match SignalRGB's Sinowealth Numpad layout (model ID 0x4E).  |
|                                                                           |
|    Esc   Tab   Bksp  Fn                                                   |
|    NL    /     *     -                                                    |
|    7     8     9     +                                                    |
|    4     5     6                                                          |
|    1     2     3     Enter                                                |
|    0           .                                                          |
\*-------------------------------------------------------------------------*/
const keyboard_keymap_overlay_values leobog_k21_layout
{
    KEYBOARD_SIZE_EMPTY,
    {
        {
            /* Built from edit_keys */
        },
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*--------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                            |
        |   Zone,   Row,    Column,    Value,    Key,                      Alternate Name,      OpCode,                        |
        \*--------------------------------------------------------------------------------------------------------------------*/
        {   0,      0,      0,        102,       KEY_EN_ESCAPE,            KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      0,      1,        108,       KEY_EN_TAB,               KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      0,      2,        114,       KEY_EN_BACKSPACE,         KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      0,      3,        120,       KEY_EN_RIGHT_FUNCTION,    KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },

        {   0,      1,      0,        103,       KEY_EN_NUMPAD_LOCK,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      1,      1,        109,       KEY_EN_NUMPAD_DIVIDE,     KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      1,      2,        115,       KEY_EN_NUMPAD_TIMES,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      1,      3,        121,       KEY_EN_NUMPAD_MINUS,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },

        {   0,      2,      0,        104,       KEY_EN_NUMPAD_7,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      2,      1,        110,       KEY_EN_NUMPAD_8,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      2,      2,        116,       KEY_EN_NUMPAD_9,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      2,      3,        122,       KEY_EN_NUMPAD_PLUS,       KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },

        {   0,      3,      0,        105,       KEY_EN_NUMPAD_4,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      3,      1,        111,       KEY_EN_NUMPAD_5,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      3,      2,        117,       KEY_EN_NUMPAD_6,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },

        {   0,      4,      0,        106,       KEY_EN_NUMPAD_1,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      4,      1,        112,       KEY_EN_NUMPAD_2,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      4,      2,        118,       KEY_EN_NUMPAD_3,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      4,      3,        124,       KEY_EN_NUMPAD_ENTER,      KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },

        {   0,      5,      0,        107,       KEY_EN_NUMPAD_0,          KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      5,      2,        119,       KEY_EN_NUMPAD_PERIOD,     KEY_EN_UNUSED,       KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
    }
};


/*-------------------------------------------------------------------------*\
|  DEVICE MODEL MAPPING                                                     |
\*-------------------------------------------------------------------------*/
const sinowealth_device_map sinowealth_10c_keyboards{
    {
        0xCD, { "AULA F75", aula_f75_layout },
    },
    {
        0xA4, { "AULA F99", aula_f99_layout },
    },
    {
        0x0B, { "AULA F87 Pro", aula_f87_layout },
    },
    {
        0xA3, { "LEOBOG Hi75C Pro", leobog_hi75c_pro_layout },
    },
    {
        0x4E, { "LEOBOG K21", leobog_k21_layout },
    },
    {
        0x20, { "Redragon K686 Eisa Pro", redragon_k686_eisa_pro },
    },
    {
        0x05, { "Redragon K686 Eisa Pro", redragon_k686_eisa_pro },
    },
};
