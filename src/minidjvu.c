/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * minidjvu.c - an example of using the library
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

#include <minidjvu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* TODO: remove duplicated code */


/* options */
int32 dpi = 300;
int dpi_specified = 0;
int verbose = 0;
int smooth = 0;
int match = 0;
int aggression = 100;
int erosion = 0;
int clean = 0;
int warnings = 0;

/* Under Windows, there is usually no strcasecmp.
 * So here's the rewrite.
 */
static int my_strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;
    while(*s1)
    {
        int d;
        c1 = tolower(*s1++); c2 = tolower(*s2++);
        d = c1 - c2;
        if (d) return d;
    }
    return *s2;
}

static int ends_with_ignore_case(const char *s, const char *prefix)
{
    size_t sl = strlen(s);
    size_t pl = strlen(prefix);
    if (sl < pl) return 0;
    return !my_strcasecmp(s + sl - pl, prefix);
}

static void show_usage_and_exit(void)
{
    printf("minidjvu - encoding/decoding single-page bitonal DjVu files\n");
    printf("Program version %s, library version %s.\n\n", MDJVU_VERSION, mdjvu_get_version());
    printf("Usage:\n");
    printf("    minidjvu [options] <input file> <output file>\n");
    printf("Formats supported:\n");

    if (mdjvu_have_tiff_support())
        printf("    DjVu (single-page bitonal), PBM, Windows BMP, TIFF.\n");
    else
        printf("    DjVu (single-page bitonal), PBM, Windows BMP; TIFF support is OFF.\n");

    printf("Options:\n");
    printf("    -a <n>, --aggression <n>:  set aggression level (default 100)\n");
    printf("    -c, --clean                remove small black pieces\n");
    printf("    -d <n> --dpi <n>:          set resolution in dots per inch\n");
    printf("    -e, --erosion              sacrifice quality to get smaller files\n");
    printf("    -l, --lossy:               apply all lossy options (-s -c -m -e)\n");
    printf("    -m, --match:               match and substitute patterns\n");
    printf("    -s, --smooth:              remove some badly looking pixels\n");
    printf("    -v, --verbose:             print messages about everything\n");
    printf("    -w, --warnings:            do not suppress TIFF warnings\n");
    exit(2);
}

static int decide_if_bmp(const char *path)
{
    return ends_with_ignore_case(path, ".bmp");
}

static int decide_if_djvu(const char *path)
{
    return ends_with_ignore_case(path, ".djvu")
        || ends_with_ignore_case(path, ".djv");
}

static int decide_if_tiff(const char *path)
{
    return ends_with_ignore_case(path, ".tiff")
        || ends_with_ignore_case(path, ".tif");
}

/* ========================================================================= */

static mdjvu_bitmap_t load_bitmap(const char *path)
{
    mdjvu_error_t error;
    mdjvu_bitmap_t bitmap;

    if (decide_if_bmp(path))
    {
        if (verbose) printf("loading from Windows BMP file `%s'\n", path);
        bitmap = mdjvu_load_bmp(path, &error);
    }
    else if (decide_if_tiff(path))
    {
        if (verbose) printf("loading from TIFF file `%s'\n", path);
        if (!warnings)
            mdjvu_disable_tiff_warnings();
        if (dpi_specified)
            bitmap = mdjvu_load_tiff(path, NULL, &error);
        else
            bitmap = mdjvu_load_tiff(path, &dpi, &error);
        if (verbose) printf("resolution is "MDJVU_INT32_FORMAT" dpi\n", dpi);
    }
    else
    {
        if (verbose) printf("loading from PBM file `%s'\n", path);
        bitmap = mdjvu_load_pbm(path, &error);
    }

    if (!bitmap)
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }

    if (smooth)
    {
        if (verbose) printf("smoothing the bitmap\n");
        mdjvu_smooth(bitmap);
    }

    return bitmap;
}

