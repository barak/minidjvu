/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * jb2save.cpp - saving JB2 raw streams
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
#include "jb2coder.h"
#include <stdlib.h>

static int open_bitmap_record(mdjvu_image_t img, int32 index,
    bool with_blit, int32 *table, int32 &library_size, JB2Encoder &jb2,
    mdjvu_error_t *perr, int erosion)
{
    table[index] = -2;
    mdjvu_bitmap_t bitmap = mdjvu_image_get_bitmap(img, index);
    mdjvu_bitmap_t proto =
        mdjvu_image_get_substitution(img,
            mdjvu_image_get_prototype(img, bitmap));
    int32 proto_index = mdjvu_bitmap_get_index(proto);
    if (proto_index == -1)
    {
        // encode directly

        if (with_blit)
            jb2.open_record(jb2_new_symbol_add_to_image_and_library);
        else
            jb2.open_record(jb2_new_symbol_add_to_library_only);
        if (erosion)
        {
            mdjvu_bitmap_t flip_candidates = mdjvu_get_erosion_mask(bitmap);
            jb2.encode(bitmap, NULL, flip_candidates);
            mdjvu_bitmap_destroy(flip_candidates);
        }
        else
            jb2.encode(bitmap);
    }
    else
    {
        // cross-encode

        if (table[proto_index] == -2)
        {
            if (perr) *perr = mdjvu_get_error(mdjvu_error_recursive_prototypes);
            return 0; // prototype recursion error
        }
        if (table[proto_index] == -1)
        {
            // prototype is not yet encoded - encode it
            int32 bl, bt, bw, bh;
            mdjvu_bitmap_get_bounding_box(bitmap, &bl, &bt, &bw, &bh);
            assert(!bl);
            assert(!bt);
            assert(bw == mdjvu_bitmap_get_width(bitmap));
            assert(bh == mdjvu_bitmap_get_height(bitmap));

            int result = open_bitmap_record
                (img, proto_index, false, table, library_size, jb2, perr, erosion);
            if (!result) return 0;
            jb2.close_record();
        }

        if (with_blit)
            jb2.open_record(jb2_matched_symbol_with_refinement_add_to_image_and_library);
        else
            jb2.open_record(jb2_matched_symbol_with_refinement_add_to_library_only);
        jb2.matching_symbol_index.set_interval(0, library_size - 1);
        jb2.zp.encode(table[proto_index], jb2.matching_symbol_index);

        if (erosion)
        {
            mdjvu_bitmap_t flip_candidates = mdjvu_get_erosion_mask(bitmap);
            jb2.encode(bitmap, proto, flip_candidates);
            mdjvu_bitmap_destroy(flip_candidates);
        }
        else
            jb2.encode(bitmap, proto);
    }
    table[index] = library_size++;
    return 1;
}

MDJVU_IMPLEMENT int mdjvu_file_save_jb2(mdjvu_image_t image, mdjvu_file_t f, mdjvu_error_t *perr, int erosion)
{
    if (!mdjvu_image_has_prototypes(image))
        mdjvu_find_prototypes(image);

    if (perr) *perr = NULL;
    int32 n = mdjvu_image_get_bitmap_count(image);
    int32 b = mdjvu_image_get_blit_count(image);
    JB2Encoder jb2((FILE *) f);
    ZPEncoder &zp = jb2.zp;

    /* opening record */
    jb2.open_record(jb2_start_of_image);
        zp.encode(mdjvu_image_get_width(image),  jb2.image_size);
        zp.encode(mdjvu_image_get_height(image), jb2.image_size);
        zp.encode(0, jb2.eventual_image_refinement); /* seems to be unused */
    jb2.close_record();

    /* Now let's start. */
    int32 library_size = 0;

    // The library table keeps indices of shapes in the encoded library.
    // If the shape wasn't yet encoded, the value is -1.
    // If the shape is to be encoded, the value is -2.
    int32 *library_table = (int32 *) malloc(n * sizeof(int32));
    int32 i;
    for (i = 0; i < n; i++) library_table[i] = -1;

    /* encode all blits with bitmaps as necessary */
    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bitmap =
            mdjvu_image_get_substitution(image,
                mdjvu_image_get_blit_bitmap(image, i));
        int32 bmp_i = mdjvu_bitmap_get_index(bitmap);

        if (library_table[bmp_i] == -1)
        {
            // we have not encoded the bitmap yet
            // encode it!
            if (!open_bitmap_record(image, bmp_i, true, library_table,
                                                        library_size,
                                                        jb2, perr, erosion))
            {
                free(library_table);
                return 0;
            }
        }
        else
        {
            // we have our bitmap already
            assert(library_table[bmp_i] >= 0);
            jb2.open_record(jb2_matched_symbol_copy_to_image_without_refinement);
            jb2.matching_symbol_index.set_interval(0, library_size - 1);
            jb2.zp.encode(library_table[bmp_i], jb2.matching_symbol_index);
        }
        jb2.encode_blit(image, i, mdjvu_bitmap_get_width(bitmap),
                                  mdjvu_bitmap_get_height(bitmap));
        jb2.close_record();
    }

    /* end of data record */
    jb2.open_record(jb2_end_of_data);
    jb2.close_record();

    free(library_table);
    return 1;
}

MDJVU_IMPLEMENT int mdjvu_save_jb2(mdjvu_image_t image, const char *path, mdjvu_error_t *perr, int erosion)
{
    FILE *f = fopen(path, "wb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    int result = mdjvu_file_save_jb2(image, (mdjvu_file_t) f, perr, erosion);
    fclose(f);
    return result;
}
