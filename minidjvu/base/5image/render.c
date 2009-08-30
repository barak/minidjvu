/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * render.c - rendering a split image (an implementation of mdjvu_render())
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

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_render(mdjvu_image_t img)
{
    int32 width  = mdjvu_image_get_width (img);
    int32 height = mdjvu_image_get_height(img);
    unsigned char **b = mdjvu_create_2d_array(width, height);
    unsigned char *row_buffer = (unsigned char *) malloc(width);
    int32 blit_count = mdjvu_image_get_blit_count(img);
    int32 i;
    mdjvu_bitmap_t result = mdjvu_bitmap_create(width, height);

    /* Fill the canvas with white */
    for (i = 0; i < height; i++)
        memset(b[i], 0, width);

    /* Render the split image blit by blit */
    for (i = 0; i < blit_count; i++)
    {
        int32 x = mdjvu_image_get_blit_x(img, i);
        int32 y = mdjvu_image_get_blit_y(img, i);
        mdjvu_bitmap_t current_bitmap = mdjvu_image_get_blit_bitmap(img, i);
        int32 w = mdjvu_bitmap_get_width(current_bitmap);
        int32 h = mdjvu_bitmap_get_height(current_bitmap);

        /* Now w and h are dimensions of the current shape,
         * and width and height are dimensions of the whole image.
         */

        int32 min_col = x >= 0 ? 0 : - x;
        int32 max_col_plus_one = x + w <= width ? w : width - x;

        int32 min_row = y >= 0 ? 0 : - y;
        int32 max_row_plus_one = y + h <= height ? h : height - y;

        int32 row;

        /* Render the current blit row by row */
        for (row = min_row; row < max_row_plus_one; row++)
        {
            int32 col;
            unsigned char *target = b[y + row] + x;
            mdjvu_bitmap_unpack_row(current_bitmap, row_buffer, row);
            for (col = min_col; col < max_col_plus_one; col++)
                target[col] |= row_buffer[col];
        }
    }

    free(row_buffer);

    /* Convert 2D array to a Bitmap and return it */
    mdjvu_bitmap_pack_all(result, b);
    mdjvu_destroy_2d_array(b);
    return result;
}
