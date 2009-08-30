/*
 * delegate.h - choosing a representative over a class of letters
 */

MDJVU_FUNCTION void mdjvu_multipage_choose_representatives
        (int32 npages,
         mdjvu_image_t *pages,
         int32 max_tag,
         int32 *tags,
         mdjvu_bitmap_t *representatives);
