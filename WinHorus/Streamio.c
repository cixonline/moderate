//  streamio.c - Buffered line-based IO for Windows and MVS
//               © 1993-1997 Pete Jordan, Horus Communications
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include "winhorus.h"
#if !defined(MVS)
    #include <io.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif
#include <stdarg.h>
#include <errno.h>
#include "context.h"
#if defined(MVS)
    #include "@stream.h"
#else
    #include "_stream.h"
#endif
#include "debugdef.h"

#define SIGNATURE 0xdeadbeefL

typedef struct {
    UINT uMaxOpenFiles;
    LPHSTREAM lpOpenFileTable;
    UINT uOpenCount;
    UINT uBuffcount;
    UINT uBuffsize;
    UINT uFormatBuffsize;
    int nErrcode;
    int nOSErrcode;
} ContextData, FAR *LPContextData;



static WHCONTEXT hContext=NULL;



#if defined(MVS)
    #pragma inline(_checkHandle)
    #pragma inline(checkHandle)
    #pragma inline(recfmBits)
    #pragma inline(_reopenFile)
    #pragma inline(_flushBuffer)
    #pragma inline(_flushFile)
    #pragma inline(_readRawChar)
    #pragma inline(_writeRawChar)
#endif

static int PASCAL _closeFile(LPContextData lpContextData, LPStream lpStream);
static int PASCAL _flushBuffer(LPStream lpStream, LPFileBuffer lpFileBuffer);
#if !defined(MVS)
    static int PASCAL _truncateFile(LPStream lpStream, LONG lPosition);
#endif
static LONG PASCAL _fileSize(LPStream lpStream);



void CALLBACK streamioClientPostlude(LPVOID lpContext)
{
    LPContextData lpContextData=(LPContextData)lpContext;

    if (lpContextData->lpOpenFileTable) {
        UINT uFile;

        for (uFile=0; uFile<lpContextData->uOpenCount; uFile++)
            _closeFile(lpContextData, (LPStream)lpContextData->lpOpenFileTable[uFile]);

        gfree((LPVOID)lpContextData->lpOpenFileTable);	lpContextData->lpOpenFileTable = NULL;
    }

    lpContextData->uOpenCount=0;
}



void PASCAL streamioPrelude(void)
{
    static ContextData defaultContext={
        DEFAULT_OPEN_FILES,
        NULL,
        0,
        DEFAULT_BUFFCOUNT,
        DEFAULT_BUFFSIZE,
        DEFAULT_FORMAT_BUFFSIZE,
        0, 0
    };

    hContext=registerContext((LPVOID)(LPContextData)&defaultContext,
                             sizeof(ContextData), NULL, streamioClientPostlude);

}



UINT FAR PASCAL _EXPORT defaultBuffcount(UINT uNewBuffcount)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    UINT uOldBuffcount=lpContextData->uBuffcount;

    if (uNewBuffcount>0) {
        if (uNewBuffcount<MIN_BUFFCOUNT)
            uNewBuffcount=MIN_BUFFCOUNT;
        else if (uNewBuffcount>MAX_BUFFCOUNT)
            uNewBuffcount=MAX_BUFFCOUNT;

        lpContextData->uBuffcount=uNewBuffcount;
    }

    return(uOldBuffcount);
}



UINT FAR PASCAL _EXPORT defaultBuffsize(UINT uNewBuffsize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    UINT uOldBuffsize=lpContextData->uBuffsize;

    if (uNewBuffsize>0) {
        if (uNewBuffsize<MIN_BUFFSIZE)
            uNewBuffsize=MIN_BUFFSIZE;
        else if (uNewBuffsize>MAX_BUFFSIZE)
            uNewBuffsize=MAX_BUFFSIZE;

        lpContextData->uBuffsize=uNewBuffsize;
    }

    return(uOldBuffsize);
}



UINT FAR PASCAL _EXPORT defaultFormatBuffsize(UINT uNewBuffsize)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    UINT uOldBuffsize=lpContextData->uFormatBuffsize;

    if (uNewBuffsize>0) {
        if (uNewBuffsize<MIN_FORMAT_BUFFSIZE)
            uNewBuffsize=MIN_FORMAT_BUFFSIZE;
        else if (uNewBuffsize>MAX_FORMAT_BUFFSIZE)
            uNewBuffsize=MAX_FORMAT_BUFFSIZE;

        lpContextData->uFormatBuffsize=uNewBuffsize;
    }

    return(uOldBuffsize);
}



int FAR PASCAL _EXPORT maxOpenFiles(UINT uNewLimit)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    UINT uOldLimit=lpContextData->uMaxOpenFiles;
    int nStatus=0;

    if (uNewLimit>0) {
        if (uNewLimit<MIN_OPEN_FILES)
            uNewLimit=MIN_OPEN_FILES;
        else if (uNewLimit>MAX_OPEN_FILES)
            uNewLimit=MAX_OPEN_FILES;

        if (uNewLimit>uOldLimit && lpContextData->lpOpenFileTable) {
            LPHSTREAM lpNewTable=(LPHSTREAM)grealloc((LPVOID)lpContextData->lpOpenFileTable,
                                                     uNewLimit*sizeof(HSTREAM));

            if (!lpNewTable)
                nStatus=lpContextData->nErrcode=STREAMIO_MEMORY;
            else {
                lpContextData->lpOpenFileTable=lpNewTable;
                lpContextData->uMaxOpenFiles=uNewLimit;
            }
        } else
            lpContextData->uMaxOpenFiles=uNewLimit;

    }

    return(nStatus ? nStatus : (int)uOldLimit);
}



static LPStream PASCAL _checkHandle(HSTREAM hStream, LPContextData lpContextData)
{
    if (hStream && lpContextData->lpOpenFileTable) {
        UINT uStream;

        for (uStream=0; uStream<lpContextData->uOpenCount; uStream++)
            if (lpContextData->lpOpenFileTable[uStream]==hStream)
                return((LPStream)hStream);

    }

    return(NULL);
}



static LPContextData PASCAL checkHandle(HSTREAM hStream)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);

    if (hStream && lpContextData->lpOpenFileTable) {
        UINT uStream;

        for (uStream=0; uStream<lpContextData->uOpenCount; uStream++)
            if (lpContextData->lpOpenFileTable[uStream]==hStream)
                return(lpContextData);

    }

    lpContextData->nErrcode=STREAMIO_CLOSED;
    return(NULL);
}



#if defined(_DEBUG_STREAMIO)
    static void debugCheckOpen(LPStreamData lpStreamData)
    {
        if (!lpStreamData->pFileDebug) {
            char szDebugFile[64];
            int nLength;

            *szDebugFile='\'';
            strcpy(szDebugFile+1, lpStreamData->szFilename);
            nLength=strlen(szDebugFile);

            if (szDebugFile[nLength-1]==')') {
                char *pName=szDebugFile+nLength-1;

                *pName='\0';
                while(*(--pName)!='(');
                *pName='.';
                nLength--;
            }

            if (nLength+2>45)
                szDebugFile[41]='\0';

            strcat(szDebugFile, ".$'");
            lpStreamData->pFileDebug=fopen(szDebugFile, "a");

            fprintf(lpStreamData->pFileDebug, "--- %s ---\n",
                    lpStreamData->szFilename);

        }
    }



    static void debugDumpBuffers(LPStreamData lpStreamData, char cTag, LONG lRequest, UINT uIndex)
    {
        debugCheckOpen(lpStreamData);
        fprintf(lpStreamData->pFileDebug, "%c%6d %2u|", cTag, lRequest, uIndex);

        if (lpStreamData->lpFileBuffers) {
            UINT uBuffer;

            for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
                LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

                fprintf(lpStreamData->pFileDebug, "%2d %5u %6d %4d %c %8p|",
                        uBuffer, lpFileBuffer->dwAccess, lpFileBuffer->lPosition,
                        lpFileBuffer->nEOB, lpFileBuffer->fDirty ? '*' : '.',
                        lpFileBuffer->lpBuffer);

            }
        } else
            fputs("Unallocated", lpStreamData->pFileDebug);

        fputc('\n', lpStreamData->pFileDebug);
    }
#endif



static LPFileBuffer PASCAL findBuffer(LPStream lpStream, LONG lPosition)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;
    LONG lBufferPosition=(lPosition/lpStreamData->uFileBuffsize)*lpStreamData->uFileBuffsize;
    DWORD dwOldest;
    UINT uBuffer;
    int nUnassigned= -1, nOldest= -1;
    LPFileBuffer lpFileBuffer;

    if (lpStreamData->lpFileBuffers) {
        dwOldest=0xffffffffL;

        for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount && nUnassigned<0; uBuffer++) {
            lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

            if (!lpFileBuffer->lpBuffer || lpFileBuffer->lPosition<0L)
                nUnassigned=(int)uBuffer;
            else if (lpFileBuffer->lPosition==lBufferPosition) {
                _ALERT(3, "%s - buffer for %ld found (%u)",
                       lpStreamData->szFilename, lPosition, uBuffer);

                lpStreamData->dwHits++;

                #if defined(_DEBUG_STREAMIO)
                    debugDumpBuffers(lpStreamData, 'o', lPosition, uBuffer);
                #endif

                return(lpFileBuffer);
            } else if (lpFileBuffer->dwAccess<dwOldest) {
                dwOldest=lpFileBuffer->dwAccess;
                nOldest=(int)uBuffer;
            }
        }
    } else {
        lpStreamData->lpFileBuffers=(LPFileBuffer)gcalloc(sizeof(FileBuffer), (WORD)lpStreamData->uFileBuffcount);

        if (!lpStreamData->lpFileBuffers) {
            lpStream->nErrcode=STREAMIO_MEMORY;

            #if defined(_DEBUG_STREAMIO)
                debugDumpBuffers(lpStreamData, '!', lPosition, -1);
            #endif

            return(NULL);
        }

        for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
            lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;
            lpFileBuffer->dwAccess=0L;
            lpFileBuffer->lPosition= -1L;
            lpFileBuffer->nEOB=0;
            lpFileBuffer->fDirty=FALSE;
            lpFileBuffer->lpBuffer=NULL;
        }

        nUnassigned=0;
    }

    if (nUnassigned>=0) {
        lpFileBuffer=lpStreamData->lpFileBuffers+nUnassigned;

        if (!lpFileBuffer->lpBuffer) {
            lpFileBuffer->lpBuffer=(LPSTR)gmalloc(lpStreamData->uFileBuffsize);

            if (!lpFileBuffer->lpBuffer)
                if (nOldest>=0)
                    nUnassigned= -1;
                else {
                    lpStream->nErrcode=STREAMIO_MEMORY;

                    #if defined(_DEBUG_STREAMIO)
                        debugDumpBuffers(lpStreamData, '!', lPosition, -2);
                    #endif

                    return(NULL);
                }
            else
                _ALERT(3, "%s - buffer allocated for %ld (%d) %lx",
                       lpStreamData->szFilename, lPosition, nUnassigned,
                       lpFileBuffer->lpBuffer);

        }
    }

    if (nUnassigned<0) {
        lpFileBuffer=lpStreamData->lpFileBuffers+nOldest;

        if (lpFileBuffer->lPosition>=0 && lpFileBuffer->fDirty) {
            int nStatus=_flushBuffer(lpStream, lpFileBuffer);

            if (nStatus) {
                lpStream->nErrcode=nStatus;

                #if defined(_DEBUG_STREAMIO)
                    debugDumpBuffers(lpStreamData, '!', lPosition, -3);
                #endif

                return(NULL);
            }
        }

        _ALERT(3, "%s - buffer reused for %ld (was %ld) %lx",
               lpStreamData->szFilename, lPosition,
               lpFileBuffer->lPosition, lpFileBuffer->lpBuffer);

    }

    lpStreamData->dwMisses++;
    lpFileBuffer->dwAccess=0L;
    lpFileBuffer->lPosition=lBufferPosition;
    lpFileBuffer->fDirty=FALSE;
    lpFileBuffer->nEOB=0;

    #if defined(_DEBUG_STREAMIO)
        if (nUnassigned<0)
            debugDumpBuffers(lpStreamData, '*', lPosition, nOldest);
        else
            debugDumpBuffers(lpStreamData, 'n', lPosition, nUnassigned);

    #endif

    return(lpFileBuffer);
}



