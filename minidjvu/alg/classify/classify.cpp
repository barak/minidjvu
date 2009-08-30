/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * classify.c - classifying patterns
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


#include "mdjvucfg.h"
#include "minidjvu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Stuff for not using malloc in C++
 * (left here for DjVuLibre compatibility)
 */
#ifdef __cplusplus
# define MALLOC(Type)    new Type
# define FREE(p)         delete p
# define MALLOCV(Type,n) new Type[n]
# define FREEV(p)        delete [] p
#else
# define MALLOC(Type)    ((Type*)malloc(sizeof(Type)))
# define FREE(p)         do{if(p)free(p);}while(0)
# define MALLOCV(Type,n) ((Type*)malloc(sizeof(Type)*(n)))
# define FREEV(p)        do{if(p)free(p);}while(0)
#endif


/* Classes are single-linked lists with an additional pointer to the last node.
 * This is an class item.
 */
typedef struct ClassNode
{
    mdjvu_pattern_t ptr;
    struct ClassNode *next;        /* NULL if this node is the last one */
    struct ClassNode *global_next; /* next among all nodes to classify  */
    int32 tag;                     /* filled before the final dumping   */
} ClassNode;

/* Classes themselves are composed in double-linked list. */
typedef struct Class
{
    ClassNode *first, *last;
    struct Class *prev_class;
    struct Class *next_class;
    int32 last_page; /* last page on which this class was met */
} Class;


typedef struct Classification
{
    Class *first_class;
    ClassNode *first_node, *last_node;
} Classification;

/* Creates an empty class and links it to the list of classes. */
static Class *new_class(Classification *cl)
{
    Class *c = MALLOC(Class);
    c->first = c->last = NULL;
    c->prev_class = NULL;
    c->next_class = cl->first_class;
    if (cl->first_class) cl->first_class->prev_class = c;
    cl->first_class = c;
    return c;
}

/* Unlinks a class and deletes it. Its nodes are not deleted. */
static void delete_class(Classification *cl, Class *c)
{
    Class *prev = c->prev_class, *next = c->next_class;

    if (prev)
        prev->next_class = next;
    else
        cl->first_class = next;

    if (next)
        next->prev_class = prev;

    FREE(c);
}

/* Creates a new node and adds it to the given class. */
static ClassNode *new_node(Classification *cl, Class *c, mdjvu_pattern_t ptr)
{
    ClassNode *n = MALLOC(ClassNode);
    n->ptr = ptr;
    n->next = c->first;
    c->first = n;
    if (!c->last) c->last = n;
    n->global_next = NULL;

    if (cl->last_node)
        cl->last_node->global_next = n;
    else
        cl->first_node = n;

    cl->last_node = n;
    return n;
}

/* Merge two classes and delete one of them. */
static Class *merge(Classification *cl, Class *c1, Class *c2)
{
    if (!c1->first)
    {
        delete_class(cl, c1);
        return c2;
    }
    if (c2->first)
    {
        c1->last->next = c2->first;
        c1->last = c2->last;
    }
    delete_class(cl, c2);
    return c1;
}

/* Puts a tag on each node corresponding to its class. */
static unsigned put_tags(Classification *cl)
{
    int32 tag = 1;
    Class *c = cl->first_class;
    while (c)
    {
        ClassNode *n = c->first;
        while (n)
        {
            n->tag = tag;
            n = n->next;
        }
        c = c->next_class;
        tag++;
    }
    return tag - 1;
}

/* Deletes all classes; nodes are untouched. */
static void delete_all_classes(Classification *cl)
{
    Class *c = cl->first_class;
    while (c)
    {
        Class *t = c;
        c = c->next_class;
        FREE(t);
    }
}

/* Compares p with nodes from c until a meaningful result. */
static int compare_to_class(mdjvu_pattern_t p, Class *c, int32 dpi,
                            mdjvu_matcher_options_t options)
{
    int r = 0;
    ClassNode *n = c->first;
    while(n)
    {
        r = mdjvu_match_patterns(p, n->ptr, dpi, options);
        if (r) break;
        n = n->next;
    }
    return r;
}

static void classify(Classification *cl, mdjvu_pattern_t p,
                     int32 dpi, mdjvu_matcher_options_t options,
                     int32 page /* of current pattern */)
{
    Class *class_of_this = NULL;
    Class *c, *next_c = NULL;
    for (c = cl->first_class; c; c = next_c)
    {
        next_c = c->next_class; /* That's because c may be deleted in merging */

        if (class_of_this == c) continue;
        if (c->last_page < page - 1) continue; /* multipage optimization */
        if (compare_to_class(p, c, dpi, options) != 1) continue;

        if (class_of_this)
            class_of_this = merge(cl, class_of_this, c);
        else
            class_of_this = c;
    }
    if (!class_of_this) class_of_this = new_class(cl);
    if (page > class_of_this->last_page)
        class_of_this->last_page = page;
    new_node(cl, class_of_this, p);
}

static int32 get_tags_from_classification(mdjvu_pattern_t *b, int32 *r, int32 n, Classification *cl)
{
    int32 i;
    int32 max_tag = put_tags(cl);
    ClassNode *node;

    delete_all_classes(cl);

    i = 0;
    node = cl->first_node;
    while (node)
    {
        ClassNode *t;
        while (!b[i]) r[i++] = 0;
        r[i++] = node->tag;
        t = node;
        node = node->global_next;
        FREE(t);
    }
    if (i < n) while (i < n) r[i++] = 0;
    return max_tag;
}

static void init_classification(Classification *c)
{
    c->first_class = NULL;
    c->first_node = c->last_node = NULL;
}

