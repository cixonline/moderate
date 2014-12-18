#ifndef _HORUS_WINHORUS_H
#define _HORUS_WINHORUS_H



//  winhorus.h - Windows utility routine library definitions
//               © 1993-1996 Pete Jordan, Horus Communications
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include "windefs.h"
#include "compat32.h"
#include "whutils.h"
#include "gmalloc.h"
#include "streamio.h"
#if defined(MVS)
    #include "pdslist.h"
#endif



#if defined(__cplusplus)
extern "C" {
#endif

DWORD FAR PASCAL WinHorusVersion(void);
DWORD FAR PASCAL _EXPORT WinHorusInit(HINSTANCE hInstance);

#if defined(MVS)
    void WinHorusPrelude(void);
    void WinHorusPostlude(void);
#endif

#if defined(__cplusplus)
}
#endif



#endif
