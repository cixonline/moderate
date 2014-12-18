#ifndef _HORUS_STREAMIO_H
#define _HORUS_STREAMIO_H



//  streamio.h - Buffered line-based IO for Windows and MVS
//               © 1993-1996 Pete Jordan, Horus Communication
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



DECLARE_HANDLE32(HSTREAM);
typedef HSTREAM *PHSTREAM;
typedef HSTREAM FAR *LPHSTREAM;



enum {
    #if defined(MVS)
        OF_READ                 =0x0000,
        OF_WRITE                =0x0001,
        OF_READWRITE            =0x0002,
    #endif

    OF_APPENDFLAG               =0x0004,
    OF_READAPPEND               =0x0006,
    OF_APPEND                   =0x0005,

    #if defined(MVS)
        OF_FIXED                =0x0010,
        OF_VARIABLE             =0x0020,
        OF_UNFORMATTED          =0x0030,
        OF_BLOCKED              =0x0040,
        OF_STANDARD             =0x0080,
        OF_SPANNED              =0x0080,

        OF_FBS                  =0x00d0,
    #else
    //  OF_SHARE_COMPAT         =0x0000,
    //  OF_SHARE_EXCLUSIVE      =0x0010,
    //  OF_SHARE_DENY_WRITE     =0x0020,
    //  OF_SHARE_DENY_READ      =0x0030,
    //  OF_SHARE_DENY_NONE      =0x0040,
    #endif

    OF_NOCONVERT                =0x0000,
    OF_LFCONVERT                =0x0100,
    OF_CRCONVERT                =0x0200,
    OF_CRLFCONVERT              =0x0300,

    #if defined(MVS)
        OF_RECORD_MODE          =0x0400,
        OF_NO_STRIP             =0x0800,
        OF_RECORD_FIXED         =0x0c00,

        OF_ASA_PRINT            =0x1000,
        OF_MACHINE_PRINT        =0x2000,
        OF_MEMORY               =0x4000,
        OF_HIPERSPACE           =0x8000,

        OF_BASEMASK             =0x0030,
        OF_EXTMASK              =0x30c0,
        OF_RECFMASK             =0x30f0,
    #else
        OF_CPM_EOF              =0x0400,
    #endif

    OF_OPENMASK                 =0x00f3,    //  mask off private bits for _lopen()
    OF_MODEMASK                 =0x0003,    //  basic open mode bits
    OF_CONVERTMASK              =0x0300     //  output conversion mode bits
};



#if defined(MVS)
    enum {
        OF_BLOCKS               =0x0001,
        OF_TRACKS               =0x0002,
        OF_CYLINDERS            =0x0003
    };



    DECLARE_HANDLE32(HOPENDATA);
#endif



enum {
    STREAMIO_EOF                = -1,
    STREAMIO_OPENMODE           = -2,
    STREAMIO_OPEN               = -3,
    STREAMIO_REOPEN             = -4,
    STREAMIO_SEEK               = -5,
    STREAMIO_WRITE              = -6,
    STREAMIO_READ               = -7,
    STREAMIO_TRUNCATE           = -8,
    STREAMIO_MEMORY             = -9,
    STREAMIO_CLOSED             = -10,
    STREAMIO_NOFILE             = -11,
    STREAMIO_OVERFLOW           = -12,
    STREAMIO_OPENCOUNT          = -13,
    STREAMIO_ARGERROR           = -14,
    STREAMIO_OSFILEERROR        = -15,

    STREAMIO_READCOUNT          = STREAMIO_STATUS|0x01,
    STREAMIO_BUFFCOUNT          = STREAMIO_STATUS|0x02,
    STREAMIO_BUFFSIZE           = STREAMIO_STATUS|0x03
};



