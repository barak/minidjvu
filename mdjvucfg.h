/*
 * mdjvucfg.h - file to be included first in all C sources in the library
 */


/*
 * This is needed, for example, to distinguish between dllexport and dllimport.
 */
#define MINIDJVU_INCLUDED_FROM_INSIDE

/*
 * Bridge to autoconf configuration file.
 */
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif
