/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * thinning.h - reimplementation of frames.c. Yeah, I know.
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

/* Plasma OCR - an OCR engine
 *
 * thinning.h - getting a "framework" of a letter by thinning it.
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


/* Here's a picture illustrating what is a framework
 * (view with monospace font):
 *
 *  Original letter:         Its framework:
 *
  .....@@@@@@@@........  .....................
  ...@@@@@@@@@@@@......  ......@@@@@..........
  ..@@@@@@@@@@@@@@.....  .....@@...@@@........
  ..@@@@@...@@@@@@@....  ....@@......@@.......
  ..@@@@.....@@@@@@....  ....@........@.......
  .@@@@@.....@@@@@@....  ....@........@.......
  .@@@@@.....@@@@@@....  ....@........@.......
  ..@@@@.....@@@@@@....  ....@........@.......
  ..........@@@@@@@....  .............@.......
  .......@@@@@@@@@@....  .............@.......
  .....@@@@@@@@@@@@....  ........@@@@@@.......
  ...@@@@@@@@@@@@@@....  ......@@@....@.......
  ..@@@@@@@..@@@@@@....  .....@@......@.......
  .@@@@@@....@@@@@@....  ...@@@.......@.......
  .@@@@@.....@@@@@@....  ...@.........@.......
  @@@@@......@@@@@@....  ..@@.........@.......
  @@@@@......@@@@@@....  ..@..........@.......
  @@@@@.....@@@@@@@....  ..@..........@.......
  @@@@@@....@@@@@@@.@@@  ..@@.........@.......
  .@@@@@@@@@@@@@@@@@@@@  ...@@.....@@@@@......
  .@@@@@@@@@@@@@@@@@@@@  ....@@..@@@...@@@@@..
  ..@@@@@@@@@.@@@@@@@@.  .....@@@@............
  ....@@@@@....@@@@@...  .....................


*/


#ifndef PLASMA_OCR_THINNING_H
#define PLASMA_OCR_THINNING_H


/* Get a framework of the letter.
 * Returns a bitmap with the same width and height as the source.
 * The returned result must be disposed with free_bitmap_with_margins().
 * Might return NULL if malloc() failed.
 *
 * Input:  0 - white, nonzero - black
 * Output: 0 - white, 1 - black
 *
 * WARNING: the original bitmap is filled with garbage.
 *
 * Takes time O(width * height * thickness), whatever thickness means.
 * Just don't use it on the whole page, use on letters instead.
 */
unsigned char **thin(unsigned char **pixels, int width, int height,
                     int use_8_connectivity /* nonzero - true */);


/* One pass of thinning. The outermost black pixels are cleared.
 * `pixels': an array with margins, (0/1)
 * `buffer': a temporary array, w * h
 * Low-level routine.
 *
 * Returns 0 if the image has changed.
 */
int peel(unsigned char **pixels, unsigned char **buffer, int w, int h);


/* The result has dimensions (w + 2) * (h + 2)
 * and is to be freed with free_bitmap_with_margins(..., w + 2, h + 2).
 */
unsigned char **inverted_peel(unsigned char **pixels, int w, int h);


#endif
