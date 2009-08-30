/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * nosubst.c - guessing what chunks are not letters and should not be changed
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

/*
 * This implementation is very dumb and sub-optimal,
 * but since it's not a bottleneck, I'm not going to optimize it.
 */


#include "mdjvucfg.h"
#include "minidjvu.h"
#include <assert.h>


/*
 * returns true if segments [a1, a1 + l1 - 1] and [a2, a2 + l2 - 1] intersect.
 */
static int segments_intersect_or_touch(int32 a1, int32 l1, int32 a2, int32 l2)
{
    return a1 <= a2 + l2 && a2 <= a1 + l1;
}

static int blits_intersect_or_touch(mdjvu_image_t image, int32 b1, int32 b2)
{
    int32 x1 = mdjvu_image_get_blit_x(image, b1);
    int32 x2 = mdjvu_image_get_blit_x(image, b2);
    int32 y1 = mdjvu_image_get_blit_y(image, b1);
    int32 y2 = mdjvu_image_get_blit_y(image, b2);
    mdjvu_bitmap_t bitmap1 = mdjvu_image_get_blit_bitmap(image, b1);
    mdjvu_bitmap_t bitmap2 = mdjvu_image_get_blit_bitmap(image, b2);
    int32 w1 = mdjvu_bitmap_get_width(bitmap1);
    int32 w2 = mdjvu_bitmap_get_width(bitmap2);
    int32 h1 = mdjvu_bitmap_get_height(bitmap1);
    int32 h2 = mdjvu_bitmap_get_height(bitmap2);
    return segments_intersect_or_touch(x1, w1, x2, w2)
        && segments_intersect_or_touch(y1, h1, y2, h2);
}

static void make_no_subst(mdjvu_image_t image, int32 blit)
{
    int32 i, b;
    mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, blit);
    if (mdjvu_image_get_not_a_letter_flag(image, bitmap)) return;
    mdjvu_image_set_not_a_letter_flag(image, bitmap, 1);

    /* infect all blits that intersect with this */
    b = mdjvu_image_get_blit_count(image);
    for (i = 0; i < b; i++)
    {
        if (blits_intersect_or_touch(image, blit, i))
            make_no_subst(image, i);
    }
}

MDJVU_IMPLEMENT void mdjvu_calculate_not_a_letter_flags(mdjvu_image_t image)
{
    int32 i, b;
    assert(mdjvu_image_has_suspiciously_big_flags(image));
    mdjvu_image_enable_not_a_letter_flags(image);
    b = mdjvu_image_get_blit_count(image);
    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, i);
        if (mdjvu_image_get_suspiciously_big_flag(image, bitmap))
            make_no_subst(image, i);
    }
}
