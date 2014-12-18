#ifndef _HORUS__STREAM_H
#define _HORUS__STREAM_H



//  _stream.h - Buffered line-based IO for Windows - internal definitions
//              © 1993-1995 Pete Jordan, Horus Communication
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



enum {
    MIN_BUFFSIZE=128,
    DEFAULT_BUFFSIZE=1024,
    MAX_BUFFSIZE=32767,

    MIN_BUFFCOUNT=1,
    #if defined(MVS)
        DEFAULT_BUFFCOUNT=8,
        MAX_BUFFCOUNT=8192,
    #else
        DEFAULT_BUFFCOUNT=2,
        MAX_BUFFCOUNT=256,
    #endif

    MIN_FORMAT_BUFFSIZE=512,
    DEFAULT_FORMAT_BUFFSIZE=1024,
    MAX_FORMAT_BUFFSIZE=32767,

    MIN_OPEN_FILES=8,
    DEFAULT_OPEN_FILES=32,
    MAX_OPEN_FILES=128
};



typedef struct {
    DWORD dwAccess;
    LONG lPosition;
    int nEOB;
    BOOL fDirty;
    LPSTR lpBuffer;
} FileBuffer, FAR *LPFileBuffer;



typedef struct {
    LPFileBuffer lpFileBuffer;
    LONG lPosition;
    int nPtr;
    BOOL fLFPending;
    LPCSTR lpcTranslate;
} StreamBuffer, FAR *LPStreamBuffer;



#if defined(MVS)
    enum {OF_FREE=0x8000};

    typedef struct {
        WORD wFlags;
        UINT uBlockSize;
        UINT uPrimaryAlloc;
        UINT uSecondaryAlloc;
        UINT uDirectoryAlloc;
    } AllocData, FAR *LPAllocData;
#endif



typedef struct {
    HFILE hFile;
    char szFilename[128];
    WORD wFlags;
    #if defined(MVS)
        AllocData allocData;
        char szOpenMode[128];
        char szDDName[10];
    #endif
    LONG lFileSize;
    UINT uRefCount;
    UINT uFileBuffcount;
    UINT uFileBuffsize;
    LPFileBuffer lpFileBuffers;
    DWORD dwAccess;
    DWORD dwHits;
    DWORD dwMisses;
    DWORD dwFlushes;
    UINT uFormatBuffsize;
    LPSTR lpFormatBuffer;
    #if defined(_DEBUG_STREAMIO)
        FILE *pFileDebug;
    #endif
} StreamData, FAR *LPStreamData;



typedef struct {
    LPStreamData lpStreamData;
    int nErrcode;
    int nOSErrcode;
    LPStreamBuffer in;
    LPStreamBuffer out;
} Stream, FAR *LPStream;



#if defined(MVS)
    #define OSseek(hFile, lPos) \
        (fseek(hFile, lPos, SEEK_SET)==0)

    #define OSread(hFile, lpBuffer, nCount) \
        fread(lpBuffer, 1, nCount, hFile)

    #define OSwrite(hFile, lpBuffer, nCount) \
        (fwrite(lpBuffer, 1, nCount, hFile)==nCount)

    #define OSclose(hFile) fclose(hFile)

    #define _lastError() (errno)
#else
    #define OSseek(hFile, lPos) \
        (_llseek(hFile, lPos, SEEK_SET)>=0)

    #define OSread(hFile, lpBuffer, nCount) \
        ((int)_lread(hFile, lpBuffer, nCount))

    #define OSwrite(hFile, lpBuffer, nCount) \
        (_lwrite(hFile, lpBuffer, nCount)!=(UINT)HFILE_ERROR)

    #define OSclose(hFile) _lclose(hFile)

    #if defined(_WIN32)
        #define _lastError() ((int)GetLastError())
    #else
        #define _lastError() (errno)
    #endif
#endif



#if defined(__cplusplus)
extern "C" {
#endif

void PASCAL streamioPrelude(void);

#if defined(__cplusplus)
}
#endif



#endif
