//  gmalloc.c - Windows global memory allocation routine library
//              © 1994-1996 Pete Jordan, Horus Communication
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.
//
//              10Jan1996 - rewritten to seperate header information from
//              client-accessible allocations.



#include "winhorus.h"
#include <string.h>
#include <stdarg.h>
#include "context.h"
#include "debugdef.h"
#if defined(MVS)
    #include "@gmalloc.h"
#else
    #include "_gmalloc.h"
#endif



typedef struct {
    WORD wBucketSize;
    WORD wThresholdSize;
    WORD wReallocSize;
    WORD wGuardSize;
    LPHeaderBlock lpHeaderBlock;
    LPHeader lpBucketHeader;
    LPHeader lpBlockHeader;
    LPHeader lpFreeHeader;
    WORD wFlags;
    int nCode;
    GmallocCallback gmallocCallback;
} ContextData, FAR *LPContextData;



static WHCONTEXT hContext=NULL;



#if defined(MVS)
    #pragma inline(freeHeader)
    #pragma inline(grabHeader)
    #pragma inline(validateBlock)
    #pragma inline(checkOverflow)
    #pragma inline(findBlock)
#endif



#if defined(_DEBUG)
    static void FAR PASCAL gmallocCallback(LPCSTR lpcszTag, int nCode, HPVOID hpBlock, LONG lSize)
    {
        _ALERT(0, "%s - nCode: %d, hpBlock: %lx, lSize: %ld",
               lpcszTag, nCode, hpBlock, lSize);
    }



    static LPVOID debugMalloc(LONG lSize, LPSTR lpszText)
    {
        LPVOID lpAlloc=_OSmalloc(lSize);

        _ALERT(2, "Allocate %s (%ld) %lx", lpszText, lSize, lpAlloc);
        return(lpAlloc);
    }



    static void debugFree(LPVOID lpAlloc, LPSTR lpszText)
    {
        _ALERT(2, "Free %s %lx", lpszText, lpAlloc);
        _OSfree(lpAlloc);
    }
#endif



void CALLBACK gmallocClientPostlude(LPVOID lpContext)
{
    LPContextData lpContextData=(LPContextData)lpContext;
    LPHeader lpNextBlock;
    LPHeader lpNextBucket;
    LPHeaderBlock lpHeaderBlock=lpContextData->lpHeaderBlock;

    for (lpNextBlock=lpContextData->lpBlockHeader;
         lpNextBlock;
         lpNextBlock=lpNextBlock->lpNext)
        if (!lpNextBlock->tag.block.lpBucket)
            OSfree(lpNextBlock->tag.block.lpBlock, "large block");

    for (lpNextBucket=lpContextData->lpBucketHeader;
         lpNextBucket;
         lpNextBucket=lpNextBucket->lpNext)
        OSfree(lpNextBucket->tag.bucket.lpAddress, "bucket");

    while (lpHeaderBlock) {
        LPHeaderBlock lpThisBlock=lpHeaderBlock;

        lpHeaderBlock=lpThisBlock->lpNext;
        OSfree(lpThisBlock, "header block");
    }

    lpContextData->lpFreeHeader=NULL;
    lpContextData->lpBlockHeader=NULL;
    lpContextData->lpBucketHeader=NULL;
    lpContextData->lpHeaderBlock=NULL;
}



void PASCAL gmallocPrelude(void)
{
    static ContextData defaultContext={
        DEFAULT_BUCKETSIZE,
        DEFAULT_THRESHOLDSIZE,
        DEFAULT_REALLOCSIZE,
        DEFAULT_GUARDSIZE,
        NULL, NULL, NULL, NULL,
        0, GMALLOC_OK,
        #if defined(_DEBUG)
            gmallocCallback
        #else
            NULL
        #endif
    };

    hContext=registerContext((LPContextData)&defaultContext,
                             sizeof(ContextData), NULL, gmallocClientPostlude);

}



