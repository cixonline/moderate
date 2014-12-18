#ifndef _WINHORUS_TABCNTRL_H
#define _WINHORUS_TABCNTRL_H



#if !defined(_WIN32)
typedef struct {
    long style;
    BYTE cdit;
    int x;
    int y;
    int cx;
    int cy;
//  char dtMenuName[];      //  Variable-length string.
//  char dtClassName[];     //  Variable-length string.
//  char dtCaptionText[];   //  Variable-length string.
} DLGTEMPLATE, FAR *LPDLGTEMPLATE;
#endif



typedef struct {
    LPSTR lpszTag;
    LPSTR lpszTemplate;
    WNDPROC fpDlgProc;
} TabDescription, FAR *LPTabDescription;



typedef struct {
    LPDLGTEMPLATE lpDlgTemplate;
    WNDPROC fpDlgProc;
} TabItem, FAR *LPTabItem;



typedef struct {
    HWND hwndTab;
    int nIndex;
    int nFrame;
    LPTabItem lpTabList;
} TabData, FAR *LPTabData;



#define DefDlgProcTab(hwnd, uMsg, wParam, lParam, lpTabData, lpbRecursionFlag) \
    ((lpTabData) && (lpTabData)->nIndex>=0 ? \
	(lpTabData)->lpTabList[(lpTabData)->nIndex].fpDlgProc((hwnd), (uMsg), (wParam), (lParam)) : \
	DefDlgProcEx((hwnd), (uMsg), (wParam), (lParam), (lpbRecursionFlag)))

#define DefMDIProcTab(hwnd, uMsg, wParam, lParam, lpTabData, lpbRecursionFlag) \
    ((lpTabData) && (lpTabData)->nIndex>=0 ? \
	(lpTabData)->lpTabList[(lpTabData)->nIndex].fpDlgProc((hwnd), (uMsg), (wParam), (lParam)) : \
	DefAmeolMDIDlgProc((hwnd), (uMsg), (wParam), (lParam)))


LPTabData FAR PASCAL _EXPORT initTabs(HWND hwnd, int nFrame, int nTabCount,
				      LPTabDescription lpTabDescription, int nInitialTab);
LPTabData FAR PASCAL _EXPORT initMDITabs(HWND hwnd, int nFrame, int nTabCount,
				      LPTabDescription lpTabDescription, int nInitialTab);
void FAR PASCAL _EXPORT freeTabs(HWND hwnd, LPTabData lpTabData);
void FAR PASCAL _EXPORT closeTab(HWND hwnd, LPTabData lpTabData);
BOOL FAR PASCAL _EXPORT openMDITab(HWND hwnd, LPTabData lpTabData);
BOOL FAR PASCAL _EXPORT openTab(HWND hwnd, LPTabData lpTabData);



#endif

