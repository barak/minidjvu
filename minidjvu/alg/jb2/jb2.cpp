/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * jb2.cpp - loading JB2 files by interpreting them record-by-record
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
#include "zp.h"
#include "jb2coder.h"

// A piece of my old code... should it be eliminated?
template<class T> inline T *
append_to_list(T *&list, int32 &count, int32 &allocated)
{
    if (allocated == count)
    {
        allocated <<= 1;
        list = (T *) realloc(list, allocated * sizeof(T));
    }
    return &list[count++];
}

static mdjvu_bitmap_t decode_lib_shape/*{{{*/
    (JB2Decoder &jb2, mdjvu_image_t img, bool with_blit, mdjvu_bitmap_t proto)
{
    int32 blit = -1; // to please compilers
    int32 index = mdjvu_image_get_bitmap_count(img);

    mdjvu_bitmap_t shape = jb2.decode(img, proto);
    if (with_blit)
    {
        blit = jb2.decode_blit(img, index);
    }

    int32 x, y;
    mdjvu_bitmap_t cropped = mdjvu_bitmap_remove_margins(shape, &x, &y);
    mdjvu_bitmap_exchange(cropped, shape);
    mdjvu_bitmap_destroy(cropped);

    if (with_blit)
    {
        mdjvu_image_set_blit_x(img, blit, mdjvu_image_get_blit_x(img, blit) + x);
        mdjvu_image_set_blit_y(img, blit, mdjvu_image_get_blit_y(img, blit) + y);
    }

    return shape;
}/*}}}*/

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_load_from_jb2_file(mdjvu_file_t file)/*{{{*/
{
    FILE *f = (FILE *) file;
    JB2Decoder jb2(f);
    ZPDecoder &zp = jb2.zp;

    int32 d = 0;
    int32 t = jb2.decode_record_type();

    if (t == jb2_require_dictionary_or_reset)
    {
        d = zp.decode(jb2.required_dictionary_size);
        t = jb2.decode_record_type();
    }

    if (t != jb2_start_of_image) return NULL;

    int32 w = zp.decode(jb2.image_size);
    int32 h = zp.decode(jb2.image_size);
    zp.decode(jb2.eventual_image_refinement); // dropped
    jb2.symbol_column_number.set_interval(1, w);
    jb2.symbol_row_number.set_interval(1, h);

    mdjvu_image_t img = mdjvu_image_create(w, h); /* d is dropped for now - XXX*/

    int32 lib_count = 0, lib_alloc = 128;
    mdjvu_bitmap_t *library;

    library = (mdjvu_bitmap_t *) malloc(lib_alloc * sizeof(mdjvu_bitmap_t));

    while(1)
    {
        t = jb2.decode_record_type();
        switch(t)
        {
            case jb2_new_symbol_add_to_image_and_library:
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, true, NULL);
            break;
            case jb2_new_symbol_add_to_library_only:
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, false, NULL);
            break;
            case jb2_new_symbol_add_to_image_only:
            {
                jb2.decode(img);
                jb2.decode_blit(img, mdjvu_image_get_bitmap_count(img)-1);
            }
            break;
            case jb2_matched_symbol_with_refinement_add_to_image_and_library:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    return NULL;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, true, library[match]);
            }
            break;
            case jb2_matched_symbol_with_refinement_add_to_library_only:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    return NULL;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, false, library[match]);
            }
            break;
            case jb2_matched_symbol_with_refinement_add_to_image_only:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    return NULL;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                jb2.decode(img, library[match]);
                jb2.decode_blit(img, mdjvu_image_get_bitmap_count(img)-1);
            }
            break;
            case jb2_matched_symbol_copy_to_image_without_refinement:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    return NULL;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                jb2.decode_blit(img, match);
            }
            break;
            case jb2_non_symbol_data:
            {
                mdjvu_bitmap_t bmp = jb2.decode(img);
                int32 x = zp.decode(jb2.symbol_column_number) - 1;
                int32 y = h - zp.decode(jb2.symbol_row_number);
                mdjvu_image_add_blit(img, x, y, bmp);
            }
            break;

            case jb2_require_dictionary_or_reset:
                jb2.reset();
            break;

            case jb2_comment:
            {
                int32 len = zp.decode(jb2.comment_length);
                while (len--) zp.decode(jb2.comment_octet);
            }
            break;

            case jb2_end_of_data:
                free(library);
                return img;
            default:
                free(library);
                mdjvu_image_destroy(img);
                return NULL;
        } // switch
    } // while(1)
}/*}}}*/

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_load_from_jb2(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    mdjvu_image_t result = mdjvu_load_from_jb2_file((mdjvu_file_t) f);
    fclose(f);
    return result;
}