void FAR PASCAL _EXPORT hmemset(HPVOID hpDestination, int ch, LONG lCount)
{
    #if defined(MVS) || defined(_WIN32)
        memset(hpDestination, ch, lCount);
    #else
        HPSTR hpDest=(HPSTR)hpDestination;
        WORD wOffset=OFFSETOF(hpDest);
        LONG lFirstChunk=0x10000L-wOffset;

        if (lCount<0x10000L && lCount<=lFirstChunk)
            _fmemset(hpDest, ch, (WORD)lCount);
        else {
            if (wOffset) {
                _fmemset(hpDest, ch, (WORD)lFirstChunk);
                hpDest+=lFirstChunk;
                lCount-=lFirstChunk;
            }

            while (lCount>0x8000L) {
                _fmemset(hpDest, ch, 0x8000);
                hpDest+=0x8000;
                lCount-=0x8000;
            }

            if (lCount)
                _fmemset(hpDest, ch, (WORD)lCount);

        }
    #endif
}



int FAR PASCAL _EXPORT gstatus(void)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);

    return(lpContextData->nCode);
}



LPCSTR FAR PASCAL _EXPORT gstrerror(int nStatus)
{
    switch (nStatus) {

    case GMALLOC_OSALLOC:   return("Operating system memory allocation failure");
    case GMALLOC_NULL:      return("NULL memory block pointer");
    case GMALLOC_UNDERFLOW: return("Application error - memory block underflow");
    case GMALLOC_OVERFLOW:  return("Application error - memory block overflow");
    case GMALLOC_INVALID:   return("Memory block pointer was not allocated by gmalloc");

    default:                return("");

    }
}



WORD FAR PASCAL _EXPORT gallocsize(WORD wNewBucketSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    WORD wOldBucketSize=lpContextData->wBucketSize;

    if (wNewBucketSize>0) {
        if (wNewBucketSize<MIN_BUCKETSIZE)
            wNewBucketSize=MIN_BUCKETSIZE;
        else if (wNewBucketSize&1)
            wNewBucketSize&=0xfffe;

        if (lpContextData->wThresholdSize>(wNewBucketSize>>1))
            lpContextData->wThresholdSize=wNewBucketSize>>1;

        lpContextData->wBucketSize=wNewBucketSize;
    }

    return(wOldBucketSize);
}



WORD FAR PASCAL _EXPORT gthreshold(WORD wNewThresholdSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    WORD wOldThresholdSize=lpContextData->wThresholdSize;

    if (wNewThresholdSize>0) {
        if (wNewThresholdSize>(lpContextData->wBucketSize>>1))
            wNewThresholdSize=lpContextData->wBucketSize>>1;
        else if (wNewThresholdSize<MIN_THRESHOLDSIZE)
            wNewThresholdSize=MIN_THRESHOLDSIZE;

        lpContextData->wThresholdSize=wNewThresholdSize;
    }

    return(wOldThresholdSize);
}



WORD FAR PASCAL _EXPORT greallocsize(WORD wNewReallocSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    WORD wOldReallocSize=lpContextData->wBucketSize;

    if (wNewReallocSize>0) {
        if (wNewReallocSize<lpContextData->wGuardSize+sizeof(Block))
            wNewReallocSize=lpContextData->wGuardSize+sizeof(Block);

        lpContextData->wReallocSize=wNewReallocSize;
    }

    return(wOldReallocSize);
}



WORD FAR PASCAL _EXPORT gguardsize(WORD wNewGuardSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    WORD wOldGuardSize=lpContextData->wGuardSize;

    if (wNewGuardSize>0) {
        if (wNewGuardSize>MAX_GUARDSIZE)
            wNewGuardSize=MAX_GUARDSIZE;

    if (lpContextData->wReallocSize<wNewGuardSize+sizeof(Block))
        lpContextData->wReallocSize=wNewGuardSize+sizeof(Block);

        lpContextData->wGuardSize=wNewGuardSize;
    }

    return(wOldGuardSize);
}



