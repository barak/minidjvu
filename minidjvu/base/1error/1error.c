/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * 1error.c - error handling
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
#include "minidjvu.h"

MDJVU_IMPLEMENT const char *mdjvu_get_error_message(mdjvu_error_t error)
{
    return (const char *) error;
}

MDJVU_IMPLEMENT mdjvu_error_t mdjvu_get_error(MinidjvuErrorType e)
{
    switch(e)
    {
        case mdjvu_error_fopen_write:
            return (mdjvu_error_t) "unable to write to file";
        case mdjvu_error_fopen_read:
            return (mdjvu_error_t) "unable to read from file";
        case mdjvu_error_io:
            return (mdjvu_error_t) "I/O error";
        case mdjvu_error_corrupted_pbm:
            return (mdjvu_error_t) "bad PBM file";
        case mdjvu_error_corrupted_bmp:
            return (mdjvu_error_t) "bad Windows BMP file";
        case mdjvu_error_corrupted_djvu:
            return (mdjvu_error_t) "bad DjVu file";
        case mdjvu_error_corrupted_jb2:
            return (mdjvu_error_t) "bad bilevel data in DjVu file";
        case mdjvu_error_wrong_djvu_type:
            return (mdjvu_error_t) "unsupported type of DjVu file";
        case mdjvu_error_djvu_no_Sjbz:
            return (mdjvu_error_t) "bilevel data not found in DjVu file";
        case mdjvu_error_recursive_prototypes:
            return (mdjvu_error_t) "somehow prototype references recursed";
        default:
            return (mdjvu_error_t)
                "some weird error happened, probably caused by a bug";
    }
}
