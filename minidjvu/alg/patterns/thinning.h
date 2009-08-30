/* Plasma OCR - an OCR engine
 *
 * thinning.h - morphological thinning, thickening and skeletonization
 *
 * Copyright (C) 2006  Ilya Mezhirov
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
 */


/* For an illustrated definition of thinning, thickening and skeletonization,
 * see HIPR:
 * 
 *  http://www.cee.hw.ac.uk/hipr/html/thin.html      -  Thinning
 *  http://www.cee.hw.ac.uk/hipr/html/thick.html     -  Thickening
 *  http://www.cee.hw.ac.uk/hipr/html/skeleton.html  -  Skeletonization
 *
 * Thinning is done in two passes: mark and sweep.
 * Thickening is implemented cheaply: invert-thin-invert.
 * The skeleton is computed simply by iterative thinning.
 */


#ifndef PLASMA_OCR_THINNING_H
#define PLASMA_OCR_THINNING_H


#include "common.h"

FUNCTIONS_BEGIN

/* Get a framework (skeleton, medial axis) of the letter by iterative thinning.
 * 
 * Returns a bitmap with the same width and height as the source.
 * The returned result is to be disposed with free_bitmap_with_margins().
 * Might return NULL if malloc() failed.
 *
 * Input:  0 - white, nonzero - black
 * Output: 0 - white, 1 - black
 *
 * Takes time O(width * height * thickness), whatever thickness means.
 * Just don't use it on the whole page, use on letters instead.
 */
unsigned char **skeletonize(unsigned char **pixels, int width, int height,
                            int use_8_connectivity /* nonzero - true */);


/* Same as skeletonize(), but the original bitmap is overwritten with garbage
 * and it also accepts `make_it_0_or_1' (0 - all pixels are already 0/1).
 */
unsigned char **skeletonize_internal(unsigned char **pixels, int width, int height,
                                     int use_8_connectivity, int make_it_0_or_1);


/* Low-level thinning routine.
 * Thins by 1 pixel (does both mark and sweep).
 * `pixels': an array with margins, (0/1)
 * `buffer': a temporary array, w * h
 *
 * Returns 1 if the image has changed, otherwise 0.
 */
int peel(unsigned char **pixels, unsigned char **buffer, int w, int h);


/* Thins the image N times.
 * The result is to be freed with free_bitmap_with_margins().
 */
unsigned char **thin(unsigned char **pixels, int w, int h, int N);


/* Thickens the image N times.
 * The result has size (w + N * 2) x (h + N * 2)
 * and is to be freed with free_bitmap_with_margins(..., w + N * 2, h + N * 2).
 */
unsigned char **thicken(unsigned char **pixels, int w, int h, int N);

FUNCTIONS_END

#endif
