/*
 * proto.c - searching for prototypes
 */

#include "mdjvucfg.h"
#include "minidjvu.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define THRESHOLD 21

static const int32 maxint = INT32_MAX;
/* bigint is unused? */
/* static const int32 bigint = INT32_MAX / 100 - 1; */

/* THIS THING IS THE PRIMARY BOTTLENECK FOR LOSSLESS COMPRESSION
 * TODO: OPTIMIZE IT!
 * ceiling is an optimization - if we have more, quit
 */
static int diff(mdjvu_bitmap_t image,
                mdjvu_bitmap_t prototype,
                unsigned char **image_uncompressed,
                unsigned char **proto_uncompressed,
                int32 ceiling)
{
    int32 pw = mdjvu_bitmap_get_width (prototype);
    int32 ph = mdjvu_bitmap_get_height(prototype);
    int32 iw = mdjvu_bitmap_get_width (image);
    int32 ih = mdjvu_bitmap_get_height(image);
    int32 shift_x, shift_y;
    int32 s = 0, i, n = pw + 2;
    unsigned char *ir;
    unsigned char *pr;

    if (abs(iw - pw) > 2) return maxint;
    if (abs(ih - ph) > 2) return maxint;

    ir = (unsigned char *) malloc(n);
    pr = (unsigned char *) malloc(n);

    /* (shift_x, shift_y) is a shift of image with respect to prototype */
    shift_x = (pw - pw/2) - (iw - iw/2); /* center favors right */
    shift_y = ph/2 - ih/2;               /* center favors top */

    memset(pr, 0, n);
    memset(ir, 0, n);

    for (i = -1; i <= ph; i++)
    {
        int32 y = i - shift_y;

        if (y >= 0 && y < ih)
            memcpy(ir + shift_x + 1, image_uncompressed[y], iw);
        else
            memset(ir, 0, n); /* SUB-OPTIMAL? */

        if (i >= 0 && i < ph)
            memcpy(pr + 1, proto_uncompressed[i], pw);
        else
            memset(pr, 0, n);

        /* reusing y */
        for (y = 0; y < n; y++) if (ir[y] != pr[y]) s++;
        if (s > ceiling)
        {
            free(ir);
            free(pr);
            return s;
        }
    }

    free(ir);
    free(pr);

    return s;
}

static void find_prototypes
    (mdjvu_image_t dict, unsigned char ***uncompressed_dict, mdjvu_image_t img)
{
    int32 d = dict ? mdjvu_image_get_bitmap_count(dict) : 0;
    int32 i, n = mdjvu_image_get_bitmap_count(img);
    unsigned char ***uncompressed_bitmaps = (unsigned char ***)
        malloc(n * sizeof(unsigned char **));

    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(img, i);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        uncompressed_bitmaps[i] = mdjvu_create_2d_array(w, h);
        mdjvu_bitmap_unpack_all_0_or_1(current, uncompressed_bitmaps[i]);
    }

    if (!mdjvu_image_has_prototypes(img))
        mdjvu_image_enable_prototypes(img);
    if (!mdjvu_image_has_substitutions(img))
        mdjvu_image_enable_substitutions(img);
    if (!mdjvu_image_has_masses(img))
        mdjvu_image_enable_masses(img); /* calculates them, not just enables */
    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(img, i);
        int32 mass = mdjvu_image_get_mass(img, current);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        int32 max_score = w * h * THRESHOLD / 100;
        int32 j;
        mdjvu_bitmap_t best_match = NULL;
        int32 best_score = max_score;

        for (j = 0; j < d; j++)
        {
            int32 score;
            mdjvu_bitmap_t candidate = mdjvu_image_get_bitmap(dict, j);
            int32 c_mass = mdjvu_image_get_mass(dict, candidate);
            if (abs(mass - c_mass) > best_score) continue;
            score = diff(current,
                         candidate,
                         uncompressed_bitmaps[i],
                         uncompressed_dict[j],
                         best_score);
            if (score < best_score)
            {
                best_score = score;
                best_match = candidate;
                if (!score)
                    break; /* a perfect match is found */
            }
        }

        if (best_score) for (j = 0; j < i; j++)
        {
            int32 score;
            mdjvu_bitmap_t candidate = mdjvu_image_get_bitmap(img, j);
            int32 c_mass = mdjvu_image_get_mass(img, candidate);
            if (abs(mass - c_mass) > best_score) continue;
            score = diff(current,
                         candidate,
                         uncompressed_bitmaps[i],
                         uncompressed_bitmaps[j],
                         best_score);
            if (score < best_score)
            {
                best_score = score;
                best_match = candidate;
                if (!score)
                    break; /* a perfect match is found */
            }
        }

        if (best_score)
            mdjvu_image_set_prototype(img, current, best_match);
        else
            mdjvu_image_set_substitution(img, current, best_match);
    }

    /* destroy uncompressed bitmaps */
    for (i = 0; i < n; i++)
    {
        mdjvu_destroy_2d_array(uncompressed_bitmaps[i]);
    }
    free(uncompressed_bitmaps);
}

MDJVU_IMPLEMENT void mdjvu_find_prototypes(mdjvu_image_t img)
{
    find_prototypes(NULL, NULL, img);
}

MDJVU_IMPLEMENT void mdjvu_multipage_find_prototypes(mdjvu_image_t dict,
                                                     int32 npages,
                                                     mdjvu_image_t *pages,
                                                     void (*report)(void *, int ),
                                                     void *param)
{
    int i;
    int32 n = mdjvu_image_get_bitmap_count(dict);
    unsigned char ***uncompressed_dict_bitmaps = (unsigned char ***)
        malloc(n * sizeof(unsigned char **));

    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(dict, i);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        uncompressed_dict_bitmaps[i] = mdjvu_create_2d_array(w, h);
        mdjvu_bitmap_unpack_all_0_or_1(current, uncompressed_dict_bitmaps[i]);
    }

    if (!mdjvu_image_has_masses(dict))
        mdjvu_image_enable_masses(dict); /* calculates them, not just enables */

    for (i = 0; i < npages; i++)
    {
        find_prototypes(dict, uncompressed_dict_bitmaps, pages[i]);
        report(param, i);
    }

    /* destroy uncompressed bitmaps */
    for (i = 0; i < n; i++)
    {
        mdjvu_destroy_2d_array(uncompressed_dict_bitmaps[i]);
    }
    free(uncompressed_dict_bitmaps);
}