MDJVU_IMPLEMENT int32 mdjvu_classify_patterns
    (mdjvu_pattern_t *b, int32 *r, int32 n, int32 dpi,
     mdjvu_matcher_options_t options)
{
    int32 i;
    Classification cl;
    init_classification(&cl);

    for (i = 0; i < n; i++) if (b[i]) classify(&cl, b[i], dpi, options, 1);

    return get_tags_from_classification(b, r, n, &cl);
}

#ifndef NO_MINIDJVU

MDJVU_IMPLEMENT int32 mdjvu_classify_bitmaps
    (mdjvu_image_t image, int32 *result, mdjvu_matcher_options_t options)
{
    int32 i, n = mdjvu_image_get_bitmap_count(image);
    int32 dpi = mdjvu_image_get_resolution(image);
    mdjvu_pattern_t *patterns = MALLOCV(mdjvu_pattern_t, n);
    int32 max_tag;

    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_bitmap(image, i);
        if (mdjvu_image_get_not_a_letter_flag(image, bitmap))
            patterns[i] = NULL;
        else
            patterns[i] = mdjvu_pattern_create(bitmap);
    }

    max_tag = mdjvu_classify_patterns(patterns, result, n, dpi, options);

    for (i = 0; i < n; i++)
        if (patterns[i]) mdjvu_pattern_destroy(patterns[i]);
    FREEV(patterns);

    return max_tag;
}


/* ____________________________   multipage stuff   ________________________ */

/* FIXME: wrong dpi handling */
MDJVU_IMPLEMENT int32 mdjvu_multipage_classify_patterns
    (int32 npages, int32 total_patterns_count, int32 *npatterns,
     mdjvu_pattern_t **patterns, int32 *result,
     int32 *dpi, mdjvu_matcher_options_t options,
     void (*report)(void *, int), void *param)
{
    /* a kluge for NULL patterns */
    /* FIXME: do it decently */
    mdjvu_pattern_t *all_patterns = MDJVU_MALLOCV(mdjvu_pattern_t, total_patterns_count);
    int32 patterns_gathered;
    int32 page;
    int32 max_tag;

    Classification cl;
    init_classification(&cl);

    patterns_gathered = 0;
    for (page = 0; page < npages; page++)
    {
        int32 n = npatterns[page];
        int32 d = dpi[page];
        mdjvu_pattern_t *p = patterns[page];

        int32 i;
        for (i = 0; i < n; i++)
        {
            all_patterns[patterns_gathered++] = p[i];
            if (p[i]) classify(&cl, p[i], d, options, page);
        }
        report(param, page);
    }

    max_tag = get_tags_from_classification
        (all_patterns, result, total_patterns_count, &cl);

    MDJVU_FREEV(all_patterns);
    return max_tag;
}

MDJVU_IMPLEMENT int32 mdjvu_multipage_classify_bitmaps
    (int32 npages, int32 total_patterns_count, mdjvu_image_t *pages,
     int32 *result, mdjvu_matcher_options_t options,
     void (*report)(void *, int), void *param)
{
    int32 max_tag, k, page;
    int32 *npatterns = (int32 *) malloc(npages * sizeof(int32));
    int32 *dpi = (int32 *) malloc(npages * sizeof(int32));
    mdjvu_pattern_t *patterns = (mdjvu_pattern_t *)
        malloc(total_patterns_count * sizeof(mdjvu_pattern_t));
    mdjvu_pattern_t **pointers = (mdjvu_pattern_t **)
        malloc(npages * sizeof(mdjvu_pattern_t *));

    int32 patterns_created = 0;
    for (page = 0; page < npages; page++)
    {
        mdjvu_image_t current_image = pages[page];
        int32 c = npatterns[page] = mdjvu_image_get_bitmap_count(current_image);
        int32 i;
        dpi[page] = mdjvu_image_get_resolution(current_image);

        pointers[page] = patterns + patterns_created;
        for (i = 0; i < c; i++)
        {
            patterns[patterns_created++] = mdjvu_pattern_create(
                mdjvu_image_get_bitmap(current_image, i)
            );
        }
    }

    max_tag = mdjvu_multipage_classify_patterns
        (npages, total_patterns_count, npatterns,
         pointers, result, dpi, options, report, param);

    for (k = 0; k < total_patterns_count; k++)
    {
        mdjvu_pattern_destroy(patterns[k]);
    }
    free(patterns);
    free(pointers);
    free(npatterns);
    free(dpi);

    return max_tag;
}


MDJVU_IMPLEMENT void mdjvu_multipage_get_dictionary_flags
   (int32 n,
    int32 *npatterns,
    int32 max_tag,
    int32 *tags,
    unsigned char *dictionary_flags)
{
    int32 page_number;
    int32 *first_page_met = (int32 *) malloc((max_tag + 1) * sizeof(int32));
    int32 i, total_bitmaps_passed = 0;
    memset(dictionary_flags, 0, max_tag + 1);
    for (i = 0; i <= max_tag; i++) first_page_met[i] = -1;

    for (page_number = 0; page_number < n; page_number++)
    {
        int32 bitmap_count = npatterns[page_number];

        for (i = 0; i < bitmap_count; i++)
        {
            int32 tag = tags[total_bitmaps_passed++];
            if (!tag) continue; /* skip non-substitutable bitmaps */

            if (first_page_met[tag] == -1)
            {
                /* never met this tag before */
                first_page_met[tag] = page_number;
            }
            else if (first_page_met[tag] != page_number)
            {
                /* met this tag on another page */
                dictionary_flags[tag] = 1;
            }
        }
    }

    free(first_page_met);
}


#endif /* NO_MINIDJVU */
