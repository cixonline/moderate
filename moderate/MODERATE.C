/* MODERATE.C - Moderate main program
 *
 * Copyright 1993-1996 CIX, All Rights Reserved
 *
 * Use, duplication, and disclosure of this file is governed
 * by a license agreement between CIX. and the licensee.
 *
 * Written by Pete Jordan
 * CIX Ameol Development Group, Wales
 */

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
#include "bldinfo.h"

#define	MIN_HCTOOLS_VERSION			0x00010A00L  /* Minimum HCTools version = 1.07 */
#define	MIN_WINHORUS_VERSION		0x00020A00L  /* Minimum WinHorus version = 2.06 */

/* Addon version structure.
 */
VERSION verStruct =
	{
	MAKEADDONVERSION,
	AMEOL_2_00,
	"P.Jordan, Y.Hassan, S.Palmer, S.Mott",
	"Moderator functions for Ameol2"
	};

LPGlobals lpGlobals = NULL;

BOOL _EXPORT CALLBACK eventProc(int wEvent, LPARAM lParam1, LPARAM lParam2);
BOOL _EXPORT CALLBACK aboutProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK aboutProcReal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK dllInfoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT aboutReal_OnInitDialog(HWND, HWND, LPARAM );
HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE hInstance,
									  LPCSTR lpTemplateName,
									  int x, int y, int cx, int cy, LPARAM lParam);

BOOL _EXPORT CALLBACK MDImessageEditProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL _EXPORT CALLBACK MailMDImessageEditProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

void newConfigureSetup(HWND hwnd);

#define	GetPPInt(lpkey,lpsect,value)		(AmGetPrivateProfileInt((lpkey),(lpsect),(value),setup.szIniFile))

char str_ClassName[15]           = "mod_edit_flist";
char str_NotifyClassName[17]     = "mod_notify_flist";

char str_Window[9]				 = "Moderate";

char str_Left[14]				 = "mod_MDI_Left";
char str_Top[12]				 = "mod_MDI_Top";
char str_Width[14]				 = "mod_MDI_Width";
char str_Height[15]				 = "mod_MDI_Height";

char str_NotifyLeft[19]			 = "mod_MDINotify_Left";
char str_NotifyTop[18]			 = "mod_MDINotify_Top";
char str_NotifyWidth[20]		 = "mod_MDINotify_Width";
char str_NotifyHeight[21]		 = "mod_MDINotify_Height";

/* This function initialises the global structure.
 */
static BOOL initGlobals( HINSTANCE hInst )
{
	static Globals globals;
	WORD wWinVer;

	WinHorusInit(hInst);

	/* Get and save the Windows version
	 */
	wWinVer = LOWORD( GetVersion() );
	wWinVer = (( (WORD)(LOBYTE( wWinVer ) ) ) << 8 ) | ( (WORD)HIBYTE( wWinVer ) );

	/* Allocate and initialise the global structure.
	 */
	lpGlobals = (LPGlobals)gmallocz(sizeof(Globals));
	if( NULL == lpGlobals )
		{
		alert(NULL, "Moderate: gmalloc() in initGlobals() has failed");
		return(FALSE);
		}
	lpGlobals->hInst = hInst;
	lpGlobals->hwndAmeol = NULL;
	lpGlobals->hwndMDIFlist = NULL;
	lpGlobals->bMenusInstalled = FALSE;
	lpGlobals->lpEventProc = NULL;
	lpGlobals->context.hConference = NULL;
	lpGlobals->context.lpcConfName = NULL;
	lpGlobals->context.hTopic = NULL;
	lpGlobals->context.lpcTopicName = NULL;
	lpGlobals->context.bInEditor = FALSE;
	lpGlobals->contextFlist = lpGlobals->context;
	lpGlobals->wHelpMessageID = RegisterWindowMessage( HELPMSGSTRING );
	lpGlobals->hfBold = NULL;
	lpGlobals->hfNormal = NULL;
	lpGlobals->hfMessageEdit = NULL;
	lpGlobals->hfDescEdit = NULL;
	lpGlobals->hfBoldEdit = NULL;
	lpGlobals->bRecursionFlag = FALSE;
	lpGlobals->cTotalMenus = 0;
	lpGlobals->wWinVer = wWinVer;
	lpGlobals->dragCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_DRAG));
	lpGlobals->noDragCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_NODRAG));
	versionDecode( GetAmeolVersion(), &lpGlobals->AmeolVerInfo );
	lpGlobals->bEvaluation = lpGlobals->AmeolVerInfo.bEval;
	return(TRUE);
}



