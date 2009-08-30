/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * 4image.h - manipulating split images, the main data structure of minidjvu
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

typedef struct MinidjvuImage *mdjvu_image_t;

/* Create a split image with the given read-only parameters.
 * The created image will not contain any blits or bitmaps.
 * Resolution is set to default (300 dpi) and may be changed later.
 */
MDJVU_FUNCTION mdjvu_image_t mdjvu_image_create(int32 width, int32 height);

/* Destroy a split image, freeing all its bitmaps and blits. */
MDJVU_FUNCTION void mdjvu_image_destroy(mdjvu_image_t);

/* Get the width of a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_width(mdjvu_image_t);

/* Get the height of a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_height(mdjvu_image_t);

/* Free some memory by indication that no additions to the image are planned.
 * You still may do any additions after freezing.
 * Calling freeze() after every addition is possible, but inefficient.
 */
MDJVU_FUNCTION void mdjvu_image_freeze(mdjvu_image_t);

/* _______________________________   blits   _______________________________ */

/* Get the number of blits in a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_blit_count(mdjvu_image_t);

MDJVU_FUNCTION int32 mdjvu_image_get_blit_x(mdjvu_image_t, int32 blit_index);
MDJVU_FUNCTION int32 mdjvu_image_get_blit_y(mdjvu_image_t, int32 blit_index);

MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_image_get_blit_bitmap(mdjvu_image_t, int32 blit_index);

MDJVU_FUNCTION void mdjvu_image_set_blit_x(mdjvu_image_t, int32 blit_index, int32 x);
MDJVU_FUNCTION void mdjvu_image_set_blit_y(mdjvu_image_t, int32 blit_index, int32 y);

MDJVU_FUNCTION void mdjvu_image_set_blit_bitmap(mdjvu_image_t, int32 blit_index, mdjvu_bitmap_t);

MDJVU_FUNCTION int32 mdjvu_image_add_blit(mdjvu_image_t, int32 x, int32 y, mdjvu_bitmap_t);

/* _________________________   bitmaps in an image   _______________________ */

/* Get the number of bitmaps in a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_bitmap_count(mdjvu_image_t);

/* Get a bitmap by index. (index may be from 0 to get_bitmap_count() - 1. */
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_image_get_bitmap(mdjvu_image_t, int32);

/* Append a bitmap. */
MDJVU_FUNCTION int32 mdjvu_image_add_bitmap(mdjvu_image_t, mdjvu_bitmap_t);

/* Create a new bitmap and add it. */
MDJVU_FUNCTION mdjvu_bitmap_t
mdjvu_image_new_bitmap(mdjvu_image_t, int32 w, int32 h);

MDJVU_FUNCTION void
mdjvu_image_delete_bitmap_by_index(mdjvu_image_t, int32 index);

MDJVU_FUNCTION void mdjvu_image_delete_bitmap(mdjvu_image_t, int32 index);

/* Get the index of a bitmap by linear search. Returns -1 if not found. */
MDJVU_FUNCTION int32
mdjvu_image_get_bitmap_index(mdjvu_image_t, mdjvu_bitmap_t);

/* ______________________   additional info for images   ___________________ */

MDJVU_FUNCTION int32 mdjvu_image_get_resolution(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_set_resolution(mdjvu_image_t, int32 dpi);
MDJVU_FUNCTION void mdjvu_image_set_dictionary(mdjvu_image_t, mdjvu_image_t);
MDJVU_FUNCTION mdjvu_image_t mdjvu_image_get_dictionary(mdjvu_image_t);

/* _______________________   additional info for blits   ___________________ */

/* ______________________   additional info for bitmaps   __________________ */


/* _______________________________   render   ______________________________ */

MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_render(mdjvu_image_t);

/* _______________________________   wrapper   _____________________________ */

#ifdef MINIDJVU_WRAPPERS

    struct MinidjvuImage
    {
         inline int32 get_resolution()
             {return mdjvu_image_get_resolution(this);}
         inline void set_resolution(int32 dpi)
             {return mdjvu_image_set_resolution(this, dpi);}
    };

    #ifdef MINIDJVU_NO_WRAPPER_PREFIX
        #define Image MinidjvuImage
    #endif

#endif
