/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * tiffload.c - loading TIFF bitmaps
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
#include <stdlib.h>
#include <string.h>

#if HAVE_TIFF

static mdjvu_bitmap_t load_tiff(const char *path, int32 *presolution, mdjvu_error_t *perr)
{
    uint16 photometric;
    uint32 w, h;
    uint16 bits_per_sample = 0, samples_per_pixel = 0;
    float dpi;
    mdjvu_bitmap_t result;
    tsize_t scanline_size;
    unsigned char *scanline;
    uint32 i;

    TIFF *tiff = TIFFOpen(path, "r");
    *perr = NULL;
    if (!tiff)
    {
        *perr = mdjvu_get_error(mdjvu_error_fopen_read);
        return NULL;
    }

    /* test if bitonal */
    TIFFGetFieldDefaulted(tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetFieldDefaulted(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
    if (bits_per_sample != 1 || samples_per_pixel != 1)
    {
        *perr = mdjvu_get_error(mdjvu_error_corrupted_tiff);
        TIFFClose(tiff);
        return NULL;
    }

    /* photometric */
    photometric = PHOTOMETRIC_MINISWHITE;
    TIFFGetFieldDefaulted(tiff, TIFFTAG_PHOTOMETRIC, &photometric);

    /* image size */
    if (!TIFFGetFieldDefaulted(tiff, TIFFTAG_IMAGEWIDTH, &w)
     || !TIFFGetFieldDefaulted(tiff, TIFFTAG_IMAGELENGTH, &h))
    {
        *perr = mdjvu_get_error(mdjvu_error_corrupted_tiff);
        TIFFClose(tiff);
        return NULL;
    }

    /* get the resolution */
    if (presolution && TIFFGetFieldDefaulted(tiff, TIFFTAG_XRESOLUTION, &dpi))
    {
        *presolution = (int32) dpi;
    }

    result = mdjvu_bitmap_create(w, h);

    scanline_size = TIFFScanlineSize(tiff);

    if (scanline_size < mdjvu_bitmap_get_packed_row_size(result))
    {
        *perr = mdjvu_get_error(mdjvu_error_corrupted_tiff);
        TIFFClose(tiff);
        mdjvu_bitmap_destroy(result);
        return NULL;
    }

    scanline = (unsigned char *) malloc(scanline_size);

    for (i = 0; i < h; i++)
    {
        if (TIFFReadScanline(tiff, (tdata_t)scanline, i, 0) < 0)
        {
            *perr = mdjvu_get_error(mdjvu_error_corrupted_tiff);
            TIFFClose(tiff);
            free(scanline);
            mdjvu_bitmap_destroy(result);
            return NULL;
        }

        if (photometric != PHOTOMETRIC_MINISWHITE)
        {
            /* invert the row */
            int32 k;
            int32 s = (int32) scanline_size;
            for (k = 0; k < s; k++)
                scanline[k] = ~scanline[k];
        }

        /* clear the padding bits */
        if (scanline_size & 7)
            scanline[scanline_size - 1] &= ~(0xFF >> (scanline_size & 7));

        memcpy(mdjvu_bitmap_access_packed_row(result, i),
               scanline,
               mdjvu_bitmap_get_packed_row_size(result));
    }

    free(scanline);

    TIFFClose(tiff);
    return result;
}

#endif /* HAVE_TIFF */

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_load_tiff(const char *path, int32 *presolution, mdjvu_error_t *perr)
{
    #if HAVE_TIFF
        return load_tiff(path, presolution, perr);
    #else
        *perr = mdjvu_get_error(mdjvu_error_tiff_support_disabled);
        return NULL;
    #endif
}