static LPHeader PASCAL findBlock(LPContextData lpContextData, HPVOID hpBlock)
{
    LPHeader lpBlockHeader;

    for (lpBlockHeader=lpContextData->lpBlockHeader;
         lpBlockHeader && thisBlock(lpBlockHeader)!=(HPSTR)hpBlock;
         lpBlockHeader=lpBlockHeader->lpNext);

    _ALERT(5, "findBlock => %lx", lpBlockHeader);
    return(lpBlockHeader);
}



static int PASCAL checkOverflow(LPHeader lpHeader)
{
    LPBlock lpBlock=lpHeader->tag.block.lpBlock;
    HPSTR hpStart=thisBlock(lpHeader);
    HPSTR hpPtr=hpStart+lpHeader->tag.block.lSize;
    HPSTR hpEnd=hpStart+lpHeader->tag.block.lAlloc;
    int nCode;

    if (lpBlock->dwSig1==SIGNATURE &&
        lpBlock->dwSig2==SIGNATURE &&
        lpBlock->lpHeader==lpHeader &&
        lpBlock->lAlloc==lpHeader->tag.block.lAlloc)
        nCode=GMALLOC_OK;
    else {
        lpBlock->dwSig1=lpBlock->dwSig2=SIGNATURE;
        lpBlock->lpHeader=lpHeader;
        lpBlock->lAlloc=lpHeader->tag.block.lAlloc;
        nCode=GMALLOC_UNDERFLOW;
    }

    while (hpPtr<hpEnd && *hpPtr==(char)GUARD_PATTERN)
        hpPtr++;

    if (hpPtr<hpEnd)
        nCode=GMALLOC_OVERFLOW;

    return(nCode);
}



static LPHeader PASCAL validateBlock(LPContextData lpContextData, LPCSTR lpcszTag, HPVOID hpBlock)
{
    LPHeader lpHeader=NULL;
    int nCode;

    _ALERT(5, "validateBlock(\"%s\", %lx)", lpcszTag, hpBlock);

    if (!hpBlock)
        nCode=GMALLOC_NULL;
    else {
        LPBlock lpBlock=(LPBlock)((HPBlock)hpBlock-1);

        lpHeader=lpBlock->lpHeader;

        if (lpBlock->dwSig1==SIGNATURE &&
            lpBlock->dwSig2==SIGNATURE &&
            lpHeader &&
            lpHeader->headerType==HEAD_BLOCK &&
            thisBlock(lpHeader)==(HPSTR)hpBlock)
            nCode=GMALLOC_OK;
        else {
            nCode=GMALLOC_UNDERFLOW;
            lpHeader=findBlock(lpContextData, hpBlock);
        }

        if (lpHeader==NULL)
            nCode=GMALLOC_INVALID;
        else if (lpContextData->wFlags&GMALLOC_CHECK_MASK)
            nCode=checkOverflow(lpHeader);

    }

    lpContextData->nCode=nCode;
    _ALERT(5, "validateBlock - nCode:%d, lpHeader:%lx", nCode, lpHeader);

    if (nCode!=GMALLOC_OK && lpContextData->gmallocCallback)
        lpContextData->gmallocCallback(lpcszTag, nCode, hpBlock,
                                       lpHeader ? lpHeader->tag.block.lSize : 0L);

    return(lpHeader);
}



