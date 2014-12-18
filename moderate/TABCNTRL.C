//  tabcntrl.c - tabbed dialog switch code adapted from the sample
//  code in "Windows 3.1: A Developer's Guide" by Jeffrey M. Richter.

//  28Feb1995, PCJJ
//  Win32 support added.
//
//  9Mar1995, PCJJ
//  Tied to the Windows common tab control; all relevant code put in here.
//
//  27Apr1995, PCJJ
//  Bugs in Win32 code fixed. Diagnostics added (compile with TABDEBUG defined).



#include <windows.h>
#include <windowsx.h>
#include "amctrls.h"
#include "ameolapi.h"
#include "winhorus.h"
#include "globals.h"
#include "tabcntrl.h"

#define WIDTHSTRING "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

void WINAPI RePositionControl(HWND hDlg, int id);

enum {
    PREDEFINEDCNTRLBIT =0x80,
    BUTTONCNTRLCODE    =0x80,
    EDITCNTRLCODE      =0x81,
    STATICCNTRLCODE    =0x82,
    LISTBOXCNTRLCODE   =0x83,
    SCROLLBARCNTRLCODE =0x84,
    COMBOBOXCNTRLCODE  =0x85
};

static char *szPredefinedClassNames[]={
    "BUTTON",
    "EDIT",
    "STATIC",
    "LISTBOX",
    "SCROLLBAR",
    "COMBOBOX"
};

#define DLGTOCLIENTX( x, units )   MulDiv( x, units, 4 )
#define DLGTOCLIENTY( y, units )   MulDiv( y, units, 8 )

#if !defined(_WIN32)
typedef struct {
    int  x;
    int  y;
    int  cx;
    int  cy;
    int  id;
    long style;
//  char dtilClass[];       //  Variable-length string.
//  char dtilText[];        //  Variable-length string.
//  BYTE dtilInfo;          //  # bytes in following memory block.
//  BYTE dtilData;          //  Variable-length memory block.
} DLGITEMTEMPLATE, FAR *LPDLGITEMTEMPLATE;
#endif



typedef struct {
    short int PointSize;
//  char  szTypeFace[];     //  Variable-length string.
} FONTINFO, FAR *LPFONTINFO;



#if defined(_WIN32)
static LPDLGTEMPLATE lockDlgRes(HMODULE hInstance, LPCSTR lpcszResName)
#else
static LPDLGTEMPLATE lockDlgRes(HINSTANCE hInstance, LPCSTR lpcszResName)
#endif
{
    HRSRC hrsrc;
    HGLOBAL hglb;

    if ((hrsrc=FindResource(hInstance, lpcszResName, RT_DIALOG)) &&
	(hglb=LoadResource(hInstance, hrsrc)))
	return((LPDLGTEMPLATE)LockResource(hglb));
    else
	return(NULL);

}

LPTabData FAR PASCAL _EXPORT initTabs(HWND hwnd, int nFrame, int nTabCount,
				      LPTabDescription lpTabDescription, int nInitialTab)
{
#if defined(_WIN32)
    HMODULE hInstance=InstanceFromWindow(hwnd);
#else
    HINSTANCE hInstance=InstanceFromWindow(hwnd);
#endif
    LPTabData lpTabData=gmallocz(sizeof(TabData));
    int nTab;

    if (!lpTabData)
	return(NULL);

    lpTabData->hwndTab=GetDlgItem(hwnd, nFrame);
    lpTabData->nIndex= -1;
    lpTabData->nFrame=nFrame;
    lpTabData->lpTabList=gcallocz((WORD) sizeof(TabItem), (WORD) nTabCount);

    if (!lpTabData->lpTabList) 
	{
		gfree(lpTabData);	lpTabData = NULL;
		return(NULL);
    }

    for (nTab=0; nTab<nTabCount; nTab++) {
	TC_ITEM tie;
	LPDLGTEMPLATE lpDlgTemplate=lockDlgRes(hInstance, lpTabDescription[nTab].lpszTemplate);

	if (!lpDlgTemplate) 
	{
	    gfree(lpTabData->lpTabList);	lpTabData->lpTabList = NULL;
	    gfree(lpTabData);				lpTabData = NULL;
	    return(NULL);
	}

	tie.mask=TCIF_TEXT;
	tie.pszText=lpTabDescription[nTab].lpszTag;
	TabCtrl_InsertItem(lpTabData->hwndTab, nTab, &tie);
	lpTabData->lpTabList[nTab].lpDlgTemplate=lpDlgTemplate;
	lpTabData->lpTabList[nTab].fpDlgProc=lpTabDescription[nTab].fpDlgProc;
    }

    SetWindowFont(lpTabData->hwndTab, lpGlobals->hfNormal, FALSE);
    TabCtrl_SetCurSel(lpTabData->hwndTab, nInitialTab);
    openTab(hwnd, lpTabData);
    return(lpTabData);
}


