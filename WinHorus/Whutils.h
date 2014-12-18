#ifndef _HORUS_WHUTILS_H
#define _HORUS_WHUTILS_H



//  whutils.h - WinHorus utility routine library definitions
//              © 1993-1996 Pete Jordan, Horus Communications
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include "windefs.h"
#include "compat32.h"



typedef struct {
    int nVersion;
    int nRelease;
    int nRevision;
    int nBeta;
    BOOL bEval;
} VersionInfo, FAR *LPVersionInfo;



enum {
    OS_STATUS               =0x0100,
    GMALLOC_STATUS          =0x0200,
    STREAMIO_STATUS         =0x0300,
    #if defined(MVS)
        PDSLIST_STATUS      =0x0400,
        SVC99_STATUS        =0x0500,
    #endif

    SOURCE_MASK             =0x7f00,
    STATUS_MASK             =0x00ff
};



typedef struct {
    unsigned uSource;
    int nStatus;
} StatusItem, *PStatusItem, FAR *LPStatusItem;



typedef struct {
    int nCount;
    int nSet;
    int nErrcode;
    StatusItem statusItem[1];
} Status, *PStatus, FAR *LPStatus;



typedef void (FAR PASCAL *AlertDisplay)(LPCSTR lpcszMessage);



#define xor(a, b) (!(a)!= !(b))



#if !defined(MVS)
    #define GetDlgData(hDlg, DataType) \
        (DataType FAR *)GetWindowLong((hDlg), DWL_USER)

    #define SetDlgData(hDlg, lpData) \
        SetWindowLong((hDlg), DWL_USER, (DWORD)(LPVOID)(lpData))

    #define InitDlgData(hDlg, DataType) \
        (SetDlgData((hDlg), gmallocz(sizeof(DataType))), \
         GetDlgData((hDlg), DataType))

    #define FreeDlgData(hDlg) \
        gfree((LPVOID)GetWindowLong((hDlg), DWL_USER))



    #define NextDlgCtl(hwnd, id) \
        FORWARD_WM_NEXTDLGCTL((hwnd), GetDlgItem((hwnd), (id)), TRUE, PostMessage)

    #define GetWindowProc(hwnd) ((WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC))
#endif



#if defined(__cplusplus)
extern "C" {
#endif

LPCSTR FAR PASCAL _EXPORT ebcdic2ascii(void);
LPCSTR FAR PASCAL _EXPORT ascii2ebcdic(void);
AlertDisplay FAR PASCAL _EXPORT alertDisplay(AlertDisplay newDisplay);
UINT FAR PASCAL _EXPORT alertLevel(UINT newLevel);
void FAR PASCAL _EXPORT versionDecode(DWORD dwVersionCode, LPVersionInfo lpVersionInfo);
BOOL FAR PASCAL _EXPORT versionCheck(DWORD dwVersionCheck, DWORD dwVersionMinimum);
#if defined(MVS)
    int getCobolString(PSTR pszDest, PSTR psSource, int nLength);
    int putCobolString(PSTR psDest, PSTR pszSource, int nLength);
#else
    int FAR PASCAL _EXPORT RadioButton(HWND hDlg, int nGroupID, int nButtonID);
    #define WritePrivateProfileItem WRITEPRIVATEPROFILEITEM
    BOOL FAR _EXPORT WritePrivateProfileItem(LPCSTR lpApplicationName, LPCSTR lpKeyName,
                                             LPCSTR lpFileName, LPCSTR lpFormat,...);
    BOOL FAR PASCAL GetPrivateProfileBool(LPCSTR lpApplicationName, LPCSTR lpKeyName, BOOL bDefault, LPCSTR lpFileName);
#endif
int FAR PASCAL _EXPORT lstrncmp(LPCSTR lpcS1, LPCSTR lpcS2, UINT uMax);
int FAR PASCAL _EXPORT lstrncmpi(LPCSTR lpcS1, LPCSTR lpcS2, UINT uMax);
LPSTR FAR PASCAL _EXPORT lstrcpynz(LPSTR lpDest, LPCSTR lpcSrc, int nMax);
LPSTR FAR PASCAL _EXPORT strupper(LPSTR lpDest, LPCSTR lpcSrc);
LPSTR FAR PASCAL _EXPORT strlower(LPSTR lpDest, LPCSTR lpcSrc);
LPSTR FAR PASCAL _EXPORT strmixed(LPSTR lpDest, LPCSTR lpcSrc);
void FAR PASCAL _EXPORT clearStatus(LPStatus lpStatus);
LPStatus FAR PASCAL _EXPORT initStatus(int nCount);
BOOL FAR PASCAL _EXPORT setStatus(LPStatus lpStatus, unsigned uSource, int nStatus);
BOOL FAR PASCAL _EXPORT setError(LPStatus lpStatus, int nErrcode);
BOOL FAR PASCAL _EXPORT decodeStatus(LPStatus lpStatus, int nItem, LPSTR lpszBuffer, int nMax);

#if defined(__cplusplus)
}
#endif



#endif