#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)

//  Windows: DLL initialisation.

{
    if (fdwReason==DLL_PROCESS_ATTACH)
	return(initGlobals((HINSTANCE)hInstance));

    return(TRUE);
}
#else
int CALLBACK LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)

//  Windows: DLL initialisation.

{
    initGlobals((HINSTANCE)hInstance);
    return(TRUE);
}
#endif

LPVERSION _EXPORT CALLBACK AddonVersion(void)
{
	return( &verStruct );
}

/* This function adds menu commands to Moderate
 */
void addAmeolMenus(void)
{
	if( !lpGlobals->bMenusInstalled )
		{
		LPAmeolMenu		lpAmeolMenu;

		/* Add menus for Ameol2 which has only one menu bar.
			*/
		lpAmeolMenu = &lpGlobals->ameolMenu[0];
		lpAmeolMenu->menu.hInst = lpGlobals->hInst;
		lpAmeolMenu->menu.wPopupMenu = AMEOL2_CIX_MENU;
		lpAmeolMenu->menu.wMenuPos = -1;
		lpAmeolMenu->menu.lpMenuText = setup.bPrefixMenuCommands ? "Mod: Create F&orum..." : "Create F&orum...";
		lpAmeolMenu->menu.lpMenuHelpText = "Create a new forum on CIX";
		lpAmeolMenu->menu.wKey = 0;
		InsertAmeolMenu(&lpAmeolMenu->menu);
		lpAmeolMenu->action = ACTION_CREATE_CONF;
		lpAmeolMenu->uSmallTool = IDB_CREATE_CONF_16;
		lpAmeolMenu->uBigTool = IDB_CREATE_CONF_24;
		lpAmeolMenu->fHasButton = TRUE;

		lpAmeolMenu = &lpGlobals->ameolMenu[1];
		lpAmeolMenu->menu.hInst = lpGlobals->hInst;
		lpAmeolMenu->menu.wPopupMenu = AMEOL2_CIX_MENU;
		lpAmeolMenu->menu.wMenuPos = -1;
		lpAmeolMenu->menu.lpMenuText = setup.bPrefixMenuCommands ? "Mod: Edit Topic&s..." : "Edit Topic&s...";
		lpAmeolMenu->menu.lpMenuHelpText = "Create, prune and remove topics in a CIX forum";
		lpAmeolMenu->menu.wKey = 0;
		InsertAmeolMenu(&lpAmeolMenu->menu);
		lpAmeolMenu->action = ACTION_EDIT_TOPIC;
		lpAmeolMenu->uSmallTool = IDB_EDIT_TOPIC_16;
		lpAmeolMenu->uBigTool = IDB_EDIT_TOPIC_24;
		lpAmeolMenu->fHasButton = TRUE;

		lpAmeolMenu = &lpGlobals->ameolMenu[3];
		lpAmeolMenu->menu.hInst = lpGlobals->hInst;
		lpAmeolMenu->menu.wPopupMenu = AMEOL2_CIX_MENU;
		lpAmeolMenu->menu.wMenuPos = -1;
		lpAmeolMenu->menu.lpMenuText = setup.bPrefixMenuCommands ? "Mod: Ed&it File List..." : "Ed&it File List...";
		lpAmeolMenu->menu.lpMenuHelpText = "Edit Moderator's file list for a topic";
		lpAmeolMenu->menu.wKey = 0;
		InsertAmeolMenu(&lpAmeolMenu->menu);
		lpAmeolMenu->action = ACTION_EDIT_FLIST;
		lpAmeolMenu->uSmallTool = IDB_EDIT_FILELIST_16;
		lpAmeolMenu->uBigTool = IDB_EDIT_FILELIST_24;
		lpAmeolMenu->fHasButton = TRUE;

		lpGlobals->lpFlistMenuItem = &lpGlobals->ameolMenu[3];

		lpAmeolMenu = &lpGlobals->ameolMenu[4];
		lpAmeolMenu->menu.hInst = lpGlobals->hInst;
		lpAmeolMenu->menu.wPopupMenu = AMEOL2_HELP_MENU;
		lpAmeolMenu->menu.wMenuPos = 3;
		lpAmeolMenu->menu.lpMenuText = "&Moderate Help";
		lpAmeolMenu->menu.lpMenuHelpText = "Display the contents for Moderate help";
		lpAmeolMenu->menu.wKey = 0;
		InsertAmeolMenu(&lpAmeolMenu->menu);
		lpAmeolMenu->action = ACTION_HELP;
		lpAmeolMenu->uSmallTool = IDB_HELP_CONTENTS_16;
		lpAmeolMenu->uBigTool = IDB_HELP_CONTENTS_24;
		lpAmeolMenu->fHasButton = TRUE;

		lpAmeolMenu = &lpGlobals->ameolMenu[5];
		lpAmeolMenu->menu.hInst = lpGlobals->hInst;
		lpAmeolMenu->menu.wPopupMenu = AMEOL2_HELP_MENU;
		lpAmeolMenu->menu.wMenuPos = -1;
		lpAmeolMenu->menu.lpMenuText = "About Moderate...";
		lpAmeolMenu->menu.lpMenuHelpText = "Provides information about the moderate addon";
		lpAmeolMenu->menu.wKey = 0;
		InsertAmeolMenu(&lpAmeolMenu->menu);
		lpAmeolMenu->action = ACTION_ABOUT;
		lpAmeolMenu->uSmallTool = IDB_HELP_ABOUT_16;
		lpAmeolMenu->uBigTool = IDB_HELP_ABOUT_24;
		lpAmeolMenu->fHasButton = TRUE;

		/* Total number of commands added to the menus.
			*/
		lpGlobals->cTotalMenus = 6;
		lpGlobals->bMenusInstalled=TRUE;
		}
}

