#ifndef MK4100_LAYOUT_H
#define MK4100_LAYOUT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mk4100_key_position {
    const char *name;
    unsigned slot;
    float x;
    float y;
    float width;
    float height;
} mk4100_key_position;

/* The verified JYK200 104M layout: 104 physical LEDs within 126 RGB slots. */
const mk4100_key_position *mk4100_get_key_positions(size_t *count);

#ifdef __cplusplus
}
#endif
#endif
