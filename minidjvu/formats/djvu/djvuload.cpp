/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * djvuload.c - functions to load from single-page DjVuBitonal files
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
#include <stdlib.h>
#include <stdio.h>

static uint32 read_uint32_most_significant_byte_first(FILE *f)
{
    uint32 r = fgetc(f) << 24;
    r |= fgetc(f) << 16;
    r |= fgetc(f) << 8;
    r |= fgetc(f);
    return r;
}

typedef struct IFFChunk
{
    uint32 id;
    uint32 length;
    uint32 skipped;
    struct IFFChunk *parent;
} IFFChunk;

static void skip_in_chunk(IFFChunk *chunk, unsigned len)
{
    while (chunk)
    {
        chunk->skipped += len;
        chunk = chunk->parent;
    }
}

static void skip_to_next_sibling_chunk(FILE *file, IFFChunk *chunk)
{
    skip_in_chunk(chunk->parent, chunk->length + 8);
    fseek(file, (chunk->length + 1) & ~1, SEEK_CUR);
    chunk->id = read_uint32_most_significant_byte_first(file);
    chunk->length = read_uint32_most_significant_byte_first(file);
    chunk->skipped = 0;
}

static void get_child_chunk(FILE *file, IFFChunk *chunk, IFFChunk *parent)
{
    chunk->id = read_uint32_most_significant_byte_first(file);
    chunk->length = read_uint32_most_significant_byte_first(file);
    chunk->skipped = 0;
    chunk->parent = parent;
    skip_in_chunk(parent, 8);
}

static int find_sibling_chunk(FILE *file, IFFChunk *chunk, uint32 id)
{
    while (chunk->id != id)
    {
        if (chunk->parent && chunk->parent->skipped >= chunk->parent->length)
            return 0;
        skip_to_next_sibling_chunk(file, chunk);
    }
    return 1;
}

#define CHUNK_ID_AT_AND_T 0x41542654
#define CHUNK_ID_FORM     0x464F524D
#define ID_DJVU           0x444A5655
#define CHUNK_ID_Sjbz     0x536A627A

MDJVU_IMPLEMENT int mdjvu_locate_jb2_chunk(mdjvu_file_t file, int32 *plength, mdjvu_error_t *perr)
{
    IFFChunk FORM, Sjbz;
    FILE *f = (FILE *) file;
    uint32 i = read_uint32_most_significant_byte_first(f);
    if (perr) *perr = NULL;
    if (i != CHUNK_ID_AT_AND_T)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_djvu);
        return 0;
    }

    get_child_chunk(f, &FORM, NULL);
    if (!find_sibling_chunk(f, &FORM, CHUNK_ID_FORM))
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_djvu);
        return 0;
    }

    if (read_uint32_most_significant_byte_first(f) != ID_DJVU)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_wrong_djvu_type);
        return 0;
    }

    skip_in_chunk(&FORM, 4);

    get_child_chunk(f, &Sjbz, &FORM);
    if (!find_sibling_chunk(f, &Sjbz, CHUNK_ID_Sjbz))
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_djvu_no_Sjbz);
        return 0;
    }

    *plength = (int32) Sjbz.length;

    return 1;
}

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_file_load_djvu_page(mdjvu_file_t file, mdjvu_error_t *perr)
{
    int32 length;
    if (!mdjvu_locate_jb2_chunk(file, &length, perr))
        return NULL;
    return mdjvu_file_load_jb2(file, length, perr);
}

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_load_djvu_page(const char *path, mdjvu_error_t *perr)
{
    mdjvu_image_t result;
    FILE *f = fopen(path, "rb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_read);
        return NULL;
    }
    result = mdjvu_file_load_djvu_page((mdjvu_file_t) f, perr);
    fclose(f);
    return result;
}

