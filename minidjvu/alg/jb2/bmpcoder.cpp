/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * bmpcoder.cpp - encoding/decoding bitmaps (DjVu 2 specification, 8.5.9-10)
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
#include "bmpcoder.h"
#include <stdlib.h>
#include <string.h>

// JB2BitmapCoder implementation {{{

JB2BitmapCoder::JB2BitmapCoder(ZPMemoryWatcher *w) :
    symbol_width(0, jb2_big_positive_number, w),
    symbol_height(0, jb2_big_positive_number, w),
    symbol_width_difference
        (jb2_big_negative_number, jb2_big_positive_number, w),
    symbol_height_difference
        (jb2_big_negative_number, jb2_big_positive_number, w)
{
}

void JB2BitmapCoder::reset_numcontexts()
{
    symbol_width.reset();
    symbol_height.reset();
    symbol_width_difference.reset();
    symbol_height_difference.reset();
}

void JB2BitmapCoder::code_row_directly
    (int32 n, unsigned char *up2, unsigned char *up1, unsigned char *target,
     unsigned char *erosion)
{
    // demands right margin 2 from up2 and 3 from up1

    /* CONTEXT is 10-bit integer organized like this:
     *
     *     up2 -> | |A|B|C| |
     *     up1 -> |D|E|F|G|H|  - picture
     *  target -> |I|J|.| | |
     *
     *  CONTEXT: most significant -> |J|I|H|G|F|E|D|C|B|A| <- least significant
     */

    uint16 context = 0;

    // initialize bits B, C, F, G and H
    if (up2[0]) context = 2;
    if (up2[1]) context |= 4;
    if (up1[0]) context |= 0x20;
    if (up1[1]) context |= 0x40;
    if (up1[2]) context |= 0x80;

    for (int32 i = n; i--;)
    {
        int pixel = code_pixel(bitmap_direct[context], target++, *erosion++);
        context >>= 1;
        context &= 0x17B; // clear H, C and J

        up1++; up2++;

        if (up2[1]) context |= 4;     // fill C
        if (up1[2]) context |= 0x80;  // fill H
        if (pixel)  context |= 0x200; // fill J
    }
}

// TODO: optimize it by unpacking "0 or 1" and ||ing with shifts
void JB2BitmapCoder::code_row_by_refinement
    (int32 n, unsigned char *up1, unsigned char *target, unsigned char *p_up, unsigned char *p_sm, unsigned char *p_dn,
     unsigned char *erosion)
{
    // demands right margin 2 from all but target and left margin 1 from p_

    /* CONTEXT is 11-bit integer organized like this:
     *
     *     up1 -> |A|B|C|
     *  target -> |D|.| | - picture
     *
     *    p_up -> | |E| |
     *    p_sm -> |F|G|H| - prototype
     *    p_dn -> |I|J|K|
     *
     *  CONTEXT: 0 0 0 0  0 K J I  H G F E  D C B A
     */

    uint16 context = 0;
    if (up1[0])   context  = 2;       // B
    if (up1[1])   context |= 4;       // C
    if (p_up[0])  context |= 0x10;    // E
    if (p_sm[-1]) context |= 0x20;    // F
    if (p_sm[0])  context |= 0x40;    // G
    if (p_sm[1])  context |= 0x80;    // H
    if (p_dn[-1]) context |= 0x100;   // I
    if (p_dn[0])  context |= 0x200;   // J
    if (p_dn[1])  context |= 0x400;   // K


    int32 x = n;
    while (x--)
    {
        int pixel = code_pixel(bitmap_refine[context], target++, *erosion++);
        context >>= 1;
        context &= 0x363; // clear C, D, E, H and K

        up1++; p_up++; p_sm++; p_dn++;

        if (up1[1])  context |= 4;     // C
        if (pixel)   context |= 8;     // D
        if (p_up[0]) context |= 0x10;  // E
        if (p_sm[1]) context |= 0x80;  // H
        if (p_dn[1]) context |= 0x400; // K
    }
}