static LPHeader PASCAL grabHeader(LPContextData lpContextData)
{
    LPHeader lpHeader=NULL;
    LPHeaderBlock lpHeaderBlock;

    for (lpHeaderBlock=lpContextData->lpHeaderBlock;
         lpHeaderBlock && !lpHeaderBlock->lpFree;
        lpHeaderBlock=lpHeaderBlock->lpNext);

    if (lpHeaderBlock) {
        lpHeader=lpHeaderBlock->lpFree;
        lpHeaderBlock->lpFree=lpHeader->lpNext;

        if (lpHeader->lpNext) {
            lpHeader->lpNext->lpPrevious=NULL;
            lpHeader->lpNext=NULL;
        }

        lpHeaderBlock->nFreeCount--;
    } else {
        lpHeaderBlock=(LPHeaderBlock)OSmalloc(sizeof(HeaderBlock)+HEADERS_PER_BLOCK*sizeof(Header), "header block");

        if (lpHeaderBlock) {
            int nPtr;

            lpHeaderBlock->lpNext=lpContextData->lpHeaderBlock;
            lpHeaderBlock->lpPrevious=NULL;

            if (lpHeaderBlock->lpNext)
                lpHeaderBlock->lpNext->lpPrevious=lpHeaderBlock;

            lpContextData->lpHeaderBlock=lpHeaderBlock;
            lpHeader=(LPHeader)(lpHeaderBlock+1);

            for (nPtr=1; nPtr<HEADERS_PER_BLOCK; nPtr++) {
                lpHeader[nPtr-1].lpNext=lpHeader+nPtr;
                lpHeader[nPtr].lpPrevious=lpHeader+nPtr-1;
                lpHeader[nPtr].lpHeaderBlock=lpHeaderBlock;
                lpHeader[nPtr].headerType=HEAD_UNUSED;
            }

            lpHeader[HEADERS_PER_BLOCK-1].lpNext=NULL;
            lpHeader->lpHeaderBlock=lpHeaderBlock;
            lpHeader->headerType=HEAD_UNUSED;
            lpHeader->lpNext=lpHeader->lpPrevious=NULL;
            lpHeaderBlock->nFreeCount=HEADERS_PER_BLOCK-1;
            lpHeaderBlock->lpFree=lpHeader+1;
        } else
            lpContextData->nCode=GMALLOC_OSALLOC;

    }

    return(lpHeader);
}



static void PASCAL freeHeader(LPContextData lpContextData, LPHeader lpHeader)
{
    LPHeaderBlock lpHeaderBlock=lpHeader->lpHeaderBlock;

    lpHeaderBlock->nFreeCount++;

    if (lpHeaderBlock->nFreeCount<HEADERS_PER_BLOCK) {
        lpHeader->headerType=HEAD_UNUSED;
        lpHeader->lpPrevious=NULL;
        lpHeader->lpNext=lpHeaderBlock->lpFree;
        lpHeaderBlock->lpFree=lpHeader;

    if (lpHeader->lpNext)
        lpHeader->lpNext->lpPrevious=lpHeader;

    } else {
        if (lpHeaderBlock->lpNext)
            lpHeaderBlock->lpNext->lpPrevious=lpHeaderBlock->lpPrevious;

        if (lpHeaderBlock->lpPrevious)
            lpHeaderBlock->lpPrevious->lpNext=lpHeaderBlock->lpNext;
        else
            lpContextData->lpHeaderBlock=lpHeaderBlock->lpNext;

        OSfree(lpHeaderBlock, "header block");
    }
}



