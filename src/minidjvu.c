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

#define BMP_RESOLUTION 72 /* Resolution (in dpi) to put in BMP files */

/* options */
int32 dpi;
int verbose;
int smooth;

static void set_default_options(void)
{
    dpi = 300;
    verbose = 0;
    smooth = 0;
}

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

static void show_usage_and_exit(void)
{
    printf("minidjvu - encoding/decoding single-page bitonal DjVu files\n");
    printf("Program version %s, library version %s.\n\n", MDJVU_VERSION, mdjvu_get_version());
    printf("Usage:\n");
    printf("    minidjvu [options] <input file> <output file>\n");
    printf("Formats supported:\n");
    printf("    DjVu (single-page bitonal), PBM, Windows BMP.\n");
    printf("Options:\n");
    printf("    -d <value>, --dpi <value>: set resolution in dots per inch\n");
    printf("    -v, --verbose:             print messages about everything\n");
    printf("    -s, --smooth:              smooth the image (flips \"bad\" pixels)\n");
    exit(2);
}

static int decide_if_bmp(const char *path)
{
    const char bmp_extension[] = ".bmp";
    size_t len = strlen(path);
    const char *tail = path + len - sizeof(bmp_extension) + 1;
    return !my_strcasecmp(tail, bmp_extension);
}

static int decide_if_djvu(const char *path)
{
    const char djvu_extension1[] = ".djvu";
    const char djvu_extension2[] = ".djv";
    size_t len = strlen(path);
    const char *tail1 = path + len - sizeof(djvu_extension1) + 1;
    const char *tail2 = path + len - sizeof(djvu_extension2) + 1;
    return !my_strcasecmp(tail1, djvu_extension1)
        || !my_strcasecmp(tail2, djvu_extension2);
}

/* ========================================================================= */

static mdjvu_bitmap_t load_bitmap(const char *path)
{
    int is_bmp = decide_if_bmp(path);
    mdjvu_error_t error;
    mdjvu_bitmap_t bitmap;

    if (is_bmp)
    {
        if (verbose) printf("loading from Windows BMP file `%s'\n", path);
        bitmap = mdjvu_load_bmp(path, &error);
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
    int is_bmp = decide_if_bmp(path);
    mdjvu_error_t error;

    if (is_bmp)
    {
        if (verbose) printf("saving to Windows BMP file `%s'\n", path);
        mdjvu_save_bmp(bitmap, path, BMP_RESOLUTION, &error);
    }
    else
    {
        if (verbose) printf("saving to PBM file `%s'\n", path);
        mdjvu_save_pbm(bitmap, path, &error);
    }

    if (error)
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
        printf("loaded; the page has %d bitmaps and %d blits\n",
               mdjvu_image_get_bitmap_count(image),
               mdjvu_image_get_blit_count(image));
    }
    return image;
}

static void sort_and_save_image(mdjvu_image_t image, const char *path)
{
    mdjvu_error_t error;

    if (verbose) printf("sorting letters\n");
    mdjvu_sort_blits_and_bitmaps(image);

    /* this step is not listed in the documentation
     * because it's automatically done by mdjvu_save_djvu_page().
     * The only reason it's invoked here is for printing verbose messages.
     */
    if (verbose) printf("finding prototypes\n");
    mdjvu_find_prototypes(image);

    if (verbose) printf("encoding to `%s'\n", path);

    if (!mdjvu_save_djvu_page(image, path, &error))
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
        printf("bitmap %d x %d rendered\n",
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

static void encode(int argc, char **argv)
{
    mdjvu_bitmap_t bitmap;
    mdjvu_image_t image;

    if (verbose) printf("\nENCODING\n");
    if (verbose) printf("________\n\n");

    bitmap = load_bitmap(argv[1]);

    if (verbose) printf("splitting the bitmap into letters\n");
    image = mdjvu_split(bitmap, dpi, /* options:*/ NULL);
    mdjvu_bitmap_destroy(bitmap);
    if (verbose)
    {
        printf("the splitted image has %d pieces\n",
                mdjvu_image_get_blit_count(image));
    }
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
        printf("bitmap %d x %d rendered\n",
               mdjvu_bitmap_get_width(bitmap),
               mdjvu_bitmap_get_height(bitmap));
    }

    if (smooth)
    {
        if (verbose) printf("smoothing the bitmap\n");
        mdjvu_smooth(bitmap);
    }

    if (verbose) printf("splitting the bitmap into letters\n");
    image = mdjvu_split(bitmap, dpi, /* options:*/ NULL);
    mdjvu_bitmap_destroy(bitmap);
    if (verbose)
    {
        printf("the splitted image has %d pieces\n",
                mdjvu_image_get_blit_count(image));
    }
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
    set_default_options();
    for (i = 1; i < argc && argv[i][0] == '-'; i++)
    {
        char *option = argv[i] + 1;
        if (same_option(option, "verbose"))
            verbose = 1;
        else if (same_option(option, "smooth"))
            smooth = 1;
        else if (same_option(option, "dpi"))
        {
            i++;
            if (i == argc) show_usage_and_exit();
            dpi = atoi(argv[i]);
            if (dpi < 20 || dpi > 2000)
            {
                fprintf(stderr, "bad resolution\n");
                exit(2);
            }
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

    mdjvu_check_sanity(); /* checks sizeof(int32) == 4 and such gibberish */

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
