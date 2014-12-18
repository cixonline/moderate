#ifndef _HORUS_CONTEXT_H
#define _HORUS_CONTEXT_H



//  context.h - Windows DLL client-private data maintenance definitions
//              © 1994-1996 Pete Jordan, Horus Communications
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



DECLARE_HANDLE(WHCONTEXT);



typedef void (CALLBACK *LPFNCONTEXT)(LPVOID);



void PASCAL contextPrelude(HINSTANCE hInst);
void PASCAL contextPostlude(void);
WHCONTEXT PASCAL registerContext(LPVOID lpDefaultData, WORD wDataSize,
                                LPFNCONTEXT lpfnPrelude, LPFNCONTEXT lpfnPostlude);
LPVOID PASCAL getContext(WHCONTEXT hContext);



#endif

