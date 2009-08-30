/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * jb2coder.h - coding character positions and JB2 records
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

#ifndef MDJVU_JB2CODER_H
#define MDJVU_JB2CODER_H

#include "bmpcoder.h"

// struct JB2Rect - a simple rectangle class {{{

struct JB2Rect
{
    int left, top, width, height;
    inline JB2Rect() {}
    inline JB2Rect(int l, int t, int w, int h)
        : left(l), top(t), width(w), height(h) {}
};

// struct JB2Rect }}}

// JB2Coder interface {{{

/* Watcher interface + implementation {{{ */

struct Watcher : ZPMemoryWatcher
{
    int32 count;
    Watcher() : count(0) {}

    virtual void handle_allocation()
    {
        count++;
    }

    void reset() {count = 0;}
};

/* Watcher }}} */

class JB2Coder
{
    public:
        Watcher watcher;
        ZPNumContext
            image_size,
            matching_symbol_index,
            symbol_column_number,
            symbol_row_number,
            same_line_column_offset,
            same_line_row_offset,
            new_line_column_offset,
            new_line_row_offset,
            comment_length,
            comment_octet,
            required_dictionary_size;

        ZPBitContext eventual_image_refinement, offset_type;
        JB2Coder();

    protected:
        ZPNumContext record_type;
        JB2Rect first, prev3, prev2, prev1;
        int line_counter;
        void reset_numcontexts();
};

// JB2Coder interface }}}

// JB2Decoder interface {{{

struct JB2Decoder : JB2Coder, JB2BitmapDecoder
{
    ZPDecoder zp;
    JB2Decoder(FILE *f, int32 chunk_length);
    JB2RecordType decode_record_type();

    // decodes character position and creates a new blit
    int32 decode_blit(mdjvu_image_t, int32 shape_index);

    void reset(); // resets numcontexts as required by "reset" record

    private:
        void decode_character_position(int32 &x, int32 &y, int32 w, int32 h);
};

// JB2Decoder interface }}}

// JB2Encoder interface {{{

struct JB2Encoder : JB2Coder, JB2BitmapEncoder
{
    ZPEncoder zp;
    JB2Encoder(FILE *f);

    // encodes blit position
    // w and h are passed here in case of substitution
    void encode_blit(mdjvu_image_t img, int32 blit, int32 w, int32 h);

    void open_record(JB2RecordType);
    void close_record(); // takes care of the DjVu 3 numcontext reset

    private:
        void encode_character_position(int x, int y, int w, int h);
        bool no_symbols_yet;
};

// JB2Encoder interface }}}

#endif
