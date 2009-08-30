/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * average.c - computing average bitmap (NOT WORKING YET)
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
#include <assert.h>
#include "minidjvu/alg/patterns/bitmaps.h"


static void find_mass_center(mdjvu_bitmap_t bitmap, int32 mass, int32 *cx, int32 *cy)
{
    int32 x, y;
    int32 sx = 0, sy = 0;
    int32 w = mdjvu_bitmap_get_width(bitmap);
    int32 h = mdjvu_bitmap_get_height(bitmap);
    unsigned char *row = (unsigned char *) malloc(w);

    for (y = 0; y < h; y++)
    {
        mdjvu_bitmap_unpack_row(bitmap, row, y);
        for (x = 0; x < w; x++)
        {
            if (row[x])
            {
                sx += x;
                sy += y;
            }
        }
    }

    *cx = sx / mass;
    *cy = sy / mass;

    free(row);
}


MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_average(mdjvu_image_t image,
                                             mdjvu_bitmap_t *bitmaps,
                                             int32 n)
{
    /* Mass centers */
    int32 *cx;
    int32 *cy;
    int32 i;
    int32 min_x = 0, min_y = 0, max_x_plus_1 = 0, max_y_plus_1 = 0;
    int32 *buf;
    int32 buf_w, buf_h;
    unsigned char *row;
    int32 tmp_x, tmp_y;
    int32 threshold = n / 2;
    mdjvu_bitmap_t result;

    if (n == 1)
    {
        return mdjvu_bitmap_clone(bitmaps[0]);
    }

    cx = (int32 *) malloc(n * sizeof(int32));
    cy = (int32 *) malloc(n * sizeof(int32));
    /* Note: calculating mass centers here is DIRTY KLUGE
     * because it was done when comparing patterns
     * (and in more smart way)
     */
    mdjvu_image_enable_masses(image);

    for (i = 0; i < n; i++)
    {
        int32 w = mdjvu_bitmap_get_width(bitmaps[i]);
        int32 h = mdjvu_bitmap_get_height(bitmaps[i]);
        int32 mass = mdjvu_image_get_mass(image, bitmaps[i]);
        find_mass_center(bitmaps[i], mass, &cx[i], &cy[i]);
        if (-cx[i] < min_x) min_x = -cx[i];
        if (-cy[i] < min_y) min_y = -cy[i];
        if (w-cx[i] > max_x_plus_1) max_x_plus_1 = w-cx[i];
        if (h-cy[i] > max_y_plus_1) max_y_plus_1 = h-cy[i];
    }

    buf_w = max_x_plus_1 - min_x;
    buf_h = max_y_plus_1 - min_y;
    buf = (int32 *) calloc(buf_w * buf_h, sizeof(int32));
    row = (unsigned char *) malloc(buf_w);

    /* Now adding the bitmaps to the buffer */
    for (i = 0; i < n; i++)
    {
        int32 w = mdjvu_bitmap_get_width(bitmaps[i]);
        int32 h = mdjvu_bitmap_get_height(bitmaps[i]);
        int32 sx = min_x + cx[i], sy = min_y + cy[i];
        int32 x, y;

        for (y = 0; y < h; y++)
        {
            int32 *buf_row = buf + buf_w * (y - sy);
            mdjvu_bitmap_unpack_row(bitmaps[i], row, y);
            for (x = 0; x < w; x++)
            {
                if (row[x])
                    buf_row[x - sx]++;
            }
        }
    }

    result = mdjvu_bitmap_create(buf_w, buf_h);
    for (i = 0; i < buf_h; i++)
    {
        int32 j;
        for (j = 0; j < buf_w; j++)
        {
            row[j] = ( buf[i * buf_w + j] > threshold ? 1 : 0 );
            print_bitmap(&row, buf_w, 1);
        }
    }

    mdjvu_bitmap_remove_margins(result, &tmp_x, &tmp_y);

    free(row);
    free(cx);
    free(cy);
    free(buf);

    return result;
}