/* This function goes through the list of menu commands
 * added by Moderate and removes them.
 */
void removeAmeolMenus(void)
{
	if( lpGlobals->bMenusInstalled )
		{
		UINT nMenu;

		for( nMenu = 0; nMenu < lpGlobals->cTotalMenus; nMenu++ )
			DeleteAmeolMenu( lpGlobals->ameolMenu[nMenu].menu.hMenu, lpGlobals->ameolMenu[nMenu].menu.wID );
		lpGlobals->bMenusInstalled = FALSE;
		}
}

/* This is the Moderate load entry-point. It is called when
 * Moderate is loaded by Ameol or Ameol2.
 */
BOOL _EXPORT CALLBACK Load( HWND hwndParent )
{
	static BOOL bLoaded = FALSE;

	WNDCLASS wc;
	WORD wWinVer;
	HBRUSH hbrMDIDlgBackgnd;

//	HCAT hCat;
	
//	hCat = OpenCategory( "MAIN" );

	wWinVer = LOWORD( GetVersion() );
	wWinVer = (( (WORD)(LOBYTE( wWinVer ) ) ) << 8 ) | ( (WORD)HIBYTE( wWinVer ) );
#ifdef WIN32
	hbrMDIDlgBackgnd = (HBRUSH)( wWinVer < 0x35F ? (COLOR_WINDOW+1) : (COLOR_BTNFACE+1) );
#else
	hbrMDIDlgBackgnd = (HBRUSH)(COLOR_WINDOW+1);
#endif

	/* Create and register the MDISamp window class */
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)flistProc;
	wc.hIcon			= LoadIcon( lpGlobals->hInst, "IID_SCALES" );
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	wc.hInstance		= lpGlobals->hInst;
	wc.cbWndExtra		= GWW_EXTRA;
	wc.cbClsExtra		= 0;
	wc.hbrBackground	= hbrMDIDlgBackgnd;//(HBRUSH)( COLOR_WINDOW + 1 );
	wc.lpszClassName	= str_ClassName;
	wc.lpszMenuName	    = NULL;
	if( !RegisterClass( &wc ) )
		return( FALSE );

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)MDImessageEditProc;
	wc.hIcon			= LoadIcon( lpGlobals->hInst, "IID_SCALES" );
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	wc.hInstance		= lpGlobals->hInst;
	wc.cbWndExtra		= GWW_EXTRA;
	wc.cbClsExtra		= 0;
	wc.hbrBackground	= hbrMDIDlgBackgnd;//(HBRUSH)( COLOR_WINDOW + 1 );
	wc.lpszClassName	= str_NotifyClassName;
	wc.lpszMenuName	    = NULL;
	if( !RegisterClass( &wc ) )
		return( FALSE );

	/* Fail if we're already installed.
	 * This won't work in 32-bit version.
	 */
	if( bLoaded )
		{
		alert( hwndParent, "Moderate is already installed!" );
		return( FALSE );
		}

	/* Check that the correct version of HCTools is installed.
	 */
	if( !versionCheck( HCToolsVersion(), MIN_HCTOOLS_VERSION ) )
		{
		char szBuff[40];

		lstrcpy( szBuff, ExpandVersion(MIN_HCTOOLS_VERSION) );
		alert( hwndParent, "You need at least version %s of hctools.dll to run this version of Moderate"
				 " - version %s is currently installed", (LPSTR)szBuff, ExpandVersion( HCToolsVersion() ) );
		return( FALSE );
		}

	/* Check that the correct version of WinHorus is installed.
	 */
	else if( !versionCheck( WinHorusVersion(), MIN_WINHORUS_VERSION ) )
		{
		char szBuff[40];

		lstrcpy(szBuff, ExpandVersion(MIN_WINHORUS_VERSION));
		alert( hwndParent, "You need at least version %s of winhorus.dll to run this version of Moderate"
				 " - version %s is currently installed", (LPSTR)szBuff, ExpandVersion( WinHorusVersion() ) );
		return( FALSE );
		}

	/* We're loaded, so initialise.
	 */
	bLoaded=TRUE;
	Amctl_Init();
	getSetup( hwndParent );
	lpGlobals->hwndAmeol = hwndParent;
	addAmeolMenus();

	/* Register the Ameol AE_MSGCHANGE event to trap topic changes,
	 * AE_DELETECONFERENCE and AE_DELETETOPIC so we can delete our files
	 * and AE_CONNECTSTART and AE_CONNECTEND for topic data processing
	 */
	lpGlobals->lpEventProc = (LPFNEEVPROC)MakeProcInstance( eventProc, lpGlobals->hInst );
	RegisterEvent( AE_MSGCHANGE, lpGlobals->lpEventProc );
	RegisterEvent( AE_DELETECONFERENCE, lpGlobals->lpEventProc );
	RegisterEvent( AE_DELETETOPIC, lpGlobals->lpEventProc );
	RegisterEvent( AE_CONNECTSTART, lpGlobals->lpEventProc );
	RegisterEvent( AE_CONNECTEND, lpGlobals->lpEventProc );

	if( lpGlobals->AmeolVerInfo.nVersion != 1 )
		RegisterEvent( AE_CONFIGADDONDLG, lpGlobals->lpEventProc );
	

	/* Check whether the last blink failed and, if so, 
	 * process any topic data files left over from the last blink - shouldn't
	 * be any, but could be left after a crash
	 */
	if( AmGetPrivateProfileInt( "Settings", "recover", FALSE, setup.szIniFile ) )
		updateTopicInfo(hwndParent);
	return( TRUE );
}



