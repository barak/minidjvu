/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * compress.h - using all the stuff to compress
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

typedef struct MinidjvuCompressionOptions *mdjvu_compression_options_t;

/*
 * By default, options correspond to `minidjvu' run with no options.
 * That is, lossless encoding and not verbose.
 */
MDJVU_FUNCTION mdjvu_compression_options_t mdjvu_compression_options_create(void);
MDJVU_FUNCTION void mdjvu_compression_options_destroy(mdjvu_compression_options_t);

/*
 * This function gives sets matcher options.
 * To disable the matcher, pass NULL here (it's the default).
 * By calling this, you give the matcher options into ownership.
 * That is, DON'T destroy matcher options afterwards.
 */
MDJVU_FUNCTION void mdjvu_set_matcher_options(mdjvu_compression_options_t, mdjvu_matcher_options_t);

MDJVU_FUNCTION void mdjvu_set_clean(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_verbose(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_no_prototypes(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_report(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_report_start_page(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_report_total_pages(mdjvu_compression_options_t, int);

MDJVU_FUNCTION void mdjvu_compress_image(mdjvu_image_t, mdjvu_compression_options_t);
MDJVU_FUNCTION mdjvu_image_t mdjvu_compress_multipage(int n, mdjvu_image_t *pages, mdjvu_compression_options_t);
