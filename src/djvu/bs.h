/*
 * bs.h - BZZ-coder from DjVuLibre, a general purpose compressor
 * based on the Burrows-Wheeler (or "block sorting") transform.
 */

#include "../jb2/zp.h"

class BSEncoder
{
    public:
        BSEncoder(FILE *, int blocksize = 1024);
        ~BSEncoder();
        
        long tell(void) const;
        void flush(void);
        void close(void);
        
        size_t write(void *buffer, size_t sz);
        void write8  (unsigned int card);
        void write16 (unsigned int card);
        void write24 (unsigned int card);
        void write32 (unsigned int card);
        
    private:
        size_t writall(void *buffer, size_t size);
        unsigned int encode(void);

        // Data
        long            offset;
        int             bptr;
        unsigned int   blocksize;
        int             size;
        unsigned char  *data;
        
        // Coder
        ZPEncoder gzp;
        ZPBitContext ctx[300];
};

