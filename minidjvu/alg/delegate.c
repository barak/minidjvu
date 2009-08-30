/*
 * delegate.c - choosing a representative over a class of letters
 */

#include "mdjvucfg.h"
#include "minidjvu.h"
#include <string.h>

MDJVU_IMPLEMENT void mdjvu_multipage_choose_representatives
        (int32 npages,
         mdjvu_image_t *pages,
         int32 max_tag,
         int32 *tags,
         mdjvu_bitmap_t *representatives)
{
    int page_number;
    int32 total_bitmaps_passed = 0;
    memset(representatives, 0, (max_tag + 1) * sizeof(mdjvu_bitmap_t));
    for (page_number = 0; page_number < npages; page_number++)
    {
        mdjvu_image_t page = pages[page_number];
        int32 bitmap_count = mdjvu_image_get_bitmap_count(page);
        int32 i; /* index of bitmap in a page */

        for (i = 0; i < bitmap_count; i++)
        {
            int32 tag = tags[total_bitmaps_passed++];
            if (!tag) continue; /* skip non-substitutable bitmaps */
            if (!representatives[tag])
                representatives[tag] = mdjvu_image_get_bitmap(page, i);
        }
    }
}
