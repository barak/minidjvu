/*
 * djvusave.c - saving DjVuBitonal pages
 */

#include "mdjvucfg.h"
#include "minidjvu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MDJVU_IMPLEMENT int mdjvu_file_save_djvu_page(mdjvu_image_t image, mdjvu_file_t file, const char *dict_name, mdjvu_error_t *perr, int erosion)
{
    mdjvu_iff_t FORM, INFO, INCL, Sjbz;

    mdjvu_write_big_endian_int32(MDJVU_IFF_ID("AT&T"), file);
    FORM = mdjvu_iff_write_chunk(MDJVU_IFF_ID("FORM"), file);
        mdjvu_write_big_endian_int32(MDJVU_IFF_ID("DJVU"), file);

        if (dict_name)
        {
            INCL = mdjvu_iff_write_chunk(MDJVU_IFF_ID("INCL"), file);
                fwrite(dict_name, 1, strlen(dict_name), (FILE *) file);
            mdjvu_iff_close_chunk(INCL, file);
        }

        INFO = mdjvu_iff_write_chunk(MDJVU_IFF_ID("INFO"), file);
            mdjvu_write_info_chunk(file, image);
        mdjvu_iff_close_chunk(INFO, file);

        Sjbz = mdjvu_iff_write_chunk(MDJVU_IFF_ID("Sjbz"), file);
            if (!mdjvu_file_save_jb2(image, file, perr, erosion)) return 0;
        mdjvu_iff_close_chunk(Sjbz, file);
    mdjvu_iff_close_chunk(FORM, file);

    return 1;
}


MDJVU_IMPLEMENT int mdjvu_file_save_djvu_dictionary(mdjvu_image_t image, mdjvu_file_t file, mdjvu_error_t *perr, int erosion)
{
    mdjvu_iff_t FORM, Djbz;

    mdjvu_write_big_endian_int32(MDJVU_IFF_ID("AT&T"), file);
    FORM = mdjvu_iff_write_chunk(MDJVU_IFF_ID("FORM"), file);
        mdjvu_write_big_endian_int32(MDJVU_IFF_ID("DJVI"), file);

        Djbz = mdjvu_iff_write_chunk(MDJVU_IFF_ID("Djbz"), file);
            if (!mdjvu_file_save_jb2_dictionary(image, file, perr, erosion))
                return 0;
        mdjvu_iff_close_chunk(Djbz, file);
    mdjvu_iff_close_chunk(FORM, file);

    return 1;
}

MDJVU_IMPLEMENT int mdjvu_save_djvu_page(mdjvu_image_t image, const char *path, const char *dict, mdjvu_error_t *perr, int erosion)
{
    int result;
    FILE *f = fopen(path, "wb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_djvu_page(image, (mdjvu_file_t) f, dict, perr, erosion);
    fclose(f);
    return result;
}

MDJVU_IMPLEMENT int mdjvu_save_djvu_dictionary(mdjvu_image_t image, const char *path, mdjvu_error_t *perr, int erosion)
{
    int result;
    FILE *f = fopen(path, "wb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_djvu_dictionary(image, (mdjvu_file_t) f, perr, erosion);
    fclose(f);
    return result;
}
