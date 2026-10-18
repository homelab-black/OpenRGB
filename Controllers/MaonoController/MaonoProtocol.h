/*---------------------------------------------------------*\
| MaonoProtocol.h                                           |
|                                                           |
|   Register frame used by every Maono Link device plugin   |
|                                                           |
|   [RID]    HID report ID, only on devices that use one    |
|   [0]      0xC4 frame start                               |
|   [1]      frame length, counted from 0xC4 to checksum    |
|   [2..3]   0x00 0x00                                      |
|   [4]      frame type: 0x03 write register(s)             |
|                        0x04 read register                 |
|                        0x05 write register block          |
|                        0x06 read register block           |
|   [5..6]   register address, little endian                |
|   [7..8]   value (write) / count (block read), LE         |
|   [9..10]  checksum, LE: 0x10000 - sum(bytes [0..8])      |
|   ...      zero padding to the report size                |
|                                                           |
|   Reverse engineered from Maono Link 4.0.88 with REA      |
|   (Ghidra 12.1.4), see docs/PROTOCOL.md                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstring>

#define MAONO_MAX_REPORT_SIZE           65
#define MAONO_FRAME_START               0xC4
#define MAONO_FRAME_TYPE_WRITE          0x03
#define MAONO_FRAME_TYPE_READ           0x04
#define MAONO_FRAME_TYPE_WRITE_BLOCK    0x05
#define MAONO_FRAME_TYPE_READ_BLOCK     0x06

/*---------------------------------------------------------*\
| Builds one frame exactly like the Maono Link plugin for   |
| that device and returns the number of bytes to write.     |
|   report_id < 0  : no report ID byte, buffer starts with  |
|                    0xC4 (Maono Link writes 64 bytes)      |
|   report_id >= 0 : buffer starts with the report ID       |
|                    (Maono Link writes 65 bytes)           |
\*---------------------------------------------------------*/
static inline int MaonoBuildFrame(unsigned char* buf, int report_id, unsigned char type, unsigned short reg, unsigned short value)
{
    memset(buf, 0x00, MAONO_MAX_REPORT_SIZE);

    int base = 0;

    if(report_id >= 0)
    {
        buf[0] = (unsigned char)report_id;
        base   = 1;
    }

    buf[base + 0]  = MAONO_FRAME_START;
    buf[base + 1]  = 0x0B;
    buf[base + 2]  = 0x00;
    buf[base + 3]  = 0x00;
    buf[base + 4]  = type;
    buf[base + 5]  = (unsigned char)(reg & 0xFF);
    buf[base + 6]  = (unsigned char)(reg >> 8);
    buf[base + 7]  = (unsigned char)(value & 0xFF);
    buf[base + 8]  = (unsigned char)(value >> 8);

    unsigned short sum = 0;

    for(int i = 0; i <= 8; i++)
    {
        sum = (unsigned short)(sum + buf[base + i]);
    }

    unsigned short checksum = (unsigned short)(0x10000 - sum);

    buf[base + 9]  = (unsigned char)(checksum & 0xFF);
    buf[base + 10] = (unsigned char)(checksum >> 8);

    return(base + 64);
}