void _EXPORT CALLBACK Exec(HWND hwndParent)

//  Ameol: Addon started from Utilities menu - display "About" dialog.

{
    FARPROC lpProc;

    getResources();
    lpProc=MakeProcInstance(aboutProc, lpGlobals->hInst);
    StdDialogBox((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_ABOUT", 
				 (HWND)hwndParent, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
    freeResources();
}



void actionMenu(HWND hwndParent, MenuAction action)

//  Invoke an installed menu item.

{
    switch (action) {

    case ACTION_CREATE_CONF: {
        //  "Global|Mod: Create CIX Conference..." menu entry activated
        FARPROC lpProc=MakeProcInstance(createConfProc, lpGlobals->hInst);

 	StdDialogBox((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_CREATE_CONF", 
				 (HWND)hwndParent, (DLGPROC)lpProc);
	FreeProcInstance(lpProc);
        break;
    }

    case ACTION_EDIT_TOPIC: {
        //  "Global|Mod: Edit CIX Topics..." menu entry activated
        FARPROC lpProc=MakeProcInstance(editTopicProc, lpGlobals->hInst);

 	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_TOPIC_MAINTENANCE", (HWND)hwndParent,
                          (DLGPROC)lpProc, (LPARAM)(DWORD)lpGlobals->context.hConference);

	FreeProcInstance(lpProc);
        break;
    }

    case ACTION_CONFIGURE:
	//  "Settings|Moderate..." menu entry activated
	configureSetup(hwndParent);
        break;

    case ACTION_EDIT_FLIST: {
        //  "Topic|Mod: Edit File List..." menu entry activated
	BOOL bOK=TRUE;                  //  set to FALSE if Cancel in topic select

	if (lpGlobals->context.hConference && lpGlobals->context.hTopic && 	IsModerator( lpGlobals->context.hConference, NULL ) )
//	    enableFileListMenuItem(QUERY))
	    lpGlobals->contextFlist=lpGlobals->context;
	else {
	    //  Message window hasn't got focus or isn't on a conf the user
	    //  moderates - ask for conf/topic

		FARPROC lpProc=MakeProcInstance(topicProc, lpGlobals->hInst);

	    bOK=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_SELECT_TOPIC", (HWND)hwndParent,
				  (DLGPROC)lpProc, (LPARAM)(DWORD)&lpGlobals->contextFlist);

	    FreeProcInstance(lpProc);
	}

	if (bOK && lpGlobals->contextFlist.hConference!=NULL && lpGlobals->contextFlist.hTopic!=NULL) {
	    //  OK: Do that thang...
	    /*FARPROC lpProc=MakeProcInstance(flistProc, lpGlobals->hInst);

	    StdDialogBoxParam(lpGlobals->hInst, "DID_FLIST_FRAME", hwndParent,
			      lpProc, (DWORD)&lpGlobals->contextFlist);

	    FreeProcInstance(lpProc);*/
/*		if( IsWindow(lpGlobals->hwndMDIFlist) )
			StdOpenMDIDialog( lpGlobals->hwndMDIFlist );
		else */{
			int x, y;
			int cx, cy;

			x = GetPPInt( str_Window, str_Left, 10 );
			y = GetPPInt( str_Window, str_Top, 10 );
			cx = GetPPInt( str_Window, str_Width, 500 );
			cy = GetPPInt( str_Window, str_Height, 340 );
			lpGlobals->hwndMDIFlist = CreateMDIDialogParam( lpGlobals->hInst, str_ClassName, x, y, cx, cy, (LPARAM)NULL);
			if(IsWindow(lpGlobals->hwndMDIFlist))
				UpdateWindow( lpGlobals->hwndMDIFlist );
		}
	}

	break;
    }

    case ACTION_HELP:
	//  "Help|Moderate Help" menu entry activated
	HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HELP_INDEX, 0);
	break;

	 case ACTION_MAIN:
		Exec(hwndParent);
		break;

	 case ACTION_ABOUT:
		 {
			FARPROC lpProc=MakeProcInstance(aboutProcReal, lpGlobals->hInst);
			StdDialogBox((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_REAL_ABOUT", 
						 (HWND)hwndParent, (DLGPROC)lpProc);
			FreeProcInstance(lpProc);
		 }

    }
}

/* Thus function responds to the Ameol command event.
 * We locate the command ID in our menu table and
 * dispatch it.
 */
void _EXPORT CALLBACK Command(HWND hwndParent, WORD wID)
{
	UINT nMenu;

	getResources();
	for( nMenu = 0; nMenu < lpGlobals->cTotalMenus; nMenu++ )
		if( lpGlobals->ameolMenu[ nMenu ].menu.wID == wID )
			{
			actionMenu( hwndParent, lpGlobals->ameolMenu[ nMenu ].action );
			break;
			}
	freeResources();
}

BOOL _EXPORT CALLBACK Unload(WORD wExitType)

//  Ameol: Unload DLL function: tidy up. Unregister our Ameol event handler
//  and remove the edit flist menu entry.

{
    if (lpGlobals) {
	if (lpGlobals->lpEventProc) {
	    //  Unregister event handler
	    UnregisterEvent(AE_CONNECTEND, lpGlobals->lpEventProc);
	    UnregisterEvent(AE_CONNECTSTART, lpGlobals->lpEventProc);
	    UnregisterEvent(AE_DELETETOPIC, lpGlobals->lpEventProc);
	    UnregisterEvent(AE_DELETECONFERENCE, lpGlobals->lpEventProc);
	    UnregisterEvent(AE_MSGCHANGE, lpGlobals->lpEventProc);

		if( lpGlobals->AmeolVerInfo.nVersion != 1 )
			UnregisterEvent(AE_CONFIGADDONDLG, lpGlobals->lpEventProc);

		FreeProcInstance(lpGlobals->lpEventProc);
	    lpGlobals->lpEventProc=NULL;
	}

	removeAmeolMenus();
    }

    return(TRUE);
}



/* Get standard fonts according to the Ameol standard/small display size option.
 */
void getResources(void)
{
	BOOL bStdFont;
	LOGFONT lfNormal;
	LOGFONT lfDescEdit;

	/* Get the Ameol 8pt helv normal and bold fonts. If stdfont is
	 * non-zero, get the 10pt size ones.
	 */
	bStdFont = AmGetPrivateProfileBool( "Settings", "stdfont", FALSE, setup.szIniFile );
	if( bStdFont )
		{
		lpGlobals->hfNormal = GetAmeolFont(7);
		lpGlobals->hfBold = GetAmeolFont(8);
		lpGlobals->hfDescEdit = GetAmeolFont(7);
		}
	else
		{
		lpGlobals->hfNormal = GetAmeolFont(5);
		lpGlobals->hfBold = GetAmeolFont(6);
		lpGlobals->hfDescEdit = GetAmeolFont(5);
		}

	/* Get the LOGFONT details for the two Ameol fonts we retrieved.
	 */
	lpGlobals->hfMessageEdit = GetAmeolFont( 0 );
	GetObject( lpGlobals->hfMessageEdit, sizeof(LOGFONT), &lfDescEdit );
	GetObject( lpGlobals->hfNormal, sizeof(LOGFONT), &lfNormal );

	/* Make copies of them.
	 */
	lfDescEdit.lfHeight = lfNormal.lfHeight;
	lfDescEdit.lfWidth = 0;
	lfDescEdit.lfWeight = FW_NORMAL;
//	lpGlobals->hfDescEdit = CreateFontIndirect( &lfDescEdit );
	lfDescEdit.lfWeight = FW_BOLD;
	lpGlobals->hfBoldEdit = CreateFontIndirect( &lfDescEdit );
}



void freeResources(void)

//  Free resources we have created

{
    DeleteObject(lpGlobals->hfBoldEdit);
//    DeleteObject(lpGlobals->hfDescEdit);
}



BOOL enableFileListMenuItem(Enable enable)

//  Change the state of the edit flist menu item.

{
    static BOOL bEnabled=TRUE;      //  Previous state, so we don't change unless we need to

	return TRUE;

    switch (enable) {

    case ENABLE:
        if (!bEnabled) {
            EnableMenuItem(lpGlobals->lpFlistMenuItem->menu.hMenu,
                           lpGlobals->lpFlistMenuItem->menu.wID,
                           MF_BYCOMMAND|MF_ENABLED);

            bEnabled=TRUE;
        }

        break;

    case DISABLE:
        if (bEnabled) {
            EnableMenuItem(lpGlobals->lpFlistMenuItem->menu.hMenu,
                           lpGlobals->lpFlistMenuItem->menu.wID,
                           MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

            bEnabled=FALSE;
        }

        break;

    }

    return(bEnabled);
}

/* This function handles events from the main
 * Ameol program.
 */
BOOL _EXPORT CALLBACK eventProc( int wEvent, LPARAM lParam1, LPARAM lParam2 )
{
	static BOOL bLostFocus = TRUE;
	static HTOPIC hPrevTopic = NULL;

	switch( wEvent )
		{
		case AE_CONFIGADDONDLG:
			{
				newConfigureSetup((HWND)lParam1);
				return TRUE;
			}
		case AE_MSGCHANGE:
			/* The Ameol displayed message has changed...
			 */
			if( lParam1== -1 )
				{
            /* Ameol message window closed/lost focus
				 */
				lpGlobals->context.hConference = NULL;
            lpGlobals->context.lpcConfName = NULL;
            lpGlobals->context.hTopic = NULL;
            lpGlobals->context.lpcTopicName = NULL;
            enableFileListMenuItem( ENABLE );
            bLostFocus = TRUE;
				}
			else {
            /* An Ameol message window is open and has focus, lParam1 is the
             * Ameol message handle, lParam2 is the topic handle
				 */
            lpGlobals->context.hTopic = (HTOPIC)lParam2;
            if( bLostFocus || lpGlobals->context.hTopic != hPrevTopic )
					{
					/* Message window just opened or regained focus, or current topic
					 * has changed
					 */
					lpGlobals->context.lpcTopicName = GetTopicName( lpGlobals->context.hTopic );
					lpGlobals->context.hConference = ConferenceFromTopic( lpGlobals->context.hTopic );
					lpGlobals->context.lpcConfName = GetConferenceName( lpGlobals->context.hConference );
					if( lstrcmpi( lpGlobals->context.lpcConfName, "mail" ) == 0 ||
						 lstrcmpi( lpGlobals->context.lpcConfName, "Usenet" ) == 0 ||
						 lstrcmpi( lpGlobals->context.lpcConfName, "News" ) == 0 )
							enableFileListMenuItem( DISABLE );
					else
						{
						/* Valid conference - check for moderator and non-local topic
						 */
						if( IsModerator( lpGlobals->context.hConference, NULL ) )
							{
							/* User moderates this conf - check topic is non-local
							 */
							TOPICINFO topicInfo;

							GetTopicInfo( lpGlobals->context.hTopic, &topicInfo );
							enableFileListMenuItem( topicInfo.wFlags & TF_LOCAL ? DISABLE : ENABLE );
							}
						else
							enableFileListMenuItem(DISABLE);
						}
					hPrevTopic = lpGlobals->context.hTopic;
					}
				bLostFocus = FALSE;
				}
        break;

		case AE_DELETECONFERENCE:
			/* A conference has beend deleted, so delete CTI files
			 * if appropriate.
			 */
			if( lParam1 > 0 )
				deleteConfFiles( (HCONF)lParam1 );
			break;

		case AE_DELETETOPIC:
			/* A topic has been deleted
			 * Delete .FLM, .FLD and .FLB files if they exist
			 */
			deleteTopicFiles( (HTOPIC)lParam1 );
			break;

		case AE_CONNECTSTART: 
			{
			/* Check to see if there are any topic data requests pending - add
			 * them to the outbasket if so
			 */
				if( lParam2 & (GS_PROCESSCIX|GS_DOWNLOADCIX) )
					getPendingTopicInfo( GetFocus() );
				break;
			}

		case AE_CONNECTEND:
        /* Process any topic data request files left from the last blink
		   */
			updateTopicInfo( GetFocus() );
			break;
		}
	return( TRUE );
}

static BOOL about_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    char szBuff[64];

    SetWindowFont(GetDlgItem(hwnd, CID_VERSIONTEXT), lpGlobals->hfNormal, FALSE);
    wsprintf(szBuff, "Version %s", ExpandVersion(verStruct.dwVersion));
    SetDlgItemText(hwnd, CID_VERSIONTEXT, szBuff);
    return(TRUE);
}



static LPCSTR about_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_ABOUT, id));
}