void JB2BitmapCoder::code_image_directly(mdjvu_bitmap_t shape, mdjvu_bitmap_t erosion_mask)
{
    int32 w = mdjvu_bitmap_get_width(shape);
    int32 h = mdjvu_bitmap_get_height(shape);
    unsigned char *up2 = (unsigned char *) calloc(w + 3, 1); // 3 bytes are right margin
    unsigned char *up1 = (unsigned char *) calloc(w + 3, 1);
    unsigned char *target = (unsigned char *) malloc(w + 3);
    unsigned char *erosion = (unsigned char *) calloc(w, 1);
    assert(!erosion_mask || mdjvu_bitmap_get_width(erosion_mask) == w);
    target[w] = target[w + 1] = target[w + 2] = 0;

    for (int32 y = 0; y < h; y++)
    {
        load_row(shape, y, target);
        if (erosion_mask)
            mdjvu_bitmap_unpack_row(erosion_mask, erosion, y);
        code_row_directly(w, up2, up1, target, erosion);
        save_row(shape, y, target, erosion_mask != NULL);

        unsigned char *t = up2;
        up2 = up1;
        up1 = target;
        target = t;
    }

    free(up2);
    free(up1);
    free(target);
    free(erosion);
}

void JB2BitmapCoder::code_image_by_refinement/*{{{*/
    (mdjvu_bitmap_t shape, mdjvu_bitmap_t prototype, mdjvu_bitmap_t erosion_mask)
{
    int32 w = mdjvu_bitmap_get_width(shape);
    int32 h = mdjvu_bitmap_get_height(shape);
    int32 pw = mdjvu_bitmap_get_width(prototype);
    int32 ph = mdjvu_bitmap_get_height(prototype);

    int32 max_width = w > pw ? w : pw;
    unsigned char *up1    = (unsigned char *) calloc(max_width + 2, 1);
    unsigned char *target = (unsigned char *) calloc(max_width + 2, 1);
    unsigned char *erosion  = (unsigned char *) calloc(max_width, 1);
    unsigned char *buf_prototype_up = (unsigned char *) calloc(max_width + 3, 1);
    unsigned char *buf_prototype_sm = (unsigned char *) calloc(max_width + 3, 1);
    unsigned char *buf_prototype_dn = (unsigned char *) calloc(max_width + 3, 1);
    unsigned char *prototype_up = buf_prototype_up + 1; // to have left margin of 1
    unsigned char *prototype_sm = buf_prototype_sm + 1; // to have left margin of 1
    unsigned char *prototype_dn = buf_prototype_dn + 1; // to have left margin of 1

    // align (see DjVu2 specs, page 32, bottom)
    int center_x = w - w / 2; // this favors right (but that agrees with specs)
    int center_y = h / 2; // this favors top (but that agrees with specs)
    int proto_center_x = pw - pw / 2;
    int proto_center_y = ph / 2;

    // (shift_x, shift_y) is a top left corner shift of symbol being decoded
    // with respect to prototype's top left corner.
    int shift_x = proto_center_x - center_x;
    int shift_y = proto_center_y - center_y;

    // prepare upper row -> sm, same row -> dn (to be raised in the loop)
    int y;
    int proto_unpack = shift_x < 0 ? -shift_x : 0;
    int code_shift = shift_x > 0 ? shift_x : 0;

    y = shift_y - 1;
    if (y >= 0 && y < ph)
        mdjvu_bitmap_unpack_row_0_or_1(prototype, prototype_sm + proto_unpack, y);
    y = shift_y;
    if (y >= 0 && y < ph)
        mdjvu_bitmap_unpack_row_0_or_1(prototype, prototype_dn + proto_unpack, y);

    for (y = 0; y < (int) h; y++)
    {
        // prepare three prototype rows by unpacking lower one
        // and shifting all others

        int proto_y_dn = y + shift_y + 1;

        if (proto_y_dn >= 0)
        {
            // we have to get prototype_dn or clear it
            if (proto_y_dn < ph)
            {
                // rotate prototype rows to get a place for prototype_dn
                unsigned char *t = prototype_up;
                prototype_up = prototype_sm;
                prototype_sm = prototype_dn;
                prototype_dn = t;

                mdjvu_bitmap_unpack_row_0_or_1(prototype,
                                       prototype_dn + proto_unpack,
                                       proto_y_dn);
            }
            else if (proto_y_dn < ph + 3)
            {
                unsigned char *t = prototype_up;
                prototype_up = prototype_sm;
                prototype_sm = prototype_dn;
                prototype_dn = t;
                memset(prototype_dn, 0, max_width);
            }
            // else all three prototype rows are and will be empty - do nothing
        }

        // code y-th row
        load_row(shape, y, target);
        if (erosion_mask)
            mdjvu_bitmap_unpack_row(erosion_mask, erosion, y);
        code_row_by_refinement(w, up1, target,
                               prototype_up + code_shift,
                               prototype_sm + code_shift,
                               prototype_dn + code_shift, erosion);
        save_row(shape, y, target, erosion_mask != NULL);

        unsigned char *t = up1;
        up1 = target;
        target = t;
    }

    free(up1);
    free(target);
    free(erosion);
    free(buf_prototype_up);
    free(buf_prototype_sm);
    free(buf_prototype_dn);
}/*}}}*/

