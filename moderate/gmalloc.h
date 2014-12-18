#ifndef _HORUS_GMALLOC_H
#define _HORUS_GMALLOC_H



//  gmalloc.h - Windows global memory allocation routine library definitions
//              © 1994 Pete Jordan, Horus Communication
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include "compat32.h"


#ifndef HPVOID
typedef void HUGE *HPVOID;
#endif


#if defined(__cplusplus)
extern "C" {
#endif

void FAR PASCAL _EXPORT hmemset(HPVOID hpDestination, int ch, LONG lCount);
WORD FAR PASCAL _EXPORT gallocsize(WORD wNewBucketSize);
WORD FAR PASCAL _EXPORT gthreshold(WORD wNewThresholdSize);
WORD FAR PASCAL _EXPORT greallocsize(WORD wNewReallocSize);
WORD FAR PASCAL _EXPORT gguardsize(WORD wNewGuardSize);
BOOL PASCAL FAR _EXPORT gfree(HPVOID lpBlock);
LPVOID PASCAL FAR _EXPORT gmalloc(LONG lSize);
LPVOID PASCAL FAR _EXPORT gmallocz(LONG lSize);
LPVOID PASCAL FAR _EXPORT gcalloc(WORD wUnits, WORD wSize);
LPVOID PASCAL FAR _EXPORT gcallocz(WORD wUnits, WORD wSize);
LPVOID PASCAL FAR _EXPORT grealloc(HPVOID hpOldBlock, LONG lNewSize);
LPVOID PASCAL FAR _EXPORT greallocz(HPVOID hpOldBlock, LONG lNewSize);

#if defined(__cplusplus)
}
#endif



#endif