#if defined(MVS)
    HOPENDATA FAR PASCAL _EXPORT createData(WORD wAllocType, UINT uBlockSize,
                                            UINT uPrimaryAlloc, UINT uSecondaryAlloc, UINT uDirectoryAlloc)
    {
        LPAllocData lpAllocData=(LPAllocData)gmalloc(sizeof(AllocData));

        if (lpAllocData) {
            lpAllocData->wFlags=wAllocType|OF_FREE;
            lpAllocData->uBlockSize=uBlockSize;
            lpAllocData->uPrimaryAlloc=uPrimaryAlloc;
            lpAllocData->uSecondaryAlloc=uSecondaryAlloc;
            lpAllocData->uDirectoryAlloc=uDirectoryAlloc;
        }

        return((HOPENDATA)lpAllocData);
    }



    HOPENDATA FAR PASCAL _EXPORT createLike(HSTREAM hStream)
    {
        if (checkHandle(hStream))
            return((HOPENDATA)(&((LPStream)hStream)->lpStreamData->allocData));
        else
            return(NULL);

    }



    static LPCSTR PASCAL recfmBits(WORD wFlags)
    {
        switch (wFlags&OF_EXTMASK) {

        case OF_BLOCKED:
            return("B");

        case OF_STANDARD:
            return("S");

        case OF_STANDARD|OF_BLOCKED:
            return("BS");

        case OF_ASA_PRINT:
        case OF_MACHINE_PRINT|OF_ASA_PRINT:
            return("A");

        case OF_ASA_PRINT|OF_BLOCKED:
        case OF_MACHINE_PRINT|OF_ASA_PRINT|OF_BLOCKED:
            return("BA");

        case OF_ASA_PRINT|OF_STANDARD:
        case OF_MACHINE_PRINT|OF_ASA_PRINT|OF_STANDARD:
            return("SA");

        case OF_ASA_PRINT|OF_STANDARD|OF_BLOCKED:
        case OF_MACHINE_PRINT|OF_ASA_PRINT|OF_STANDARD|OF_BLOCKED:
            return("BSA");

        case OF_MACHINE_PRINT:
            return("M");

        case OF_MACHINE_PRINT|OF_BLOCKED:
            return("BM");

        case OF_MACHINE_PRINT|OF_STANDARD:
            return("SM");

        case OF_MACHINE_PRINT|OF_STANDARD|OF_BLOCKED:
            return("BSM");

        default:
            return("");

        };
    }



    static HFILE PASCAL OSOpenFile(LPCSTR lpcFilename, WORD wFlags, LPSTR lpszOpenMode,
                                   UINT uRecLength, LPAllocData lpAllocData, LPINT lpnOSErrcode)
    {
        char szBuffer[32];
        WORD wOpenFlags=wFlags&OF_MODEMASK;
        HFILE hFile;

        switch (wOpenFlags&OF_MODEMASK) {

        case OF_READ:       lstrcpy(lpszOpenMode, "rb");      break;
        case OF_WRITE:      lstrcpy(lpszOpenMode, "w+b");     break;
        case OF_READWRITE:  lstrcpy(lpszOpenMode, "r+b");     break;

        default:
            *lpnOSErrcode=0;
            return(NULL);

        }

        if (wFlags&OF_RECFMASK)
            switch (wFlags&OF_BASEMASK) {

            case OF_FIXED:
                lstrcat(lpszOpenMode, ",recfm=F");
                lstrcat(lpszOpenMode, recfmBits(wFlags));
                break;

            case OF_VARIABLE:
                lstrcat(lpszOpenMode, ",recfm=V");
                lstrcat(lpszOpenMode, recfmBits(wFlags));
                break;

            case OF_UNFORMATTED:
                if (wFlags&OF_ASA_PRINT)
                    lstrcat(lpszOpenMode, ",recfm=UA");
                else if (wFlags&OF_MACHINE_PRINT)
                    lstrcat(lpszOpenMode, ",recfm=UM");
                else
                    lstrcat(lpszOpenMode, ",recfm=U");

            default:
                if (wFlags&OF_ASA_PRINT)
                    lstrcat(lpszOpenMode, ",recfm=A");

            }

        if (uRecLength) {
            wsprintf(szBuffer, ",lrecl=%u", uRecLength);
            lstrcat(lpszOpenMode, szBuffer);
        }

        if (lpAllocData) {
            if (lpAllocData->uBlockSize) {
                wsprintf(szBuffer, ",blksize=%u", lpAllocData->uBlockSize);
                lstrcat(lpszOpenMode, szBuffer);
            }

            if (lpAllocData->uPrimaryAlloc || lpAllocData->uSecondaryAlloc || lpAllocData->uDirectoryAlloc) {
                switch (lpAllocData->wFlags&OF_CYLINDERS) {

                case OF_CYLINDERS:
                    lstrcat(lpszOpenMode, ",space=(cyl,(");
                    break;

                case OF_TRACKS:
                    lstrcat(lpszOpenMode, ",space=(trk,(");
                    break;

                default:
                    wsprintf(szBuffer, ",space=(%u,(", lpAllocData->uBlockSize);
                    lstrcat(lpszOpenMode, szBuffer);
                    break;

                }

                if (lpAllocData->uPrimaryAlloc) {
                    wsprintf(szBuffer, "%u", lpAllocData->uPrimaryAlloc);
                    lstrcat(lpszOpenMode, szBuffer);
                }

                if (lpAllocData->uSecondaryAlloc) {
                    wsprintf(szBuffer, ",%u", lpAllocData->uSecondaryAlloc);
                    lstrcat(lpszOpenMode, szBuffer);
                } else
                    lstrcat(lpszOpenMode, ",");

                if (lpAllocData->uDirectoryAlloc) {
                    wsprintf(szBuffer, ",%u))", lpAllocData->uDirectoryAlloc);
                    lstrcat(lpszOpenMode, szBuffer);
                } else
                    lstrcat(lpszOpenMode, "))");

            }
        }

        if (wFlags&OF_HIPERSPACE)
            lstrcat(lpszOpenMode, ",type=memory(hiperspace)");
        else if (wFlags&OF_MEMORY)
            lstrcat(lpszOpenMode, ",type=memory");
        else if (wFlags&OF_RECORD_MODE)
            lstrcat(lpszOpenMode, ",type=record");
        else
            lstrcat(lpszOpenMode, ",byteseek");

        hFile=fopen(lpcFilename, lpszOpenMode);

        if (hFile==HFILE_ERROR && wOpenFlags&OF_READWRITE) {
            *lpszOpenMode='w';
            hFile=fopen(lpcFilename, lpszOpenMode);
        }

        *lpnOSErrcode=hFile==HFILE_ERROR ? _lastError() : 0;

        #if defined(_DEBUG)
            if (hFile==HFILE_ERROR) {
                _ALERT(1, "fopen(\"%s\", \"%s\") failed", lpcFilename, lpszOpenMode);
                _ALERT(1, strerror(*lpnOSErrcode));
            } else
                _ALERT(2, "fopen(\"%s\", \"%s\") => %x", lpcFilename, lpszOpenMode, hFile);

        #endif

        *lpszOpenMode='r';
        return(hFile);
    }

#else

    static HFILE PASCAL OSOpenFile(LPCSTR lpcFilename, WORD wFlags, LPINT lpnOSErrcode)
    {
        WORD wOpenFlags=wFlags&OF_OPENMASK;
        HFILE hFile=_lopen(lpcFilename, wOpenFlags);

        if (hFile==HFILE_ERROR)
            if (wOpenFlags&(OF_WRITE|OF_READWRITE)) {
                hFile=_lcreat(lpcFilename, 0);

                if (hFile!=HFILE_ERROR) {
                    _lclose(hFile);
                    hFile=_lopen(lpcFilename, wOpenFlags);
                }
            }

        *lpnOSErrcode=hFile==HFILE_ERROR ? _lastError() : 0;
        return(hFile);
    }
#endif



#if defined(MVS)
    HSTREAM FAR PASCAL _EXPORT openFile(LPCSTR lpcFilename, WORD wFlags, UINT uRecLength,
                                        HOPENDATA hAllocData)
#else
    HSTREAM FAR PASCAL _EXPORT openFile(LPCSTR lpcFilename, WORD wFlags)
