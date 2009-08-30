/* Plasma OCR - an OCR engine
 *
 * thinning.c - morphological thinning, thickening and skeletonization
 *
 * Copyright (C) 2006  Ilya Mezhirov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "bitmaps.h"
#include "thinning.h"
#include <stdlib.h>
#include <string.h>


/* Indices into tables are donuts of 8 bits
 * (see comments for get_table_value()).
 *
 * The mark table gives 1 for the following donuts:
 *
 *   ?0?  ?10  ?0?  ?1?  ?0?
 *   1 1  ? 1  0 0  1 1  0 0
 *   ?0?  ???  ?0?  ?1?  ?10
 *
 * and all of their reflections and mirrorings. `?' is a wildcard.
 */

static unsigned char mark_table[32] = {
    127, 127, 255, 255, 127, 127, 255, 204,
     63,  15, 255,  12,  63,  15, 255,  12,
    119, 119, 255, 255,  85,  85,  68,  68,
    127, 127, 255, 255, 127, 127, 255, 204
};


/* The sweep table gives 1 for the first 4 of 5 donut types shown for the mark table.
 */
static unsigned char sweep_table[32] = {
     51,  51, 204, 204,  51,  51, 204, 204,
     12,  12, 255,  12,  12,  12, 255,  12,
     68,  68, 255, 255,  68,  68,  68,  68,
    127, 127, 255, 255, 127, 127, 255, 204
};


/* The final touch table returns 0 only on donuts of this type:
 *
 *  01?
 *  1 0
 *  ?00
 *
 */
static unsigned char final_touch_table[32] = {
    255, 255, 255, 255, 255, 255, 255, 255,
    243, 243, 238, 255, 255, 255, 238, 255,
    187, 255, 252, 252, 187, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
};


/* Tables are organized as follows.
 * Each table stores 256 bits.
 * Consider a donut-shaped array of 8 bits:
 *
 *    A B C
 *    D   E
 *    F G H
 *
 * It is an index to the table.
 * The table says 1 iff table[EDCBA] & (1 << FGH) is nonzero.
 */

static int get_table_value(unsigned char *table, unsigned char **prow, int x)
{
    int i, shift;
    unsigned char *upper = prow[-1];
    unsigned char *row   = *prow;
    unsigned char *lower = prow[1];

    i  = upper[x - 1];
    i += upper[x]     << 1;
    i += upper[x + 1] << 2;
    i += row[x - 1]   << 3;
    i += row[x + 1]   << 4;

    shift  = lower[x - 1] << 2;
    shift += lower[x]     << 1;
    shift += lower[x + 1];

    return table[i] & (1 << shift);
}


/* Mark phase. For each pixel, a donut of 8 neighbors is considered.
 * If the mark table says 0 on the donut, then the pixel is a candidate for sweeping.
 */
static void mark(unsigned char **pixels, unsigned char **candidates, int w, int h)
{
    int x, y;

    for (y = 0; y < h; y++)
    {
        unsigned char **prow = pixels + y;
        unsigned char *row = *prow;
        unsigned char *candidates_row = candidates[y];

        for (x = 0; x < w; x++) if (row[x])
        {
            candidates_row[x] = get_table_value(mark_table, prow, x);
        }
    }
}


/* Sweep phase. If a pixel is a candidate for deletion
 * and its donut of neighbors scores 0 in the sweep table,
 * then the pixel is set to white.
 * Note that a cleaned pixel may affect subsequent sweeping.
 *
 * Returns 1 if the image has changed, 0 otherwise.
 */
static int sweep(unsigned char **pixels, unsigned char **candidates, int w, int h)
{
    int x, y;
    int result = 0;

    for (y = 0; y < h; y++)
    {
        unsigned char **prow = pixels + y;
        unsigned char *row = *prow;
        unsigned char *candidates_row = candidates[y];

        for (x = 0; x < w; x++)
        {
            if (row[x] && !candidates_row[x]
             && !get_table_value(sweep_table, prow, x))
            {
                row[x] = 0;
                result = 1;
            }
        }
    }
    return result;
}


/* Final touch. For each pixel, a donut of 8 neighbors is considered.
 * If the final touch table says 0 on the donut, then the pixel is cleaned.
 */
static void force_8_connectivity(unsigned char **pixels, int w, int h)
{
    int x, y;

    for (y = 0; y < h; y++)
    {
        unsigned char **prow = pixels + y;
        unsigned char *row = *prow;

        for (x = 0; x < w; x++) if (row[x])
        {
            row[x] = ( get_table_value(final_touch_table, prow, x) ? 1 : 0 );
        }
    }
}


int peel(unsigned char **pixels, unsigned char **buffer, int w, int h)
{
    mark(pixels, buffer, w, h);
    return sweep(pixels, buffer, w, h);
}


unsigned char **thin(unsigned char **pixels, int w, int h, int N)
{
    unsigned char **result = provide_margins(pixels, w, h, /* make_it_0_or_1: */ 1);
    unsigned char **buffer = allocate_bitmap(w, h);

    while (N--)
    {
        if (!peel(result, buffer, w, h))
            break;
    }
    
    free_bitmap(buffer);
    return result;
}


/* (see thinning.h) */
unsigned char **skeletonize_internal(unsigned char **pixels, int w, int h,
                                     int use_8_connectivity, int make_it_0_or_1)
{
    unsigned char **buffer = provide_margins(pixels, w, h, make_it_0_or_1);

    if (!buffer)
        return NULL;

    /* The main peeling cycle.
     *
     * Note that `pixels' and `buffer' are swapped:
     * we use buffer as the main pixel array,
     * and the original pixels array is used to mark candidates for sweeping.
     */
    while (peel(buffer, pixels, w, h)) {}

    if (use_8_connectivity)
        force_8_connectivity(buffer, w, h);

    return buffer;
}


unsigned char **skeletonize(unsigned char **pixels, int w, int h, int use_8_connectivity)
{
    unsigned char **buffer = copy_bitmap(pixels, w, h);
    unsigned char **result = skeletonize_internal(buffer, w, h, use_8_connectivity, 1);
    free_bitmap(buffer);
    return result;
}


unsigned char **thicken(unsigned char **pixels, int w, int h, int N)
{
    int r_w = w + (N + 1) * 2;
    int r_h = h + (N + 1) * 2;
    unsigned char **aux = allocate_bitmap(w + N * 2, h + N * 2);
    unsigned char **buf = allocate_bitmap(r_w, r_h);
    unsigned char **pbuf = MALLOC(unsigned char *, r_h);

    int y, x;
    clear_bitmap(buf, r_w, r_h);

    for (y = 0; y < r_h; y++)
        pbuf[y] = buf[y] + 1;
    pbuf++;

    for (y = 0; y < h; y++) for (x = 0; x < w; x++)
    {
        buf[y + N + 1][x + N + 1] = pixels[y][x];
    }

    invert_bitmap(buf, r_w, r_h, 1); /* we get margins of 2 black pixels */
    while (N--)
    {
        if (!peel(pbuf, aux, w + N * 2, h + N * 2))
            break;
    }
    invert_bitmap(buf, r_w, r_h, 0);

    free_bitmap(aux);
    FREE(buf);
    return pbuf;
}
