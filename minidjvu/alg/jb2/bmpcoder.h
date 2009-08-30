/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * bmpcoder.h - encoding/decoding bitmaps (DjVu 2 specification, 8.5.9-10)
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

#ifndef MDJVU_BMPCODER_H
#define MDJVU_BMPCODER_H

#include "jb2const.h"
#include "zp.h"

class JB2BitmapCoder
{
    public:
        void reset_numcontexts(); // this was introduced in DjVu 3
    protected:
        ZPBitContext bitmap_direct[1024];
        ZPBitContext bitmap_refine[2048];
        ZPNumContext
            symbol_width,
            symbol_height,
            symbol_width_difference,
            symbol_height_difference;
        inline JB2BitmapCoder(ZPMemoryWatcher *w = NULL) :
            symbol_width(0, jb2_big_positive_number, w),
            symbol_height(0, jb2_big_positive_number, w),
            symbol_width_difference
                (jb2_big_negative_number, jb2_big_positive_number, w),
            symbol_height_difference
                (jb2_big_negative_number, jb2_big_positive_number, w)
        {
        }

        virtual ~JB2BitmapCoder() {}

        void code_row_directly(int32 n, unsigned char *up2,
                                        unsigned char *up1,
                                        unsigned char *target,
                                        unsigned char *erosion);
        void code_row_by_refinement(int32 n,
                                    unsigned char *up1,
                                    unsigned char *target,
                                    unsigned char *p_up,
                                    unsigned char *p_sm,
                                    unsigned char *p_dn,
                                    unsigned char *erosion);
        void code_image_directly(mdjvu_bitmap_t, mdjvu_bitmap_t erosion_mask);
        void code_image_by_refinement(mdjvu_bitmap_t, mdjvu_bitmap_t prototype, mdjvu_bitmap_t erosion_mask);

        virtual int code_pixel(ZPBitContext &, unsigned char *pixel, int erosion) = 0;
        virtual void load_row(mdjvu_bitmap_t, int32 y, unsigned char *row) = 0;
        virtual void save_row(mdjvu_bitmap_t, int32 y, unsigned char *row, int erosion) = 0;
};

class JB2BitmapDecoder : public JB2BitmapCoder
{
    public:
        mdjvu_bitmap_t decode(mdjvu_image_t,
                              mdjvu_bitmap_t prototype = NULL);
        JB2BitmapDecoder(ZPDecoder &, ZPMemoryWatcher *w = NULL);
    private:
        ZPDecoder &zp;
        virtual int code_pixel(ZPBitContext &, unsigned char *pixel, int erosion);
        virtual void load_row(mdjvu_bitmap_t, int32 y, unsigned char *row);
        virtual void save_row(mdjvu_bitmap_t, int32 y, unsigned char *row, int erosion);
};

class JB2BitmapEncoder : public JB2BitmapCoder
{
    public:
        void encode(mdjvu_bitmap_t, mdjvu_bitmap_t prototype = NULL, mdjvu_bitmap_t erosion_mask = NULL);
        JB2BitmapEncoder(ZPEncoder &, ZPMemoryWatcher *w = NULL);
    private:
        ZPEncoder &zp;
        virtual int code_pixel(ZPBitContext &, unsigned char *pixel, int erosion);
        virtual void load_row(mdjvu_bitmap_t, int32 y, unsigned char *row);
        virtual void save_row(mdjvu_bitmap_t, int32 y, unsigned char *row, int erosion);
};

#endif