#endif
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    WORD wOpenMode=wFlags&OF_MODEMASK;
    LPStream lpStream;
    LPStreamData lpStreamData;
    int nOSErrcode;

    #if defined(MVS)
        char szFilename[128];
        fldata_t fldataBuffer;

        _ALERT(1, "openFile(\"%s\", 0x%x, %u, %lx)",
               lpcFilename, wFlags, uRecLength, hAllocData);

    #else
        _ALERT(1, "openFile(\"%s\", 0x%x)", lpcFilename, wFlags);
    #endif

    if (lpContextData->uOpenCount>=lpContextData->uMaxOpenFiles) {
        lpContextData->nErrcode=STREAMIO_OPENCOUNT;
        _ALERT(1, "StreamIO: STREAMIO_OPENCOUNT");
        return(NULL);
    }

    if (!lpContextData->lpOpenFileTable) {
        lpContextData->lpOpenFileTable=(LPHSTREAM)gmalloc(lpContextData->uMaxOpenFiles*sizeof(HSTREAM));

        if (!lpContextData->lpOpenFileTable) {
            lpContextData->nErrcode=STREAMIO_MEMORY;
            _ALERT(1, "StreamIO: STREAMIO_MEMORY 1");
            return(NULL);
        }

        lpContextData->uOpenCount=0;
    }

    lpStream=(LPStream)gmalloc(sizeof(Stream));

    if (!lpStream) {
        lpContextData->nErrcode=STREAMIO_MEMORY;
        _ALERT(1, "StreamIO: STREAMIO_MEMORY 2");
        return(NULL);
    }

    lpStreamData=(LPStreamData)gmalloc(sizeof(StreamData));

    if (!lpStreamData) {
        lpContextData->nErrcode=STREAMIO_MEMORY;
        gfree(lpStream);	lpStream = NULL;
        _ALERT(1, "StreamIO: STREAMIO_MEMORY 3");
        return(NULL);
    }

    lpStream->lpStreamData=lpStreamData;
    lpStream->nErrcode=0;
    lpStream->nOSErrcode=0;

    #if defined(MVS)
        //  Kludge for CodeBase...
        if (wFlags&OF_HIPERSPACE) {
            wFlags&= ~(OF_HIPERSPACE|OF_MEMORY);
            wFlags|=OF_RECORD_MODE;
            hAllocData=createData(OF_CYLINDERS, 0, 15, 10, 0);
        }

        if (wFlags&OF_RECORD_MODE &&
            (wFlags&(OF_MEMORY|OF_HIPERSPACE) ||
             wFlags&OF_BASEMASK!=OF_FIXED ||
             uRecLength==0))
            wFlags&= ~OF_RECORD_MODE;

        lpStreamData->hFile=OSOpenFile(lpcFilename, wFlags, lpStreamData->szOpenMode,
                                       uRecLength, (LPAllocData)hAllocData, &nOSErrcode);

    #else
        lpStreamData->hFile=OSOpenFile(lpcFilename, wFlags, &nOSErrcode);
    #endif

    if (lpStreamData->hFile==HFILE_ERROR) 
	{
        lpContextData->nErrcode=STREAMIO_OPEN;
        lpContextData->nOSErrcode=nOSErrcode;
        gfree(lpStreamData);	lpStreamData = NULL;
        gfree(lpStream);		lpStream = NULL;
        return(NULL);
    }

    #if defined(MVS)
        if (fldata(lpStreamData->hFile, szFilename, &fldataBuffer)) {
            lstrcpy(lpStreamData->szFilename, lpcFilename);
            *lpStreamData->szDDName='\0';
        } else if (fldataBuffer.__dsname) {
            lstrcpy(lpStreamData->szFilename, fldataBuffer.__dsname);
            lstrcpy(lpStreamData->szDDName, szFilename);
        } else {
            lstrcpy(lpStreamData->szFilename, szFilename);
            *lpStreamData->szDDName='\0';
        }
    #else
        lstrcpy(lpStreamData->szFilename, lpcFilename);
    #endif

    lpStreamData->wFlags=wFlags;
    lpStreamData->uRefCount=1;
    lpStreamData->uFileBuffcount=lpContextData->uBuffcount;

    #if defined(MVS)
        lpStreamData->uFileBuffsize=uRecLength ? uRecLength : lpContextData->uBuffsize;
    #else
        lpStreamData->uFileBuffsize=lpContextData->uBuffsize;
    #endif

    lpStreamData->lpFileBuffers=NULL;
    lpStreamData->dwAccess=0L;
    lpStreamData->dwHits=0L;
    lpStreamData->dwMisses=0L;
    lpStreamData->dwFlushes=0L;

    if (wOpenMode==OF_WRITE && !(wFlags&OF_APPEND))
        lpStreamData->lFileSize=0L;
    else {
        LONG lSize;

        lpStream->out=NULL;
        lpStreamData->lFileSize= -1L;
        lSize=_fileSize(lpStream);
        _ALERT(2, "file size is %ld", lSize);

        if (lSize<0L) {
            _ALERT(1, "openFile - _fileSize error: %ld", lSize);
            lpContextData->nErrcode=(int)lSize;
            lpContextData->nOSErrcode=lpStream->nOSErrcode;
            OSclose(lpStreamData->hFile);
            gfree(lpStreamData);	lpStreamData = NULL;
            gfree(lpStream);		lpStream = NULL;
            return(NULL);
        }
    }

    if (wOpenMode==OF_READ || wOpenMode==OF_READWRITE) {
        lpStream->in=(LPStreamBuffer)gmalloc(sizeof(StreamBuffer));

        if (!lpStream->in) {
            lpContextData->nErrcode=STREAMIO_MEMORY;
            OSclose(lpStreamData->hFile);
            gfree(lpStreamData);	lpStreamData = NULL;
            gfree(lpStream);		lpStream = NULL;
            _ALERT(1, "StreamIO: STREAMIO_MEMORY 3");
            return(NULL);
        }

        lpStream->in->lpFileBuffer=NULL;
        lpStream->in->lPosition=0L;
        lpStream->in->nPtr=0;
        lpStream->in->fLFPending=FALSE;
        lpStream->in->lpcTranslate=NULL;
    } else
        lpStream->in=NULL;

    if (wOpenMode==OF_WRITE || wOpenMode==OF_READWRITE) {
        lpStream->out=(LPStreamBuffer)gmalloc(sizeof(StreamBuffer));

        if (!lpStream->out) {
            lpContextData->nErrcode=STREAMIO_MEMORY;
            OSclose(lpStreamData->hFile);

            if (lpStream->in)
			{
                gfree(lpStream->in);	lpStream->in = NULL;
			}

            gfree(lpStreamData);	lpStreamData = NULL;
            gfree(lpStream);		lpStream = NULL;
            _ALERT(1, "StreamIO: STREAMIO_MEMORY 4");
            return(NULL);
        }

        lpStream->out->lpFileBuffer=NULL;
        lpStream->out->nPtr=0;

        if (wFlags&OF_APPENDFLAG)
            lpStream->out->lPosition=lpStreamData->lFileSize;
        else {
            lpStream->out->lPosition=0L;

            #if !defined(MVS)
                if (wOpenMode==OF_WRITE)
                    _truncateFile(lpStream, 0L);

            #endif
        }

        lpStream->out->fLFPending=FALSE;
        lpStream->out->lpcTranslate=NULL;
    } else
        lpStream->out=NULL;

    #if defined(MVS)
        if (hAllocData) {
            LPAllocData lpAllocData=(LPAllocData)hAllocData;

            lpStreamData->allocData= *lpAllocData;

            if (lpAllocData->wFlags&OF_FREE) {
                gfree(lpAllocData);	lpAllocData = NULL;
                lpStreamData->allocData.wFlags&= ~OF_FREE;
            }
        } else
            lmemset(&lpStreamData->allocData, 0, sizeof(AllocData));

    #endif

    lpStreamData->uFormatBuffsize=lpContextData->uFormatBuffsize;
    lpStreamData->lpFormatBuffer=NULL;
    lpContextData->lpOpenFileTable[lpContextData->uOpenCount]=(HSTREAM)lpStream;
    lpContextData->uOpenCount++;
    _ALERT(2, "openFile succeeded - %lx", lpStream);
    return((HSTREAM)lpStream);
}



LPCSTR FAR PASCAL _EXPORT streamioErrtext(int nErrcode)
{
    switch (nErrcode) {

    case STREAMIO_EOF:         return("End of file");
    case STREAMIO_OPENMODE:    return("File open mode incorrect for this operation");
    case STREAMIO_OPEN:        return("Error opening file");
    case STREAMIO_REOPEN:      return("Error reopening suspended file");
    case STREAMIO_SEEK:        return("Seek error");
    case STREAMIO_WRITE:       return("Write error");
    case STREAMIO_READ:        return("Read error");
    case STREAMIO_TRUNCATE:    return("Error truncating file");
    case STREAMIO_MEMORY:      return("Memory allocation error");
    case STREAMIO_CLOSED:      return("Invalid file handle; file closed?");
    case STREAMIO_NOFILE:      return("Error performing stat() on suspended file");
    case STREAMIO_OVERFLOW:    return("Buffer overflow in readLine()");
    case STREAMIO_OPENCOUNT:   return("Attempt to open too many files");
    case STREAMIO_ARGERROR:    return("Argument error in call to StreamIO routine");
    case STREAMIO_OSFILEERROR: return("Operating system file error");

    default:                   return("");

    }
}



int FAR PASCAL _EXPORT lastStreamError(HSTREAM hStream)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LPStream lpStream=_checkHandle(hStream, lpContextData);

    if (lpStream)
        return(lpStream->nErrcode);
    else
        return(lpContextData->nErrcode);

}



int FAR PASCAL _EXPORT lastOSError(HSTREAM hStream)
{
    LPContextData lpContextData=(LPContextData)getContext(hContext);
    LPStream lpStream=_checkHandle(hStream, lpContextData);

    if (lpStream)
        return(lpStream->nOSErrcode);
    else
        return(lpContextData->nOSErrcode);

}



LPSTR FAR PASCAL _EXPORT openFileName(HSTREAM hStream)
{
    LPContextData lpContextData=checkHandle(hStream);

    if (lpContextData) {
        LPStream lpStream=(LPStream)hStream;

        return((LPSTR)lpStream->lpStreamData->szFilename);
    } else
        return(NULL);

}



HFILE FAR PASCAL _EXPORT fileHandle(HSTREAM hStream)
{
    LPContextData lpContextData=checkHandle(hStream);

    if (lpContextData) {
        LPStream lpStream=(LPStream)hStream;
        return(lpStream->lpStreamData->hFile);
    } else
        return((HFILE)0);

}



HSTREAM FAR PASCAL _EXPORT dupFile(HSTREAM hStream)
{
    LPContextData lpContextData=checkHandle(hStream);

    _ALERT(2, "dupFile(%lx)", hStream);

    if (lpContextData) {
        LPStream lpOldStream=(LPStream)hStream;
        LPStream lpNewStream;

        _ALERT(1, "dupFile - %s", lpOldStream->lpStreamData->szFilename);

        if (lpContextData->uOpenCount>=lpContextData->uMaxOpenFiles) {
            lpContextData->nErrcode=STREAMIO_OPENCOUNT;
            return(NULL);
        }

        lpNewStream=(LPStream)gmalloc(sizeof(Stream));

        if (!lpNewStream) {
            lpContextData->nErrcode=STREAMIO_MEMORY;
            return(NULL);
        }

        *lpNewStream= *lpOldStream;

        if (lpOldStream->in) {
            lpNewStream->in=(LPStreamBuffer)gmalloc(sizeof(StreamBuffer));

            if (!lpNewStream->in) 
			{
                lpContextData->nErrcode=STREAMIO_MEMORY;
                gfree(lpNewStream);	lpNewStream = NULL;
                return(NULL);
            }

            *lpNewStream->in= *lpOldStream->in;
        }

        if (lpOldStream->out) {
            lpNewStream->out=(LPStreamBuffer)gmalloc(sizeof(StreamBuffer));

            if (!lpNewStream->out) {
                lpContextData->nErrcode=STREAMIO_MEMORY;

                if (lpNewStream->in)
				{
                    gfree(lpNewStream->in);	lpNewStream->in = NULL;
				}

                gfree(lpNewStream);	lpNewStream = NULL;
                return(NULL);
            }

            *lpNewStream->out= *lpOldStream->out;
        }

        lpOldStream->lpStreamData->uRefCount++;
        lpContextData->lpOpenFileTable[lpContextData->uOpenCount]=(HSTREAM)lpNewStream;
        lpContextData->uOpenCount++;
        _ALERT(2, "dupFile succeeded - %lx", lpNewStream);
        return((HSTREAM)lpNewStream);
    } else
        return(NULL);

}



static int PASCAL _reopenFile(LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;

    if (lpStreamData->hFile==HFILE_ERROR) {
        #if defined(MVS)
            lpStreamData->hFile=fopen(lpStreamData->szFilename, lpStreamData->szOpenMode);
        #else
            lpStreamData->hFile=_lopen(lpStreamData->szFilename, lpStreamData->wFlags&OF_OPENMASK);
        #endif

        if (lpStreamData->hFile==HFILE_ERROR) {
            lpStream->nErrcode=STREAMIO_REOPEN;
            lpStream->nOSErrcode=_lastError();
            return(STREAMIO_REOPEN);
        }
    }

    return(0);
}



int FAR PASCAL _EXPORT reopenFile(HSTREAM hStream)
{
    if (checkHandle(hStream))
        return(_reopenFile((LPStream)hStream));
    else
        return(STREAMIO_CLOSED);

}



