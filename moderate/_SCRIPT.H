#ifndef _HORUS_HCTOOLS__SCRIPT_H
#define _HORUS_HCTOOLS__SCRIPT_H



//  _script.h - Ameol script creation utility routine internal definitions
//              © 1993, 1994 Pete Jordan, Horus Communication
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include "..\winhorus\winhorus.h"



typedef struct {
    HSTREAM scriptStream;
    char scriptPrefix[9];
    int scriptIndex;
    BOOL scriptSuspend;
    BOOL scriptEmpty;
    int scriptErrcode;
} Script, FAR *LPScript;



#endif

