/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * 5image.c - manipulating split images, the main data structure of minidjvu
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
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* _______________________________   artefacts   ___________________________ */

/* Artefacts are pieces of information stored for every bitmap. */
/* Each artefact type is initially disabled, but may be turned on later. */

/* When adding new artefact type, see if it needs destructor other than just
 * cleaning the memory. Modify mdjvu_image_destroy() as necessary.
 */
enum
{
    a_prototype,
    a_substitution,
    a_mass,
    a_big,
    a_nosubst,
    artefacts_count
};

const int32 artefact_sizes[] = {sizeof(mdjvu_bitmap_t),
                                sizeof(mdjvu_bitmap_t),
                                sizeof(int32),
                                1, 1};

#define MAX_ARTEFACT_SIZE 16  /* supposing that pointers can't have size > 16 */

static void initialize_artefact(void **artefacts, mdjvu_bitmap_t bitmap, int a)
{
    int32 index = mdjvu_bitmap_get_index(bitmap);
    switch(a)
    {
        case a_prototype:
            ((mdjvu_bitmap_t *) artefacts[a_prototype])[index] = NULL;
        break;
        case a_substitution:
            ((mdjvu_bitmap_t *) artefacts[a_substitution])[index] = NULL;
        break;
        case a_mass:
            ((int32 *) artefacts[a_mass])[index] =
                mdjvu_bitmap_get_mass(bitmap);
        break;
        case a_nosubst:
            ((unsigned char *) artefacts[a_nosubst])[index] = 0;
        break;
        case a_big:
            ((unsigned char *) artefacts[a_big])[index] = 0;
        break;
    }
}

static void initialize_artefacts(void **artefacts, mdjvu_bitmap_t bitmap)
{
    int a;
    for (a = 0; a < artefacts_count; a++)
    {
        if (artefacts[a])
            initialize_artefact(artefacts, bitmap, a);
    }
}

typedef struct
{
    int32 width, height;

    /* bitmaps */
    mdjvu_bitmap_t *bitmaps;
    int32 bitmaps_count, bitmaps_allocated;

    /* blits */
    int32 *x, *y;
    mdjvu_bitmap_t *blits;
    int32 blits_count, blits_allocated;

    /* image additional info */
    mdjvu_bitmap_t *dictionary;
    int32 dictionary_size;
    int32 resolution; /* 0 - unknown */

    void *artefacts[artefacts_count];
} Image;

/* ______________________________   create   ________________________________ */

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_image_create(int32 width, int32 height)
{
    int i;
    Image *image = (Image *) malloc(sizeof(Image));
    image->width = width;
    image->height = height;

    image->bitmaps_allocated = 16;
    image->bitmaps = (mdjvu_bitmap_t *)
        malloc(image->bitmaps_allocated * sizeof(mdjvu_bitmap_t));
    image->bitmaps_count = 0;

    image->blits_allocated = 32;
    image->x = (int32 *) malloc(image->blits_allocated * sizeof(int32));
    image->y = (int32 *) malloc(image->blits_allocated * sizeof(int32));
    image->blits = (mdjvu_bitmap_t *)
        malloc(image->blits_allocated * sizeof(mdjvu_bitmap_t));
    image->blits_count = 0;

    image->dictionary_size = 0;
    image->dictionary = NULL;
    image->resolution = 0;

    for (i = 0; i < artefacts_count; i++)
        image->artefacts[i] = NULL;

    return (mdjvu_image_t) image;
}

/* ______________________________   destroy   _______________________________ */

#define IMG ((Image *) image)

MDJVU_IMPLEMENT void mdjvu_image_destroy(mdjvu_image_t image)
{
    int32 i;
    int k;
    free(IMG->blits);
    free(IMG->x);
    free(IMG->y);
    for (k = 0; k < artefacts_count; k++)
    {
        if (IMG->artefacts[k])
            free(IMG->artefacts[k]);
    }
    for (i = 0; i < IMG->bitmaps_count; i++)
        mdjvu_bitmap_destroy(IMG->bitmaps[i]);
    free(IMG->bitmaps);
    free(IMG);
}