LPTabData FAR PASCAL _EXPORT initMDITabs(HWND hwnd, int nFrame, int nTabCount,
				      LPTabDescription lpTabDescription, int nInitialTab)
{
#if defined(_WIN32)
    HMODULE hInstance=InstanceFromWindow(hwnd);
#else
    HINSTANCE hInstance=InstanceFromWindow(hwnd);
#endif
    LPTabData lpTabData=gmallocz(sizeof(TabData));
    int nTab;

    if (!lpTabData)
		return(NULL);

    lpTabData->hwndTab=GetDlgItem(hwnd, nFrame);
    lpTabData->nIndex= -1;
    lpTabData->nFrame=nFrame;
    lpTabData->lpTabList=gcallocz((WORD) sizeof(TabItem), (WORD) nTabCount);

    if (!lpTabData->lpTabList) 
	{
		gfree(lpTabData);	lpTabData = NULL;
		return(NULL);
    }

    for (nTab=0; nTab<nTabCount; nTab++) 
	{
		TC_ITEM tie;
		LPDLGTEMPLATE lpDlgTemplate=lockDlgRes(hInstance, lpTabDescription[nTab].lpszTemplate);

		if (!lpDlgTemplate) 
		{
			gfree(lpTabData->lpTabList);	lpTabData->lpTabList = NULL;
			gfree(lpTabData);				lpTabData = NULL;
			return(NULL);
		}

		tie.mask=TCIF_TEXT;
		tie.pszText=lpTabDescription[nTab].lpszTag;
		TabCtrl_InsertItem(lpTabData->hwndTab, nTab, &tie);
		lpTabData->lpTabList[nTab].lpDlgTemplate=lpDlgTemplate;
		lpTabData->lpTabList[nTab].fpDlgProc=lpTabDescription[nTab].fpDlgProc;
    }

    SetWindowFont(lpTabData->hwndTab, lpGlobals->hfNormal, FALSE);
    TabCtrl_SetCurSel(lpTabData->hwndTab, nInitialTab);
    openMDITab(hwnd, lpTabData);
    return(lpTabData);
}


