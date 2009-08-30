/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * jb2coder.cpp - coding character positions and JB2 records
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
#include "jb2coder.h"

// JB2Coder implementation {{{

JB2Coder::JB2Coder() :
    watcher(),
    image_size(0, jb2_big_positive_number, &watcher),
    matching_symbol_index(0, 0, &watcher),// changed on the fly
    symbol_column_number(0, 0, &watcher), // changed after start_of_image
    symbol_row_number(0, 0, &watcher),    // changed after start_of_image
    same_line_column_offset
        (jb2_big_negative_number, jb2_big_positive_number, &watcher),
    same_line_row_offset
        (jb2_big_negative_number, jb2_big_positive_number, &watcher),
    new_line_column_offset
        (jb2_big_negative_number, jb2_big_positive_number, &watcher),
    new_line_row_offset
        (jb2_big_negative_number, jb2_big_positive_number, &watcher),
    comment_length(0, jb2_big_positive_number, &watcher),
    comment_octet(0, 255, &watcher),
    required_dictionary_size(0, jb2_big_positive_number, &watcher),
    record_type(0, 11, &watcher),
    first(-1, 0, 0, 1),
    line_counter(0)
{
}

void JB2Coder::reset_numcontexts()
{
    record_type.reset();
    image_size.reset();
    matching_symbol_index.reset();
    symbol_column_number.reset();
    symbol_row_number.reset();
    same_line_column_offset.reset();
    same_line_row_offset.reset();
    new_line_column_offset.reset();
    new_line_row_offset.reset();
    comment_length.reset();
    comment_octet.reset();
    required_dictionary_size.reset();
}

// JB2Coder implementation }}}

JB2Decoder::JB2Decoder(FILE *f)
 : JB2BitmapDecoder(zp), zp(f) {}
JB2Encoder::JB2Encoder(FILE *f)
 : JB2BitmapEncoder(zp), zp(f), no_symbols_yet(true) {}

// Coding character positions {{{

void JB2Decoder::decode_character_position(int32 &x, int32 &y, int32 w, int32 h)/*{{{*/
{
    // implements section 8.5.8 (page 31) of DjVu 2 specs
    // FIXME: too much code is duplicated with encode_character_position()

    if (zp.decode(offset_type))
    {
        // new line

        x = first.left + zp.decode(new_line_column_offset);
        int dy = zp.decode(new_line_row_offset);
        y = first.top + first.height - 1 - dy; // their offset is up, not down

        // start a new line with the current symbol
        line_counter = 1;
        first.left = x;
        first.top = y;
        first.width = w;
        first.height = h;
        prev1 = first;
    }
    else
    {
        x = prev1.left + prev1.width - 1 + zp.decode(same_line_column_offset);
        int baseline;

        if (line_counter < 3)
        {
            baseline = first.top + first.height;
        }
        else
        {
            int b1 = prev1.top + prev1.height;
            int b2 = prev2.top + prev2.height;
            int b3 = prev3.top + prev3.height;

            // baseline is the median of b1, b2 and b3
            if (b1 > b2) {int t = b1; b1 = b2; b2 = t;}
            if (b1 > b3) {int t = b1; b1 = b3; b3 = t;}
            if (b2 > b3) {int t = b2; b2 = b3; b3 = t;}

            baseline = b2;
        }

        y = baseline - h - zp.decode(same_line_row_offset);

        // continue the current line with the current symbol
        line_counter++;
        prev3 = prev2;
        prev2 = prev1;
        prev1.left = x;
        prev1.top = y;
        prev1.width = w;
        prev1.height = h;
    }
}/*}}}*/
void JB2Encoder::encode_character_position(int x, int y, int w, int h)/*{{{*/
{
    // implements section 8.5.8 (page 31) of DjVu 2 specs
    // FIXME: too much code is duplicated with decode_character_position()

    int new_line;
    if (no_symbols_yet)
    {
        no_symbols_yet = false;
        new_line = true;
    }
    else
    {
        new_line = x < prev1.left; // taken from DjVuLibre
    }

    zp.encode(new_line ? 1 : 0, offset_type);
    if (new_line)
    {
        // new line

        zp.encode(x - first.left, new_line_column_offset);
        zp.encode(first.top + first.height - 1 - y, new_line_row_offset);

        // start a new line with the current symbol
        line_counter = 1;
        first.left = x;
        first.top = y;
        first.width = w;
        first.height = h;
        prev1 = first;
    }
    else
    {
        zp.encode(x - prev1.left - prev1.width + 1, same_line_column_offset);
        int baseline;

        if (line_counter < 3)
        {
            baseline = first.top + first.height;
        }
        else
        {
            int b1 = prev1.top + prev1.height;
            int b2 = prev2.top + prev2.height;
            int b3 = prev3.top + prev3.height;

            // baseline is the median of b1, b2 and b3
            if (b1 > b2) {int t = b1; b1 = b2; b2 = t;}
            if (b1 > b3) {int t = b1; b1 = b3; b3 = t;}
            if (b2 > b3) {int t = b2; b2 = b3; b3 = t;}

            baseline = b2;
        }

        zp.encode(baseline - h - y, same_line_row_offset);

        // continue the current line with the current symbol
        line_counter++;
        prev3 = prev2;
        prev2 = prev1;
        prev1.left = x;
        prev1.top = y;
        prev1.width = w;
        prev1.height = h;
    }
}/*}}}*/

// Coding character positions }}}

int32 JB2Decoder::decode_blit(mdjvu_image_t img, int32 shape_index)
{
    mdjvu_bitmap_t shape = mdjvu_image_get_bitmap(img, shape_index);
    int32 w = mdjvu_bitmap_get_width(shape);
    int32 h = mdjvu_bitmap_get_height(shape);
    int32 x, y;
    decode_character_position(x, y, w, h);
    return mdjvu_image_add_blit(img, x, y, shape);
}

void JB2Encoder::encode_blit(mdjvu_image_t img, int32 blit, int32 w, int32 h)
{
    int32 x = mdjvu_image_get_blit_x(img, blit);
    int32 y = mdjvu_image_get_blit_y(img, blit);
    encode_character_position(x, y, w, h);
}

void JB2Encoder::open_record(JB2RecordType type)
{
    zp.encode(type, record_type);
}

void JB2Encoder::close_record()
{
    if (watcher.count > JB2_NUMBER_CONTEXTS_MEMORY_BOUND)
    {
        zp.encode(jb2_require_dictionary_or_reset, record_type);
        JB2Coder::reset_numcontexts();
        JB2BitmapEncoder::reset_numcontexts();
        watcher.reset();
    }
}

void JB2Decoder::reset()
{
    JB2Coder::reset_numcontexts();
    JB2BitmapDecoder::reset_numcontexts();
}

JB2RecordType JB2Decoder::decode_record_type()
{
    return (JB2RecordType) zp.decode(record_type);
}
