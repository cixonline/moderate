#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include "amctrls.h"
#include "ameolapi.h"
#include "hctools.h"
#include "winhorus.h"
#include "setup.h"
#include "moderate.h"
#include "help.h"
#include "globals.h"
#include <malloc.h>
#include <string.h>

#define WS_MDICHILD   (WS_CHILD | WS_CLIPSIBLINGS | WS_SYSMENU | WS_CAPTION |\
					   WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |\
					   WS_VISIBLE |WS_BORDER | WS_DLGFRAME | WS_TABSTOP)


HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE hInstance,
									  LPCSTR lpTemplateName,
									  int x, int y, int cx, int cy, LPARAM lParam)
{

	HWND                     hDlg ;
	HWND					 hWndActive;
#ifndef WIN32
	DWORD					 theResult;
#endif
	MDICREATESTRUCT          MDIcs ;
	BOOL					 maxMDI;
	char FAR * titleString = "File List Manager";
	HWND  amWindows[7];
	HWND hClientWnd;

	GetAmeolWindows((HWND *)&amWindows);
	hClientWnd = GetParent(amWindows[4]);
	if(!IsWindow(hClientWnd))
		hClientWnd = amWindows[3];
#ifndef WIN32
	theResult = (DWORD)(SendMessage( hClientWnd , WM_MDIGETACTIVE, 0,0L));
	maxMDI     = (BOOL) HIWORD(theResult);
	hWndActive = (HWND) LOWORD(theResult);
#else
	hWndActive = (HWND)(SendMessage( hClientWnd , WM_MDIGETACTIVE, 0,(LPARAM)(LPBOOL)&maxMDI));
#endif
                                  
	if(!IsWindow(hWndActive) && !IsWindow(hClientWnd))
		return NULL;

	MDIcs.szClass =      lpTemplateName;
	MDIcs.szTitle =      titleString;
   	MDIcs.hOwner =       hInstance ;
   	MDIcs.x =            x;
   	MDIcs.y =            y;
	MDIcs.cx =           cx;
   	MDIcs.cy =           cy;
	MDIcs.lParam =       lParam;
	MDIcs.style =		 ( maxMDI ) ? WS_MAXIMIZE : WS_MINIMIZE;
	MDIcs.style |= WS_MDICHILD;
	hDlg = (HWND) LOWORD( SendMessage( hClientWnd , WM_MDICREATE, 0,
                                     (LPARAM) (LPMDICREATESTRUCT) &MDIcs ) ) ;
	if(hDlg == NULL)
	{
//		alert(hClientWnd, "Error Creating Moderate Main Window");
		return NULL;
	}
	else
	{
		StdOpenMDIDialog ( hDlg );
		return ( hDlg ) ;
	}
} // end of CreateMDIDialogParam()