// JB2BitmapCoder }}}

// JB2BitmapDecoder implementation {{{

JB2BitmapDecoder::JB2BitmapDecoder(ZPDecoder &z, ZPMemoryWatcher *w)
    : JB2BitmapCoder(w), zp(z) {}

int JB2BitmapDecoder::code_pixel(ZPBitContext &context, unsigned char *pixel, int erosion)
{
    return *pixel = zp.decode(context);
}

mdjvu_bitmap_t JB2BitmapDecoder::decode(mdjvu_image_t img, mdjvu_bitmap_t proto)
{
    if (proto)
    {
        int32 pw = mdjvu_bitmap_get_width(proto);
        int32 ph = mdjvu_bitmap_get_height(proto);
        int32 w = pw + zp.decode(symbol_width_difference);
        int32 h = ph + zp.decode(symbol_height_difference);
        mdjvu_bitmap_t shape = mdjvu_image_new_bitmap(img, w, h);

        code_image_by_refinement(shape, proto, NULL);

        return shape;
    }
    else
    {
        int32 w = zp.decode(symbol_width);
        int32 h = zp.decode(symbol_height);
        mdjvu_bitmap_t shape = mdjvu_image_new_bitmap(img, w, h);

        code_image_directly(shape, NULL);

        return shape;
    }
}

void JB2BitmapDecoder::load_row(mdjvu_bitmap_t sh, int32 y, unsigned char *row){}

void JB2BitmapDecoder::save_row(mdjvu_bitmap_t sh, int32 y, unsigned char *row, int erosion)
{
    mdjvu_bitmap_pack_row(sh, row, y);
}

// JB2BitmapDecoder }}}

// JB2BitmapEncoder implementation {{{

JB2BitmapEncoder::JB2BitmapEncoder(ZPEncoder &z, ZPMemoryWatcher *w):
    JB2BitmapCoder(w), zp(z) {}

int JB2BitmapEncoder::code_pixel(ZPBitContext &context, unsigned char *pixel, int erosion)
{
    if (erosion)
        *pixel = context.get_more_probable_bit();
    zp.encode(*pixel, context);
    return *pixel;
}

void JB2BitmapEncoder::encode(mdjvu_bitmap_t sh, mdjvu_bitmap_t proto, mdjvu_bitmap_t erosion_mask)
{
    if (proto)
    {
        int32 pw = mdjvu_bitmap_get_width(proto);
        int32 ph = mdjvu_bitmap_get_height(proto);
        int32 w = mdjvu_bitmap_get_width(sh);
        int32 h = mdjvu_bitmap_get_height(sh);
        zp.encode(w - pw, symbol_width_difference);
        zp.encode(h - ph, symbol_height_difference);

        code_image_by_refinement(sh, proto, erosion_mask);
    }
    else
    {
        int32 w = mdjvu_bitmap_get_width(sh);
        int32 h = mdjvu_bitmap_get_height(sh);
        zp.encode(w, symbol_width);
        zp.encode(h, symbol_height);

        code_image_directly(sh, erosion_mask);
    }
}

void JB2BitmapEncoder::save_row(mdjvu_bitmap_t sh, int32 y, unsigned char *row, int erosion)
{
    if (erosion)
        mdjvu_bitmap_pack_row(sh, row, y);
}

void JB2BitmapEncoder::load_row(mdjvu_bitmap_t sh, int32 y, unsigned char *row)
{
    mdjvu_bitmap_unpack_row_0_or_1(sh, row, y);
}

// JB2BitmapEncoder }}}