BOOL PASCAL FAR _EXPORT gfree(HPVOID hpBlock)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LPHeader lpBlockHeader=validateBlock(lpContextData, "gfree", hpBlock);
    LPHeader lpBucketHeader;

    _ALERT(4, "gfree(%x)", hpBlock);

    if (!lpBlockHeader)
        return(FALSE);

    if (lpBlockHeader->lpNext)
        lpBlockHeader->lpNext->lpPrevious=lpBlockHeader->lpPrevious;

    if (lpBlockHeader->lpPrevious)
        lpBlockHeader->lpPrevious->lpNext=lpBlockHeader->lpNext;
    else
        lpContextData->lpBlockHeader=lpBlockHeader->lpNext;

    lpBlockHeader->headerType=HEAD_FREE;
    lpBlockHeader->tag.block.lSize=0L;
    lpBucketHeader=lpBlockHeader->tag.block.lpBucket;

    if (!lpBucketHeader) {
        OSfree(lpBlockHeader->tag.block.lpBlock, "large block");
        freeHeader(lpContextData, lpBlockHeader);
    } else {
        LPHeader lpFreeList;
        LPHeader lpPrevFree=NULL;
        HPSTR hpBlockStart=thisBlock(lpBlockHeader);

        for (lpFreeList=lpBucketHeader->tag.bucket.lpFree;
             lpFreeList && thisBlock(lpFreeList)<hpBlockStart;
             lpFreeList=lpFreeList->lpNext)
            lpPrevFree=lpFreeList;

        if (!lpFreeList)
            lpBlockHeader->lpNext=NULL;
        else if (nextBlock(lpBlockHeader)!=thisBlock(lpFreeList)) {
            lpBlockHeader->lpNext=lpFreeList;
            lpFreeList->lpPrevious=lpBlockHeader;
        } else {
            _ALERT(5, "consolidating next...");
            lpBlockHeader->tag.block.lAlloc+=blockSize(lpFreeList);
            lpBlockHeader->lpNext=lpFreeList->lpNext;

            if (lpBlockHeader->lpNext)
                lpBlockHeader->lpNext->lpPrevious=lpBlockHeader;

            freeHeader(lpContextData, lpFreeList);
        }

        if (!lpPrevFree) {
            lpBucketHeader->tag.bucket.lpFree=lpBlockHeader;
            lpBlockHeader->lpPrevious=NULL;
        } else if (nextBlock(lpPrevFree)!=hpBlockStart) {
            lpPrevFree->lpNext=lpBlockHeader;
            lpBlockHeader->lpPrevious=lpPrevFree;
        } else {
            _ALERT(5, "consolidating previous...");
            lpPrevFree->tag.block.lAlloc+=blockSize(lpBlockHeader);
            lpPrevFree->lpNext=lpBlockHeader->lpNext;

            if (lpPrevFree->lpNext)
                lpPrevFree->lpNext->lpPrevious=lpPrevFree;

            freeHeader(lpContextData, lpBlockHeader);
            lpBlockHeader=lpPrevFree;
        }

        lpBlockHeader->tag.block.lpBlock->lAlloc= -lpBlockHeader->tag.block.lAlloc;

        _ALERT(5, "block size: %ld, bucket size: %ld",
               blockSize(lpBlockHeader), lpBucketHeader->tag.bucket.lSize);

        if (blockSize(lpBlockHeader)==lpBucketHeader->tag.bucket.lSize) {
            if (lpBucketHeader->lpNext)
                lpBucketHeader->lpNext->lpPrevious=lpBucketHeader->lpPrevious;

            if (lpBucketHeader->lpPrevious)
                lpBucketHeader->lpPrevious->lpNext=lpBucketHeader->lpNext;
            else
                lpContextData->lpBucketHeader=lpBucketHeader->lpNext;

            OSfree(lpBucketHeader->tag.bucket.lpAddress, "bucket");
            freeHeader(lpContextData, lpBlockHeader);
            freeHeader(lpContextData, lpBucketHeader);
        }
    }

    return(TRUE);
}




