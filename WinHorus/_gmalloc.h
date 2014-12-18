#ifndef _HORUS__GMALLOC_H
#define _HORUS__GMALLOC_H



//  _gmalloc.c - Windows global memory allocation library private definitions
//               © 1994-1996 Pete Jordan, Horus Communication
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



typedef char HUGE *HPSTR;
typedef DWORD HUGE *HPDWORD;



#define SIGNATURE 0xdeadbeefL
#define GUARD_PATTERN '\xae'



enum {
    #if defined(MVS)
        DEFAULT_BUCKETSIZE=32768,
        DEFAULT_THRESHOLDSIZE=8192,
    #else
        DEFAULT_BUCKETSIZE=16384,
        DEFAULT_THRESHOLDSIZE=4096,
    #endif
    MIN_BUCKETSIZE=1024,
    MIN_THRESHOLDSIZE=256,
    DEFAULT_REALLOCSIZE=256,
    DEFAULT_GUARDSIZE=16,
    MAX_GUARDSIZE=256
};



enum {HEADERS_PER_BLOCK=64};



enum {
    GMALLOC_CHECK_OVERFLOW  = 0x0001,
    GMALLOC_CHECK_AUTO      = 0x0002,
    GMALLOC_CHECK_INTERNAL  = 0x0004,

    GMALLOC_CHECK_MASK      = GMALLOC_CHECK_OVERFLOW|GMALLOC_CHECK_AUTO|GMALLOC_CHECK_INTERNAL
};



typedef enum {HEAD_UNUSED, HEAD_BUCKET, HEAD_BLOCK, HEAD_FREE} HeaderType;
typedef struct tagHeaderBlock HeaderBlock, FAR *LPHeaderBlock;
typedef struct tagHeader Header, FAR *LPHeader;
typedef struct tagBlock Block, FAR *LPBlock, HUGE *HPBlock;



struct tagHeaderBlock {
    LPHeaderBlock lpNext;
    LPHeaderBlock lpPrevious;
    LPHeader lpFree;
    int nFreeCount;
};



struct tagHeader {
    LPHeader lpNext;
    LPHeader lpPrevious;
    LPHeaderBlock lpHeaderBlock;
    HeaderType headerType;

    union {
        struct {
            LPHeader lpBucket;
            LPBlock lpBlock;
            LONG lAlloc;
            LONG lSize;
        } block;

        struct {
            LPVOID lpAddress;
            LPHeader lpFree;
            LONG lSize;
        } bucket;
    } tag;
};



struct tagBlock {
    DWORD dwSig1;
    LPHeader lpHeader;
    LONG lAlloc;
    DWORD dwSig2;
};



#define blockSize(lpHeader) (LONG)(((lpHeader)->tag.block.lAlloc+sizeof(Block)))
#define thisBlock(lpHeader) ((HPSTR)((lpHeader)->tag.block.lpBlock+1))
#define nextBlock(lpHeader) (thisBlock(lpHeader)+blockSize(lpHeader))



#if defined(MVS)
    #define _OSmalloc(lAlloc) malloc(lAlloc)
    #define _OSfree(lpBlock) free(lpBlock)
#else
    #define _OSmalloc(lAlloc) GlobalAllocPtr(GMEM_MOVEABLE, (lAlloc))
    #define _OSfree(lpBlock) GlobalFreePtr(lpBlock)
#endif



#if defined(_DEBUG)
    #define OSmalloc(lAlloc, lpszText) debugMalloc((lAlloc), (lpszText))
    #define OSfree(lpBlock, lpszText) debugFree((lpBlock), (lpszText))
#else
    #define OSmalloc(lAlloc, lpszText) _OSmalloc(lAlloc)
    #define OSfree(lpBlock, lpszText) _OSfree(lpBlock)
#endif



void PASCAL gmallocPrelude(void);



#endif