void EXPORT FAR PASCAL MyYield(void)
{
int i;
MSG msg;

	for (i = 0; i < 10; i++)
	{
  		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			return;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static BOOL CALLBACK enumShowChildProc(HWND hwnd, LPARAM lParam)
{
    HWND hwndFrame = (HWND)LOWORD(lParam);
	int cmdShow    = (int)HIWORD(lParam);

    if (hwnd!=hwndFrame) 
	{
		ShowWindow(hwnd, cmdShow);
		MyYield();
    }

    return(TRUE);  //  Continue the enumeration
}



void FAR PASCAL _EXPORT ShowHideChildren(HWND hwnd, LPTabData lpTabData, BOOL show)

//  Destroy all controls inside the tab panel

{
    if (lpTabData) 
	{
		EnumChildWindows(hwnd, (WNDENUMPROC)enumShowChildProc,
				 (LPARAM)MAKELPARAM(GetDlgItem(hwnd, lpTabData->nFrame), show));

    }
}

void FAR PASCAL _EXPORT freeTabs(HWND hwnd, LPTabData lpTabData)
{
    if (lpTabData) 
	{
		if (lpTabData->lpTabList) 
		{
			closeTab(hwnd, lpTabData);
			gfree(lpTabData->lpTabList);	lpTabData->lpTabList = NULL;
		}

		gfree(lpTabData);	lpTabData = NULL;
    }
}



static BOOL CALLBACK enumChildProc(HWND hwnd, LPARAM lParam)
{
    HWND hwndFrame=(HWND)lParam;

	//  Don't destroy the frame window
    if (hwnd!=hwndFrame && hwnd != GetDlgItem(GetParent(hwnd),990)) 
	{
//		RECT rcChild, rcChangeable;

//		GetWindowRect(hwnd, &rcChild);
//		GetWindowRect(hwndFrame, &rcChangeable);

//		if (IntersectRect(&rcChild, &rcChangeable, &rcChild))
	    //  Rectangles intersect -- destroy the window
			DestroyWindow(hwnd);

    }

    return(TRUE);  //  Continue the enumeration
}



void FAR PASCAL _EXPORT closeTab(HWND hwnd, LPTabData lpTabData)

//  Destroy all controls inside the tab panel

{
    if (lpTabData) 
	{
		if (lpTabData->lpTabList && lpTabData->nIndex>=0)
			FORWARD_WM_DESTROY(hwnd, lpTabData->lpTabList[lpTabData->nIndex].fpDlgProc);

		EnumChildWindows(hwnd, (WNDENUMPROC)enumChildProc,
				 (LPARAM)GetDlgItem(hwnd, lpTabData->nFrame));

		lpTabData->nIndex= -1;
    }
}


#if defined(_WIN32)
static LPWORD normalise(LPVOID lpPtr, LPVOID lpBase, int nPos)

//  Align pointer to double-word boundary

{
    LPWORD lpResult=(LPWORD)lpPtr;
    BOOL bOdd=(lpResult-(LPWORD)lpBase)&1;

#if defined(_TABDEBUG)
    bOdd=query(NULL, MB_YESNO|(bOdd ? MB_DEFBUTTON1 : MB_DEFBUTTON2), "%d: Offset is %s - increment?", nPos, bOdd ? "odd" : "even")==IDYES;
#endif
    return(bOdd ? lpResult+1 : lpResult);
}



static int strcpy16to8(LPSTR lpTo, LPWORD lpFrom)
{
    int nCount=0;

    while (*lpFrom) {
	*lpTo++ =(char)(*lpFrom++);
	nCount++;
    }

    *lpTo='\0';
    return(nCount);
}



#if defined(_TABDEBUG)
#include <stdarg.h>

static void checkResource(HWND hwnd, LPWORD lpBase, LPSTR lpText,...)
{
    LPSTR lpBuff=gmalloc(1024);
    int nOffset=0;
    int nCheck;

    if (!lpBuff) {
	alert(hwnd, "gmalloc() failed in checkResource!");
	return;
    }

    do {
	va_list args;
	LPSTR lpPtr;
	int nPtr;

	va_start(args, lpText);
	lpPtr=lpBuff+wvsprintf(lpBuff, lpText, args)+4;
	va_end(args);
	lstrcat(lpBuff, "\r\n\r\n");

	for (nPtr=0; nPtr<15; nPtr++) {
	    int nIndex=nPtr+nOffset;
	    UCHAR c1=lpBase[nIndex]>>8;
	    UCHAR c2=lpBase[nIndex]&0xff;

	    if (c1<32 || 126<c1)
		c1=' ';

	    if (c2<32 || 126<c2)
		c2=' ';

	    lpPtr+=wsprintf(lpPtr, "%04d: %04x %c%c\r\n", nIndex, lpBase[nIndex], c1, c2);
	}

	nCheck=query(hwnd, MB_YESNOCANCEL, lpBuff);

	switch (nCheck) {

	case IDYES:		nOffset+=16;	break;
	case IDNO:		nOffset-=16;	break;

	}
    } while (nCheck!=IDCANCEL);

    gfree(lpBuff);	lpBuff = NULL;
}
#endif
#endif


BOOL FAR PASCAL _EXPORT openMDITab(HWND hwnd, LPTabData lpTabData)
{
#if defined(_WIN32)
    HMODULE hInstance=InstanceFromWindow(hwnd);
    LPWORD p, lpCreateParams;
#else
    HINSTANCE hInstance=InstanceFromWindow(hwnd);
    LPBYTE p, lpCreateParams;
#endif
    LPBYTE lpszClass, lpszText = NULL;
    HWND hwndPrevChild=GetDlgItem(hwnd, lpTabData->nFrame);
    HFONT hFont;
    int nIndex=TabCtrl_GetCurSel(lpTabData->hwndTab);
    LPDLGTEMPLATE lpDlgTemplate;
    LPDLGITEMTEMPLATE lpDlgItemTemplate;
    LPFONTINFO lpFontInfo;
    HWND hwndChild;
    int nNumControls;
    RECT rc;

    if (nIndex<0)
		return(FALSE);

	ShowWindow(GetDlgItem(hwnd, 990), SW_HIDE);

    lpTabData->nIndex=nIndex;
    lpDlgTemplate=lpTabData->lpTabList[nIndex].lpDlgTemplate;

    hFont=GetWindowFont(hwndPrevChild);//(hwnd);

    //  Ignore everything in Dialog Template except for the number
    //  of controls.
    nNumControls=lpDlgTemplate->cdit;

#if defined(_WIN32)
    p=(LPWORD)(&lpDlgTemplate->cy+1); 	//  Start of Menu name
#if defined(_TABDEBUG)
    checkResource(hwnd, p, "Start of menu name");
#endif

    if (*p==0xffff)			//  Skip the menu name string
	p+=2;
    else
	while(*p++);

#if defined(_TABDEBUG)
    checkResource(hwnd, p, "Start of class name");
#endif

    if (*p==0xffff)			//  Skip the Class name string
		p+=2;
    else
		while(*p++);

#if defined(_TABDEBUG)
    checkResource(hwnd, p, "Start of caption");
#endif
#else
    p=(LPBYTE)(&lpDlgTemplate->cy+1); 	//  Start of Menu name
    while (*p++);           		//  Skip the menu name string
    while (*p++);           		//  Skip the Class name string
#endif
    while (*p++);           		//  Skip the Caption string
    lpFontInfo=(LPFONTINFO)p; 		//  Start of FONTINFO (if exists)

    //  Find address of first DLGITEMTEMPLATE structure
    if (lpDlgTemplate->style&DS_SETFONT) 
	{
#if defined(_WIN32)
		p=(LPWORD)(&lpFontInfo->PointSize+1);
#if defined(_TABDEBUG)
		checkResource(hwnd, p, "Start of font name");
#endif
#else
		p=(LPBYTE)(&lpFontInfo->PointSize+1);
#endif
		while (*p++);  			//  Skip the Type face name string
		lpDlgItemTemplate=(LPDLGITEMTEMPLATE)p;
	} 
	else
		lpDlgItemTemplate=(LPDLGITEMTEMPLATE)lpFontInfo;

    //  Create all of the child controls
    while (nNumControls--) 
	{
#if defined(_WIN32)
		LPWORD lpwzClass=(LPWORD)(&lpDlgItemTemplate->id+1);
		BYTE szClassBuffer[64];
		LPWORD lpwzText;
		BYTE szTextBuffer[128];
		int nCount;

		lpDlgItemTemplate=(LPDLGITEMTEMPLATE)normalise(lpDlgItemTemplate, lpDlgTemplate, 1);
		lpwzClass=(LPWORD)(&lpDlgItemTemplate->id+1);

		if (*lpwzClass==0xffff) 
		{
			nCount=2;
			lpwzText=lpwzClass+2;
			lpszClass=(LPBYTE)szPredefinedClassNames[*(lpwzClass+1)-PREDEFINEDCNTRLBIT];
		} 
		else 
		{
			lpszClass=szClassBuffer;
			nCount=strcpy16to8(szClassBuffer, lpwzClass)+1;
			lpwzText=lpwzClass+nCount;
		}

#if defined(_TABDEBUG)
		checkResource(hwnd, (LPINT)lpDlgItemTemplate, "Start of child control %d of %d, class \"%s\"",
		      lpDlgTemplate->cdit-nNumControls, lpDlgTemplate->cdit, lpszClass);
#endif

		if (*lpwzText==0xffff) 
		{
			nCount+=2;
			lpszText=(LPBYTE)MAKELONG(*(lpszText+1), 0);
		} 
		else 
		{
			lpszText=szTextBuffer;
			nCount+=strcpy16to8(szTextBuffer, lpwzText)+1;
		}

		lpCreateParams=normalise(lpwzClass+nCount-1, lpDlgTemplate, 2);
#else
		lpszClass=(LPBYTE)(&lpDlgItemTemplate->style+1);

		if (*lpszClass&PREDEFINEDCNTRLBIT) 
		{
			lpszText=lpszClass+1;
			lpszClass=(LPBYTE)szPredefinedClassNames[(WORD)(*lpszClass)-PREDEFINEDCNTRLBIT];
		} 
		else
			for (lpszText=lpszClass; *lpszText++;);

	//  Find address of number-of-bytes-in-additional-data
		for (lpCreateParams=lpszText; *lpCreateParams++;);
#endif

	//  Do not create any windows with an ID of uIDChangeableArea
	//  This control was used for reference when the template was
	//  created and should not be created.
		if (lpDlgItemTemplate->id!=lpTabData->nFrame) 
		{
			RECT			rcTab;
			DWORD			tInt, cxFact, cyFact;
            
			GetClientRect(GetDlgItem(hwnd, 990), &rcTab);

			cxFact = ((rcTab.right-rcTab.left)*100)/50;
			cyFact = ((rcTab.bottom-rcTab.top)*100)/14;

			tInt	  = (DWORD)(lpDlgItemTemplate->x)*(DWORD)cxFact / 100L;
			rc.left   = (short)tInt;
			tInt	  = (DWORD)(lpDlgItemTemplate->y)*(DWORD)cyFact / 100L;
			rc.top    = (short)tInt;
			tInt	  = (DWORD)(lpDlgItemTemplate->x+lpDlgItemTemplate->cx)*(DWORD)cxFact / 100L;
			rc.right  = (short)tInt;
			tInt	  = (DWORD)(lpDlgItemTemplate->y+lpDlgItemTemplate->cy)*(DWORD)cyFact / 100L;
			rc.bottom = (short)tInt;

			hwndChild=CreateWindowEx(WS_EX_NOPARENTNOTIFY,
				     (LPCSTR)lpszClass, (LPCSTR)lpszText,
				     lpDlgItemTemplate->style,
				     rc.left,
					 rc.top,
					 rc.right-rc.left,
					 rc.bottom-rc.top,
				     hwnd, (HMENU)lpDlgItemTemplate->id, hInstance,
				     lpCreateParams);

			if (!hwndChild) 
			{
		//  The child couldn't be created
				alert(hwnd, "Failed to create control %d (class %s)",
						lpDlgTemplate->cdit-nNumControls+1, lpszClass);

				return(FALSE);
			}

	    //  Tell the new control to use the same font as dialog box
			SetWindowFont(hwndChild, hFont, FALSE);

	    // Fix the Z-Order of the controls
			SetWindowPos(hwndChild, hwndPrevChild, 0, 0, 0, 0,
				 SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			
			RePositionControl(hwnd, lpDlgItemTemplate->id);

			hwndPrevChild=hwndChild;
		}

	// Point to the next DlgItemTemplate
		lpDlgItemTemplate=(LPDLGITEMTEMPLATE)(lpCreateParams+1+ *lpCreateParams);
    }
	
    FORWARD_WM_CREATE(hwnd, 0, lpTabData->lpTabList[nIndex].fpDlgProc);
    return(TRUE);
}

BOOL FAR PASCAL _EXPORT openTab(HWND hwnd, LPTabData lpTabData)
{
#if defined(_WIN32)
    HMODULE hInstance=InstanceFromWindow(hwnd);
    LPWORD p, lpCreateParams;
#else
    HINSTANCE hInstance=InstanceFromWindow(hwnd);
    LPBYTE p, lpCreateParams;
#endif
    LPBYTE lpszClass, lpszText=NULL;
    HWND hwndPrevChild=GetDlgItem(hwnd, lpTabData->nFrame);
    HFONT hFont;
    int nIndex=TabCtrl_GetCurSel(lpTabData->hwndTab);
    LPDLGTEMPLATE lpDlgTemplate;
    LPDLGITEMTEMPLATE lpDlgItemTemplate;
    LPFONTINFO lpFontInfo;
    HWND hwndChild;
    int nNumControls;
    RECT rc;

    if (nIndex<0)
	return(FALSE);

    lpTabData->nIndex=nIndex;
    lpDlgTemplate=lpTabData->lpTabList[nIndex].lpDlgTemplate;
    hFont=GetWindowFont(hwnd);

    //  Ignore everything in Dialog Template except for the number
    //  of controls.
    nNumControls=lpDlgTemplate->cdit;

#if defined(_WIN32)
    p=(LPWORD)(&lpDlgTemplate->cy+1); 	//  Start of Menu name
#if defined(_TABDEBUG)
    checkResource(hwnd, p, "Start of menu name");
#endif

    if (*p==0xffff)			//  Skip the menu name string
	p+=2;
    else
	while(*p++);

#if defined(_TABDEBUG)
    checkResource(hwnd, p, "Start of class name");
#endif

    if (*p==0xffff)			//  Skip the Class name string
	p+=2;
    else
	while(*p++);

#if defined(_TABDEBUG)
    checkResource(hwnd, p, "Start of caption");
#endif
#else
    p=(LPBYTE)(&lpDlgTemplate->cy+1); 	//  Start of Menu name
    while (*p++);           		//  Skip the menu name string
    while (*p++);           		//  Skip the Class name string
#endif
    while (*p++);           		//  Skip the Caption string
    lpFontInfo=(LPFONTINFO)p; 		//  Start of FONTINFO (if exists)

    //  Find address of first DLGITEMTEMPLATE structure
    if (lpDlgTemplate->style&DS_SETFONT) {
#if defined(_WIN32)
	p=(LPWORD)(&lpFontInfo->PointSize+1);
#if defined(_TABDEBUG)
	checkResource(hwnd, p, "Start of font name");
#endif
#else
	p=(LPBYTE)(&lpFontInfo->PointSize+1);
#endif
	while (*p++);  			//  Skip the Type face name string
	lpDlgItemTemplate=(LPDLGITEMTEMPLATE)p;
    } else
	lpDlgItemTemplate=(LPDLGITEMTEMPLATE)lpFontInfo;

    //  Create all of the child controls
    while (nNumControls--) {
#if defined(_WIN32)
	LPWORD lpwzClass=(LPWORD)(&lpDlgItemTemplate->id+1);
	BYTE szClassBuffer[64];
	LPWORD lpwzText;
	BYTE szTextBuffer[128];
	int nCount;

	lpDlgItemTemplate=(LPDLGITEMTEMPLATE)normalise(lpDlgItemTemplate, lpDlgTemplate, 1);
	lpwzClass=(LPWORD)(&lpDlgItemTemplate->id+1);

	if (*lpwzClass==0xffff) {
	    nCount=2;
	    lpwzText=lpwzClass+2;
	    lpszClass=(LPBYTE)szPredefinedClassNames[*(lpwzClass+1)-PREDEFINEDCNTRLBIT];
	} else {
	    lpszClass=szClassBuffer;
	    nCount=strcpy16to8(szClassBuffer, lpwzClass)+1;
	    lpwzText=lpwzClass+nCount;
	}

#if defined(_TABDEBUG)
	checkResource(hwnd, (LPINT)lpDlgItemTemplate, "Start of child control %d of %d, class \"%s\"",
		      lpDlgTemplate->cdit-nNumControls, lpDlgTemplate->cdit, lpszClass);
#endif

	if (*lpwzText==0xffff) {
	    nCount+=2;
	    lpszText=(LPBYTE)MAKELONG(*(lpszText+1), 0);
	} else {
	    lpszText=szTextBuffer;
	    nCount+=strcpy16to8(szTextBuffer, lpwzText)+1;
	}

	lpCreateParams=normalise(lpwzClass+nCount-1, lpDlgTemplate, 2);
#else
	lpszClass=(LPBYTE)(&lpDlgItemTemplate->style+1);

	if (*lpszClass&PREDEFINEDCNTRLBIT) {
	    lpszText=lpszClass+1;
	    lpszClass=(LPBYTE)szPredefinedClassNames[(WORD)(*lpszClass)-PREDEFINEDCNTRLBIT];
	} else
	    for (lpszText=lpszClass; *lpszText++;);

	//  Find address of number-of-bytes-in-additional-data
	for (lpCreateParams=lpszText; *lpCreateParams++;);
#endif

	//  Do not create any windows with an ID of uIDChangeableArea
	//  This control was used for reference when the template was
	//  created and should not be created.
	if (lpDlgItemTemplate->id!=lpTabData->nFrame) {
	    SetRect(&rc, lpDlgItemTemplate->x,
			 lpDlgItemTemplate->y,
			 lpDlgItemTemplate->x+lpDlgItemTemplate->cx,
			 lpDlgItemTemplate->y+lpDlgItemTemplate->cy);

	    MapDialogRect(hwnd, &rc);

	    hwndChild=CreateWindowEx(WS_EX_NOPARENTNOTIFY,
				     (LPCSTR)lpszClass, (LPCSTR)lpszText,
				     lpDlgItemTemplate->style,
				     rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				     hwnd, (HMENU)lpDlgItemTemplate->id, hInstance,
				     lpCreateParams);

	    if (!hwndChild) {
		//  The child couldn't be created
		alert(hwnd, "Failed to create control %d (class %s)",
		      lpDlgTemplate->cdit-nNumControls+1, lpszClass);

		return(FALSE);
	    }

	    //  Tell the new control to use the same font as dialog box
	    SetWindowFont(hwndChild, hFont, FALSE);

	    // Fix the Z-Order of the controls
	    SetWindowPos(hwndChild, hwndPrevChild, 0, 0, 0, 0,
			 SWP_NOMOVE|SWP_NOSIZE);

	    hwndPrevChild=hwndChild;
	}

	// Point to the next DlgItemTemplate
	lpDlgItemTemplate=(LPDLGITEMTEMPLATE)(lpCreateParams+1+ *lpCreateParams);
    }

    FORWARD_WM_INITDIALOG(hwnd, 0, 0, lpTabData->lpTabList[nIndex].fpDlgProc);
    return(TRUE);
}