/* ______________________________   get/set   ______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_get_width(mdjvu_image_t image)
    { return IMG->width; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_height(mdjvu_image_t image)
    { return IMG->height; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_bitmap_count(mdjvu_image_t image)
    { return IMG->bitmaps_count; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_count(mdjvu_image_t image)
    { return IMG->blits_count; }

/* ______________________________   bitmaps   ______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_add_bitmap(mdjvu_image_t image, mdjvu_bitmap_t bmp)
{
    if (IMG->bitmaps_count == IMG->bitmaps_allocated)
    {
        int i;
        IMG->bitmaps_allocated <<= 1;
        IMG->bitmaps = (mdjvu_bitmap_t *) realloc(IMG->bitmaps,
                            IMG->bitmaps_allocated * sizeof(mdjvu_bitmap_t));
        for (i = 0; i < artefacts_count; i++)
        {
            if (IMG->artefacts[i])
            {
                IMG->artefacts[i] = realloc(IMG->artefacts[i],
                            IMG->bitmaps_allocated * artefact_sizes[i]);
            }
        }
    }
    IMG->bitmaps[IMG->bitmaps_count] = bmp;
    mdjvu_bitmap_set_index(bmp, IMG->bitmaps_count);
    initialize_artefacts(IMG->artefacts, bmp);
    return IMG->bitmaps_count++;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_bitmap(mdjvu_image_t image, int32 i)
{
    return IMG->bitmaps[i];
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_new_bitmap(mdjvu_image_t image, int32 w, int32 h)
{
    mdjvu_bitmap_t bmp = mdjvu_bitmap_create(w, h);
    mdjvu_image_add_bitmap(image, bmp);
    return bmp;
}

MDJVU_IMPLEMENT void mdjvu_image_exchange_bitmaps
    (mdjvu_image_t image, int32 i1, int32 i2)
{
    int a;
    mdjvu_bitmap_t b1 = IMG->bitmaps[i1], b2 = IMG->bitmaps[i2];
    if (i1 == i2) return;

    IMG->bitmaps[i1] = b2;
    IMG->bitmaps[i2] = b1;
    mdjvu_bitmap_set_index(b1, i2);
    mdjvu_bitmap_set_index(b2, i1);

    for (a = 0; a < artefacts_count; a++)
    {
        if (IMG->artefacts[a])
        {
            char buf[MAX_ARTEFACT_SIZE];
            memcpy(buf,
                   ((char *) IMG->artefacts[a]) + i1 * artefact_sizes[a],
                   artefact_sizes[a]);
            memcpy(((char *) IMG->artefacts[a]) + i1 * artefact_sizes[a],
                   ((char *) IMG->artefacts[a]) + i2 * artefact_sizes[a],
                   artefact_sizes[a]);
            memcpy(((char *) IMG->artefacts[a]) + i2 * artefact_sizes[a],
                   buf,
                   artefact_sizes[a]);
        }
    }
}

MDJVU_IMPLEMENT int mdjvu_image_check_indices(mdjvu_image_t image)
{
    int32 n = IMG->bitmaps_count;
    int32 i;
    for (i = 0; i < n; i++)
    {
        if (mdjvu_bitmap_get_index(IMG->bitmaps[i]) != i)
            return 0;
    }
    return 1;
}

MDJVU_IMPLEMENT void mdjvu_image_remove_unused_bitmaps(mdjvu_image_t image)
{
    int32 b = IMG->blits_count;
    int32 n = IMG->bitmaps_count;
    int32 i, filled;
    int32 *use_count = (int32 *) calloc(n, sizeof(int32));
    mdjvu_bitmap_t *new_bitmaps;
    int32 new_bitmaps_count, a;
    void *new_artefacts[artefacts_count];

    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bmp = IMG->blits[i];
        int32 index = mdjvu_bitmap_get_index(bmp);
        use_count[index]++;
    }

    new_bitmaps_count = 0;
    for (i = 0; i < n; i++)
    {
        if (use_count[i])
            new_bitmaps_count++;
    }

    /* create new bitmap and artefact placeholders */
    new_bitmaps = (mdjvu_bitmap_t *)
        malloc(new_bitmaps_count * sizeof(mdjvu_bitmap_t));
    for (a = 0; a < artefacts_count; a++)
    {
        if (IMG->artefacts[a])
            new_artefacts[a] = malloc(new_bitmaps_count * artefact_sizes[a]);
        else
            new_artefacts[a] = NULL;
    }

    filled = 0;
    for (i = 0; i < n; i++)
    {
        if (use_count[i])
        {
            mdjvu_bitmap_set_index(IMG->bitmaps[i], filled);
            new_bitmaps[filled] = IMG->bitmaps[i];
            for (a = 0; a < artefacts_count; a++)
            {
                if (IMG->artefacts[a])
                {
                    memcpy((char *) new_artefacts[a] + filled * artefact_sizes[a],
                           (char *) IMG->artefacts[a] + i * artefact_sizes[a],
                           artefact_sizes[a]);
                }
            }
            filled++;
        }
        else
            mdjvu_bitmap_destroy(IMG->bitmaps[i]);
    }

    free(use_count);
    free(IMG->bitmaps);
    IMG->bitmaps = new_bitmaps;
    IMG->bitmaps_count = IMG->bitmaps_allocated = new_bitmaps_count;

    for (a = 0; a < artefacts_count; a++)
    {
        if (IMG->artefacts[a])
        {
            free(IMG->artefacts[a]);
            IMG->artefacts[a] = new_artefacts[a];
            /* another place to delete artefacts that need deleting... */
        }
    }
}