LPVOID PASCAL FAR _EXPORT gmalloc(LONG lSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LONG lAlloc=lSize+lpContextData->wGuardSize;
    LPHeader lpBucketHeader=NULL;
    LPHeader lpBlockHeader=NULL;
    LPBlock lpBlock=NULL;

    _ALERT(4, "gmalloc(%ld)", lSize);

    if (lSize<=0)
        return(NULL);

    if (lAlloc&0xf)
        lAlloc=(lAlloc&0xfffffff0L)+0x10L;

    if (lAlloc>(LONG)lpContextData->wThresholdSize) {
        lpBlock=(LPBlock)OSmalloc(sizeof(Block)+lAlloc, "large block");

        if (!lpBlock) {
            lpContextData->nCode=GMALLOC_OSALLOC;
            return(NULL);
        }

        lpBlockHeader=grabHeader(lpContextData);

        if (!lpBlockHeader) {
            OSfree(lpBlock, "large block");
            return(NULL);
        }

        lpBlock->dwSig1=lpBlock->dwSig2=SIGNATURE;
    } else {
        lpBucketHeader=lpContextData->lpBucketHeader;

        while (lpBucketHeader && !lpBlockHeader) {
            for (lpBlockHeader=lpBucketHeader->tag.bucket.lpFree;
                 lpBlockHeader && lpBlockHeader->tag.block.lAlloc<lAlloc;
                 lpBlockHeader=lpBlockHeader->lpNext);

            if (!lpBlockHeader)
                lpBucketHeader=lpBucketHeader->lpNext;

        }

        if (lpBlockHeader)
            lpBlock=lpBlockHeader->tag.block.lpBlock;
        else {
            lpBlock=(LPBlock)OSmalloc(lpContextData->wBucketSize, "bucket");

            if (!lpBlock) {
                lpContextData->nCode=GMALLOC_OSALLOC;
                return(NULL);
            }

            lpBucketHeader=grabHeader(lpContextData);

            if (!lpBucketHeader) {
                OSfree(lpBlock, "bucket");
                return(NULL);
            }

            lpBlockHeader=grabHeader(lpContextData);

            if (!lpBlockHeader) {
                freeHeader(lpContextData, lpBucketHeader);
                OSfree(lpBlock, "bucket");
                return(NULL);
            }

            lpBucketHeader->lpPrevious=NULL;
            lpBucketHeader->lpNext=lpContextData->lpBucketHeader;
            lpContextData->lpBucketHeader=lpBucketHeader;

            if (lpBucketHeader->lpNext)
                lpBucketHeader->lpNext->lpPrevious=lpBucketHeader;

            lpBucketHeader->headerType=HEAD_BUCKET;
            lpBucketHeader->tag.bucket.lpAddress=lpBlock;
            lpBucketHeader->tag.bucket.lpFree=lpBlockHeader;
            lpBucketHeader->tag.bucket.lSize=lpContextData->wBucketSize;
            lpBlockHeader->lpPrevious=lpBlockHeader->lpNext=NULL;
            lpBlockHeader->headerType=HEAD_FREE;
            lpBlockHeader->tag.block.lpBucket=lpBucketHeader;
            lpBlockHeader->tag.block.lpBlock=lpBlock;
            lpBlockHeader->tag.block.lAlloc=lpContextData->wBucketSize-sizeof(Block);
            lpBlockHeader->tag.block.lSize=0L;
            lpBlock->dwSig1=lpBlock->dwSig2=SIGNATURE;
            lpBlock->lpHeader=lpBlockHeader;
            lpBlock->lAlloc= -lpBlockHeader->tag.block.lAlloc;
        }

        if (lpBlockHeader->tag.block.lAlloc-lAlloc<=sizeof(Block)) {
            lAlloc=lpBlockHeader->tag.block.lAlloc;

            if (lpBlockHeader->lpNext)
                lpBlockHeader->lpNext->lpPrevious=lpBlockHeader->lpPrevious;

            if (lpBlockHeader->lpPrevious)
                lpBlockHeader->lpPrevious->lpNext=lpBlockHeader->lpNext;
            else
                lpBucketHeader->tag.bucket.lpFree=lpBlockHeader->lpNext;

        } else {
            LPHeader lpFreeHeader=lpBlockHeader;
            LPBlock lpFreeBlock;

            lpBlockHeader=grabHeader(lpContextData);

            if (!lpBlockHeader)
                return(NULL);

            lpFreeHeader->tag.block.lpBlock=(LPBlock)((LPSTR)(lpFreeHeader->tag.block.lpBlock)+lAlloc+sizeof(Block));
            lpFreeHeader->tag.block.lAlloc-=lAlloc+sizeof(Block);
            lpFreeBlock=lpFreeHeader->tag.block.lpBlock;
            lpFreeBlock->dwSig1=lpFreeBlock->dwSig2=SIGNATURE;
            lpFreeBlock->lpHeader=lpFreeHeader;
            lpFreeBlock->lAlloc= -lpFreeHeader->tag.block.lAlloc;
        }
    }

    lpBlockHeader->lpPrevious=NULL;
    lpBlockHeader->lpNext=lpContextData->lpBlockHeader;
    lpContextData->lpBlockHeader=lpBlockHeader;

    if (lpBlockHeader->lpNext)
        lpBlockHeader->lpNext->lpPrevious=lpBlockHeader;

    lpBlockHeader->headerType=HEAD_BLOCK;
    lpBlockHeader->tag.block.lpBucket=lpBucketHeader;
    lpBlockHeader->tag.block.lpBlock=lpBlock;
    lpBlockHeader->tag.block.lAlloc=lAlloc;
    lpBlockHeader->tag.block.lSize=lSize;
    lpBlock->lpHeader=lpBlockHeader;
    lpBlock->lAlloc=lAlloc;

    if (lAlloc>lSize)
        hmemset(thisBlock(lpBlockHeader)+lSize, GUARD_PATTERN, lAlloc-lSize);

    _ALERT(4, "gmalloc => %lx", thisBlock(lpBlockHeader));
    return((LPVOID)thisBlock(lpBlockHeader));
}



