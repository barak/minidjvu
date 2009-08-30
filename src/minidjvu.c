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

#define BMP_RESOLUTION 72 /* Resolution to put in BMP files */

void show_usage_and_exit()
{
    printf("minidjvu - decoding single-page bitonal DjVu files\n");
    printf("Program version %s, library version %s.\n\n", MDJVU_VERSION, mdjvu_get_version());
    printf("Usage:\n");
    printf("    minidjvu <input.djvu> <output>\n");
    printf("No options supported.\n");
    printf("Output file can be PBM or Windows BMP (determined by extension).\n");
    printf("(warning: resolution record in BMP may mismatch).\n"); /* FIXME */
    exit(2);
}

int decide_if_bmp(const char *path)
{
    /* The point of all this nightmare
     * is case-insensitive comparison.
     * We check if path ends with ".bmp".
     */
    const char bmp_extension[] = ".bmp";
    const int size = 5;
    char buf[5]; // not to have variable-sized array

    size_t len = strlen(path);
    const char *tail = path + len - size + 1;
    int i;
    if (len < size - 1) return 0;
    strncpy(buf, tail, size);
    for (i = 0; i < size; i++)
    {
        buf[i] = tolower(buf[i]);
    }
    return !strcmp(tail, bmp_extension);
}

int main(int argc, char **argv)
{
    int is_bmp;
    mdjvu_image_t image;    /* a sequence of blits (what is stored in DjVu) */
    mdjvu_bitmap_t bitmap;  /* the result                                   */

    mdjvu_check_sanity(); /* checks sizeof(int32) == 4 and such */

    if (argc != 3)
        show_usage_and_exit();
    is_bmp = decide_if_bmp(argv[2]);

    image = mdjvu_load_from_djvu(argv[1]);
    if (!image)
    {
        fprintf(stderr, "Error while loading file `%s'\n", argv[1]);
        exit(1);
    }
    bitmap = mdjvu_render(image);
    mdjvu_image_destroy(image);

    if (is_bmp)
        mdjvu_save_to_bmp(bitmap, argv[2], BMP_RESOLUTION);
    else
        mdjvu_save_to_pbm(bitmap, argv[2]);

    mdjvu_bitmap_destroy(bitmap);
}
