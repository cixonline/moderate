#ifndef _HORUS_GMALLOC_H
#define _HORUS_GMALLOC_H



//  gmalloc.h - Windows global memory allocation routine library definitions
//              © 1994-1996 Pete Jordan, Horus Communication
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include "compat32.h"



typedef void HUGE *HPVOID;



enum {
    GMALLOC_OK              = 0,
    GMALLOC_OSALLOC         = -100,
    GMALLOC_NULL            = -101,
    GMALLOC_UNDERFLOW       = -102,
    GMALLOC_OVERFLOW        = -103,
    GMALLOC_INVALID         = -104
};



typedef void (FAR PASCAL *GmallocCallback)(LPCSTR lpcszTag, int nCode, HPVOID hpBlock, LONG lSize);



#if defined(__cplusplus)
extern "C" {
#endif

void FAR PASCAL _EXPORT hmemset(HPVOID hpDestination, int ch, LONG lCount);
int FAR PASCAL _EXPORT gstatus(void);
LPCSTR FAR PASCAL _EXPORT gstrerror(int nStatus);
WORD FAR PASCAL _EXPORT gallocsize(WORD wNewBucketSize);
WORD FAR PASCAL _EXPORT gthreshold(WORD wNewThresholdSize);
WORD FAR PASCAL _EXPORT greallocsize(WORD wNewReallocSize);
WORD FAR PASCAL _EXPORT gguardsize(WORD wNewGuardSize);
BOOL PASCAL FAR _EXPORT gfree(HPVOID hpBlock);
LPVOID PASCAL FAR _EXPORT gmalloc(LONG lSize);
LPVOID PASCAL FAR _EXPORT gmallocz(LONG lSize);
LPVOID PASCAL FAR _EXPORT gcalloc(WORD wUnits, WORD wSize);
LPVOID PASCAL FAR _EXPORT gcallocz(WORD wUnits, WORD wSize);
LPVOID PASCAL FAR _EXPORT grealloc(HPVOID hpOldBlock, LONG lNewSize);
LPVOID PASCAL FAR _EXPORT greallocz(HPVOID hpOldBlock, LONG lNewSize);
void FAR PASCAL _EXPORT gcallback(GmallocCallback gmallocCallback);
int FAR PASCAL _EXPORT gcheckblock(LPCSTR lpcszTag, HPVOID hpBlock);
BOOL FAR PASCAL _EXPORT gcheckall(LPCSTR lpcszTag, GmallocCallback gmallocCallback);

#if defined(__cplusplus)
}
#endif



#endif