static void about_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HELP_INDEX, 0);
}



static void about_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_ABOUT_CONF:
        //  Simulate "Global|Mod: Create Forum..."
        actionMenu(hwnd, ACTION_CREATE_CONF);
        break;

    case CID_ABOUT_TOPIC:
        //  Simulate "Topic|Mod: Edit Topics..."
        actionMenu(hwnd, ACTION_EDIT_TOPIC);
        break;

    case CID_CONFIGURE:
        //  Simulate "Settings|Moderate..."
        actionMenu(hwnd, ACTION_CONFIGURE);
        break;

    case CID_ABOUT_FLIST:
        //  Simulate "Topic|Mod: Edit File List..."
        actionMenu(hwnd, ACTION_EDIT_FLIST);
	break;

    case CID_DLL_INFO: {
        //  Display version info of support DLLs
        FARPROC lpProc;

        lpProc=MakeProcInstance(dllInfoProc, lpGlobals->hInst);
        StdDialogBox((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_DLL_INFO", 
					 (HWND)hwnd, (DLGPROC)lpProc);
        FreeProcInstance(lpProc);
        break;
    }

    case CID_OK:
    case CID_CANCEL:
	EndDialog(hwnd, 0);
        break;

    }
}

static void aboutReal_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK:
    case CID_CANCEL:
	EndDialog(hwnd, 0);
        break;

    }
}


