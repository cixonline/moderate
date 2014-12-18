//  context.h - Windows DLL client-private data maintenance routines
//              © 1994 Pete Jordan, Horus Communication
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include <windows.h>
#include <windowsx.h>
#include <toolhelp.h>
#include <string.h>
#include "context.h"



enum {
    MAX_CLIENTS=128,
    MAX_CONTEXTS=8
};



typedef struct {
    WORD wDataSize;
    LPVOID lpDefaultData;
    LPFNCONTEXT lpfnPrelude;
    LPFNCONTEXT lpfnPostlude;
} ContextData;



typedef struct {
    BOOL bInitialised;
    LPVOID lpData;
} ClientContextData;



typedef struct {
    HTASK hTask;
    LPFNNOTIFYCALLBACK lpNotifyProc;
    ClientContextData lpContexts[MAX_CONTEXTS+1];
} ClientData, FAR *LPClientData;



static ClientData clientData[MAX_CLIENTS];
static ContextData contextData[MAX_CONTEXTS+1];
static UINT uContexts=0;
static HINSTANCE hInstance=NULL;



static LPClientData PASCAL findClient(HTASK hTask)
{
    UINT uClientPtr;

    for (uClientPtr=0;
         uClientPtr<MAX_CLIENTS && clientData[uClientPtr].hTask!=hTask;
         uClientPtr++);

    return(uClientPtr<MAX_CLIENTS ? (LPClientData)clientData+uClientPtr : NULL);
}



static LPClientData PASCAL currentClient(void)
{
    return(findClient(GetCurrentTask()));
}



static void PASCAL freeClient(LPClientData lpClientData)
{
    UINT uContext;

    for (uContext=uContexts; uContext>0; uContext--) {
        if (contextData[uContext].lpfnPostlude &&
            lpClientData->lpContexts[uContext].bInitialised)
            contextData[uContext].lpfnPostlude(lpClientData->lpContexts[uContext].lpData);

        lpClientData->lpContexts[uContext].bInitialised=FALSE;
        lpClientData->lpContexts[uContext].lpData=NULL;
    }

    lpClientData->lpContexts->bInitialised=FALSE;
    GlobalFreePtr(lpClientData->lpContexts->lpData);
    lpClientData->lpContexts->lpData=NULL;
#ifdef WINDLL
    NotifyUnRegister(lpClientData->hTask);
#endif
    lpClientData->hTask=NULL;
}



BOOL FAR PASCAL _export trackClient(WORD wID, DWORD dwData)
{
    if (wID==NFY_EXITTASK) {
        LPClientData lpClientData=currentClient();

        if (lpClientData)
            freeClient(lpClientData);

    }

    return(FALSE);
}



static LPClientData PASCAL newClient(void)
{
    LPClientData lpClientData=NULL;
    UINT uClientPtr;

    for (uClientPtr=0;
         uClientPtr<MAX_CLIENTS && clientData[uClientPtr].hTask;
         uClientPtr++);

    if (uClientPtr<MAX_CLIENTS) {
        UINT uContextPtr;
        LPSTR lpOffset;

        lpClientData=(LPClientData)clientData+uClientPtr;
        lpClientData->hTask=GetCurrentTask();

        if (!lpClientData->lpNotifyProc)
            lpClientData->lpNotifyProc=(LPFNNOTIFYCALLBACK)MakeProcInstance((FARPROC)trackClient, hInstance);
#ifdef WINDLL
        NotifyRegister(lpClientData->hTask, lpClientData->lpNotifyProc, NF_NORMAL);
#endif
        lpClientData->lpContexts->lpData=(LPVOID)GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, contextData->wDataSize);
        lpOffset=(LPSTR)lpClientData->lpContexts->lpData;

        for (uContextPtr=1; uContextPtr<=uContexts; uContextPtr++) {
            lpClientData->lpContexts[uContextPtr].lpData=(LPVOID)lpOffset;
            lpClientData->lpContexts[uContextPtr].bInitialised=FALSE;
            lpOffset+=contextData[uContextPtr].wDataSize;
        }

        lpClientData->lpContexts->bInitialised=TRUE;
    }

    return(lpClientData);
}



static void PASCAL newClientContext(LPClientData lpClientData, UINT uContext)
{
    _fmemcpy(lpClientData->lpContexts[uContext].lpData,
             contextData[uContext].lpDefaultData,
             contextData[uContext].wDataSize);

    if (contextData[uContext].lpfnPrelude)
        contextData[uContext].lpfnPrelude(lpClientData->lpContexts[uContext].lpData);

    lpClientData->lpContexts[uContext].bInitialised=TRUE;
}



void PASCAL contextPrelude(HINSTANCE hInst)
{
    UINT uClientPtr;
    UINT uContextPtr;

    hInstance=hInst;

    for (uClientPtr=0; uClientPtr<MAX_CLIENTS; uClientPtr++) {
        LPClientData lpClientData=(LPClientData)clientData+uClientPtr;

        lpClientData->hTask=NULL;
        lpClientData->lpNotifyProc=NULL;

        for (uContextPtr=0; uContextPtr<=MAX_CONTEXTS; uContextPtr++) {
            lpClientData->lpContexts[uContextPtr].bInitialised=FALSE;
            lpClientData->lpContexts[uContextPtr].lpData=NULL;
        }
    }

    for (uContextPtr=0; uContextPtr<=MAX_CONTEXTS; uContextPtr++) {
        contextData[uContextPtr].wDataSize=0;
        contextData[uContextPtr].lpDefaultData=NULL;
        contextData[uContextPtr].lpfnPrelude=NULL;
        contextData[uContextPtr].lpfnPostlude=NULL;
    }
}



void PASCAL contextPostlude(void)
{
    UINT uClientPtr;

    for (uClientPtr=0; uClientPtr<MAX_CLIENTS; uClientPtr++) {
        LPClientData lpClientData=(LPClientData)clientData+uClientPtr;

        if (lpClientData->hTask)
            freeClient(lpClientData);

        if (lpClientData->lpNotifyProc) {
            FreeProcInstance((FARPROC)lpClientData->lpNotifyProc);
            lpClientData->lpNotifyProc=NULL;
        }
    }
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
    LPClientData lpClientData=currentClient();
    UINT uContext=(UINT)hContext;

    if (uContext<1 || uContext>uContexts)
        return(NULL);

    if (!lpClientData) {
        lpClientData=newClient();

        if (!lpClientData)
            return(NULL);

    }

    if (!lpClientData->lpContexts[uContext].bInitialised)
        newClientContext(lpClientData, uContext);

    return(lpClientData->lpContexts[uContext].lpData);
}
