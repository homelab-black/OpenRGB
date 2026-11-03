/*---------------------------------------------------------*\
| legion9_profile_guard_test.cpp                            |
|                                                           |
|   A strip-only edit has to name all 101 keys, and an      |
|   empty live read must not take the lamp array over.      |
|   Built on its own. It is not part of the OpenRGB binary. |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "Legion9ProfileGuard.h"

static int CountNamedKeys(int direct_static, int host_per_led, const uint32_t* colors, size_t count)
{
    int only_color = 1;
    int named = 0;
    size_t index;

    for(index = 1; index < count; index++)
    {
        if(colors[index] != colors[0])
        {
            only_color = 0;
            break;
        }
    }

    for(index = 0; index < count; index++)
    {
        if(Legion9KeepColorGroup(direct_static, host_per_led, only_color, colors[index]))
        {
            named++;
        }
    }

    return(named);
}

int main()
{
    uint32_t keys[101];
    size_t used;
    size_t overflow;
    int index;

    /* Empty live read and no key edit: do not seize the strip. */
    if(Legion9HaveKeyColors(1, 0) != 0 || Legion9SeizeLampArray(Legion9HaveKeyColors(1, 0)) != 0)
    {
        fprintf(stderr, "empty live read still takes the lamp array\n");
        return(1);
    }

    /* Live colors are present, so a strip-only edit may update the lamps. */
    if(Legion9HaveKeyColors(0, 0) != 1 || Legion9SeizeLampArray(1) != 1)
    {
        fprintf(stderr, "strip edit with live key colors was refused\n");
        return(1);
    }

    /* One black key and 100 white keys. Legion 9 names all 101. */
    keys[0] = 0x000000;

    for(index = 1; index < 101; index++)
    {
        keys[index] = 0xFFFFFF;
    }

    if(CountNamedKeys(1, 1, keys, 101) != 101)
    {
        fprintf(stderr, "strip-only edit dropped a black key\n");
        return(1);
    }

    /* Other keyboards still omit that black key. */
    if(CountNamedKeys(0, 1, keys, 101) != 100)
    {
        fprintf(stderr, "other devices kept a black key they used to drop\n");
        return(1);
    }

    /* 45 one-key groups fit in a 960-byte report. The 46th does not. */
    used = 7;

    for(index = 0; index < 45; index++)
    {
        if(!Legion9GroupFits(960, used, 1, 1, &used))
        {
            fprintf(stderr, "group %d of 45 did not fit\n", index);
            return(1);
        }
    }

    overflow = used;

    if(Legion9GroupFits(960, overflow, 1, 1, &overflow))
    {
        fprintf(stderr, "group 46 fit and would be a second report\n");
        return(1);
    }

    /* 101 keys that share one color fit in that same report. */
    used = 7;

    if(!Legion9GroupFits(960, used, 1, 101, &used))
    {
        fprintf(stderr, "101 keys of one color did not fit\n");
        return(1);
    }

    return(0);
}
