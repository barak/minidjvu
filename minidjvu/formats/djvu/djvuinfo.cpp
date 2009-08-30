/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * djvuinfo.c - dealing with DjVu INFO chunk
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

#define DEFAULT_VERSION_STAMP 24
#define DEFAULT_RESOLUTION 300 /* used only if unknown */
#define DEFAULT_GAMMA 278

MDJVU_FUNCTION void mdjvu_write_info_chunk(mdjvu_file_t f, mdjvu_image_t image)
{
    int32 w = mdjvu_image_get_width(image);
    int32 h = mdjvu_image_get_height(image);
    int32 dpi = mdjvu_image_get_resolution(image);

    if (!dpi) dpi = DEFAULT_RESOLUTION;

    mdjvu_write_big_endian_int16((uint16) w, f);
    mdjvu_write_big_endian_int16((uint16) h, f);
    mdjvu_write_little_endian_int16(DEFAULT_VERSION_STAMP, f);
    mdjvu_write_little_endian_int16((uint16) dpi, f);
    mdjvu_write_little_endian_int16(DEFAULT_GAMMA, f);
}

MDJVU_IMPLEMENT void mdjvu_read_info_chunk(mdjvu_file_t f, int32 *pw, int32 *ph, int32 *pdpi)
{
    int16 w = mdjvu_read_big_endian_int16(f);
    int16 h = mdjvu_read_big_endian_int16(f);
    int16 dpi;
    mdjvu_read_little_endian_int16(f); /* version stamp */
    dpi = mdjvu_read_little_endian_int16(f);
    mdjvu_read_little_endian_int16(f); /* gamma */

    if (pw) *pw = w;
    if (ph) *ph = h;
    if (pdpi) *pdpi = dpi;
}
