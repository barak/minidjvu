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

MDJVU_IMPLEMENT int mdjvu_file_save_djvu_page(mdjvu_image_t image, mdjvu_file_t file, const char *dict_name, mdjvu_error_t *perr, int erosion)
{
    mdjvu_iff_t FORM, INFO, INCL, Sjbz;

    mdjvu_write_big_endian_int32(MDJVU_IFF_ID("AT&T"), file);
    FORM = mdjvu_iff_write_chunk(MDJVU_IFF_ID("FORM"), file);
        mdjvu_write_big_endian_int32(MDJVU_IFF_ID("DJVU"), file);

        if (dict_name)
        {
            INCL = mdjvu_iff_write_chunk(MDJVU_IFF_ID("INCL"), file);
                fwrite(dict_name, 1, strlen(dict_name), (FILE *) file);
            mdjvu_iff_close_chunk(INCL, file);
        }

        INFO = mdjvu_iff_write_chunk(MDJVU_IFF_ID("INFO"), file);
            mdjvu_write_info_chunk(file, image);
        mdjvu_iff_close_chunk(INFO, file);

        Sjbz = mdjvu_iff_write_chunk(MDJVU_IFF_ID("Sjbz"), file);
            if (!mdjvu_file_save_jb2(image, file, perr, erosion)) return 0;
        mdjvu_iff_close_chunk(Sjbz, file);
    mdjvu_iff_close_chunk(FORM, file);

    return 1;
}


MDJVU_IMPLEMENT int mdjvu_file_save_djvu_dictionary(mdjvu_image_t image, mdjvu_file_t file, mdjvu_error_t *perr, int erosion)
{
    mdjvu_iff_t FORM, Djbz;

    mdjvu_write_big_endian_int32(MDJVU_IFF_ID("AT&T"), file);
    FORM = mdjvu_iff_write_chunk(MDJVU_IFF_ID("FORM"), file);
        mdjvu_write_big_endian_int32(MDJVU_IFF_ID("DJVI"), file);

        Djbz = mdjvu_iff_write_chunk(MDJVU_IFF_ID("Djbz"), file);
            if (!mdjvu_file_save_jb2_dictionary(image, file, perr, erosion))
                return 0;
        mdjvu_iff_close_chunk(Djbz, file);
    mdjvu_iff_close_chunk(FORM, file);

    return 1;
}

MDJVU_IMPLEMENT int mdjvu_save_djvu_page(mdjvu_image_t image, const char *path, const char *dict, mdjvu_error_t *perr, int erosion)
{
    int result;
    FILE *f = fopen(path, "wb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_djvu_page(image, (mdjvu_file_t) f, dict, perr, erosion);
    fclose(f);
    return result;
}

MDJVU_IMPLEMENT int mdjvu_save_djvu_dictionary(mdjvu_image_t image, const char *path, mdjvu_error_t *perr, int erosion)
{
    int result;
    FILE *f = fopen(path, "wb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_djvu_dictionary(image, (mdjvu_file_t) f, perr, erosion);
    fclose(f);
    return result;
}