static int PASCAL _flushBuffer(LPStream lpStream, LPFileBuffer lpFileBuffer)
{
    if (lpFileBuffer->fDirty && lpFileBuffer->nEOB>0) {
        LPStreamData lpStreamData=lpStream->lpStreamData;
        LONG lPosition=lpFileBuffer->lPosition;
        BOOL fOK;

        #if defined(MVS)
            int nOSErrcode;

            if (lpStreamData->wFlags&OF_RECORD_MODE)
                lPosition/=lpStreamData->uFileBuffsize;

        #endif

        if (lpStreamData->hFile==HFILE_ERROR) {
            int nStatus=_reopenFile(lpStream);

            if (nStatus)
                return(nStatus);

        }

        _ALERT(3, "%s - flushing %ld (%d bytes)",
               lpStreamData->szFilename, lPosition, lpFileBuffer->nEOB);

        lpStreamData->dwFlushes++;
        fOK=OSseek(lpStreamData->hFile, lPosition);

        #if defined(MVS)
            nOSErrcode=_lastError();

            if (!fOK && lpStreamData->wFlags&OF_RECORD_MODE) {
                fOK=fseek(lpStreamData->hFile, 0, SEEK_END)==0;

                if (fOK) {
                    LONG lEnd=ftell(lpStreamData->hFile);

                    if (lEnd<0)
                        fOK=FALSE;
                    else
                        while (lEnd<lPosition && fOK) {
                            fOK=fwrite("", 1, 1, lpStreamData->hFile)==1;
                            lEnd++;
                        }

                    if (!fOK)
                        nOSErrcode=_lastError();

                }
            }

            if (!fOK) {
                _ALERT(1, "_flushBuffer seek error - %s", strerror(nOSErrcode));
                lpStream->nOSErrcode=nOSErrcode;
                lpStream->nErrcode=STREAMIO_SEEK;
                return(STREAMIO_SEEK);
            }
        #else
            if (!fOK) {
                _ALERT(1, "_flushBuffer seek error");
                lpStream->nOSErrcode=_lastError();
                lpStream->nErrcode=STREAMIO_SEEK;
                return(STREAMIO_SEEK);
            }
        #endif

        if (!OSwrite(lpStreamData->hFile, lpFileBuffer->lpBuffer, lpFileBuffer->nEOB)) {
            _ALERT(1, "_flushBuffer write error");
            lpStream->nOSErrcode=_lastError();

            #if defined(MVS)
                _ALERT(1, strerror(lpStream->nOSErrcode));
            #endif

            lpStream->nErrcode=STREAMIO_WRITE;
            return(STREAMIO_WRITE);
        }

        lpFileBuffer->fDirty=FALSE;

        #if defined(_DEBUG_STREAMIO)
            debugDumpBuffers(lpStreamData, 'f', lpFileBuffer->lPosition,
                             lpFileBuffer-lpStreamData->lpFileBuffers);

        #endif
    }

    return(0);
}



static int PASCAL _flushFile(LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;

    if (lpStreamData->lpFileBuffers) {
        UINT uBuffer;

        for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
            LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

            if (lpFileBuffer->fDirty && lpFileBuffer->lpBuffer && lpFileBuffer->lPosition>=0) {
                int nStatus=_flushBuffer(lpStream, lpFileBuffer);

                if (nStatus)
                    return(nStatus);

            }
        }
    }

    return(0);
}



int FAR PASCAL _EXPORT flushFile(HSTREAM hStream)
{
    if (checkHandle(hStream))
        return(_flushFile((LPStream)hStream));
    else
        return(STREAMIO_CLOSED);

}



int PASCAL _suspendFile(LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;
    int nStatus=0;

    if (lpStreamData->hFile!=HFILE_ERROR) {
        nStatus=_flushFile(lpStream);
        OSclose(lpStreamData->hFile);
        lpStreamData->hFile=HFILE_ERROR;
    }

    return(nStatus);
}



int FAR PASCAL _EXPORT suspendFile(HSTREAM hStream)
{
    if (checkHandle(hStream))
        return(_suspendFile((LPStream)hStream));
    else
        return(STREAMIO_CLOSED);

}



int PASCAL _closeFile(LPContextData lpContextData, LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;
    int nStatus=0;

    if (lpStreamData->uRefCount<=1) {
        lpStream->nOSErrcode=0;
        nStatus=_suspendFile(lpStream);

        if (nStatus) {
            lpContextData->nErrcode=nStatus;
            lpContextData->nOSErrcode=lpStream->nOSErrcode;
        }

        _ALERT(1, "closing %s - accesses:%lu hits:%lu misses:%lu flushes:%lu",
               lpStreamData->szFilename, lpStreamData->dwAccess,
               lpStreamData->dwHits, lpStreamData->dwMisses, lpStreamData->dwFlushes);

    }

    if (lpStreamData->uRefCount>0)
        lpStreamData->uRefCount--;

    if (lpStream->in)
	{
        gfree(lpStream->in);	lpStream->in = NULL;
	}

    if (lpStream->out)
	{
        gfree(lpStream->out);	lpStream->out = NULL;
	}

    if (lpStreamData->uRefCount==0) 
	{
        if (lpStreamData->lpFormatBuffer)
		{
            gfree(lpStreamData->lpFormatBuffer);	lpStreamData->lpFormatBuffer = NULL;
		}

        #if defined(_DEBUG_STREAMIO)
            if (lpStreamData->pFileDebug) {
                fclose(lpStreamData->pFileDebug);
                lpStreamData->pFileDebug=NULL;
            }
        #endif

        if (lpStreamData->lpFileBuffers) {
            UINT uBuffer;

            for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
                LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

                if (lpFileBuffer->lpBuffer)
				{
                    gfree(lpFileBuffer->lpBuffer);	lpFileBuffer->lpBuffer = NULL;
				}

            }

            gfree(lpStreamData->lpFileBuffers);	lpStreamData->lpFileBuffers = NULL;
        }

        gfree(lpStreamData);	lpStreamData = NULL;
    }

    gfree(lpStream);	lpStream = NULL;
    return(nStatus);
}



int FAR PASCAL _EXPORT closeFile(HSTREAM hStream)
{
    LPContextData lpContextData=checkHandle(hStream);
    int nStatus=0;

    _ALERT(2, "closeFile(%lx)", hStream);

    if (!lpContextData)
        nStatus=STREAMIO_CLOSED;
    else {
        LPStream lpStream=(LPStream)hStream;
        UINT uStream;

        _ALERT(1, "closeFile - %s", lpStream->lpStreamData->szFilename);
        nStatus=_closeFile(lpContextData, lpStream);

        for (uStream=0;
             uStream<lpContextData->uOpenCount &&
             lpContextData->lpOpenFileTable[uStream]!=hStream;
             uStream++);

        while (++uStream<lpContextData->uOpenCount)
            lpContextData->lpOpenFileTable[uStream-1]=lpContextData->lpOpenFileTable[uStream];

        lpContextData->uOpenCount--;
    }

    return(nStatus);
}



static LPFileBuffer PASCAL _readFileBuffer(LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;
    LPStreamBuffer lpInBuffer=lpStream->in;
    LPFileBuffer lpFileBuffer=lpInBuffer->lpFileBuffer;

    if (!lpFileBuffer ||
        lpFileBuffer->lPosition!=lpInBuffer->lPosition ||
        lpInBuffer->nPtr<0 ||
        lpInBuffer->nPtr>=lpFileBuffer->nEOB && lpFileBuffer->nEOB==(int)lpStreamData->uFileBuffsize) {
        LONG lPtr=lpInBuffer->lPosition+lpInBuffer->nPtr;

        lpInBuffer->lpFileBuffer=lpFileBuffer=findBuffer(lpStream, lPtr);

        if (!lpFileBuffer)
            return(NULL);

        lpInBuffer->lPosition=lpFileBuffer->lPosition;
        lpInBuffer->nPtr=(int)(lPtr-lpFileBuffer->lPosition);

        if (lpFileBuffer->nEOB==0) {
            LONG lPosition=lpFileBuffer->lPosition;

            #if defined(MVS)
                if (lpStreamData->wFlags&OF_RECORD_MODE)
                    lPosition/=lpStreamData->uFileBuffsize;

            #endif

            if (lpStreamData->hFile==HFILE_ERROR &&
                !_reopenFile(lpStream))
                return(NULL);

            if (!OSseek(lpStreamData->hFile, lPosition)) {
                _ALERT(1, "_readFileBuffer seek error");
                lpStream->nOSErrcode=_lastError();

                #if defined(MVS)
                    _ALERT(1, strerror(lpStream->nOSErrcode));
                #endif

                lpStream->nErrcode=STREAMIO_SEEK;
                return(NULL);
            }

            lpFileBuffer->nEOB=OSread(lpStreamData->hFile, lpFileBuffer->lpBuffer,
                                      lpStreamData->uFileBuffsize);

            _GCHECK(2, "_readFileBuffer");

            #if defined(MVS)
                if (lpFileBuffer->nEOB==0 && !feof(lpStreamData->hFile)) {
                    lpStream->nOSErrcode=_lastError();
                    _ALERT(1, "_readFileBuffer read error - %s", strerror(lpStream->nOSErrcode));
                    lpStream->nErrcode=STREAMIO_READ;
                    return(NULL);
                }

                if ((lpStreamData->wFlags&OF_RECORD_FIXED)==OF_RECORD_MODE &&
                    lpStreamData->lFileSize<lpFileBuffer->lPosition+lpStreamData->uFileBuffsize)
                    lpFileBuffer->nEOB=lpStreamData->lFileSize%lpStreamData->uFileBuffsize;

            #else
                if (lpFileBuffer->nEOB<0) {
                    lpFileBuffer->nEOB=0;
                    lpStream->nOSErrcode=_lastError();
                    _ALERT(1, "_readFileBuffer read error");
                    lpStream->nErrcode=STREAMIO_READ;
                    return(NULL);
                }
            #endif

            #if defined(_DEBUG_STREAMIO)
                debugDumpBuffers(lpStreamData, 'r', lpFileBuffer->lPosition,
                                 lpFileBuffer-lpStreamData->lpFileBuffers);

            #endif
        }
    }

    lpFileBuffer->dwAccess=lpStreamData->dwAccess++;
    return(lpFileBuffer);
}



static int PASCAL _readRawChar(LPStream lpStream)
{
    LPFileBuffer lpFileBuffer=_readFileBuffer(lpStream);

    if (lpFileBuffer) {
        LPStreamBuffer lpInBuffer=lpStream->in;

        if (lpInBuffer->nPtr>=lpFileBuffer->nEOB) {
            lpStream->nErrcode=STREAMIO_EOF;
            return(lpStream->nErrcode);
        } else if (lpInBuffer->lpcTranslate)
            return((BYTE)lpInBuffer->lpcTranslate[(BYTE)lpFileBuffer->lpBuffer[lpInBuffer->nPtr++]]);
        else
            return((BYTE)lpFileBuffer->lpBuffer[lpInBuffer->nPtr++]);

    } else
        return(lpStream->nErrcode);

}



int FAR PASCAL _EXPORT readRawChar(HSTREAM hStream)
{
    if (checkHandle(hStream)) {
        LPStream lpStream=(LPStream)hStream;

        if (!lpStream->in) {
            lpStream->nErrcode=STREAMIO_OPENMODE;
            return(STREAMIO_OPENMODE);
        } else
            return(_readRawChar(lpStream));

    } else
        return(STREAMIO_CLOSED);

}



