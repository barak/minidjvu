#include "bitmaps.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


unsigned char **allocate_bitmap(int w, int h)
{
    unsigned char *data, **result;
    int i;

    assert(w > 0 && h > 0);

    data = MALLOC(unsigned char, w * h);
    if (!data) return NULL;

    result = MALLOC(unsigned char *, h);
    if (!result)
    {
        FREE(data);
        return NULL;
    }

    for (i = 0; i < h; i++)
    {
        result[i] = data + w * i;
    }

    return result;
}


void free_bitmap(unsigned char **p)
{
    assert(p);
    FREE(p[0]);
    FREE(p);
}


unsigned char **allocate_bitmap_with_margins(int w, int h)
{
    unsigned char **result = allocate_bitmap(w + 2, h + 2);
    int y;

    if (!result)
        return NULL;

    /* set the `result' array so that it points to buffer rows (plus margin) */
    for (y = 0; y < h + 2; y++)
    {
        result[y]++;
    }
    result++; /* now the first byte in the buffer is result[-1][-1] */
    return result;
}


/* Allocate a w * h bitmap with margins of 1 pixels at each side.
 * Copy `pixels' there and clear the margins.
 */
unsigned char **provide_margins(unsigned char **pixels,
                                int w, int h,
                                int make_it_0_or_1)
{
    unsigned char **result = allocate_bitmap_with_margins(w, h);
    int y;

    if (!result)
        return NULL;

    /* clear the top and the bottom row */
    memset(result[-1] - 1, 0, w + 2);
    memset(result[h]  - 1, 0, w + 2);

    for (y = 0; y < h; y++)
    {
        unsigned char *src_row = pixels[y];
        unsigned char *dst_row = result[y];

        /* clear left and right margin */
        dst_row[-1] = 0;
        dst_row[w]  = 0;

        if (!make_it_0_or_1)
            memcpy(dst_row, src_row, w);
        else
        {
            int x;
            for (x = 0; x < w; x++)
                dst_row[x] = (src_row[x] ? 1 : 0);
        }
    }

    return result;
}


/* Simply undo the work of allocate_bitmap_with_margin(). */
void free_bitmap_with_margins(unsigned char **pixels)
{
    assert(pixels);
    FREE(&pixels[-1][-1]); /* because we have both left and top margins of 1 pixel */
    FREE(pixels - 1);
}


void assign_bitmap(unsigned char **dst, unsigned char **src, int w, int h)
{
    int i;
    for (i = 0; i < h; i++)
        memcpy(dst[i], src[i], w);
}


unsigned char **copy_bitmap(unsigned char **src, int w, int h)
{
    unsigned char **result = allocate_bitmap(w, h);
    assign_bitmap(result, src, w, h);
    return result;
}


void strip_endpoints_4(unsigned char **result, unsigned char **pixels, int w, int h)
{
    int x, y;

    assert(result != pixels);

    for (y = 0; y < h; y++) for (x = 0; x < w; x++) if (pixels[y][x])
    {
        int degree = pixels[y - 1][x] + pixels[y + 1][x]
                   + pixels[y][x - 1] + pixels[y][x + 1];

        if(degree != 1)
            result[y][x] = 1;
    }
}

void strip_endpoints_8(unsigned char **result, unsigned char **pixels, int w, int h)
{
    int x, y;

    assert(result != pixels);

    for (y = 0; y < h; y++) for (x = 0; x < w; x++) if (pixels[y][x])
    {
        int degree = pixels[y - 1][x - 1] + pixels[y - 1][x] + pixels[y - 1][x + 1]
                   + pixels[y    ][x - 1] +                    pixels[y    ][x + 1]
                   + pixels[y + 1][x - 1] + pixels[y + 1][x] + pixels[y + 1][x + 1];

        if(degree != 1)
            result[y][x] = 1;
    }
}


void print_bitmap(unsigned char **pixels, int w, int h)
{
    int x, y;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            putchar(pixels[y][x] ? '@' : '.');
        }
        putchar('\n');
    }
}


void make_bitmap_0_or_1(unsigned char **pixels, int w, int h)
{
    int x, y;

    for (y = 0; y < h; y++)
    {
        unsigned char *row = pixels[y];

        for (x = 0; x < w; x++)
            row[x] = ( row[x] ? 1 : 0 );
    }
}


void invert_bitmap(unsigned char **pixels, int w, int h, int first_make_it_0_or_1)
{
    int x, y;

    for (y = 0; y < h; y++)
    {
        unsigned char *row = pixels[y];

        if (first_make_it_0_or_1)
        {
            for (x = 0; x < w; x++)
                row[x] = ( row[x] ? 0 : 1 );
        }
        else
        {
            for (x = 0; x < w; x++)
                row[x] = 1 - row[x];
        }
    }
}


unsigned char **allocate_bitmap_with_white_margins(int w, int h)
{
    unsigned char **result = allocate_bitmap_with_margins(w, h);
    int y;

    memset(result[-1] - 1, 0, w + 2);
    memset(result[ h] - 1, 0, w + 2);
    for (y = 0; y < h; y++)
    {
        result[y][-1] = 0;
        result[y][w] = 0;
    }

    return result;
}


void clear_bitmap(unsigned char **pixels, int w, int h)
{
    int y;

    for (y = 0; y < h; y++)
        memset(pixels[y], 0, w);
}