LPVOID PASCAL FAR _EXPORT gmallocz(LONG lSize)
{
    LPVOID lpBlock=gmalloc(lSize);

    if (lpBlock)
        hmemset(lpBlock, 0, lSize);

    return(lpBlock);
}



LPVOID PASCAL FAR _EXPORT gcalloc(WORD wUnits, WORD wSize)
{
    LONG lAlloc=(LONG)wSize*(LONG)wUnits;

    if (wUnits==0 || wSize==0 ||
        lAlloc>=0x10000L && 0x10000L%wSize)
        return(NULL);

    return(gmalloc(lAlloc));
}



LPVOID PASCAL FAR _EXPORT gcallocz(WORD wUnits, WORD wSize)
{
    LPVOID lpBlock=gcalloc(wUnits, wSize);

    if (lpBlock)
        hmemset(lpBlock, 0, (LONG)wUnits*(LONG)wSize);

    return(lpBlock);
}



LPVOID PASCAL FAR _EXPORT grealloc(HPVOID hpOldBlock, LONG lNewSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LPHeader lpBlockHeader;
    LPHeader lpBucketHeader;
    LONG lOldSize, lOldAlloc;

    _ALERT(4, "grealloc(%lx, %ld)", hpOldBlock, lNewSize);

    if (!hpOldBlock)
        return(gmalloc(lNewSize));

    if (lNewSize==0L) 
	{
        gfree(hpOldBlock);	hpOldBlock = NULL;
        return(NULL);
    }

    lpBlockHeader=validateBlock(lpContextData, "grealloc", hpOldBlock);

    if (!lpBlockHeader)
        return(NULL);

    lpBucketHeader=lpBlockHeader->tag.block.lpBucket;
    lOldSize=lpBlockHeader->tag.block.lSize;
    lOldAlloc=lpBlockHeader->tag.block.lAlloc;

    if (lNewSize>lOldAlloc) {
        LPVOID lpNewBlock=gmalloc(lNewSize);

        if (!lpNewBlock)
            return(NULL);

        lpBlockHeader=((HPBlock)lpNewBlock-1)->lpHeader;
        hmemcpy(lpNewBlock, hpOldBlock, lOldSize);
        gfree(hpOldBlock);	hpOldBlock = NULL;
    } else if (lNewSize>lOldSize ||
               lOldAlloc-lNewSize<(LONG)lpContextData->wReallocSize) {
        if (lNewSize<lOldSize)
            hmemset((HPSTR)hpOldBlock+lNewSize, GUARD_PATTERN, lOldSize-lNewSize);

        lpBlockHeader->tag.block.lSize=lNewSize;
    } else if (!lpBucketHeader) {
        LPVOID lpNewBlock=gmalloc(lNewSize);

        if (!lpNewBlock)
            return(NULL);

        lpBlockHeader=((HPBlock)lpNewBlock-1)->lpHeader;
        hmemcpy(lpNewBlock, hpOldBlock, lNewSize);
        gfree(hpOldBlock);	hpOldBlock = NULL;
    } else {
        LONG lNewAlloc=lNewSize+lpContextData->wGuardSize;
        LPHeader lpFreeHeader=grabHeader(lpContextData);
        LPHeader lpFreeList;
        LPHeader lpPrevFree=NULL;
        HPSTR hpBlockStart=(HPSTR)hpOldBlock+lNewAlloc+sizeof(Block);
        LPBlock lpFreeBlock=(LPBlock)(hpBlockStart-sizeof(Block));

        if (!lpFreeHeader)
            return(NULL);

        for (lpFreeList=lpBucketHeader->tag.bucket.lpFree;
             lpFreeList && thisBlock(lpFreeList)<hpBlockStart;
             lpFreeList=lpFreeList->lpNext)
            lpPrevFree=lpFreeList;

        lpFreeHeader->lpNext=lpFreeList;
        lpFreeHeader->lpPrevious=lpPrevFree;

        if (lpFreeList)
            lpFreeList->lpPrevious=lpFreeHeader;

        if (lpPrevFree)
            lpPrevFree->lpNext=lpFreeHeader;
        else
            lpBucketHeader->tag.bucket.lpFree=lpFreeHeader;

        lpFreeHeader->headerType=HEAD_FREE;
        lpFreeHeader->tag.block.lpBucket=lpBucketHeader;
        lpFreeHeader->tag.block.lpBlock=lpFreeBlock;
        lpFreeHeader->tag.block.lSize=0L;
        lpFreeHeader->tag.block.lAlloc=lOldAlloc-sizeof(Block)-lNewAlloc;
        lpFreeBlock->dwSig1=lpFreeBlock->dwSig2=SIGNATURE;
        lpFreeBlock->lpHeader=lpFreeHeader;
        lpFreeBlock->lAlloc= -lpFreeHeader->tag.block.lAlloc;
        lpBlockHeader->tag.block.lSize=lNewSize;
        lpBlockHeader->tag.block.lAlloc=lNewAlloc;
        lpBlockHeader->tag.block.lpBlock->lAlloc=lNewAlloc;
        hmemset((LPSTR)hpOldBlock+lNewSize, GUARD_PATTERN, lNewAlloc-lNewSize);
    }

    return((LPVOID)thisBlock(lpBlockHeader));
}



