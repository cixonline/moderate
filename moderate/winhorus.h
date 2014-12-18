#ifndef _HORUS_WINHORUS_H
#define _HORUS_WINHORUS_H



//  winhorus.h - Windows utility routine library definitions
//               © 1993-1995 Pete Jordan, Horus Communications
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include "compat32.h"
#include "gmalloc.h"
#include "streamio.h"



typedef struct {
    int nVersion, nRelease, nRevision, nBeta;
    BOOL bEval;
} VersionInfo, FAR *LPVersionInfo;



#define xor(a, b) (!(a)!= !(b))



#define GetDlgData(hDlg, DataType) \
    (DataType FAR *)GetWindowLong((hDlg), DWL_USER)

#define SetDlgData(hDlg, lpData) \
    SetWindowLong((hDlg), DWL_USER, (DWORD)(LPVOID)(lpData))

#define InitDlgData(hDlg, DataType) \
    (SetDlgData((hDlg), gmallocz(sizeof(DataType))), \
     GetDlgData((hDlg), DataType))

#define FreeDlgData(hDlg) \
    gfree((LPVOID)GetWindowLong((hDlg), DWL_USER))

#define MDI_USER	0

#define GetMDIData(hDlg, DataType) \
    (DataType FAR *)GetWindowLong((hDlg), GWL_USERDATA)

#define SetMDIData(hDlg, lpData) \
    SetWindowLong((hDlg), GWL_USERDATA, (DWORD)(LPVOID)(lpData))

#define InitMDIData(hDlg, DataType) \
    (SetDlgData((hDlg), gmallocz(sizeof(DataType))), \
     GetDlgData((hDlg), DataType))

#define FreeMDIData(hDlg) \
    gfree((LPVOID)GetWindowLong((hDlg), GWL_USERDATA))


#define NextDlgCtl(hwnd, id) \
    FORWARD_WM_NEXTDLGCTL((hwnd), GetDlgItem((hwnd), (id)), TRUE, PostMessage)

#define GetWindowProc(hwnd) ((WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC))



#if defined(__cplusplus)
extern "C" {
#endif

DWORD FAR PASCAL _EXPORT WinHorusInit(HINSTANCE hInstance);
DWORD FAR PASCAL _EXPORT WinHorusVersion(void);
void FAR PASCAL _EXPORT versionDecode(DWORD dwVersionCode, LPVersionInfo lpVersionInfo);
BOOL FAR PASCAL _EXPORT versionCheck(DWORD dwVersionCheck, DWORD dwVersionMinimum);
int FAR PASCAL _EXPORT RadioButton(HWND hDlg, int nGroupID, int nButtonID);
#define WritePrivateProfileItem WRITEPRIVATEPROFILEITEM
BOOL FAR _EXPORT WritePrivateProfileItem(LPCSTR lpApplicationName, LPCSTR lpKeyName, LPCSTR lpFileName, LPCSTR lpFormat,...);
BOOL FAR PASCAL GetPrivateProfileBool(LPCSTR lpApplicationName, LPCSTR lpKeyName, BOOL bDefault, LPCSTR lpFileName);
LPSTR FAR PASCAL _EXPORT lstrcpynz(LPSTR lpDest, LPCSTR lpcSrc, int nMax);

#if defined(__cplusplus)
}
#endif



#endif
