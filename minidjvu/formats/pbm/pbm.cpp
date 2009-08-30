/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * pbm.c - loading and saving in PBM ("portable bitmap") format
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

static void skip_to_the_end_of_line(FILE *file)
{
    while (1)
    {
        switch(fgetc(file))
        {
            case -1: case '\r': case '\n':
                return;
        }
    }
}

static void mdjvu_skip_pbm_whitespace_and_comments(mdjvu_file_t f)
{
    FILE *file = (FILE *) f;
    int c = fgetc(file);
    while(1)
    {
        switch(c)
        {
            case ' ': case '\t': case '\r': case '\n':
                c = fgetc(file);
            break;
            case '#':
                skip_to_the_end_of_line(file);
                c = fgetc(file);
            break;
            default:
                ungetc(c,file);
                return;
        }
    }
}

MDJVU_IMPLEMENT int mdjvu_save_pbm(mdjvu_bitmap_t b, const char *path, mdjvu_error_t *perr)
{
    FILE *file = fopen(path, "wb");
    int result;
    if (perr) *perr = NULL;
    if (!file)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_pbm(b, (mdjvu_file_t) file, perr);
    fclose(file);
    return result;
}

MDJVU_IMPLEMENT int mdjvu_file_save_pbm(mdjvu_bitmap_t b, mdjvu_file_t f, mdjvu_error_t *perr)
{
    FILE *file = (FILE *) f;
    int32 bytes_per_row = mdjvu_bitmap_get_packed_row_size(b);
    int32 width = mdjvu_bitmap_get_width(b);
    int32 height = mdjvu_bitmap_get_height(b);
    int32 i;

    if (perr) *perr = NULL;

    fprintf(file, "P4\n"MDJVU_INT32_FORMAT" "MDJVU_INT32_FORMAT"\n",
            width, height);

    for (i = 0; i < height; i++)
    {
        unsigned char *row = mdjvu_bitmap_access_packed_row(b, i);
        if (fwrite(row, bytes_per_row, 1, file) != 1)
        {
            if (perr) *perr = mdjvu_get_error(mdjvu_error_io);
            return 0;
        }
    }
    return 1;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_load_pbm(const char *path, mdjvu_error_t *perr)
{
    FILE *file = fopen(path, "rb");
    mdjvu_bitmap_t result;
    if (perr) *perr = NULL;
    if (!file)
    {
        if(perr) *perr = mdjvu_get_error(mdjvu_error_fopen_read);
        return NULL;
    }
    result = mdjvu_file_load_pbm((mdjvu_file_t) file, perr);
    fclose(file);
    return result;
}

#define COMPLAIN \
{ \
    if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_pbm); \
    return NULL; \
}
MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_file_load_pbm(mdjvu_file_t f, mdjvu_error_t *perr)
{
    FILE *file = (FILE *) f;
    int32 width, height, bytes_per_row, i;
    mdjvu_bitmap_t result;
    if (perr) *perr = NULL;
    if (fgetc(file) != 'P') COMPLAIN;
    if (fgetc(file) != '4') COMPLAIN;
    mdjvu_skip_pbm_whitespace_and_comments((mdjvu_file_t) file);
    if (fscanf(file,
        MDJVU_INT32_FORMAT" "MDJVU_INT32_FORMAT, &width, &height) != 2)
    {
        COMPLAIN;
    }

    /* a fancy way to write if ( || || || ) - maybe, abandon this switch? */
    switch(fgetc(file))
    {
        case ' ': case '\t': case '\r': case '\n':
            break;
        default:
            COMPLAIN;
    }

    result = mdjvu_bitmap_create(width, height);
    bytes_per_row = mdjvu_bitmap_get_packed_row_size(result);
    for (i = 0; i < height; i++)
    {
        unsigned char *current_row = mdjvu_bitmap_access_packed_row(result, i);
        if (fread(current_row, bytes_per_row, 1, file) != 1)
        {
            mdjvu_bitmap_destroy(result);
            COMPLAIN;
        }
    }
    return result;
}
