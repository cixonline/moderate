#ifndef _HORUS_HCTOOLS_H
#define _HORUS_HCTOOLS_H



//  hctools.h - Ameol addon utility routine library definitions
//              © 1993-1995 Pete Jordan, Horus Communications
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include <memory.h>
#include "compat32.h"
#include "amscript.h"
#include "amlists.h"


#ifndef WIN32
	#define InitObject(object, obSelect, obType) \
		 (_fmemset(object, 0, sizeof(obType)),    \
		  (object)->obHdr.type=obSelect,          \
		  (object)->obHdr.wSize=sizeof(obType))
#else
	#define InitObject(object, obSelect, obType) \
		 (memset(object, 0, sizeof(obType)),    \
		  (object)->obHdr.type=obSelect,          \
		  (object)->obHdr.wSize=sizeof(obType))
#endif


/* LPCSTR Cls_OnPopupHelp(HWND hwnd, int id); */
#define HANDLE_WM_POPUPHELP(hwnd, wParam, lParam, fn) \
    (LRESULT)(LPCSTR)(fn)((hwnd), (int)(wParam))
#define FORWARD_WM_POPUPHELP(hwnd, id, fn) \
    (LPCSTR)(fn)((hwnd), WM_POPUPHELP, (WPARAM)(id), 0L)

/* void Cls_OnAmHelp(HWND hwnd); */
#define HANDLE_WM_AMHELP(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_AMHELP(hwnd, fn) \
    (void)(fn)((hwnd), WM_AMHELP, 0, 0L)

/* void Cls_OnAdjustWindows(HWND hwnd, int dx, int dy); */
#define HANDLE_WM_ADJUSTWINDOWS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_ADJUSTWINDOWS(hwnd, fn, dx, dy) \
    (void)(fn)((hwnd), WM_ADJUSTWINDOWS, 0, MAKELPARAM((short)dx, (short)dy))



#if defined(__cplusplus)
extern "C" {
#endif

DWORD FAR PASCAL _EXPORT HCToolsVersion(void);
HCONF FAR PASCAL _EXPORT GetModConference(HCONF hConf);
#define AmWritePrivateProfileItem AMWRITEPRIVATEPROFILEITEM
BOOL FAR _EXPORT AmWritePrivateProfileItem(LPCSTR lpApplicationName, LPCSTR lpKeyName, LPCSTR lpFileName, LPCSTR lpFormat,...);
BOOL FAR PASCAL _EXPORT AmGetPrivateProfileBool(LPCSTR lpApplicationName, LPCSTR lpKeyName, BOOL bDefault, LPCSTR lpFileName);
void FAR PASCAL _EXPORT GetAmeolDir(LPSTR lpAmeolDir);

#if defined(__cplusplus)
}
#endif



#endif

