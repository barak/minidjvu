/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * bmp.c - saving bitmaps in Windows BMP format
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void write_uint16(FILE *f, uint16 i) // little-endian
{
    fputc(i, f);
    fputc(i >> 8, f);
}

static void write_uint32(FILE *f, uint32 i) // little-endian
{
    fputc(i, f);
    fputc(i >> 8, f);
    fputc(i >> 16, f);
    fputc(i >> 24, f);
}

static void write_bmp_header(FILE *f, uint32 w, uint32 h, int resolution)
{
    // Didn't use structures here for portability (concerning endianness).
    uint32 row_size = ((w + 31) & ~31) >> 3; // padding to 32 bit boundary
    uint32 data_size = row_size * h;
    uint32 data_offset = 62; // size of all auxiliary info in monochrome BMP
    uint32 dpm = resolution * 5000 / 127; // per inch -> per meter
    fputc('B', f);
    fputc('M', f);
    write_uint32(f, data_size + data_offset); // file size
    write_uint32(f, 0); // reserved
    write_uint32(f, data_offset);
    write_uint32(f, 40); // size of the rest of header
    write_uint32(f, w);
    write_uint32(f, h);
    write_uint16(f, 1); // number of color planes
    write_uint16(f, 1); // number of bits per pixel
    write_uint32(f, 0); // compression flag
    write_uint32(f, data_size); // data size (0 also possible here)
    write_uint32(f, dpm);
    write_uint32(f, dpm);
    write_uint32(f, 2);
    write_uint32(f, 0);
    write_uint32(f, 0); // black color
    write_uint32(f, 0xFFFFFF); // white color
}

static void save_DIB_bytes(mdjvu_bitmap_t bmp, FILE *f)
{
    int32 w = mdjvu_bitmap_get_width(bmp);
    int32 h = mdjvu_bitmap_get_height(bmp);
    int32 bytes_per_row = mdjvu_bitmap_get_packed_row_size(bmp);
    int32 DIB_row_size = ((w + 31) & ~31) >> 3; // padding to 32 bit
    unsigned char *buf = (unsigned char *) malloc(DIB_row_size);
    uint32 *p;
	uint32 i;
    for (i = h; i; i--)
    {
        uint32 j;
		int mask;
        unsigned char *row = mdjvu_bitmap_access_packed_row(bmp, i - 1);
        memcpy(buf, row, bytes_per_row);
        for (j = DIB_row_size >> 2, p = (uint32 *) buf; j; j--, p++)
        {
            *p = ~*p; // BMP stores pixels inversely (0 - black, 1 - white)
        }

        mask = ~(0xFF >> (w & 7)); // b % 8 is 1 -> 0x80; 7 - 0xFE
        if (bytes_per_row > 1)
        {
            fwrite(buf, 1, bytes_per_row - 1, f);
        }
        fputc(buf[bytes_per_row - 1] & mask, f);
        for (j = DIB_row_size - bytes_per_row; j; j--) fputc(0, f);
    }
    free(buf);
}

MDJVU_IMPLEMENT int mdjvu_save_to_bmp_file(mdjvu_bitmap_t bmp,
                                            mdjvu_file_t file,
                                            int32 resolution)
{
    FILE *f = (FILE *) file;
    write_bmp_header(f, mdjvu_bitmap_get_width(bmp),
                        mdjvu_bitmap_get_height(bmp), resolution);
    save_DIB_bytes(bmp, f);
    return 0;
}

MDJVU_IMPLEMENT int mdjvu_save_to_bmp(mdjvu_bitmap_t bmp,
                                      const char *path,
                                      int32 resolution)
{
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    mdjvu_save_to_bmp_file(bmp, (mdjvu_file_t) f, resolution);
    fclose(f);
    return 0;
}