static void save_bitmap(mdjvu_bitmap_t bitmap, const char *path)
{
    mdjvu_error_t error;
    int result;

    if (decide_if_bmp(path))
    {
        if (verbose) printf("saving to Windows BMP file `%s'\n", path);
        result = mdjvu_save_bmp(bitmap, path, dpi, &error);
    }
    else if (decide_if_tiff(path))
    {
        if (verbose) printf("saving to TIFF file `%s'\n", path);
        if (!warnings)
            mdjvu_disable_tiff_warnings();
        result = mdjvu_save_tiff(bitmap, path, &error);
    }
    else
    {
        if (verbose) printf("saving to PBM file `%s'\n", path);
        result = mdjvu_save_pbm(bitmap, path, &error);
    }

    if (!result)
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }
}

static mdjvu_image_t load_image(const char *path)
{
    mdjvu_error_t error;
    mdjvu_image_t image;

    if (verbose) printf("loading a DjVu page from `%s'\n", path);
    image = mdjvu_load_djvu_page(path, &error);
    if (!image)
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }
    if (verbose)
    {
        printf("loaded; the page has "MDJVU_INT32_FORMAT" bitmaps and "
               MDJVU_INT32_FORMAT" blits\n",
               mdjvu_image_get_bitmap_count(image),
               mdjvu_image_get_blit_count(image));
    }
    return image;
}

static void sort_and_save_image(mdjvu_image_t image, const char *path)
{
    mdjvu_error_t error;

    mdjvu_compression_options_t options = mdjvu_compression_options_create();
    mdjvu_matcher_options_t m_options = NULL;
    if (match)
    {
        m_options = mdjvu_matcher_options_create();
        mdjvu_set_aggression(m_options, aggression);
        mdjvu_set_matcher_options(options, m_options);
    }

    mdjvu_set_clean(options, clean);
    mdjvu_set_verbose(options, verbose);
    mdjvu_compress_image(image, options);
    mdjvu_compression_options_destroy(options);

    if (verbose) printf("encoding to `%s'\n", path);

    if (!mdjvu_save_djvu_page(image, path, &error, erosion))
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }
}

/* ========================================================================= */

static void decode(int argc, char **argv)
{
    mdjvu_image_t image;    /* a sequence of blits (what is stored in DjVu) */
    mdjvu_bitmap_t bitmap;  /* the result                                   */

    if (verbose) printf("\nDECODING\n");
    if (verbose) printf("________\n\n");

    image = load_image(argv[1]);
    bitmap = mdjvu_render(image);
    mdjvu_image_destroy(image);

    if (verbose)
    {
        printf("bitmap "MDJVU_INT32_FORMAT" x "MDJVU_INT32_FORMAT" rendered\n",
               mdjvu_bitmap_get_width(bitmap),
               mdjvu_bitmap_get_height(bitmap));
    }

    if (smooth)
    {
        if (verbose) printf("smoothing the bitmap\n");
        mdjvu_smooth(bitmap);
    }

    save_bitmap(bitmap, argv[2]);
    mdjvu_bitmap_destroy(bitmap);
}

static mdjvu_image_t split_and_destroy(mdjvu_bitmap_t bitmap)
{
    mdjvu_image_t image;
    if (verbose) printf("splitting the bitmap into pieces\n");
    image = mdjvu_split(bitmap, dpi, /* options:*/ NULL);
    mdjvu_bitmap_destroy(bitmap);
    if (verbose)
    {
        printf("the splitted image has "MDJVU_INT32_FORMAT" pieces\n",
                mdjvu_image_get_blit_count(image));
    }
    if (clean)
    {
        if (verbose) printf("cleaning\n");
        mdjvu_clean(image);
        if (verbose)
        {
            printf("the cleaned image has "MDJVU_INT32_FORMAT" pieces\n",
                    mdjvu_image_get_blit_count(image));
        }
    }
    return image;
}