LPVOID PASCAL FAR _EXPORT greallocz(HPVOID hpOldBlock, LONG lNewSize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LONG lOldSize;
    LPVOID lpNewBlock;
    LPHeader lpBlockHeader;

    if (!hpOldBlock)
        return(gmallocz(lNewSize));

    lpBlockHeader=validateBlock(lpContextData, "greallocz", hpOldBlock);

    if (!lpBlockHeader)
        return(NULL);

    lOldSize=lpBlockHeader->tag.block.lSize;
    lpNewBlock=grealloc(hpOldBlock, lNewSize);

    if (lpNewBlock && lNewSize>lOldSize)
        hmemset((HPSTR)lpNewBlock+lOldSize, 0, lNewSize-lOldSize);

    return(lpNewBlock);
}



void FAR PASCAL _EXPORT gcallback(GmallocCallback gmallocCallback)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);

    lpContextData->gmallocCallback=gmallocCallback;
}



int FAR PASCAL _EXPORT gcheckblock(LPCSTR lpcszTag, HPVOID hpBlock)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);

    lpContextData->wFlags|=GMALLOC_CHECK_INTERNAL;
    validateBlock(lpContextData, lpcszTag ? lpcszTag : "gcheckblock", hpBlock);
    lpContextData->wFlags&= ~GMALLOC_CHECK_INTERNAL;
    return(lpContextData->nCode);
}



BOOL FAR PASCAL _EXPORT gcheckall(LPCSTR lpcszTag, GmallocCallback gmallocCallback)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LPHeader lpHeader=lpContextData->lpBlockHeader;
    BOOL fOK=TRUE;

    if (!gmallocCallback)
        gmallocCallback=lpContextData->gmallocCallback;

    if (!lpcszTag)
        lpcszTag="gcheckall";

    while (lpHeader) {
        int nCode=checkOverflow(lpHeader);

        if (nCode!=GMALLOC_OK) {
            lpContextData->nCode=nCode;
            fOK=FALSE;

            if (gmallocCallback)
                gmallocCallback(lpcszTag, nCode, thisBlock(lpHeader), lpHeader->tag.block.lSize);
            else
                return(FALSE);

        }

        lpHeader=lpHeader->lpNext;
    }

    return(fOK);
}
