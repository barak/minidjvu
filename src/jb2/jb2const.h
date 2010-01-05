/*
 * jb2const.h - some constants defined by DjVu specification
 */

#ifndef MDJVU_JB2CONST_H
#define MDJVU_JB2CONST_H

#define JB2_NUMBER_CONTEXTS_MEMORY_BOUND 20000

enum JB2RecordType
{
    jb2_start_of_image,
    jb2_new_symbol_add_to_image_and_library,
    jb2_new_symbol_add_to_library_only,
    jb2_new_symbol_add_to_image_only,
    jb2_matched_symbol_with_refinement_add_to_image_and_library,
    jb2_matched_symbol_with_refinement_add_to_library_only,
    jb2_matched_symbol_with_refinement_add_to_image_only,
    jb2_matched_symbol_copy_to_image_without_refinement,
    jb2_non_symbol_data,
    jb2_require_dictionary_or_reset,
    jb2_comment,
    jb2_end_of_data
};

enum {jb2_big_negative_number = -262143,
      jb2_big_positive_number =  262142};

#endif
