#ifndef _HORUS_STREAMIO_H
#define _HORUS_STREAMIO_H



//  streamio.h - Buffered line-based IO for Windows
//               © 1993, 1994 Pete Jordan, Horus Communication
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include "compat32.h"



DECLARE_HANDLE32(HSTREAM);
typedef HSTREAM FAR *LPHSTREAM;



enum {
//  OF_READ                 =0x0000,
//  OF_WRITE                =0x0001,
//  OF_READWRITE            =0x0002,
    OF_APPENDFLAG           =0x0004,
    OF_READAPPEND           =0x0006,
    OF_APPEND               =0x0005,

//  OF_SHARE_COMPAT         =0x0000,
//  OF_SHARE_EXCLUSIVE	    =0x0010,
//  OF_SHARE_DENY_WRITE	    =0x0020,
//  OF_SHARE_DENY_READ	    =0x0030,
//  OF_SHARE_DENY_NONE	    =0x0040,

    OF_NOCONVERT            =0x0000,
    OF_LFCONVERT            =0x0100,
    OF_CRCONVERT            =0x0200,
    OF_CRLFCONVERT	    =0x0300,

    OF_CPM_EOF              =0x0400,

    OF_OPENMASK             =0x00f3,    //  mask off private bits for _lopen()
    OF_MODEMASK             =0x0003,    //  basic open mode bits
    OF_CONVERTMASK          =0x0300     //  output conversion mode bits
};



enum {
    STREAM_EOF              = -1,
    STREAM_OPENMODE         = -2,
    STREAM_OPEN             = -3,
    STREAM_REOPEN           = -4,
    STREAM_XSEEK             = -5,
    STREAM_WRITE            = -6,
    STREAM_READ             = -7,
    STREAM_TRUNCATE         = -8,
    STREAM_MEMORY           = -9,
    STREAM_CLOSED           = -10,
    STREAM_NOFILE           = -11,
    STREAM_OVERFLOW         = -12,
    STREAM_OPENCOUNT        = -13,
    STREAM_ARGERROR         = -14
};



#if defined(__cplusplus)
extern "C" {
#endif

UINT FAR PASCAL _EXPORT defaultBuffsize(UINT wNewBuffsize);
UINT FAR PASCAL _EXPORT defaultFormatBuffsize(UINT wNewBuffsize);
UINT FAR PASCAL _EXPORT maxOpenFiles(UINT wNewLimit);
#define openFile _OPENFILE
HSTREAM FAR PASCAL _EXPORT openFile(LPCSTR lpcFilename, WORD wFlags);
int FAR PASCAL _EXPORT lastStreamError(HSTREAM hStream);
LPSTR FAR PASCAL _EXPORT openFileName(HSTREAM hStream);
HFILE FAR PASCAL _EXPORT fileHandle(HSTREAM hStream);
int FAR PASCAL _EXPORT reopenFile(HSTREAM hStream);
int FAR PASCAL _EXPORT flushFile(HSTREAM hStream);
int FAR PASCAL _EXPORT suspendFile(HSTREAM hStream);
int FAR PASCAL _EXPORT closeFile(HSTREAM hStream);
int FAR PASCAL _EXPORT readRawChar(HSTREAM hStream);
int FAR PASCAL _EXPORT readChar(HSTREAM hStream);
int FAR PASCAL _EXPORT readLine(HSTREAM hStream, LPSTR lpLineBuff, UINT wMaxLength);
int FAR PASCAL _EXPORT readBlock(HSTREAM hStream, LPSTR lpBuffer, UINT wLength);
int FAR PASCAL _EXPORT writeRawChar(HSTREAM hStream, char ch);
int FAR PASCAL _EXPORT writeChar(HSTREAM hStream, char ch);
#define writeLine WRITELINE
int FAR _EXPORT writeLine(HSTREAM hStream, LPCSTR lpcLineFormat,...);
int FAR PASCAL _EXPORT writeBlock(HSTREAM hStream, LPCSTR lpBuffer, UINT wLength);
LPCSTR FAR PASCAL _EXPORT readTranslate(HSTREAM hStream, LPCSTR lpcNewTranslate);
LPCSTR FAR PASCAL _EXPORT writeTranslate(HSTREAM hStream, LPCSTR lpcNewTranslate);
int FAR PASCAL _EXPORT readBuffsize(HSTREAM hStream, UINT wNewBuffsize);
int FAR PASCAL _EXPORT writeBuffsize(HSTREAM hStream, UINT wNewBuffsize);
int FAR PASCAL _EXPORT formatBuffsize(HSTREAM hStream, UINT wNewBuffsize);
LONG FAR PASCAL _EXPORT readTell(HSTREAM hStream);
LONG FAR PASCAL _EXPORT writeTell(HSTREAM hStream);
LONG FAR PASCAL _EXPORT readSeek(HSTREAM hStream, LONG lNewPosition);
LONG FAR PASCAL _EXPORT writeSeek(HSTREAM hStream, LONG lNewPosition);
LONG FAR PASCAL _EXPORT truncateFile(HSTREAM hStream);
LONG FAR PASCAL _EXPORT fileSize(HSTREAM hStream);

#if defined(__cplusplus)
}
#endif



#endif
