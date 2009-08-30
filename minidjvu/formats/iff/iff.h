/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * iff.h - read/write IFF files (DjVu files are IFF)
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

/* IFF is a very simple format that allows some parts of a file to be decorated
 * as "IFF chunks". An IFF chunk looks like this:
 *  __________________________________________     _
 * |                     |    |        |       ...  |
 * | padding (if needed) | ID | length | data  ...  |
 * |_____________________|____|________|______ ... _|
 *
 * IFF chunks may be nested.
 *
 * See DjVu 2 Spec., page 5, "Structure of DjVu files" for deeper description.
 */


/* IFF chunk identifiers are 32-bit integers.
 * The macro is here because MDJVU_IFF_ID("DJVU") looks better than 0x444A5655.
 */
#define MDJVU_IFF_ID(S) \
    ((S)[0] << 24) | \
    ((S)[1] << 16) | \
    ((S)[2] << 8)  | \
     (S)[3]

/* mdjvu_iff_t represents one IFF chunk. */
typedef struct MdjvuNonexistentIffStruct *mdjvu_iff_t;

/* Get the ID of the chunk. */
MDJVU_FUNCTION int32 mdjvu_iff_get_id(mdjvu_iff_t);

/* Get the length of the chunk's data.
 * Useful only when reading; returns 0 if we're writing to the chunk instead.
 */
MDJVU_FUNCTION int32 mdjvu_iff_get_length(mdjvu_iff_t);

/* Opens a chunk for reading. The file must be seekable. */
MDJVU_FUNCTION mdjvu_iff_t mdjvu_iff_read_chunk(mdjvu_file_t);

/* Opens a chunk for writing. The file must be seekable. */
MDJVU_FUNCTION mdjvu_iff_t mdjvu_iff_write_chunk(int32 id, mdjvu_file_t);

/* Closes a chunk.
 * If we're reading, the file cursor is set to the end of chunk.
 * If we're writing, the file cursor must be at the end of chunk before calling.
 *
 * The mdjvu_iff_t object is destroyed.
 *
 * No checks are made to ensure that chunks are closed in the proper order
 * (that is, first opened - last closed). But please do it properly.
 */
MDJVU_FUNCTION void mdjvu_iff_close_chunk(mdjvu_iff_t, mdjvu_file_t);
