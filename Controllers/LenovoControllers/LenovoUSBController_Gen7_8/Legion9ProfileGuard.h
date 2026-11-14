/*---------------------------------------------------------*\
| Legion9ProfileGuard.h                                     |
|                                                           |
|   Decisions for the Legion 9 18IAX10 per-key save.        |
|   A missed key, or a lamp-array update that leaves the    |
|   key lamps out, comes back as the firmware blue.         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <stddef.h>
#include <stdint.h>

/*---------------------------------------------------------*\
| Group on the wire: index, pair count, six setting pairs, |
| color count, RGB triples, LED count, uint16 LED ids.     |
| 16 bytes plus 3 per color plus 2 per LED.                |
\*---------------------------------------------------------*/
static inline int Legion9GroupFits(size_t packet_size, size_t used, size_t color_count, size_t led_count, size_t* used_after)
{
    size_t bytes;

    if(color_count > 255 || led_count > 255)
    {
        return(0);
    }

    bytes = 16 + color_count * 3 + led_count * 2;

    if(used + bytes > packet_size)
    {
        return(0);
    }

    *used_after = used + bytes;
    return(1);
}

/*---------------------------------------------------------*\
| An empty live image and no key edit means the UI buffer  |
| is still black. Applying it is what turns the keys blue. |
\*---------------------------------------------------------*/
static inline int Legion9HaveKeyColors(int live_empty, int any_key_edit)
{
    if(live_empty && !any_key_edit)
    {
        return(0);
    }

    return(1);
}

/* Host control without the key lamps turns those lamps blue. */
static inline int Legion9SeizeLampArray(int preserve_key_lamps)
{
    return(preserve_key_lamps ? 1 : 0);
}

/*---------------------------------------------------------*\
| Return 1 when this color's keys belong in the profile.   |
| Legion 9 still names black keys. Other devices drop a    |
| black group when another color is present.               |
\*---------------------------------------------------------*/
static inline int Legion9KeepColorGroup(int direct_uses_static_profile, int host_per_led, int only_color, uint32_t color)
{
    if(color != 0x00)
    {
        return(1);
    }

    if(only_color)
    {
        return(1);
    }

    if(direct_uses_static_profile && host_per_led)
    {
        return(1);
    }

    return(0);
}
