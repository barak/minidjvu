/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * 0porting.h - a portability header
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

#ifndef MDJVU_USE_TIFFIO /* kluge not to typedef twice when using tiffio.h */
    typedef int int32;
    typedef unsigned int uint32;
    typedef unsigned short uint16;
    typedef short int16;
#endif

#define MDJVU_INT32_FORMAT "%d"
#define MDJVU_INT16_FORMAT "%d"
#define MDJVU_UINT32_FORMAT "%u"
#define MDJVU_UINT16_FORMAT "%u"

/* MDJVU_FUNCTION and MDJVU_IMPLEMENT are prefixes of exported functions.
 * MDJVU_FUNCTION is for declarations, MDJVU_IMPLEMENT is for implementations.
 * So, it's like this:
 *
 *  // foo.h
 *  MDJVU_FUNCTION mdjvu_foo(void);
 *
 *  // foo.c
 *  MDJVU_IMPLEMENT mdjvu_foo(void)
 *  {
 *      ...
 *  }
 */

#ifdef __cplusplus
    #define MDJVU_C_EXPORT_PREFIX extern "C"
#else
    #define MDJVU_C_EXPORT_PREFIX
#endif

#if defined(windows) || defined(WIN32)
    #ifdef MINIDJVU_INCLUDED_FROM_INSIDE
        #define MDJVU_FUNCTION MDJVU_C_EXPORT_PREFIX __declspec(dllexport)
        #define MDJVU_IMPLEMENT __declspec(dllexport)
    #else
        #define MDJVU_FUNCTION MDJVU_C_EXPORT_PREFIX __declspec(dllimport)
        #define MDJVU_IMPLEMENT __declspec(dllimport)
    #endif
#else
    #define MDJVU_FUNCTION MDJVU_C_EXPORT_PREFIX
    #define MDJVU_IMPLEMENT
#endif

/* Convenience macros. */
#define MDJVU_MALLOC(T) ((T *) malloc(sizeof(T)))
#define MDJVU_MALLOCV(T,N) ((T *) malloc((N) * sizeof(T)))
#define MDJVU_FREE(P) free(P)
#define MDJVU_FREEV(P) free(P)


/* Check that the portability typedefs work as expected.
 * If not, returns an error message.
 * Returns NULL if OK.
 */
MDJVU_FUNCTION const char *mdjvu_check_sanity(void);
