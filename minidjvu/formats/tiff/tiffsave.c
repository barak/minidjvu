/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * tiffsave.c - saving TIFF bitmaps
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

#if HAVE_TIFF
    #include <tiffio.h>
    #define MDJVU_USE_TIFFIO
#endif

#include "minidjvu.h"

#if HAVE_TIFF

#ifndef COMPRESSION_PACKBITS
    #define COMPRESSION_PACKBITS 32771
#endif

static int save_tiff(mdjvu_bitmap_t bitmap, const char *path, mdjvu_error_t *perr)
{
    int32 w = mdjvu_bitmap_get_width(bitmap);
    int32 h = mdjvu_bitmap_get_height(bitmap);
    int32 compression = COMPRESSION_NONE;
    int32 i;
    TIFF * tiff;

    *perr = NULL;

    if (TIFFFindCODEC(COMPRESSION_PACKBITS))
        compression = COMPRESSION_PACKBITS;

    tiff = TIFFOpen(path, "w");

    if (!tiff)
    {
        *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }

    /* FIXME: save resolution */
    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, (uint32) w);
    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, (uint32) h);
    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, (uint16) 1);
    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, (uint16) 1);
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);

    if (mdjvu_bitmap_get_packed_row_size(bitmap) != TIFFScanlineSize(tiff))
    {
        /* FIXME: not very accurate error reporting */
        *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }

    for (i = 0; i < h; i++)
        TIFFWriteScanline(tiff,
                          mdjvu_bitmap_access_packed_row(bitmap, i), i,
                          0);

    TIFFClose(tiff);

    return 1;
}

#endif /* HAVE_TIFF */

MDJVU_IMPLEMENT int mdjvu_save_tiff(mdjvu_bitmap_t bitmap, const char *path, mdjvu_error_t *perr)
{
    #if HAVE_TIFF
        return save_tiff(bitmap, path, perr);
    #else
        *perr = mdjvu_get_error(mdjvu_error_tiff_support_disabled);
        return 0;
    #endif
}
