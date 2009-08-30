/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * compress.c - recommended sequences of algorithms invocations to compress
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

struct MinidjvuCompressionOptions
{
    int clean;
    int verbose;
    int no_prototypes;
    mdjvu_matcher_options_t matcher_options;
};

MDJVU_IMPLEMENT mdjvu_compression_options_t mdjvu_compression_options_create()
{
    mdjvu_compression_options_t opt = (mdjvu_compression_options_t)
        malloc(sizeof(struct MinidjvuCompressionOptions));
    opt->clean = 0;
    opt->verbose = 0;
    opt->no_prototypes = 0;
    opt->matcher_options = NULL;
    return opt;
}

MDJVU_IMPLEMENT void mdjvu_compression_options_destroy(mdjvu_compression_options_t opt)
{
    if (opt->matcher_options)
        mdjvu_matcher_options_destroy(opt->matcher_options);
    free(opt);
}

MDJVU_IMPLEMENT void mdjvu_set_matcher_options(mdjvu_compression_options_t opt, mdjvu_matcher_options_t v)
{
    if (opt->matcher_options)
        mdjvu_matcher_options_destroy(opt->matcher_options);
    opt->matcher_options = v;
}
MDJVU_IMPLEMENT void mdjvu_set_clean(mdjvu_compression_options_t opt, int v)
    {opt->clean = v;}
MDJVU_IMPLEMENT void mdjvu_set_verbose(mdjvu_compression_options_t opt, int v)
    {opt->verbose = v;}
MDJVU_IMPLEMENT void mdjvu_set_no_prototypes(mdjvu_compression_options_t opt, int v)
    {opt->no_prototypes = v;}

MDJVU_IMPLEMENT void mdjvu_find_substitutions(mdjvu_image_t image,
                                              mdjvu_matcher_options_t options)
{
    int32 i, n = mdjvu_image_get_bitmap_count(image);
    int32 *tags = (int32 *) malloc(n * sizeof(int32));
    int32 max_tag = mdjvu_classify_bitmaps_in_image(image, tags, options);
    mdjvu_bitmap_t *representatives = (mdjvu_bitmap_t *)
        calloc(max_tag + 1 /* cause starts with 1 */, sizeof(mdjvu_bitmap_t));

    for (i = 0; i < n; i++)
    {
        if (!representatives[tags[i]])
            representatives[tags[i]] = mdjvu_image_get_bitmap(image, i);
    }

    if (!mdjvu_image_has_substitutions(image))
        mdjvu_image_enable_substitutions(image);

    for (i = 0; i < n; i++)
    {
        if (!tags[i]) continue; /* that's for images with no-subst flag */
        mdjvu_image_set_substitution(image,
                                     mdjvu_image_get_bitmap(image, i),
                                     representatives[tags[i]]);
    }

    free(representatives);
    free(tags);
}

static int32 count_prototypes(mdjvu_image_t image)
{
    int32 i, s = 0, n = mdjvu_image_get_bitmap_count(image);
    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t bmp = mdjvu_image_get_bitmap(image, i);
        if (mdjvu_image_get_prototype(image, bmp))
            s++;
    }
    return s;
}

MDJVU_IMPLEMENT void mdjvu_compress_image(mdjvu_image_t image, mdjvu_compression_options_t opt)
{
    mdjvu_compression_options_t options;
    if (opt)
        options = opt;
    else
        options = mdjvu_compression_options_create();

    if (options->verbose) puts("deciding what pieces are letters");
    mdjvu_calculate_no_substitution_flag(image);

    if (options->verbose) puts("sorting letters");
    mdjvu_sort_blits_and_bitmaps(image);

    if (options->matcher_options)
    {
        if (options->verbose) puts("matching patterns");
        mdjvu_find_substitutions(image, options->matcher_options);
        if (options->verbose) puts("adjusting substitution coordinates");
        mdjvu_adjust(image);
        if (options->verbose) puts("removing unused bitmaps");
        mdjvu_image_remove_unused_bitmaps(image);
        if (options->verbose)
        {
            printf("the image now have "MDJVU_INT32_FORMAT" bitmaps\n",
                   mdjvu_image_get_bitmap_count(image));
        }
    }

    if (options->no_prototypes)
    {
        mdjvu_image_enable_prototypes(image);
    }
    else
    {
        if (options->verbose) puts("finding prototypes");
        mdjvu_find_prototypes(image);
        if (options->verbose)
        {
            printf(MDJVU_INT32_FORMAT" bitmaps have prototypes\n",
                   count_prototypes(image));
        }
    }

    if (!opt)
        mdjvu_compression_options_destroy(options);
}