/* _______________________________   blits   _______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_add_blit(mdjvu_image_t image,
                                          int32 x, int32 y,
                                          mdjvu_bitmap_t bitmap)
{
    if (IMG->blits_count == IMG->blits_allocated)
    {
        IMG->blits_allocated <<= 1;
        IMG->x = (int32 *) realloc(IMG->x,
                                IMG->blits_allocated * sizeof(int32));
        IMG->y = (int32 *) realloc(IMG->y,
                                IMG->blits_allocated * sizeof(int32));
        IMG->blits = (mdjvu_bitmap_t *) realloc(IMG->blits,
                                IMG->blits_allocated * sizeof(mdjvu_bitmap_t));
    }
    IMG->x[IMG->blits_count] = x;
    IMG->y[IMG->blits_count] = y;
    IMG->blits[IMG->blits_count] = bitmap;
    return IMG->blits_count++;
}

MDJVU_IMPLEMENT void mdjvu_image_remove_NULL_blits(mdjvu_image_t image)
{
    int32 *new_x = (int32 *) malloc(IMG->blits_count * sizeof(int32));
    int32 *new_y = (int32 *) malloc(IMG->blits_count * sizeof(int32));
    mdjvu_bitmap_t *new_blits = (mdjvu_bitmap_t *)
        malloc(IMG->blits_count * sizeof(mdjvu_bitmap_t));
    int32 filled = 0, i;

    for (i = 0; i < IMG->blits_count; i++)
    {
        if (IMG->blits[i])
        {
            new_x[filled] = IMG->x[i];
            new_y[filled] = IMG->y[i];
            new_blits[filled] = IMG->blits[i];
            filled++;
        }
    }

    free(IMG->x);
    free(IMG->y);
    free(IMG->blits);
    IMG->x = new_x;
    IMG->y = new_y;
    IMG->blits = new_blits;
    IMG->blits_allocated = IMG->blits_count;
    IMG->blits_count = filled;
}


MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_x(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->x[i];
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_y(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->y[i];
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_x(mdjvu_image_t image, int32 i, int32 x)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->x[i] = x;
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_y(mdjvu_image_t image, int32 i, int32 y)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->y[i] = y;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t
    mdjvu_image_get_blit_bitmap(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->blits[i];
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_bitmap(mdjvu_image_t image, int32 i, mdjvu_bitmap_t b)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->blits[i] = b;
}


MDJVU_IMPLEMENT void mdjvu_image_exchange_blits(mdjvu_image_t image, int32 i, int32 j)
{
    int32 t; mdjvu_bitmap_t b;
    if (i == j) return;
    t = IMG->x[i];     IMG->x[i]     = IMG->x[j];     IMG->x[j] = t;
    t = IMG->y[i];     IMG->y[i]     = IMG->y[j];     IMG->y[j] = t;
    b = IMG->blits[i]; IMG->blits[i] = IMG->blits[j]; IMG->blits[j] = b;
}


/* _____________________________   image info   ____________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_get_resolution(mdjvu_image_t image)
{
    return IMG->resolution;
}

MDJVU_IMPLEMENT void mdjvu_image_set_resolution(mdjvu_image_t image, int32 dpi)
{
    IMG->resolution = dpi;
}

/* _____________________________   artefacts   _____________________________ */

static void enable_artefact(mdjvu_image_t image, int artefact_index)
{
    int32 i;
    int32 n = IMG->bitmaps_count;
    if (!IMG->artefacts[artefact_index])
    {
        IMG->artefacts[artefact_index] =
            malloc(IMG->bitmaps_allocated * artefact_sizes[artefact_index]);
    }
    for (i = 0; i < n; i++)
    {
        initialize_artefact(IMG->artefacts, IMG->bitmaps[i], artefact_index);
    }
}

static void disable_artefact(mdjvu_image_t image, int artefact_index)
{
    if (IMG->artefacts[artefact_index])
    {
        free(IMG->artefacts[artefact_index]);
        IMG->artefacts[artefact_index] = NULL;
    }
}

MDJVU_IMPLEMENT void mdjvu_image_enable_prototypes(mdjvu_image_t image)
    { enable_artefact(image, a_prototype); }

MDJVU_IMPLEMENT void mdjvu_image_enable_substitutions(mdjvu_image_t image)
    { enable_artefact(image, a_substitution); }

MDJVU_IMPLEMENT void mdjvu_image_enable_masses(mdjvu_image_t image)
    { enable_artefact(image, a_mass); }