static int PASCAL _readChar(LPStream lpStream)
{
    int ch;

    ch=_readRawChar(lpStream);

    #if !defined(MVS)
        if (ch=='\x1a' && lpStream->lpStreamData->wFlags&OF_CPM_EOF)
            ch=STREAMIO_EOF;
        else
    #endif
    if (ch>=0) {
        if (lpStream->in->fLFPending) {
            lpStream->in->fLFPending=FALSE;

            if (ch=='\n')
                ch=_readRawChar(lpStream);

        }

        if (ch=='\r') {
            lpStream->in->fLFPending=TRUE;
            ch='\n';
        }
    }

    return(ch);
}



int FAR PASCAL _EXPORT readChar(HSTREAM hStream)
{
    if (checkHandle(hStream)) {
        LPStream lpStream=(LPStream)hStream;

        if (!lpStream->in) {
            lpStream->nErrcode=STREAMIO_OPENMODE;
            return(STREAMIO_OPENMODE);
        } else
            return(_readChar(lpStream));

    } else
        return(STREAMIO_CLOSED);

}



int FAR PASCAL _EXPORT readLine(HSTREAM hStream, LPSTR lpLineBuff, UINT uMaxLength)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    UINT uLineLength=0;
    BOOL fFinished=FALSE;
    BOOL fOverflow=FALSE;
    LPStreamBuffer lpInBuffer;
    int ch;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpInBuffer=lpStream->in;

    if (!lpInBuffer) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(STREAMIO_OPENMODE);
    }

    do {
        ch=_readChar(lpStream);

        if (ch<=0 || ch=='\n') {
            *lpLineBuff='\0';
            fFinished=TRUE;
        } else if (uLineLength==uMaxLength) {
            *lpLineBuff='\0';
            lpInBuffer->nPtr--;
            fFinished=TRUE;
            fOverflow=TRUE;
        } else {
            *lpLineBuff++ =ch;
            uLineLength++;
        }
    } while (!fFinished);

    return(fOverflow ? STREAMIO_OVERFLOW : ch<0 && uLineLength==0 ? ch : uLineLength);
}



int FAR PASCAL _EXPORT readBlock(HSTREAM hStream, LPVOID lpBuffer, UINT uLength)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPSTR lpPtr=(LPSTR)lpBuffer;
    LPStream lpStream;
    LPStreamBuffer lpInBuffer;
    UINT uRead=0;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpStream=(LPStream)hStream;
    lpInBuffer=lpStream->in;

    if (!lpStream->in) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(STREAMIO_OPENMODE);
    }

    while (uLength>0) {
        LPFileBuffer lpFileBuffer=_readFileBuffer(lpStream);

        if (!lpFileBuffer)
            return(lpStream->nErrcode);
        else if (lpInBuffer->nPtr>=lpFileBuffer->nEOB)
            break;

        if (!lpInBuffer->lpcTranslate) {
            UINT uCount=lpFileBuffer->nEOB-lpInBuffer->nPtr;

            if (uCount>uLength)
                uCount=uLength;

            lmemcpy(lpPtr, lpFileBuffer->lpBuffer+lpInBuffer->nPtr, uCount);
            lpInBuffer->nPtr+=uCount;
            lpPtr+=uCount;
            uLength-=uCount;
            uRead+=uCount;
        } else
            while (lpInBuffer->nPtr<lpFileBuffer->nEOB && uLength>0) {
                *lpPtr++ =lpInBuffer->lpcTranslate[lpFileBuffer->lpBuffer[lpInBuffer->nPtr++]];
                uLength--;
                uRead++;
            }

    }

    return(uLength>0 && uRead==0 ? STREAMIO_EOF : uRead);
}



LONG FAR PASCAL _EXPORT readTell(HSTREAM hStream)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamBuffer lpInBuffer;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpInBuffer=lpStream->in;

    if (!lpInBuffer) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(STREAMIO_OPENMODE);
    }

    return(lpInBuffer->lPosition+lpInBuffer->nPtr);
}



LONG FAR PASCAL _EXPORT readSeek(HSTREAM hStream, LONG lNewPosition)
{
    LONG lOldPosition=readTell(hStream);
    LPStream lpStream=(LPStream)hStream;

    if (lOldPosition>=0 && lNewPosition>=0) {
        LPStreamData lpStreamData=lpStream->lpStreamData;

        lpStream->in->lPosition=(lNewPosition/lpStreamData->uFileBuffsize)*lpStreamData->uFileBuffsize;
        lpStream->in->nPtr=(int)lNewPosition%lpStreamData->uFileBuffsize;
        lpStream->in->fLFPending=FALSE;

        if (!_readFileBuffer(lpStream))
            return(lpStream->nErrcode);

    }

    return(lOldPosition);
}



int FAR PASCAL _EXPORT readRandom(HSTREAM hStream, LONG lPosition, LPVOID lpBuffer, UINT uLength)
{
    LONG lStatus=readSeek(hStream, lPosition);

    if (lStatus>=0)
        lStatus=readBlock(hStream, lpBuffer, uLength);

    return((int)lStatus);
}



static LPFileBuffer PASCAL _writeFileBuffer(LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;
    LPStreamBuffer lpOutBuffer=lpStream->out;
    LPFileBuffer lpFileBuffer=lpOutBuffer->lpFileBuffer;

    if (!lpFileBuffer ||
        lpFileBuffer->lPosition!=lpOutBuffer->lPosition ||
        lpOutBuffer->nPtr<0 ||
        lpOutBuffer->nPtr>=(int)lpStreamData->uFileBuffsize) {
        LONG lPtr=lpOutBuffer->lPosition+lpOutBuffer->nPtr;

        lpOutBuffer->lpFileBuffer=lpFileBuffer=findBuffer(lpStream, lPtr);

        if (!lpFileBuffer)
            return(NULL);

        lpOutBuffer->lPosition=lpFileBuffer->lPosition;
        lpOutBuffer->nPtr=(int)(lPtr-lpFileBuffer->lPosition);

        if (lpStream->in && lpFileBuffer->nEOB==0) {
            LONG lPosition=lpFileBuffer->lPosition;

            #if defined(MVS)
                if (lpStreamData->wFlags&OF_RECORD_MODE)
                    lPosition/=lpStreamData->uFileBuffsize;

            #endif

            if (lpStreamData->hFile==HFILE_ERROR &&
                !_reopenFile(lpStream))
                return(NULL);

            if (!OSseek(lpStreamData->hFile, lPosition)) {
                _ALERT(2, "_writeFileBuffer read seek failure - benign at end of file");

                #if defined(MVS)
                    _ALERT(2, strerror(errno));
                #endif
            } else {
                lpFileBuffer->nEOB=OSread(lpStreamData->hFile, lpFileBuffer->lpBuffer,
                                          lpStreamData->uFileBuffsize);

                _GCHECK(2, "_writeFileBuffer");

                #if defined(MVS)
                    if (lpFileBuffer->nEOB==0 && !feof(lpStreamData->hFile)) {
                        lpStream->nOSErrcode=_lastError();
                        _ALERT(1, "_writeFileBuffer read error - %s", strerror(lpStream->nOSErrcode));
                    } else if ((lpStreamData->wFlags&OF_RECORD_FIXED)==OF_RECORD_MODE &&
                             lpStreamData->lFileSize<lpFileBuffer->lPosition+lpStreamData->uFileBuffsize)
                        lpFileBuffer->nEOB=lpStreamData->lFileSize%lpStreamData->uFileBuffsize;

                #else
                    if (lpFileBuffer->nEOB<0) {
                        lpStream->nOSErrcode=_lastError();
                        _ALERT(1, "_writeFileBuffer read error");
                        lpFileBuffer->nEOB=0;
                    }
                #endif

                #if defined(_DEBUG_STREAMIO)
                    debugDumpBuffers(lpStreamData, 'R', lpFileBuffer->lPosition,
                                     lpFileBuffer-lpStreamData->lpFileBuffers);

                #endif
            }
        }
    }

    lpFileBuffer->dwAccess=lpStreamData->dwAccess++;
    return(lpFileBuffer);
}



static int PASCAL _writeRawChar(LPStream lpStream, char ch)
{
    LPFileBuffer lpFileBuffer=_writeFileBuffer(lpStream);

    if (lpFileBuffer) {
        LPStreamBuffer lpOutBuffer=lpStream->out;

        lpFileBuffer->lpBuffer[lpOutBuffer->nPtr++]=lpOutBuffer->lpcTranslate ?
                                                        lpOutBuffer->lpcTranslate[(BYTE)ch] : ch;

        lpFileBuffer->fDirty=TRUE;

        if (lpOutBuffer->nPtr>lpFileBuffer->nEOB) {
            LPStreamData lpStreamData=lpStream->lpStreamData;

            lpFileBuffer->nEOB=lpOutBuffer->nPtr;

            if (lpFileBuffer->lPosition+lpFileBuffer->nEOB>lpStreamData->lFileSize)
                lpStreamData->lFileSize=lpFileBuffer->lPosition+lpFileBuffer->nEOB;

        }

        return(0);
    } else
        return(lpStream->nErrcode);

}



int FAR PASCAL _EXPORT writeRawChar(HSTREAM hStream, char ch)
{
    if (checkHandle(hStream)) {
        LPStream lpStream=(LPStream)hStream;

        if (!lpStream->out) {
            lpStream->nErrcode=STREAMIO_OPENMODE;
            return(STREAMIO_OPENMODE);
        } else
            return(_writeRawChar(lpStream, ch));

    } else
        return(STREAMIO_CLOSED);

}



static int PASCAL _writeChar(LPStream lpStream, char ch)
{
    WORD wConversion=lpStream->lpStreamData->wFlags&OF_CONVERTMASK;
    int nStatus=0;

    if (wConversion) {
        if (ch=='\r' || ch=='\n') {
            if (ch=='\r' || !lpStream->out->fLFPending) {
                if (wConversion&OF_CRCONVERT)
                    nStatus=_writeRawChar(lpStream, '\r');

                if (nStatus==0 && (wConversion&OF_LFCONVERT))
                    nStatus=_writeRawChar(lpStream, '\n');

            }
        } else
            nStatus=_writeRawChar(lpStream, ch);

        lpStream->out->fLFPending=ch=='\r';
    } else
        nStatus=_writeRawChar(lpStream, ch);

    return(nStatus);
}



int FAR PASCAL _EXPORT writeChar(HSTREAM hStream, char ch)
{
    if (checkHandle(hStream)) {
        LPStream lpStream=(LPStream)hStream;

        if (!lpStream->out) {
            lpStream->nErrcode=STREAMIO_OPENMODE;
            return(STREAMIO_OPENMODE);
        } else
            return(_writeChar(lpStream, ch));

    } else
        return(STREAMIO_CLOSED);

}



