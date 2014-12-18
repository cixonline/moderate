//  winhorus.c - Windows utility routine library
//               © 1993-1997 Pete Jordan, Horus Communication
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include <stdarg.h>
#include "winhorus.h"
#include "context.h"
#if defined(MVS)
    #include "@gmalloc.h"
    #include "@stream.h"
#else
    #include "_gmalloc.h"
    #include "_stream.h"
#endif
#include "debugdef.h"



#define WINHORUS_VERSION 0x00030067L



#if defined(MVS)
    #pragma linkage(hinit,    COBOL)
    #pragma linkage(hexit,    COBOL)
    #pragma linkage(hversion, COBOL)



    static BOOL bInitialised=FALSE;



    void WinHorusPrelude(void)
    {
        if (!bInitialised) {
            contextPrelude(NULL);
            gmallocPrelude();
            streamioPrelude();
            bInitialised=TRUE;
            atexit(WinHorusPostlude);
        }
    }



    void WinHorusPostlude(void)
    {
        if (bInitialised) {
            bInitialised=FALSE;
            contextPostlude();
        }
    }



    void hinit(void)
    {
        WinHorusPrelude();
    }



    void hexit(void)
    {
        WinHorusPostlude();
    }



    void hversion(LPINT lpnVersion)
    {
        *lpnVersion=WINHORUS_VERSION;
    }

#elif defined(_WIN32)
#ifdef WINDLL
    #if defined(__BORLANDC__)
        BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
    #else
        BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
    #endif
    {
        switch (fdwReason) {

        case DLL_PROCESS_ATTACH:
            contextPrelude(hInstance);
            gmallocPrelude();
            streamioPrelude();
            break;

        case DLL_PROCESS_DETACH:
            contextPostlude();
            break;

        }

        return(TRUE);
    }
#endif
#else
#ifdef WINDLL
    int FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine)
    {
        contextPrelude(hInstance);
        gmallocPrelude();
        streamioPrelude();
        return(1);
    }


    int FAR PASCAL WEP(int nParam)
    {
        contextPostlude();
        return(1);
    }
#endif
#endif

DWORD FAR PASCAL _EXPORT WinHorusInit(HINSTANCE hInstance)
{
    contextPrelude(hInstance);
    gmallocPrelude();
    streamioPrelude();
	return (DWORD)(0);
}


DWORD FAR PASCAL _EXPORT WinHorusVersion(void)
{
    return(WINHORUS_VERSION);
}
