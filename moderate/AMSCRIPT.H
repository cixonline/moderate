#ifndef _HORUS_HCTOOLS_AMSCRIPT_H
#define _HORUS_HCTOOLS_AMSCRIPT_H



//  amscript.h - Ameol script creation utility routine definitions
//               © 1993-1995 Pete Jordan, Horus Communications
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include "ameolapi.h"
#include "..\winhorus\compat32.h"



DECLARE_HANDLE32(HSCRIPT);



enum {
    ERR_SCRIPT_BASE=2000,

    ERR_SCRIPT_MEMORY,
    ERR_SCRIPT_NAME_OVERFLOW,
    ERR_SCRIPT_CREATE,
    ERR_SCRIPT_NOT_OPEN,
    ERR_SCRIPT_REOPEN,
    ERR_SCRIPT_WRITE,
    ERR_SCRIPT_SUSPEND,
    ERR_SCRIPT_ACTION,
    ERR_SCRIPT_INVALID_ARGUMENT
};



#if defined(__cplusplus)
extern "C" {
#endif

HSCRIPT FAR PASCAL _EXPORT initScript(LPCSTR lpcPrefix, BOOL bSuspend);
LPCSTR FAR PASCAL _EXPORT scriptFile(HSCRIPT hScript, WORD wSelect);
#define addToScript ADDTOSCRIPT
int FAR _EXPORT addToScript(HSCRIPT hScript, LPCSTR lpcFormat,...);
#define actionScript ACTIONSCRIPT
HOOB FAR _EXPORT actionScript(HSCRIPT hScript, BYTE byScriptType, LPCSTR lpcFormat,...);
int FAR PASCAL _EXPORT abandonScript(HSCRIPT hScript);
int FAR PASCAL _EXPORT lastScriptError(HSCRIPT hScript);

#if defined(__cplusplus)
}
#endif



#endif