int FAR _EXPORT writeLine(HSTREAM hStream, LPCSTR lpcLineFormat,...)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    int nStatus=0;

    if (!lpContextData)
        nStatus=STREAMIO_CLOSED;
    else if (!lpStream->out)
        nStatus=lpStream->nErrcode=STREAMIO_OPENMODE;
    else if (lstrcmp(lpcLineFormat, "%s")==0) {
        va_list args;
        LPSTR lpPtr;
        char ch;

        va_start(args, lpcLineFormat);
        lpPtr=va_arg(args, LPSTR);
        va_end(args);

        if (!lpPtr)
            nStatus=lpStream->nErrcode=STREAMIO_ARGERROR;
        else
            while ((ch= *lpPtr++) && !nStatus)
                nStatus=_writeChar(lpStream, ch);

    } else {
        LPStreamData lpStreamData=lpStream->lpStreamData;

        if (!lpStreamData->lpFormatBuffer/* || lpStreamData->lpFormatBuffer == SIGNATURE*/) {
            lpStreamData->lpFormatBuffer=(LPSTR)gmalloc(lpStreamData->uFormatBuffsize);

            if (!lpStreamData->lpFormatBuffer)
                nStatus=lpStream->nErrcode=STREAMIO_MEMORY;

        }

        if (!nStatus) {
            va_list args;
            LPSTR lpPtr=lpStreamData->lpFormatBuffer;
            char ch;

            va_start(args, lpcLineFormat);
            wvsprintf(lpStreamData->lpFormatBuffer, lpcLineFormat, args);
            va_end(args);

            while ((ch= *lpPtr++) && !nStatus)
                nStatus=_writeChar(lpStream, ch);

        }
    }

    return(nStatus);
}



int FAR PASCAL _EXPORT writeBlock(HSTREAM hStream, LPVOID lpBuffer, UINT uLength)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPSTR lpPtr=(LPSTR)lpBuffer;
    LPStream lpStream;
    LPStreamData lpStreamData;
    LPStreamBuffer lpOutBuffer;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpStream=(LPStream)hStream;
    lpStreamData=lpStream->lpStreamData;
    lpOutBuffer=lpStream->out;

    if (!lpOutBuffer) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(STREAMIO_OPENMODE);
    }

    while (uLength>0) {
        LPFileBuffer lpFileBuffer=_writeFileBuffer(lpStream);

        if (!lpFileBuffer)
            return(lpStream->nErrcode);

        if (!lpOutBuffer->lpcTranslate) {
            UINT uCount=lpStreamData->uFileBuffsize-lpOutBuffer->nPtr;

            if (uCount>uLength)
                uCount=uLength;

            lmemcpy(lpFileBuffer->lpBuffer+lpOutBuffer->nPtr, lpPtr, uCount);
            _GCHECK(2, "writeBlock");
            lpOutBuffer->nPtr+=uCount;
            lpPtr+=uCount;
            uLength-=uCount;
        } else
            while (lpOutBuffer->nPtr<(int)lpStreamData->uFileBuffsize && uLength>0) {
                lpFileBuffer->lpBuffer[lpOutBuffer->nPtr++]=lpOutBuffer->lpcTranslate[*lpPtr++];
                uLength--;
            }

        lpFileBuffer->fDirty=TRUE;

        if (lpOutBuffer->nPtr>lpFileBuffer->nEOB) {
            lpFileBuffer->nEOB=lpOutBuffer->nPtr;

            if (lpFileBuffer->lPosition+lpFileBuffer->nEOB>lpStreamData->lFileSize)
                lpStreamData->lFileSize=lpFileBuffer->lPosition+lpFileBuffer->nEOB;

        }
    }

    return(0);
}



LONG FAR PASCAL _EXPORT writeTell(HSTREAM hStream)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamBuffer lpOutBuffer;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpOutBuffer=lpStream->out;

    if (!lpOutBuffer) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(STREAMIO_OPENMODE);
    }

    return(lpOutBuffer->lPosition+lpOutBuffer->nPtr);
}



LONG FAR PASCAL _EXPORT writeSeek(HSTREAM hStream, LONG lNewPosition)
{
    LONG lOldPosition=writeTell(hStream);
    LPStream lpStream=(LPStream)hStream;

    if (lOldPosition>=0) {
        LPStreamData lpStreamData=lpStream->lpStreamData;
        UINT uBuffer;

        if (lNewPosition<0) {
            int nStatus=_flushFile(lpStream);

            if (!nStatus && lpStreamData->hFile==HFILE_ERROR)
                nStatus=_reopenFile(lpStream);

            if (nStatus)
                return(nStatus);

            lNewPosition=_fileSize(lpStream);

            if (lNewPosition<0)
                return(lNewPosition);

        }

        lpStream->out->lPosition=(lNewPosition/lpStreamData->uFileBuffsize)*lpStreamData->uFileBuffsize;
        lpStream->out->nPtr=(int)(lNewPosition%lpStreamData->uFileBuffsize);
        lpStream->out->fLFPending=FALSE;

        if (!_writeFileBuffer(lpStream))
            return(lpStream->nErrcode);

        for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
            LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

            if (lpFileBuffer->lpBuffer && lpFileBuffer->lPosition>=0 &&
                lpFileBuffer->lPosition<lpStream->out->lPosition &&
                lpFileBuffer->nEOB<(int)lpStreamData->uFileBuffsize)
                lpFileBuffer->nEOB=lpStreamData->uFileBuffsize;

        }
    }

    return(lOldPosition);
}



int FAR PASCAL _EXPORT writeRandom(HSTREAM hStream, LONG lPosition, LPVOID lpBuffer, UINT uLength)
{
    LONG lStatus=writeSeek(hStream, lPosition);

    if (lStatus>=0)
        lStatus=writeBlock(hStream, lpBuffer, uLength);

    return((int)lStatus);
}



LPCSTR FAR PASCAL _EXPORT readTranslate(HSTREAM hStream, LPCSTR lpcNewTranslate)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamBuffer lpInBuffer;
    LPCSTR lpcOldTranslate;

    if (!lpContextData)
        return(NULL);

    lpInBuffer=lpStream->in;

    if (!lpInBuffer) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(NULL);
    }

    lpcOldTranslate=lpInBuffer->lpcTranslate;
    lpInBuffer->lpcTranslate=lpcNewTranslate;
    return(lpcOldTranslate);
}



LPCSTR FAR PASCAL _EXPORT writeTranslate(HSTREAM hStream, LPCSTR lpcNewTranslate)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamBuffer lpOutBuffer;
    LPCSTR lpcOldTranslate;

    if (!lpContextData)
        return(NULL);

    lpOutBuffer=lpStream->out;

    if (!lpOutBuffer) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(NULL);
    }

    lpcOldTranslate=lpOutBuffer->lpcTranslate;
    lpOutBuffer->lpcTranslate=lpcNewTranslate;
    return(lpcOldTranslate);
}



int FAR PASCAL _EXPORT fileBuffcount(HSTREAM hStream, UINT uNewBuffcount)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamData lpStreamData;
    UINT uOldBuffcount;
    int nStatus=0;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpStreamData=lpStream->lpStreamData;
    uOldBuffcount=lpStreamData->uFileBuffcount;

    if (uNewBuffcount>0) {
        if (uNewBuffcount<MIN_BUFFCOUNT)
            uNewBuffcount=MIN_BUFFCOUNT;
        else if (uNewBuffcount>MAX_BUFFCOUNT)
            uNewBuffcount=MAX_BUFFCOUNT;

        if (uNewBuffcount!=uOldBuffcount && lpStreamData->lpFileBuffers) {
            LPFileBuffer lpNewBuffers;
            UINT uBuffer;

            for (uBuffer=0; uBuffer<uOldBuffcount; uBuffer++) {
                LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

                if (lpFileBuffer->lpBuffer) 
				{
                    if (lpFileBuffer->fDirty)
                        _flushBuffer(lpStream, lpFileBuffer);

                    gfree(lpFileBuffer->lpBuffer);	lpFileBuffer->lpBuffer = NULL;
                }
            }

            lpNewBuffers=(LPFileBuffer)grealloc(lpStreamData->lpFileBuffers,
                                                sizeof(FileBuffer)*uNewBuffcount);

            if (!lpNewBuffers) {
                lpStream->nErrcode=STREAMIO_MEMORY;
                return(STREAMIO_MEMORY);
            }

            for (uBuffer=0; uBuffer<uNewBuffcount; uBuffer++) {
                LPFileBuffer lpFileBuffer=lpNewBuffers+uBuffer;

                lpFileBuffer->lpBuffer=NULL;
                lpFileBuffer->lPosition= -1L;
                lpFileBuffer->nEOB=0;
                lpFileBuffer->dwAccess=0L;
            }

            lpStreamData->lpFileBuffers=lpNewBuffers;
        }

        lpStreamData->uFileBuffcount=uNewBuffcount;
    }

    return(nStatus ? nStatus : (int)uOldBuffcount);
}



int FAR PASCAL _EXPORT fileBuffsize(HSTREAM hStream, UINT uNewBuffsize)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamData lpStreamData;
    UINT uOldBuffsize;
    int nStatus=0;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    lpStreamData=lpStream->lpStreamData;

    #if defined(MVS)
        if (lpStreamData->wFlags&OF_RECORD_MODE && uNewBuffsize>0)
            return(STREAMIO_ARGERROR);

    #endif

    uOldBuffsize=lpStreamData->uFileBuffsize;

    if (uNewBuffsize>0) {
        if (uNewBuffsize<MIN_BUFFSIZE)
            uNewBuffsize=MIN_BUFFSIZE;
        else if (uNewBuffsize>MAX_BUFFSIZE)
            uNewBuffsize=MAX_BUFFSIZE;

        if (uNewBuffsize!=uOldBuffsize && lpStreamData->lpFileBuffers) {
            UINT uBuffer;

            for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
                LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

                if (lpFileBuffer->lpBuffer) {
                    if (lpFileBuffer->fDirty)
                        _flushBuffer(lpStream, lpFileBuffer);

                    gfree(lpFileBuffer->lpBuffer);
                    lpFileBuffer->lpBuffer=NULL;
                }

                lpFileBuffer->lPosition= -1L;
                lpFileBuffer->nEOB=0;
                lpFileBuffer->dwAccess=0L;
            }
        }

        lpStreamData->uFileBuffsize=uNewBuffsize;
    }

    return(nStatus ? nStatus : (int)uOldBuffsize);
}



int FAR PASCAL _EXPORT formatBuffsize(HSTREAM hStream, UINT uNewBuffsize)
{
    LPContextData lpContextData=checkHandle(hStream);
    LPStream lpStream=(LPStream)hStream;
    LPStreamData lpStreamData;
    UINT uOldBuffsize;

    if (!lpContextData)
        return(STREAMIO_CLOSED);

    if (!lpStream->out) {
        lpStream->nErrcode=STREAMIO_OPENMODE;
        return(STREAMIO_OPENMODE);
    }

    lpStreamData=lpStream->lpStreamData;
    uOldBuffsize=lpStreamData->uFormatBuffsize;

    if (uNewBuffsize>0) {
        if (uNewBuffsize<MIN_FORMAT_BUFFSIZE)
            uNewBuffsize=MIN_FORMAT_BUFFSIZE;
        else if (uNewBuffsize>MAX_FORMAT_BUFFSIZE)
            uNewBuffsize=MAX_FORMAT_BUFFSIZE;

        if (uNewBuffsize>uOldBuffsize || !lpStreamData->lpFormatBuffer)
            lpStreamData->uFormatBuffsize=uNewBuffsize;

        if (lpStreamData->lpFormatBuffer && uNewBuffsize>uOldBuffsize) 
		{
            gfree(lpStreamData->lpFormatBuffer);
            lpStreamData->lpFormatBuffer=NULL;
        }
    }

    return((int)uOldBuffsize);
}