static LRESULT about_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

//  "About" box message handler.

{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, about_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, about_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, about_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, about_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}

static LRESULT aboutReal_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
		{
		HANDLE_MSG(hwnd, WM_COMMAND, aboutReal_OnCommand);
		HANDLE_MSG(hwnd, WM_INITDIALOG, aboutReal_OnInitDialog);

		default:
			return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));
		}
}

static LRESULT aboutReal_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    char szBuff[64];

    SetWindowFont(GetDlgItem(hwnd, CID_HCTOOLS_VERSION), lpGlobals->hfNormal, FALSE);
    wsprintf(szBuff, "Version %s", ExpandVersion(verStruct.dwVersion));
    SetDlgItemText(hwnd, CID_VERSIONTEXT, szBuff);
	 return( TRUE );
}

BOOL _EXPORT CALLBACK aboutProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=about_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

BOOL _EXPORT CALLBACK aboutProcReal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=aboutReal_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

static BOOL dllInfo_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    char szBuff[64];

    SetWindowFont(GetDlgItem(hwnd, CID_HCTOOLS_VERSION), lpGlobals->hfNormal, FALSE);
    wsprintf(szBuff, "Version %s", ExpandVersion(HCToolsVersion()));
    SetDlgItemText(hwnd, CID_HCTOOLS_VERSION, szBuff);
    SetWindowFont(GetDlgItem(hwnd, CID_WINHORUS_VERSION), lpGlobals->hfNormal, FALSE);
    wsprintf(szBuff, "Version %s", ExpandVersion(WinHorusVersion()));
    SetDlgItemText(hwnd, CID_WINHORUS_VERSION, szBuff);
    return(TRUE);
}



