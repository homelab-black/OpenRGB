/*---------------------------------------------------------*\
| NZXTFunctionKeyboardDevices.cpp                                  |
|                                                           |
|   Device list for Clevo per-key RGB keyboards (ITE 8291)  |
|                                                           |
|   Kyle Cascade (kyle@cascade.family)          21 Jan 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "NZXTFunctionKeyboardDevices.h"

layout_values nzxt_function_keyboard_offset_values =
    {
        {
            /* ESC          F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12   PRSC  SCLK  PSBK                         */
            0,         1,   2,   3,   4,   5,   6,   7,   8,   9,   10,   11,   12,   13,  14,  115,
            /* BKTK    1     2     3     4     5     6     7     8     9     0     -     =   BSPC  INS   HOME  PGUP  NLCK   NP/   NP*  NP-  */
            16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   30,   46,   126,   99,    100,   101,   102,  103,
            /* TAB     Q     W     E     R     T     Y     U     I     O     P     [     ]     \   DEL   END   PGDN   NP7   NP8   NP9   NP+ */
            32,   33,   34,   35,   36,   37,   38,   39,   40,   42,   42,   43,   44,   62,   62,   104,   105,    106,   97,   113,  114,
            /* CPLK    A     S     D     F     G     H     J     K     L     ;     "     #   ENTR                     NP4   NP5   NP6       */
            48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,    60,   61,                      116,   117,   118,
            /* LSFT    \     Z     X     C     V     B     N     M     ,     .     /   RSFT             ARWU          NP1   NP2   NP3  NPEN */
            64,    65,   66,   67,    68,    69,   70,    71,    72,   73,   74,   75,   77,               78,          119,   120,   121,  123,
            /* LCTL  LWIN  LALT               SPC              RALT  RFNC  RMNU  RCTL              ARWL ARWD   ARWR   NP0         NP.       */
            80,    81,    82,                85,                89,    90,   91,    92,               93,   94,   110,    124,         122
        },
        {
            /* Add more regional layout fixes here */
        }
};
