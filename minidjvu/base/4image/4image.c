/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * 4image.c - manipulating split images, the main data structure of minidjvu
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
#include <assert.h>

typedef struct
{
    int32 width, height;

    /* bitmaps */
    mdjvu_bitmap_t *bitmaps;
    int32 bitmaps_count, bitmaps_allocated;

    /* blits */
    int32 *x, *y;
    mdjvu_bitmap_t *blits;
    int32 blits_count, blits_allocated;

    /* image additional info */
    mdjvu_bitmap_t *dictionary;
    int32 dictionary_size;
    int32 resolution; /* 0 - unknown */
} Image;

/* ______________________________   create   ________________________________ */

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_image_create(int32 width, int32 height)
{
    Image *image = (Image *) malloc(sizeof(Image));
    image->width = width;
    image->height = height;

    image->bitmaps_allocated = 16;
    image->bitmaps = (mdjvu_bitmap_t *)
        malloc(image->bitmaps_allocated * sizeof(mdjvu_bitmap_t));
    image->bitmaps_count = 0;

    image->blits_allocated = 32;
    image->x = (int32 *) malloc(image->blits_allocated * sizeof(int32));
    image->y = (int32 *) malloc(image->blits_allocated * sizeof(int32));
    image->blits = (mdjvu_bitmap_t *)
        malloc(image->blits_allocated * sizeof(mdjvu_bitmap_t));
    image->blits_count = 0;

    image->dictionary_size = 0;
    image->dictionary = NULL;
    image->resolution = 0;

    return (mdjvu_image_t) image;
}

/* ______________________________   destroy   _______________________________ */

#define IMG ((Image *) image)

MDJVU_IMPLEMENT void mdjvu_image_destroy(mdjvu_image_t image)
{
    int32 i;
    for (i = 0; i < IMG->blits_count; i++)
        mdjvu_bitmap_unlink(IMG->blits[i]);
    free(IMG->blits);
    free(IMG->x);
    free(IMG->y);
    for (i = 0; i < IMG->bitmaps_count; i++)
        mdjvu_bitmap_destroy(IMG->bitmaps[i]);
    free(IMG->bitmaps);
    free(IMG);
}

/* ______________________________   get/set   ______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_get_width(mdjvu_image_t image)
    { return IMG->width; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_height(mdjvu_image_t image)
    { return IMG->height; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_bitmap_count(mdjvu_image_t image)
    { return IMG->bitmaps_count; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_count(mdjvu_image_t image)
    { return IMG->blits_count; }

/* ______________________________   bitmaps   ______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_add_bitmap(mdjvu_image_t image, mdjvu_bitmap_t bmp)
{
    if (IMG->bitmaps_count == IMG->bitmaps_allocated)
    {
        IMG->bitmaps_allocated <<= 1;
        IMG->bitmaps = (mdjvu_bitmap_t *) realloc(IMG->bitmaps,
                            IMG->bitmaps_allocated * sizeof(mdjvu_bitmap_t));
    }
    IMG->bitmaps[IMG->bitmaps_count] = bmp;
    return IMG->bitmaps_count++;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_bitmap(mdjvu_image_t image, int32 i)
{
    return IMG->bitmaps[i];
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_new_bitmap(mdjvu_image_t image, int32 w, int32 h)
{
    mdjvu_bitmap_t bmp = mdjvu_bitmap_create(w, h);
    mdjvu_image_add_bitmap(image, bmp);
    return bmp;
}


/* _______________________________   blits   _______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_add_blit(mdjvu_image_t image,
                                          int32 x, int32 y,
                                          mdjvu_bitmap_t bitmap)
{
    if (IMG->blits_count == IMG->blits_allocated)
    {
        IMG->blits_allocated <<= 1;
        IMG->x = (int32 *) realloc(IMG->x,
                                IMG->blits_allocated * sizeof(int32));
        IMG->y = (int32 *) realloc(IMG->y,
                                IMG->blits_allocated * sizeof(int32));
        IMG->blits = (mdjvu_bitmap_t *) realloc(IMG->blits,
                                IMG->blits_allocated * sizeof(mdjvu_bitmap_t));
    }
    IMG->x[IMG->blits_count] = x;
    IMG->y[IMG->blits_count] = y;
    IMG->blits[IMG->blits_count] = bitmap;
    mdjvu_bitmap_link(bitmap);
    return IMG->blits_count++;
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_x(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->x[i];
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_y(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->y[i];
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_x(mdjvu_image_t image, int32 i, int32 x)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->x[i] = x;
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_y(mdjvu_image_t image, int32 i, int32 y)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->y[i] = y;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t
    mdjvu_image_get_blit_bitmap(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->blits[i];
}

/* _____________________________   image info   ____________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_get_resolution(mdjvu_image_t image)
{
    return IMG->resolution;
}

MDJVU_IMPLEMENT void mdjvu_image_set_resolution(mdjvu_image_t image, int32 dpi)
{
    IMG->resolution = dpi;
}
