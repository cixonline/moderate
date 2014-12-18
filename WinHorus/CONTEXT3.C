//  context32.c - Windows DLL client-private data maintenance routines
//                Cut down version for Win32. Most of the code is pretty much
//                superfluous, but it maintains a uniform interface with Win16.
//                © 1994-1996 Pete Jordan, Horus Communication
//                Please see the accompanying file "copyleft.txt" for details
//                of your licence to use and distribute this program.



#include "windefs.h"
#include <string.h>
#include "context.h"
#include "debugdef.h"



enum {
    MAX_CONTEXTS=8
};



typedef struct {
    WORD wDataSize;
    LPVOID lpDefaultData;
    LPFNCONTEXT lpfnPrelude;
    LPFNCONTEXT lpfnPostlude;
    BOOL bInitialised;
    LPVOID lpData;
} ContextData, FAR *LPContextData;



static ContextData contextData[MAX_CONTEXTS+1];
static UINT uContexts=0;



void PASCAL contextPrelude(HINSTANCE hInst)
{
    UINT uContextPtr;

    for (uContextPtr=0; uContextPtr<=MAX_CONTEXTS; uContextPtr++) {
        contextData[uContextPtr].wDataSize=0;
        contextData[uContextPtr].lpDefaultData=NULL;
        contextData[uContextPtr].lpfnPrelude=NULL;
        contextData[uContextPtr].lpfnPostlude=NULL;
        contextData[uContextPtr].bInitialised=FALSE;
        contextData[uContextPtr].lpData=NULL;
    }
}



void PASCAL contextPostlude(void)
{
    UINT uContextPtr;

    for (uContextPtr=uContexts; uContextPtr>0; uContextPtr--) {
        if (contextData[uContextPtr].lpfnPostlude &&
            contextData[uContextPtr].bInitialised)
            contextData[uContextPtr].lpfnPostlude(contextData[uContextPtr].lpData);

        contextData[uContextPtr].bInitialised=FALSE;
        contextData[uContextPtr].lpData=NULL;
    }

    contextData->bInitialised=FALSE;
    #if defined(MVS)
        free(contextData->lpData);
    #else
        GlobalFreePtr(contextData->lpData);
    #endif
    contextData->lpData=NULL;
}



WHCONTEXT PASCAL registerContext(LPVOID lpDefaultData, WORD wDataSize,
                LPFNCONTEXT lpfnPrelude, LPFNCONTEXT lpfnPostlude)
{
    if (uContexts>MAX_CONTEXTS || !lpDefaultData || !wDataSize)
        return(NULL);

    contextData->wDataSize+=wDataSize;
    uContexts++;
    contextData[uContexts].wDataSize=wDataSize;
    contextData[uContexts].lpDefaultData=lpDefaultData;
    contextData[uContexts].lpfnPrelude=lpfnPrelude;
    contextData[uContexts].lpfnPostlude=lpfnPostlude;
    return((WHCONTEXT)uContexts);
}



LPVOID PASCAL getContext(WHCONTEXT hContext)
{
    LPContextData lpContextData=NULL;
    UINT uContext=(UINT)hContext;

    if (uContext<1 || uContext>uContexts) {
        _ALERT(1, "Context out of range");
        return(NULL);
    }

    lpContextData=contextData+uContext;

    if (!contextData->lpData) {
        LPSTR lpOffset;
        UINT uContextPtr;

        #if defined(MVS)
            contextData->lpData=(LPVOID)malloc(contextData->wDataSize);
        #else
            contextData->lpData=(LPVOID)GlobalAllocPtr(GMEM_MOVEABLE, contextData->wDataSize);
        #endif

        if (!contextData->lpData) {
            _ALERT(1, "GlobalAllocPtr() failed");
            return(NULL);
        }

        lpOffset=(LPSTR)contextData->lpData;

        for (uContextPtr=1; uContextPtr<=uContexts; uContextPtr++) {
            contextData[uContextPtr].lpData=(LPVOID)lpOffset;
            contextData[uContextPtr].bInitialised=FALSE;
            lpOffset+=contextData[uContextPtr].wDataSize;
        }
    }

    if (!lpContextData->bInitialised) {
#ifdef WIN32    
        memcpy(lpContextData->lpData, lpContextData->lpDefaultData,
               lpContextData->wDataSize);
#else
        _fmemcpy(lpContextData->lpData, lpContextData->lpDefaultData,
               lpContextData->wDataSize);
#endif
        if (lpContextData->lpfnPrelude)
            lpContextData->lpfnPrelude(lpContextData->lpData);

        lpContextData->bInitialised=TRUE;
    }

    return(lpContextData->lpData);
}
