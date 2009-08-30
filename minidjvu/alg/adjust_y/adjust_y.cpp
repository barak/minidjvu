/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * adjust_y.c - adjust y coordinates of blits so the text won't look bumpy
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

/*
 * This algorithm was made by Leon Bottou.
 * I remember, I had another (simpler) version
 * and I couldn't see the difference between results,
 * but he said this version is cooler.
 */


/* Estimate position of baseline.
 * The baseline position is measured in quarter pixels.
 * Using fractional pixels makes a big improvement.
 */
static int32 compute_baseline(mdjvu_bitmap_t bitmap)
{
    int32 w = mdjvu_bitmap_get_width(bitmap);
    int32 h = mdjvu_bitmap_get_height(bitmap);
    int32 *mass = (int32 *) malloc(h * sizeof(int32));
    unsigned char *row = (unsigned char *) malloc(w);
    int32 i, j, m;
    int32 tm = 0;
    for (i = 0; i < h; i++)
    {
        mdjvu_bitmap_unpack_row(bitmap, row, i);
        for (j = 0; j < w; j++)
        {
            if (row[j])
                break;
        }
        for (m = w - j; m > 0; m--)
        {
            if (row[j + m - 1])
                break;
        }
        mass[h - i - 1] = m;
        tm += m;
    }

    m = 0;
    i = 0;

    while (m * 6 < tm * 4)
    {
        m += mass[i/4];
        i += 1;
    }

    free(row);
    free(mass);

    return 4 * (h - 1) - i;
}

/*
 * Set blits to their substitutes using given x and y adjustments.
 * Before this, blits may not use dictionary bitmaps.
 */
static void update_blits(mdjvu_image_t image, int32 *x_adjust, int32 *y_adjust)
{
    int32 b = mdjvu_image_get_blit_count(image);
    int32 i;
    for (i = 0; i < b; i++)
    {
        int32 x = mdjvu_image_get_blit_x(image, i);
        int32 y = mdjvu_image_get_blit_y(image, i);
        mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, i);
        mdjvu_bitmap_t subst = mdjvu_image_get_substitution(image, bitmap);
        int32 k = mdjvu_bitmap_get_index(bitmap);

        mdjvu_image_set_blit_x(image, i, x + x_adjust[k]);
        mdjvu_image_set_blit_y(image, i, y + y_adjust[k]);
        mdjvu_image_set_blit_bitmap(image, i, subst);
    }
}

/*
 * Return the baselines array. To be MDJVU_FREEV'ed later.
 */
static int32 *get_baselines(mdjvu_image_t image)
{
    int32 i;
    int32 n = mdjvu_image_get_bitmap_count(image);
    int32 *baseline = MDJVU_MALLOCV(int32, n);
    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_bitmap(image, i);
        if (mdjvu_image_has_not_a_letter_flags(image) && !mdjvu_image_get_not_a_letter_flag(image, bitmap))
            baseline[i] = compute_baseline(bitmap);
    }
    return baseline;
}


/*
 * Computes x and y adjustsments.
 * bitmap != subst.
 */
static void compute_adjustments(mdjvu_bitmap_t bitmap, mdjvu_bitmap_t subst,
                                int32 b_base, int32 s_base,
                                int32 *dx, int32 *dy)
{
    int32 w, h, subst_w, subst_h, adjust;

    /* Compute coordinate adjustment */
    w = mdjvu_bitmap_get_width(bitmap);
    h = mdjvu_bitmap_get_height(bitmap);
    subst_w = mdjvu_bitmap_get_width(subst);
    subst_h = mdjvu_bitmap_get_height(subst);
    *dx = (w - subst_w) / 2;
    *dy = (h - subst_h) / 2;

    /* Refine vertical adjustment */
    adjust = b_base - s_base;

    if (adjust < 0)
        adjust = - (2 - adjust) / 4;
    else
        adjust =   (2 + adjust) / 4;

    if (abs(adjust - *dy) <= 1 + w/16 )
        *dy = adjust;
}

MDJVU_IMPLEMENT void mdjvu_adjust(mdjvu_image_t image)
{
    int32 b = mdjvu_image_get_blit_count(image);
    int32 n = mdjvu_image_get_bitmap_count(image);
    int32 *baseline = get_baselines(image);

    int32 i;
    int32 *x_adjust = (int32 *) calloc(n, sizeof(int32));
    int32 *y_adjust = (int32 *) calloc(n, sizeof(int32));

    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, i);
        mdjvu_bitmap_t subst = mdjvu_image_get_substitution(image, bitmap);

        if (subst == bitmap) continue;

        compute_adjustments(bitmap, subst,
                            baseline[i],
                            baseline[mdjvu_bitmap_get_index(subst)],
                            &x_adjust[i], &y_adjust[i]);
    }

    MDJVU_FREEV(baseline);

    update_blits(image, x_adjust, y_adjust);

    free(x_adjust);
    free(y_adjust);
}

static void adjust_page(mdjvu_image_t dict,
                        int32 *dict_baselines,
                        mdjvu_image_t image)
{
    int32 b = mdjvu_image_get_blit_count(image);
    int32 n = mdjvu_image_get_bitmap_count(image);
    int32 *baseline = get_baselines(image);

    int32 i;
    int32 *x_adjust = (int32 *) calloc(n, sizeof(int32));
    int32 *y_adjust = (int32 *) calloc(n, sizeof(int32));

    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, i);
        mdjvu_bitmap_t subst = mdjvu_image_get_substitution(image, bitmap);
        int32 subst_baseline;

        if (subst == bitmap) continue;

        if (mdjvu_image_has_bitmap(dict, subst))
            subst_baseline = dict_baselines[mdjvu_bitmap_get_index(subst)];
        else
        {
            assert(mdjvu_image_has_bitmap(image, subst));
            subst_baseline = baseline[mdjvu_bitmap_get_index(subst)];
        }

        compute_adjustments(bitmap, subst,
                            baseline[i],
                            subst_baseline,
                            &x_adjust[i], &y_adjust[i]);
    }

    MDJVU_FREEV(baseline);

    update_blits(image, x_adjust, y_adjust);

    free(x_adjust);
    free(y_adjust);
}


MDJVU_FUNCTION void mdjvu_multipage_adjust(mdjvu_image_t dict,
                                           int32 npages,
                                           mdjvu_image_t *pages)
{
    int32 *dict_baselines = get_baselines(dict);
    int32 i;
    for (i = 0; i < npages; i++)
        adjust_page(dict, dict_baselines, pages[i]);
    MDJVU_FREEV(dict_baselines);
}
