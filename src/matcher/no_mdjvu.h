/*
 * no_mdjvu.h - stuff for compiling the pattern matcher outside of minidjvu
 */


/*
 * to compile the pattern matcher without the rest of minidjvu,
 * do this:
 *
 *  mv no_mdjvu.h minidjvu.h
 *  touch mdjvucfg.h
 *  g++ -c *.cpp
 */

#define NO_MINIDJVU
#define MDJVU_FUNCTION
#define MDJVU_IMPLEMENT
typedef int int32;
#include "patterns.h"
#include "classify.h" /* to compile it with the classificator */
