/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * djvusave.c - saving DjVuBitonal pages
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_ID_AT_AND_T 0x41542654
#define CHUNK_ID_FORM     0x464F524D
#define ID_DJVU           0x444A5655
#define CHUNK_ID_INFO     0x494E464F
#define CHUNK_ID_Sjbz     0x536A627A

#define VERSION_STAMP 24
#define RESOLUTION 300
#define GAMMA 278


static void write_uint32_most_significant_byte_first(uint32 i, FILE *f)
{
    fputc(i >> 24, f);
    fputc(i >> 16, f);
    fputc(i >> 8, f);
    fputc(i, f);
}

static void write_uint16_most_significant_byte_first(uint16 i, FILE *f)
{
    fputc(i >> 8, f);
    fputc(i, f);
}

static void write_uint16_least_significant_byte_first(uint16 i, FILE *f)
{
    fputc(i, f);
    fputc(i >> 8, f);
}

MDJVU_IMPLEMENT int mdjvu_file_save_djvu_page(mdjvu_image_t image, mdjvu_file_t file, mdjvu_error_t *perr, int erosion)
{
    FILE *f = (FILE *) file;
    long total_length;
    char buffer[42];
    int32 w, h, dpi;
    int saving_result;

    if (perr) *perr = NULL;

    memset(buffer, 0, sizeof(buffer));
    fwrite(buffer, 1, 42, f);

    saving_result = mdjvu_file_save_jb2(image, file, perr, erosion);

    if (!saving_result)
        return 0;

    total_length = ftell(f);
    fseek(f, 42, SEEK_SET);
    w = mdjvu_image_get_width(image);
    h = mdjvu_image_get_height(image);
    dpi = mdjvu_image_get_resolution(image);
    if (!dpi) dpi = 300;

    rewind(f);

    write_uint32_most_significant_byte_first(CHUNK_ID_AT_AND_T, f);
    write_uint32_most_significant_byte_first(CHUNK_ID_FORM, f);
    write_uint32_most_significant_byte_first(total_length - 12, f);
    write_uint32_most_significant_byte_first(ID_DJVU, f);
    write_uint32_most_significant_byte_first(CHUNK_ID_INFO, f);
    write_uint32_most_significant_byte_first(10, f);
    write_uint16_most_significant_byte_first((uint16) w, f);
    write_uint16_most_significant_byte_first((uint16) h, f);
    write_uint16_least_significant_byte_first(VERSION_STAMP, f);
    write_uint16_least_significant_byte_first((uint16) dpi, f);
    write_uint16_least_significant_byte_first(GAMMA, f);
    write_uint32_most_significant_byte_first(CHUNK_ID_Sjbz, f);
    write_uint32_most_significant_byte_first(total_length - 42, f);

    return 1;
}

MDJVU_IMPLEMENT int mdjvu_save_djvu_page(mdjvu_image_t image, const char *path, mdjvu_error_t *perr, int erosion)
{
    int result;
    FILE *f = fopen(path, "wb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_djvu_page(image, (mdjvu_file_t) f, perr, erosion);
    fclose(f);
    return result;
}
