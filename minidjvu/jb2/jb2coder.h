/*
 * jb2coder.h - coding character positions and JB2 records
 */

#ifndef MDJVU_JB2CODER_H
#define MDJVU_JB2CODER_H

#include "bmpcoder.h"

// struct JB2Rect - a simple rectangle class {{{

struct JB2Rect
{
    int left, top, width, height;
    inline JB2Rect() {}
    inline JB2Rect(int l, int t, int w, int h)
        : left(l), top(t), width(w), height(h) {}
};

// struct JB2Rect }}}

// JB2Coder interface {{{

/* Watcher interface + implementation {{{ */

struct Watcher : ZPMemoryWatcher
{
    int32 count;
    Watcher() : count(0) {}

    virtual void handle_allocation()
    {
        count++;
    }

    void reset() {count = 0;}
};

/* Watcher }}} */

class JB2Coder
{
    public:
        Watcher watcher;
        ZPNumContext
            image_size,
            matching_symbol_index,
            symbol_column_number,
            symbol_row_number,
            same_line_column_offset,
            same_line_row_offset,
            new_line_column_offset,
            new_line_row_offset,
            comment_length,
            comment_octet,
            required_dictionary_size;

        ZPBitContext eventual_image_refinement, offset_type;
        JB2Coder();
        ~JB2Coder();

    protected:
        ZPNumContext record_type;
        JB2Rect first, prev3, prev2, prev1;
        int line_counter;
        void reset_numcontexts();
};

// JB2Coder interface }}}

// JB2Decoder interface {{{

struct JB2Decoder : JB2Coder, JB2BitmapDecoder
{
    ZPDecoder zp;
    JB2Decoder(FILE *f, int32 chunk_length);
    JB2RecordType decode_record_type();

    // decodes character position and creates a new blit
    int32 decode_blit(mdjvu_image_t, int32 shape_index);

    void reset(); // resets numcontexts as required by "reset" record

    private:
        void decode_character_position(int32 &x, int32 &y, int32 w, int32 h);
};

// JB2Decoder interface }}}

// JB2Encoder interface {{{

struct JB2Encoder : JB2Coder, JB2BitmapEncoder
{
    ZPEncoder zp;
    JB2Encoder(FILE *f);

    // encodes blit position
    // w and h are passed here in case of substitution
    void encode_blit(mdjvu_image_t img, int32 blit, int32 w, int32 h);

    void open_record(JB2RecordType);
    void close_record(); // takes care of the DjVu 3 numcontext reset

    private:
        void encode_character_position(int x, int y, int w, int h);
        bool no_symbols_yet;
};

// JB2Encoder interface }}}

#endif
