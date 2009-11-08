/*
 * smooth.c - pre-filtering bitmap before splitting
 */

#include "mdjvucfg.h"
#include "minidjvu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* all input rows must be 0-or-1 unpacked */
static void smooth_row(unsigned char *r, /* result    */
                       unsigned char *u, /* upper row */
                       unsigned char *t, /* this row - must have margin 1 */
                       unsigned char *l, /* lower row */
                       int32 n)
{
    int32 i;
    for (i = 0; i < n; i++)
    {
        int score = u[i] + l[i] + t[i-1] + t[i+1] - 2;

        /* if (score > 0)
            r[i] = 1;
        else */
        /* clearing black pixels doesn't look cool, alas */

        if (score < 0)
            r[i] = 0;
        else
            r[i] = t[i];
    }
}


MDJVU_IMPLEMENT void mdjvu_smooth(mdjvu_bitmap_t b)
{
    int32 w = mdjvu_bitmap_get_width(b);
    int32 h = mdjvu_bitmap_get_height(b);
    int32 i;
    unsigned char *u, *t, *l, *r;

    if (h < 3) return;

    u = (unsigned char *) calloc(w + 2, 1) + 1; /* upper row */
    t = (unsigned char *) calloc(w + 2, 1) + 1; /* this row */
    l = (unsigned char *) calloc(w + 2, 1) + 1; /* lower row */
    r = (unsigned char *) malloc(w); /* result */

    mdjvu_bitmap_unpack_row_0_or_1(b, l, 0);
    for (i = 0; i < h; i++)
    {
        unsigned char *tmp = u;
        u = t;
        t = l;
        l = tmp;

        if (i + 1 < h)
            mdjvu_bitmap_unpack_row_0_or_1(b, l, i + 1);
        else
            memset(l, 0, w);

        smooth_row(r, u, t, l, w);
        mdjvu_bitmap_pack_row(b, r, i);
    }

    free(u - 1);
    free(t - 1);
    free(l - 1);
    free(r);
}
