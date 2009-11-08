/*
 * tiff.c - just a couple of functions that are neither loading nor saving
 */

#include "mdjvucfg.h"

#if HAVE_TIFF
    #include <tiffio.h>
    #define MDJVU_USE_TIFFIO
#endif

#include "minidjvu.h"
#include <stdlib.h>

MDJVU_IMPLEMENT int mdjvu_have_tiff_support(void)
{
    #if HAVE_TIFF
        return 1;
    #else
        return 0;
    #endif
}

MDJVU_IMPLEMENT void mdjvu_disable_tiff_warnings(void)
{
    #if HAVE_TIFF
        TIFFSetWarningHandler(NULL);
    #endif
}