static void encode(int argc, char **argv)
{
    mdjvu_bitmap_t bitmap;
    mdjvu_image_t image;

    if (verbose) printf("\nENCODING\n");
    if (verbose) printf("________\n\n");

    bitmap = load_bitmap(argv[1]);

    image = split_and_destroy(bitmap);
    sort_and_save_image(image, argv[2]);
    mdjvu_image_destroy(image);
}

static void recode(int argc, char **argv)
{
    mdjvu_image_t image;
    mdjvu_bitmap_t bitmap;

    if (verbose) printf("\nRECODING\n");
    if (verbose) printf("________\n\n");

    image = load_image(argv[1]);
    bitmap = mdjvu_render(image);
    mdjvu_image_destroy(image);
    if (verbose)
    {
        printf("bitmap "MDJVU_INT32_FORMAT" x "MDJVU_INT32_FORMAT" rendered\n",
               mdjvu_bitmap_get_width(bitmap),
               mdjvu_bitmap_get_height(bitmap));
    }

    if (smooth)
    {
        if (verbose) printf("smoothing the bitmap\n");
        mdjvu_smooth(bitmap);
    }

    image = split_and_destroy(bitmap);
    sort_and_save_image(image, argv[2]);
    mdjvu_image_destroy(image);
}


/* Filtering is nondjvu->nondjvu job. */
static void filter(int argc, char **argv)
{
    mdjvu_bitmap_t bitmap;

    if (verbose) printf("\nFILTERING\n");
    if (verbose) printf("_________\n\n");

    bitmap = load_bitmap(argv[1]);
    save_bitmap(bitmap, argv[2]);
    mdjvu_bitmap_destroy(bitmap);
}

/* same_option(foo, "opt") returns 1 in three cases:
 *
 *      foo is "o" (first letter of opt)
 *      foo is "opt"
 *      foo is "-opt"
 */
static int same_option(const char *option, const char *s)
{
    if (option[0] == s[0] && !option[1]) return 1;
    if (!strcmp(option, s)) return 1;
    if (option[0] == '-' && !strcmp(option + 1, s)) return 1;
    return 0;
}

static int process_options(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc && argv[i][0] == '-'; i++)
    {
        char *option = argv[i] + 1;
        if (same_option(option, "verbose"))
            verbose = 1;
        else if (same_option(option, "smooth"))
            smooth = 1;
        else if (same_option(option, "match"))
            match = 1;
        else if (same_option(option, "erosion"))
            erosion = 1;
        else if (same_option(option, "clean"))
            clean = 1;
        else if (same_option(option, "warnings"))
            warnings = 1;
        else if (same_option(option, "lossy"))
        {
            smooth = 1;
            match = 1;
            erosion = 1;
            clean = 1;
        }
        else if (same_option(option, "dpi"))
        {
            i++;
            if (i == argc) show_usage_and_exit();
            dpi = atoi(argv[i]);
            dpi_specified = 1;
            if (dpi < 20 || dpi > 2000)
            {
                fprintf(stderr, "bad resolution\n");
                exit(2);
            }
        }
        else if (same_option(option, "aggression"))
        {
            i++;
            if (i == argc) show_usage_and_exit();
            aggression = atoi(argv[i]);
            match = 1;
        }
        else
        {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            exit(2);
        }
    }
    return i;
}

int main(int argc, char **argv)
{
    int arg_start;
    const char *sanity_error_message;

    /* check sizeof(int32) == 4 and such gibberish */
    sanity_error_message = mdjvu_check_sanity();
    if (sanity_error_message)
    {
        fprintf(stderr, "%s\n", sanity_error_message);
        exit(1);
    }

    arg_start = process_options(argc, argv);

    argc -= arg_start - 1;
    argv += arg_start - 1;

    if (argc != 3)
        show_usage_and_exit();

    if (decide_if_djvu(argv[2]))
    {
        if (decide_if_djvu(argv[1]))
            recode(argc, argv);
        else
            encode(argc, argv);
    }
    else
    {
        if (decide_if_djvu(argv[1]))
            decode(argc, argv);
        else
            filter(argc, argv);
    }

    if (verbose) printf("\n");
    return 0;
}