#if defined(__cplusplus)
extern "C" {
#endif

UINT FAR PASCAL _EXPORT defaultBuffcount(UINT uNewBuffcount);
UINT FAR PASCAL _EXPORT defaultBuffsize(UINT uNewBuffsize);
UINT FAR PASCAL _EXPORT defaultFormatBuffsize(UINT uNewBuffsize);
int FAR PASCAL _EXPORT maxOpenFiles(UINT uNewLimit);
#if defined(MVS)
    HSTREAM FAR PASCAL _EXPORT openFile(LPCSTR lpcFilename, WORD wFlags, UINT uRecLength,
                                        HOPENDATA hOpenData);
    HOPENDATA FAR PASCAL _EXPORT createData(WORD wAllocType, UINT uBlockSize,
                                            UINT uPrimaryAlloc, UINT uSecondaryAlloc, UINT uDirectoryAlloc);
    HOPENDATA FAR PASCAL _EXPORT createLike(HSTREAM hStream);
#else
    #define openFile _OPENFILE
    HSTREAM FAR PASCAL _EXPORT openFile(LPCSTR lpcFilename, WORD wFlags);
#endif
LPCSTR FAR PASCAL _EXPORT streamioErrtext(int nErrcode);
int FAR PASCAL _EXPORT lastStreamError(HSTREAM hStream);
int FAR PASCAL _EXPORT lastOSError(HSTREAM hStream);
LPSTR FAR PASCAL _EXPORT openFileName(HSTREAM hStream);
HFILE FAR PASCAL _EXPORT fileHandle(HSTREAM hStream);
HSTREAM FAR PASCAL _EXPORT dupFile(HSTREAM hStream);
int FAR PASCAL _EXPORT flushFile(HSTREAM hStream);
int FAR PASCAL _EXPORT reopenFile(HSTREAM hStream);
int FAR PASCAL _EXPORT suspendFile(HSTREAM hStream);
int FAR PASCAL _EXPORT closeFile(HSTREAM hStream);
int FAR PASCAL _EXPORT readRawChar(HSTREAM hStream);
int FAR PASCAL _EXPORT readChar(HSTREAM hStream);
int FAR PASCAL _EXPORT readLine(HSTREAM hStream, LPSTR lpLineBuff, UINT uMaxLength);
int FAR PASCAL _EXPORT readBlock(HSTREAM hStream, LPVOID lpBuffer, UINT uLength);
LONG FAR PASCAL _EXPORT readTell(HSTREAM hStream);
LONG FAR PASCAL _EXPORT readSeek(HSTREAM hStream, LONG lNewPosition);
int FAR PASCAL _EXPORT readRandom(HSTREAM hStream, LONG lPosition, LPVOID lpBuffer, UINT uLength);
int FAR PASCAL _EXPORT writeRawChar(HSTREAM hStream, char ch);
int FAR PASCAL _EXPORT writeChar(HSTREAM hStream, char ch);
#define writeLine WRITELINE
int FAR _EXPORT writeLine(HSTREAM hStream, LPCSTR lpcLineFormat,...);
int FAR PASCAL _EXPORT writeBlock(HSTREAM hStream, LPVOID lpBuffer, UINT uLength);
LONG FAR PASCAL _EXPORT writeTell(HSTREAM hStream);
LONG FAR PASCAL _EXPORT writeSeek(HSTREAM hStream, LONG lNewPosition);
int FAR PASCAL _EXPORT writeRandom(HSTREAM hStream, LONG lPosition, LPVOID lpBuffer, UINT uLength);
LPCSTR FAR PASCAL _EXPORT readTranslate(HSTREAM hStream, LPCSTR lpcNewTranslate);
LPCSTR FAR PASCAL _EXPORT writeTranslate(HSTREAM hStream, LPCSTR lpcNewTranslate);
int FAR PASCAL _EXPORT fileBuffcount(HSTREAM hStream, UINT uNewBuffcount);
int FAR PASCAL _EXPORT fileBuffsize(HSTREAM hStream, UINT uNewBuffsize);
int FAR PASCAL _EXPORT formatBuffsize(HSTREAM hStream, UINT uNewBuffsize);
#if !defined(MVS)
    LONG FAR PASCAL _EXPORT truncateFile(HSTREAM hStream);
#endif
LONG FAR PASCAL _EXPORT fileSize(HSTREAM hStream);

#if !defined(MVS)
    //  Obsolete functions...
    int FAR PASCAL _EXPORT readBuffsize(HSTREAM hStream, UINT uNewBuffsize);
    int FAR PASCAL _EXPORT writeBuffsize(HSTREAM hStream, UINT uNewBuffsize);
#endif

#if defined(__cplusplus)
}
#endif



#endif