#if !defined(MVS)
    static int PASCAL _truncateFile(LPStream lpStream, LONG lPosition)
    {
        LPStreamData lpStreamData=lpStream->lpStreamData;

        if (lpStreamData->lpFileBuffers) {
            LONG lLastBuffer=(lPosition/lpStreamData->uFileBuffsize)*lpStreamData->uFileBuffsize;
            UINT uBuffer;

            for (uBuffer=0; uBuffer<lpStreamData->uFileBuffcount; uBuffer++) {
                LPFileBuffer lpFileBuffer=lpStreamData->lpFileBuffers+uBuffer;

                if (lpFileBuffer->lpBuffer && lpFileBuffer->lPosition>=0L)
                    if (lpFileBuffer->lPosition<lLastBuffer)
                        lpFileBuffer->nEOB=lpStreamData->uFileBuffsize-1;
                    else if (lpFileBuffer->lPosition==lLastBuffer)
                        lpFileBuffer->nEOB=(int)(lPosition%lpStreamData->uFileBuffsize);
                    else {
                        lpFileBuffer->dwAccess=0L;
                        lpFileBuffer->fDirty=FALSE;
                        lpFileBuffer->lPosition= -1L;
                    }

            }
        }

        #if defined(_WIN32)
            if (SetFilePointer((HANDLE)lpStreamData->hFile, lPosition, NULL, FILE_BEGIN)!=0xffffffffL &&
                SetEndOfFile((HANDLE)lpStreamData->hFile))
                return(0);
            else {
                lpStream->nOSErrcode=_lastError();
                lpStream->nErrcode=STREAMIO_TRUNCATE;
                return(STREAMIO_TRUNCATE);
            }
        #else
            if (chsize(lpStreamData->hFile, lPosition)==0)
                return(0);
            else {
                lpStream->nOSErrcode=_lastError();
                lpStream->nErrcode=STREAMIO_TRUNCATE;
                return(STREAMIO_TRUNCATE);
            }
        #endif
    }



    LONG FAR PASCAL _EXPORT truncateFile(HSTREAM hStream)
    {
        LPStream lpStream=(LPStream)hStream;
        LONG lPosition=writeTell(hStream);
        int nStatus=0;

        if (lPosition<0)
            nStatus=(int)lPosition;
        else {
            nStatus=_reopenFile(lpStream);

            if (!nStatus)
                nStatus=_truncateFile(lpStream, lPosition);

        }

        return(nStatus ? nStatus : lPosition);
    }
#endif



static LONG PASCAL _fileSize(LPStream lpStream)
{
    LPStreamData lpStreamData=lpStream->lpStreamData;
    LONG lSize;

    #if defined(MVS)
        lSize=lpStreamData->lFileSize;

        if (lSize<0) {
            HFILE hFile=lpStreamData->hFile;
            int nStatus;

            if (hFile==HFILE_ERROR) {
                nStatus=_reopenFile(lpStream);

                if (nStatus)
                    return(nStatus);

                hFile=lpStreamData->hFile;
            }

            nStatus=fseek(hFile, 0, SEEK_END);

            if (nStatus) {
                lpStream->nOSErrcode=_lastError();
                lpStream->nErrcode=STREAMIO_SEEK;
                return(STREAMIO_SEEK);
            }

            lSize=ftell(hFile);
            _ALERT(4, "_fileSize - ftell => %ld", lSize);

            if (lSize<0) {
                lpStream->nOSErrcode=_lastError();
                _ALERT(1, strerror(lpStream->nOSErrcode));
                lpStream->nErrcode=STREAMIO_OSFILEERROR;
                return(STREAMIO_OSFILEERROR);
            }

            if (lSize>0 && lpStreamData->wFlags&OF_RECORD_MODE)
                if (lpStreamData->wFlags&OF_NO_STRIP)
                    lSize*=lpStreamData->uFileBuffsize;
                else {
                    LPFileBuffer lpFileBuffer;
                    int nPtr;

                    lSize=(lSize-1)*lpStreamData->uFileBuffsize;
                    lpFileBuffer=findBuffer(lpStream, lSize);

                    if (!lpFileBuffer)
                        return(lpStream->nErrcode);

                    if (lpFileBuffer->nEOB<=0) {
                        nStatus=fseek(hFile, -1, SEEK_CUR);

                        if (nStatus) {
                            lpStream->nOSErrcode=_lastError();
                            lpStream->nErrcode=STREAMIO_SEEK;
                            return(STREAMIO_SEEK);
                        }

                        lpFileBuffer->nEOB=fread(lpFileBuffer->lpBuffer, 1,
                                                 lpStreamData->uFileBuffsize, hFile);

                        if (lpFileBuffer->nEOB==0 && !feof(hFile))
                            lpStream->nOSErrcode=_lastError();

                        _GCHECK(2, "_fileSize");
                        _ALERT(4, "_fileSize() - fread() => %d", lpFileBuffer->nEOB);

                        if (lpFileBuffer->nEOB<0)
                            lpFileBuffer->nEOB=0;

                    }

                    for (nPtr=lpFileBuffer->nEOB;
                         nPtr>0 && !lpFileBuffer->lpBuffer[nPtr-1];
                         nPtr--);

                    lSize+=nPtr;
                }

        }
    #else
        if (lpStreamData->hFile==HFILE_ERROR) {
            char szFilename[128];
            struct stat statbuff;

            lstrcpy(szFilename, lpStreamData->szFilename);

            if (stat(szFilename, &statbuff)) {
                lpStream->nErrcode=STREAMIO_NOFILE;
                return(STREAMIO_NOFILE);
            }

            lSize=statbuff.st_size;
        } else {
            #if defined(_WIN32)
                lSize=(LONG)GetFileSize((HANDLE)lpStreamData->hFile, NULL);
            #else
                lSize=filelength(lpStreamData->hFile);
            #endif

            if (lSize<0) {
                lpStream->nOSErrcode=_lastError();
                lpStream->nErrcode=STREAMIO_OSFILEERROR;
                return(STREAMIO_OSFILEERROR);
            }
        }
    #endif

    if (lpStream->out &&
        lpStream->out->lPosition+lpStream->out->nPtr>lSize)
        lSize=lpStream->out->lPosition+lpStream->out->nPtr;

    lpStreamData->lFileSize=lSize;
    return(lSize);
}



LONG FAR PASCAL _EXPORT fileSize(HSTREAM hStream)
{
    if (checkHandle(hStream)) {
        LPStream lpStream=(LPStream)hStream;

        return(_fileSize(lpStream));
    } else
        return(STREAMIO_CLOSED);

}