MDJVU_IMPLEMENT void mdjvu_image_enable_suspiciously_big_flag(mdjvu_image_t image)
    { enable_artefact(image, a_big); }

MDJVU_IMPLEMENT void mdjvu_image_enable_no_substitution_flag(mdjvu_image_t image)
    { enable_artefact(image, a_nosubst); }


MDJVU_IMPLEMENT void mdjvu_image_disable_prototypes(mdjvu_image_t image)
    { disable_artefact(image, a_prototype); }

MDJVU_IMPLEMENT void mdjvu_image_disable_substitutions(mdjvu_image_t image)
    { disable_artefact(image, a_substitution); }

MDJVU_IMPLEMENT void mdjvu_image_disable_masses(mdjvu_image_t image)
    { disable_artefact(image, a_mass); }

MDJVU_IMPLEMENT void mdjvu_image_disable_suspiciously_big_flag(mdjvu_image_t image)
    { enable_artefact(image, a_big); }

MDJVU_IMPLEMENT void mdjvu_image_disable_no_substitution_flag(mdjvu_image_t image)
    { enable_artefact(image, a_nosubst); }


MDJVU_IMPLEMENT int mdjvu_image_has_prototypes(mdjvu_image_t image)
    { return IMG->artefacts[a_prototype] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_substitutions(mdjvu_image_t image)
    { return IMG->artefacts[a_substitution] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_masses(mdjvu_image_t image)
    { return IMG->artefacts[a_mass] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_suspiciously_big_flag(mdjvu_image_t image)
    { return IMG->artefacts[a_big] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_no_substitution_flag(mdjvu_image_t image)
    { return IMG->artefacts[a_nosubst] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_get_no_substitution_flag(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((unsigned char *) IMG->artefacts[a_nosubst])
        [mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_no_substitution_flag(mdjvu_image_t image, mdjvu_bitmap_t b, int v)
{
    ((unsigned char *) IMG->artefacts[a_nosubst])
        [mdjvu_bitmap_get_index(b)] = v ? 1 : 0; /* not to fail on 256... */
}

MDJVU_IMPLEMENT int mdjvu_image_get_suspiciously_big_flag(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((unsigned char *) IMG->artefacts[a_big])
        [mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_suspiciously_big_flag(mdjvu_image_t image, mdjvu_bitmap_t b, int v)
{
    ((unsigned char *) IMG->artefacts[a_big])
        [mdjvu_bitmap_get_index(b)] = v ? 1 : 0; /* not to fail on 256... */
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_prototype(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((mdjvu_bitmap_t *) IMG->artefacts[a_prototype])
            [mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_prototype(mdjvu_image_t image, mdjvu_bitmap_t b, mdjvu_bitmap_t prototype)
{
    ((mdjvu_bitmap_t *) IMG->artefacts[a_prototype])
        [mdjvu_bitmap_get_index(b)] = prototype;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_substitution(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    mdjvu_bitmap_t subst;
    if (!IMG->artefacts[a_substitution]) return b;
    if (!b) return b; /* special handy case */
    assert(IMG->bitmaps[mdjvu_bitmap_get_index(b)] == b);
    subst =
        ((mdjvu_bitmap_t *) IMG->artefacts[a_substitution])
            [mdjvu_bitmap_get_index(b)];
    return subst ? subst : b;
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_bitmap_mass(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((int32 *) IMG->artefacts[a_mass])[mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_substitution(mdjvu_image_t image, mdjvu_bitmap_t b, mdjvu_bitmap_t s)
{
    if (s)
        s = mdjvu_image_get_substitution(image, s);

    ((mdjvu_bitmap_t *) IMG->artefacts[a_substitution])
        [mdjvu_bitmap_get_index(b)] = s;
}

/* __________________________   removing margins   _________________________ */

MDJVU_IMPLEMENT void mdjvu_image_remove_bitmap_margins(mdjvu_image_t image)
{
    int32 *delta_x, *delta_y;
    int32 n = IMG->bitmaps_count;
    int32 b = IMG->blits_count;
    int32 i;

    delta_x = (int32 *) malloc(n * sizeof(int32));
    delta_y = (int32 *) malloc(n * sizeof(int32));

    for (i = 0; i < n; i++)
        mdjvu_bitmap_remove_margins(IMG->bitmaps[i], &delta_x[i], &delta_y[i]);

    for (i = 0; i < b; i++)
    {
        int32 blit_index = mdjvu_bitmap_get_index(IMG->blits[i]);
        IMG->x[i] += delta_x[blit_index];
        IMG->y[i] += delta_y[blit_index];
    }

    free(delta_x);
    free(delta_y);
}
