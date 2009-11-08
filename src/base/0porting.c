/*
 * 0porting.c - checking sanity of typedefs
 */

#include "../base/mdjvucfg.h"
#include <minidjvu/minidjvu.h>
#include <stdio.h>
#include <stdlib.h>

MDJVU_IMPLEMENT const char *mdjvu_check_sanity(void)
{
    if (sizeof(int32) != 4)
        return "mdjvu_check_sanity(): sizeof(int32) != 4";

    if (sizeof(uint32) != 4)
        return "mdjvu_check_sanity(): sizeof(uint32) != 4";

    if (sizeof(int16) != 2)
        return "mdjvu_check_sanity(): sizeof(int16) != 2";

    if (sizeof(uint16) != 2)
        return "mdjvu_check_sanity(): sizeof(uint16) != 2";

    return NULL;
}