#if defined(MVS) && defined(COBOL)
    //  COBOL interface routines...

    #pragma linkage(hdefbcnt, COBOL)
    #pragma linkage(hdefbsiz, COBOL)
    #pragma linkage(hmaxopen, COBOL)
    #pragma linkage(hopends,  COBOL)
    #pragma linkage(herrtext, COBOL)
    #pragma linkage(herrcode, COBOL)
    #pragma linkage(hdsname,  COBOL)
    #pragma linkage(hdupds,   COBOL)
    #pragma linkage(hropends, COBOL)
    #pragma linkage(hflushds, COBOL)
    #pragma linkage(hsuspds,  COBOL)
    #pragma linkage(hcloseds, COBOL)
    #pragma linkage(hrdchraw, COBOL)
    #pragma linkage(hrdchar,  COBOL)
    #pragma linkage(hrdline,  COBOL)
    #pragma linkage(hrdblock, COBOL)
    #pragma linkage(hrdtell,  COBOL)
    #pragma linkage(hrdseek,  COBOL)
    #pragma linkage(hrdrand,  COBOL)
    #pragma linkage(hwrchraw, COBOL)
    #pragma linkage(hwrchar,  COBOL)
    #pragma linkage(hwrline,  COBOL)
    #pragma linkage(hwrblock, COBOL)
    #pragma linkage(hwrtell,  COBOL)
    #pragma linkage(hwrseek,  COBOL)
    #pragma linkage(hwrrand,  COBOL)
    #pragma linkage(hrdxlate, COBOL)
    #pragma linkage(hwrxlate, COBOL)
    #pragma linkage(hdsbcnt,  COBOL)
    #pragma linkage(hdsbsiz,  COBOL)
    #pragma linkage(hdssize,  COBOL)

    #pragma inline(c2cobol)



    typedef struct {                            //  01 OPEN-DATA.
        char sDatasetName[44];                  //     03 DATASET-NAME      PIC X(44).
        int nRecLength;                         //     03 REC-LENGTH        PIC S9(9) COMP.
        int nFlags;                             //     03 OPEN-FLAGS        PIC S9(9) COMP.
    } OpenData, *LPOpenData;



    typedef struct {                            //  01 CREATE-DATA.
        int nAllocType;                         //     03 ALLOC-TYPE        PIC S9(9) COMP.
        int nBlockSize;                         //     03 BLOCK-SIZE        PIC S9(9) COMP.
        int nPrimaryAlloc;                      //     03 PRIMARY-ALLOC     PIC S9(9) COMP.
        int nSecondaryAlloc;                    //     03 SECONDARY-ALLOC   PIC S9(9) COMP.
        int nDirectoryAlloc;                    //     03 DIRECTORY-ALLOC   PIC S9(9) COMP.
    } CreateData, *LPCreateData;



    static void streamioError(HSTREAM hStream, PStatus pStatus, int nStatus)
    {
        if (nStatus==0)
            nStatus=lastStreamError(hStream);

        setStatus(pStatus, STREAMIO_STATUS, nStatus);

        if (nStatus==STREAMIO_MEMORY)
            setStatus(pStatus, GMALLOC_STATUS, gstatus());
        else
            setStatus(pStatus, OS_STATUS, lastOSError(hStream));

        setError(pStatus, nStatus);
    }



    static void c2cobol(LPSTR lpsDest, LPCSTR lpcszSource, int nMaxLength)
    {
        int nLength=lpcszSource ? strlen(lpcszSource) : 0;

        if (nLength==0)
            memset(lpsDest, ' ', nMaxLength);
        else if (nLength>=nMaxLength)
            memcpy(lpsDest, lpcszSource, nMaxLength);
        else {
            memcpy(lpsDest, lpcszSource, nLength);
            memset(lpsDest+nLength, ' ', nMaxLength-nLength);
        }
    }



    void hdefbcnt(int nNewBuffcount, LPINT lpnStatus)
    {
        *lpnStatus=(int)defaultBuffcount((UINT)nNewBuffcount);
    }



    void hdefbsiz(int nNewBuffsize, LPINT lpnStatus)
    {
        *lpnStatus=(int)defaultBuffsize((UINT)nNewBuffsize);
    }



    void hmaxopen(int nNewLimit, LPINT lpnStatus)
    {
        *lpnStatus=maxOpenFiles((UINT)nNewLimit);
    }



    void hopends(LPHSTREAM lphStream, LPOpenData lpOpenData, LPVOID lpCobolAlloc, LPINT lpnStatus)
    {
        LPContextData lpContextData=(LPContextData)getContext(hContext);
        char szFilename[45];
        int nPtr;
        HOPENDATA hAllocData=NULL;

        for (nPtr=0; nPtr<44 && lpOpenData->sDatasetName[nPtr]!=' '; nPtr++)
            szFilename[nPtr]=lpOpenData->sDatasetName[nPtr];

        szFilename[nPtr]='\0';

        if (lpCobolAlloc)
            if (checkHandle((HSTREAM)lpCobolAlloc))
                hAllocData=(HOPENDATA)(&((LPStream)lpCobolAlloc)->lpStreamData->allocData);
            else {
                LPCreateData lpCreateData=(LPCreateData)lpCobolAlloc;

                hAllocData=createData(lpCreateData->nAllocType, lpCreateData->nBlockSize,
                                      lpCreateData->nPrimaryAlloc, lpCreateData->nSecondaryAlloc,
                                      lpCreateData->nDirectoryAlloc);

                if (!hAllocData) {
                    lpContextData->nErrcode=STREAMIO_MEMORY;
                    *lpnStatus=STREAMIO_MEMORY;
                    *lphStream=NULL;
                    return;
                }
            }

        *lphStream=openFile(szFilename, (WORD)lpOpenData->nFlags,
                            (UINT)lpOpenData->nRecLength, hAllocData);

        *lpnStatus= *lphStream ? 0 : lastStreamError(NULL);
    }



    void herrtext(int nErrcode, LPSTR lpsErrtext, int nMaxLength)
    {
        c2cobol(lpsErrtext, streamioErrtext(nErrcode), nMaxLength);
    }



    void herrcode(LPHSTREAM lphStream, LPINT lpnErrcode)
    {
        *lpnErrcode=lastOSError(*lphStream);
    }



    void hdsname(LPHSTREAM lphStream, LPSTR lpsName, int nMaxLength, LPINT lpnStatus)
    {
        LPCSTR lpcszName=openFileName(*lphStream);

        if (lpcszName) {
            c2cobol(lpsName, lpcszName, nMaxLength);
            *lpnStatus=0;
        } else
            *lpnStatus=STREAMIO_CLOSED;

    }



    void hdupds(LPHSTREAM lphStreamNew, LPHSTREAM lphStreamOld, LPINT lpnStatus)
    {
        *lphStreamNew=dupFile(*lphStreamOld);
        *lpnStatus= *lphStreamNew ? 0 : lastStreamError(NULL);
    }



    void hropends(LPHSTREAM lphStream, LPINT lpnStatus)
    {
        *lpnStatus=checkHandle(*lphStream) ?
                       _reopenFile((LPStream)(*lphStream)) :
                       STREAMIO_CLOSED;

    }



    void hflushds(LPHSTREAM lphStream, LPINT lpnStatus)
    {
        *lpnStatus=checkHandle(*lphStream) ?
                       _flushFile((LPStream)(*lphStream)) :
                       STREAMIO_CLOSED;

    }



    void hsuspds(LPHSTREAM lphStream, LPINT lpnStatus)
    {
        *lpnStatus=checkHandle(*lphStream) ?
                       _suspendFile((LPStream)(*lphStream)) :
                       STREAMIO_CLOSED;

    }



    void hcloseds(LPHSTREAM lphStream, LPINT lpnStatus)
    {
        *lpnStatus=closeFile(*lphStream);
        *lphStream=NULL;
    }



    void hrdchraw(LPHSTREAM lphStream, LPINT lpnChar, LPINT lpnStatus)
    {
        if (checkHandle(*lphStream)) {
            LPStream lpStream=(LPStream)(*lphStream);

            if (!lpStream->in)
                *lpnStatus=lpStream->nErrcode=STREAMIO_OPENMODE;
            else {
                int nChar=_readRawChar(lpStream);

                if (nChar>=0) {
                    *lpnChar=nChar;
                    *lpnStatus=0;
                } else
                    *lpnStatus=nChar;

            }
        } else
            *lpnStatus=STREAMIO_CLOSED;

    }



    void hrdchar(PHSTREAM phStream, PINT pnChar, PStatus pStatus)
    {
        if (checkHandle(*phStream)) {
            LPStream lpStream=(LPStream)(*phStream);

            if (!lpStream->in)
                *lpnStatus=lpStream->nErrcode=STREAMIO_OPENMODE;
            else {
                int nChar=_readChar(lpStream);

                if (nChar>=0) {
                    *lpnChar=nChar;
                    *lpnStatus=0;
                } else
                    *lpnStatus=nChar;

            }
        } else
            *lpnStatus=STREAMIO_CLOSED;

    }



    void hrdline(PHSTREAM phStream, PSTR psLineBuff, int nMaxLength,
                 PINT pnLength, PStatus pStatus)
    {
        int nStatus=readLine(*phStream, psLineBuff, (UINT)nMaxLength);

        clearStatus(pStatus);

        if (nStatus>=0) {
            if (nStatus<nMaxLength)
                memset(psLineBuff+nStatus, ' ', nMaxLength-nStatus);

            setStatus(pStatus, STREAMIO_READCOUNT, nStatus);
        } else
            streamioError(*phStream, pStatus, nStatus);

    }



    void hrdblock(PHSTREAM phStream, PVOID pBuffer, int nLength,
                  PStatus pStatus)
    {
        int nStatus=readBlock(*phStream, pBuffer, (UINT)nLength);

        clearStatus(pStatus);

        if (nStatus>=0) {
            if (nStatus<nLength)
                memset((PSTR)(pBuffer)+nStatus, '\0', nLength-nStatus);

            setStatus(pStatus, STREAMIO_READCOUNT, nStatus);
        } else
            streamioError(*phStream, pStatus, nStatus);

    }



    void hrdtell(PHSTREAM phStream, PINT pnPosition, PStatus pStatus)
    {
        int nPosition=(int)readTell(*phStream);

        clearStatus(pStatus);

        if (nPosition>=0)
            *pnPosition=nPosition;
        else
            streamioError(*phStream, pStatus, nPosition);

    }



    void hrdseek(PHSTREAM phStream, int nNewPosition, PStatus pStatus)
    {
        int nStatus=(int)readSeek(*phStream, (LONG)nNewPosition);

        clearStatus(pStatus);

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hrdrand(PHSTREAM phStream, int nPosition, PVOID pBuffer,
                 int nLength, PStatus pStatus)
    {
        int nStatus=(int)readSeek(*phStream, (LONG)nPosition);

        clearStatus(pStatus);

        if (nStatus>=0) {
            nStatus=readBlock(*phStream, pBuffer, (UINT)nLength);

            if (nStatus>=0 && nStatus<nLength)
                memset((PSTR)(pBuffer)+nStatus, '\0', nLength-nStatus);

        }

        if (nStatus>=0)
            setStatus(pStatus, STREAMIO_READCOUNT, nStatus);
        else
            streamioError(*phStream, pStatus, nStatus);

    }



    void hwrchraw(PHSTREAM phStream, int nChar, PStatus pStatus)
    {
        int nStatus=0;

        clearStatus(pStatus);

        if (checkHandle(*phStream)) {
            LPStream lpStream=(LPStream)(*lphStream);

            nStatus=lpStream->out ?
                    _writeRawChar(lpStream, (char)nChar) :
                    STREAMIO_OPENMODE;

        } else
            nStatus=STREAMIO_CLOSED;

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hwrchar(PHSTREAM phStream, int nChar, PStatus pStatus)
    {
        int nStatus=0;

        clearStatus(pStatus);

        if (checkHandle(*phStream)) {
            LPStream lpStream=(LPStream)(*lphStream);

            nStatus=lpStream->out ?
                    _writeChar(lpStream, (char)nChar) :
                    STREAMIO_OPENMODE;

        } else
            nStatus=STREAMIO_CLOSED;

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hwrline(PHSTREAM phStream, PCSTR pcsLineBuff, int nLength,
                 PStatus pStatus)
    {
        int nStatus=0;

        clearStatus(pStatus);

        if (checkHandle(*phStream)) {
            LPStream lpStream=(LPStream)(*phStream);

            if (lpStream->out) {
                int nPtr;

                for (nPtr=0; nPtr<nLength && nStatus==0; nPtr++)
                    nStatus=_writeChar(lpStream, pcsLineBuff[nPtr]);

                if (nStatus==0)
                    nStatus=_writeChar(lpStream, '\n');

            } else
                nStatus=STREAMIO_OPENMODE;

        } else
            nStatus=STREAMIO_CLOSED;

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hwrblock(PHSTREAM phStream, PVOID pBuffer, int nLength,
                  PStatus pStatus)
    {
        int nStatus=writeBlock(*phStream, pBuffer, (UINT)nLength);

        clearStatus(pStatus);

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hwrtell(PHSTREAM phStream, PINT pnPosition, PStatus pStatus)
    {
        int nPosition=(int)writeTell(*phStream);

        clearStatus(pStatus);

        if (nPosition>=0)
            *pnPosition=nPosition;
        else
            streamioError(*phStream, pStatus, nPosition);

    }



    void hwrseek(PHSTREAM phStream, int nNewPosition, PStatus pStatus)
    {
        int nStatus=(int)writeSeek(*phStream, (LONG)nNewPosition);

        clearStatus(pStatus);

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hwrrand(PHSTREAM phStream, int nPosition, PVOID pBuffer,
                 int nLength, PStatus pStatus)
    {
        int nStatus=(int)writeSeek(*phStream, (LONG)nPosition);

        if (nStatus>=0)
            nStatus=writeBlock(*phStream, pBuffer, (UINT)nLength);

        clearStatus(pStatus);

        if (nStatus<0)
            streamioError(*phStream, pStatus, nStatus);

    }



    void hrdxlate(PHSTREAM phStream, PCSTR pcNewTranslate,
                  PStatus pStatus)
    {
        PCSTR pcOldTranslate=readTranslate(*phStream, pcNewTranslate);

        clearStatus(pStatus);

        if (!pcOldTranslate)
            streamioError(*psStream, pStatus, 0);

    }



    void hwrxlate(PHSTREAM phStream, PCSTR pcNewTranslate,
                  PStatus pStatus)
    {
        PCSTR pcOldTranslate=writeTranslate(*phStream, pcNewTranslate);

        clearStatus(pStatus);

        if (!pcOldTranslate)
            streamioError(*psStream, pStatus, 0);

    }



    void hdsbcnt(PHSTREAM phStream, int nNewBuffcount, PStatus pStatus)
    {
        int nOldBuffcount=(int)fileBuffcount(*phStream, (UINT)nNewBuffcount);

        clearStatus(pStatus);

        if (nOldBuffcount>=0)
            setStatus(pStatus, STREAMIO_BUFFCOUNT, nOldBuffcount);
        else
            streamioError(*psStream, pStatus, nOldBuffcount);

    }



    void hdsbsiz(PHSTREAM phStream, int nNewBuffsize, PStatus pStatus)
    {
        int nOldBuffsize=(int)fileBuffsize(*phStream, (UINT)nNewBuffsize);

        clearStatus(pStatus);

        if (nOldBuffsize>=0)
            setStatus(pStatus, STREAMIO_BUFFSIZE, nOldBuffsize);
        else
            streamioError(*psStream, pStatus, nOldBuffsize);

    }



    void hdssize(PHSTREAM phStream, PINT pnSize, PStatus pStatus)
    {
        clearStatus(pStatus);

        if (checkHandle(*phStream)) {
            int nSize=_fileSize((LPStream)(*phStream)); :

            if (nSize>=0)
                *pnSize=nSize;
            else
                streamioError(*psStream, pStatus, nSize);

        else
            streamioError(*psStream, pStatus, STREAMIO_CLOSED);

    }



#elif !defined(MVS)
    //  Obsolete routines...

    int FAR PASCAL _EXPORT readBuffsize(HSTREAM hStream, UINT uNewBuffsize)
    {
        return(fileBuffsize(hStream, uNewBuffsize));
    }



    int FAR PASCAL _EXPORT writeBuffsize(HSTREAM hStream, UINT uNewBuffsize)
    {
        return(fileBuffsize(hStream, uNewBuffsize));
    }
#endif
