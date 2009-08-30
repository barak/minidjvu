/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * smooth.c - pre-filtering bitmap before splitting
 *
 * Copyright (C) 2005  Ilya Mezhirov
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
 *
 * 
 * minidjvu is derived from DjVuLibre (http://djvu.sourceforge.net)
 * All over DjVuLibre there is a patent alert from LizardTech
 * which I guess I should reproduce (don't ask me what does this mean):
 * 
 *  ------------------------------------------------------------------
 * | DjVu (r) Reference Library (v. 3.5)
 * | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
 * | The DjVu Reference Library is protected by U.S. Pat. No.
 * | 6,058,214 and patents pending.
 * |
 * | This software is subject to, and may be distributed under, the
 * | GNU General Public License, Version 2. The license should have
 * | accompanied the software or you may obtain a copy of the license
 * | from the Free Software Foundation at http://www.fsf.org .
 * |
 * | The computer code originally released by LizardTech under this
 * | license and unmodified by other parties is deemed "the LIZARDTECH
 * | ORIGINAL CODE."  Subject to any third party intellectual property
 * | claims, LizardTech grants recipient a worldwide, royalty-free, 
 * | non-exclusive license to make, use, sell, or otherwise dispose of 
 * | the LIZARDTECH ORIGINAL CODE or of programs derived from the 
 * | LIZARDTECH ORIGINAL CODE in compliance with the terms of the GNU 
 * | General Public License.   This grant only confers the right to 
 * | infringe patent claims underlying the LIZARDTECH ORIGINAL CODE to 
 * | the extent such infringement is reasonably necessary to enable 
 * | recipient to make, have made, practice, sell, or otherwise dispose 
 * | of the LIZARDTECH ORIGINAL CODE (or portions thereof) and not to 
 * | any greater extent that may be necessary to utilize further 
 * | modifications or combinations.
 * |
 * | The LIZARDTECH ORIGINAL CODE is provided "AS IS" WITHOUT WARRANTY
 * | OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * | TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
 * | MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * +------------------------------------------------------------------
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
