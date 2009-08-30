/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * proto.c - searching for prototypes
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

#include "config.h"
#include <minidjvu.h>
#include <stdlib.h>
#include <string.h>

#define THRESHOLD 21

static const int32 maxint = ~(1 << 31);
static const int32 bigint = ~(1 << 31) / 100 - 1;

/* THIS THING IS THE PRIMARY BOTTLENECK
 * TODO: OPTIMIZE IT!
 * ceiling is an optimization - if we have more, quit
 */
static int diff(mdjvu_bitmap_t image,
                mdjvu_bitmap_t prototype,
                unsigned char **image_uncompressed,
                unsigned char **proto_uncompressed,
                int32 ceiling)
{
    int32 pw = mdjvu_bitmap_get_width (prototype);
    int32 ph = mdjvu_bitmap_get_height(prototype);
    int32 iw = mdjvu_bitmap_get_width (image);
    int32 ih = mdjvu_bitmap_get_height(image);
    int32 shift_x, shift_y;
    int32 s = 0, i, n = pw + 2;
    unsigned char *ir = (unsigned char *) malloc(n);
    unsigned char *pr = (unsigned char *) malloc(n);

    if (abs(iw - pw) > 2) return maxint;
    if (abs(ih - ph) > 2) return maxint;

    /* (shift_x, shift_y) is a shift of image with respect to prototype */
    shift_x = (pw - pw/2) - (iw - iw/2); /* center favors right */
    shift_y = ph/2 - ih/2;               /* center favors top */

    memset(pr, 0, n);
    memset(ir, 0, n);

    for (i = -1; i <= ph; i++)
    {
        int32 y = i - shift_y;

        if (y >= 0 && y < ih)
            memcpy(ir + shift_x + 1, image_uncompressed[y], iw);
        else
            memset(ir, 0, n); /* SUB-OPTIMAL? */

        if (i >= 0 && i < ph)
            memcpy(pr + 1, proto_uncompressed[i], pw);
        else
            memset(pr, 0, n);

        /* reusing y */
        for (y = 0; y < n; y++) if (ir[y] != pr[y]) s++;
        if (s > ceiling) return s;
    }

    free(ir);
    free(pr);

    return s;
}

MDJVU_IMPLEMENT void mdjvu_find_prototypes(mdjvu_image_t img)
{
    int32 i, n = mdjvu_image_get_bitmap_count(img);
    unsigned char ***uncompressed_bitmaps = (unsigned char ***)
        malloc(n * sizeof(unsigned char **));

    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(img, i);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        uncompressed_bitmaps[i] = mdjvu_create_2d_array(w, h);
        mdjvu_bitmap_unpack_all_0_or_1(current, uncompressed_bitmaps[i]);
    }

    mdjvu_image_enable_prototypes(img);
    mdjvu_image_enable_substitutions(img);
    if (!mdjvu_image_has_masses(img))
        mdjvu_image_enable_masses(img); /* calculates them, not just enables */
    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(img, i);
        int32 mass = mdjvu_image_get_bitmap_mass(img, current);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        int32 max_score = w * h * THRESHOLD / 100;
        int32 j;
        mdjvu_bitmap_t best_match = NULL;
        int32 best_score = max_score;

        for (j = 0; j < i; j++)
        {
            int32 score;
            mdjvu_bitmap_t candidate = mdjvu_image_get_bitmap(img, j);
            int32 c_mass = mdjvu_image_get_bitmap_mass(img, candidate);
            if (abs(mass - c_mass) > best_score) continue;
            score = diff(current,
                         candidate,
                         uncompressed_bitmaps[i],
                         uncompressed_bitmaps[j],
                         best_score);
            if (score < best_score)
            {
                best_score = score;
                best_match = candidate;
                if (!score)
                    break; /* a perfect match is found */
            }
        }

        if (best_score)
            mdjvu_image_set_prototype(img, current, best_match);
        else
            mdjvu_image_set_substitution(img, current, best_match);
    }

    /* destroy uncompressed bitmaps */
    for (i = 0; i < n; i++)
    {
        mdjvu_destroy_2d_array(uncompressed_bitmaps[i]);
    }
    free(uncompressed_bitmaps);
}