static LPCSTR dllInfo_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_DLL_INFO, id));
}



static void dllInfo_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK:
    case CID_CANCEL:
        EndDialog(hwnd, 0);
        break;

    }
}



static LRESULT dllInfo_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

//  "DLL Information" box message handler.

{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, dllInfo_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, dllInfo_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, dllInfo_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK dllInfoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=dllInfo_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

/* This new function handles the QueryAddon interface
 * supported in Ameol2.
 */
BOOL _EXPORT CALLBACK QueryAddon( int qryType, LPVOID qryData )
{
	BOOL fRet;

	fRet = FALSE;
	switch( qryType )
		{
		case QUERY_SUPPORT_URL: {
			SUPPORT_URL FAR * lpsu;

			/* The Report addon wants to know to where we send
			 * Moderate related bug reports.
			 */
			lpsu = (SUPPORT_URL FAR *)qryData;
			if( lpsu->fCIXService )
				strcpy( lpsu->lpURLStr, "cix:cix.support/ameol" );
			else
				strcpy( lpsu->lpURLStr, "mailto:support@cix.uk" );
			fRet = TRUE;
			break;
			}

		case QUERY_CMD_BUTTON_BITMAP: {
			LPCMDBUTTONBITMAP	lpCmdBtnBmp;
			int i;

			/* Send bitmap IDs on request.
			 */
			lpCmdBtnBmp = (LPCMDBUTTONBITMAP)qryData;
			lpCmdBtnBmp->hLib = lpGlobals->hInst;
			for( i = 0; i < TOTAL_ACTIONS && !fRet; i++ )
				if( lpCmdBtnBmp->wID == lpGlobals->ameolMenu[ i ].menu.wID && lpGlobals->ameolMenu[ i ].fHasButton )
					{
					fRet = TRUE;
					if( lpCmdBtnBmp->cxRequired == 16 )
						lpCmdBtnBmp->uBitmap = lpGlobals->ameolMenu[ i ].uSmallTool;
					else
						lpCmdBtnBmp->uBitmap = lpGlobals->ameolMenu[ i ].uBigTool;
					}
			break;
			}
      }
   return( fRet );
}
