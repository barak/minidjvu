/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * iff.c - read/write IFF files (DjVu files are IFF)
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


static void read_align(FILE *f)
{
    long pos = ftell(f);
    if (pos & 1) fgetc(f);
}

static void write_align(FILE *f)
{
    long pos = ftell(f);
    if (pos & 1) fputc(0, f);
}


typedef struct
{
    int32 id;
    uint32 start;
    uint32 length;
    int write;
} Chunk;

MDJVU_IMPLEMENT int32 mdjvu_iff_get_id(mdjvu_iff_t iff)
{
    return ((Chunk *) iff) -> id;
}

MDJVU_IMPLEMENT int32 mdjvu_iff_get_length(mdjvu_iff_t iff)
{
    return ((Chunk *) iff) -> length;
}

MDJVU_IMPLEMENT mdjvu_iff_t mdjvu_iff_read_chunk(mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    Chunk *chunk = (Chunk *) malloc(sizeof(Chunk));
    read_align(f);
    chunk->id = mdjvu_read_big_endian_int32(file);
    chunk->length = mdjvu_read_big_endian_int32(file);
    chunk->start = ftell(f);
    chunk->write = 0;

    return (mdjvu_iff_t) chunk;
}

MDJVU_IMPLEMENT mdjvu_iff_t mdjvu_iff_write_chunk(int32 id, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    Chunk *chunk = (Chunk *) malloc(sizeof(Chunk));
    write_align(f);
    mdjvu_write_big_endian_int32(id, file);
    mdjvu_write_big_endian_int32(0, file); /* will be length */

    chunk->id = id;
    chunk->start = ftell(f);
    chunk->length = 0;
    chunk->write = 1;

    return (mdjvu_iff_t) chunk;
}

MDJVU_IMPLEMENT void mdjvu_iff_close_chunk(mdjvu_iff_t iff, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    Chunk *chunk = (Chunk *) iff;

    if (chunk->write)
    {
        long pos = ftell(f);
        uint32 length = pos - chunk->start;
        fseek(f, chunk->start - 4, SEEK_SET);
        mdjvu_write_big_endian_int32(length, file);
        fseek(f, pos, SEEK_SET);
    }
    else
    {
        fseek(f, chunk->start + chunk->length, SEEK_SET);
    }

    free(chunk);
}
