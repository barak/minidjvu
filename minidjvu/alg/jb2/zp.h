/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * zp.h - ZP-coder from DjVuLibre, an adaptive arithmetical binary coder
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

/* The DjVuLibre code is a bit reorganized, but the algorithms are the same.
 * Z-coder was stripped.
 */


#ifndef ZP_H
#define ZP_H


#include <minidjvu.h>
#include <stdio.h>
#include <assert.h>

typedef int Bit;


/* ZPBitContext is an adaptation variable.
 * A ZP-coder works together with a context,
 *    and both are changed in the process,
 *    but the context is detachable.
 */

class ZPBitContext
{
    public:
        ZPBitContext();
    private:
        unsigned char value;

    friend class ZPNumContext;
    friend class ZPEncoder;
    friend class ZPDecoder;

    public:
        inline Bit get_more_probable_bit() {return value & 1;}
};

inline ZPBitContext::ZPBitContext(): value(0) {}


/* ZPMemoryWatcher is a purely abstract class.
 * Its only goal is to receive notifications
 *     when a ZPNumContext has allocated a bit context.
 * This unit contains no implementation of ZPMemoryWatcher.
 */
class ZPMemoryWatcher
{
    public:
        virtual void handle_allocation() = 0;
};


/* ZPNumContext is a tree of ZPBitContexts.
 * Its nodes are accessed by indices.
 * Root node always has the index of 0.
 *
 * Nodes are created automatically.
 */
class ZPNumContext
{
    public:
        ZPNumContext(int32 amin, int32 amax, ZPMemoryWatcher *w = NULL);
        ~ZPNumContext();
        void set_interval(int32 new_min, int32 new_max);
        void reset();
    private:
        int32 min, max;
        ZPMemoryWatcher *watcher;
        ZPBitContext *nodes;
        uint16 n;
        uint16 allocated;
        uint16 *left, *right; // 0 means nil
        uint16 new_node();
        uint16 get_left(uint16);
        uint16 get_right(uint16);
        void init();
    friend class ZPEncoder;
    friend class ZPDecoder;
};


class ZPEncoder
{
    public:
        ZPEncoder(FILE *); // does not close it on destruction
        virtual ~ZPEncoder();
        void encode_without_context(Bit);
        void encode(Bit, ZPBitContext &);
        void encode(int32, ZPNumContext &);
    private:
        FILE *file;
        void emit_byte(unsigned char);
        uint32 a, nrun, subend, buffer;
        unsigned char delay, byte, scount;
        bool closed;

        void close();
        void outbit(Bit);
        void zemit(Bit);
        void export_bits();
        void encode_mps(ZPBitContext &, uint32);
        void encode_lps(ZPBitContext &, uint32);
};


class ZPDecoder
{
    public:
        ZPDecoder(FILE *, int32 length); // does not close it on destruction
        Bit decode_without_context();
        Bit decode(ZPBitContext &);
        int32 decode(ZPNumContext &);
    private:
        FILE *file;
        uint32 a, code, fence, buffer;
        int32 bytes_left;
        unsigned char byte, scount, delay;
        bool next_byte(unsigned char &);
        void open();
        void preload();
        int32 ffz(uint32);
        Bit decode_sub(ZPBitContext &, uint32);
        Bit decode_sub_simple(uint32);
};


#endif
