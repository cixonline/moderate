#include <windows.h>
#include <windowsx.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "msgedit.h"
#include "flist.h"
#include "malloc.h"

extern char str_Window[9];
extern char str_NotifyClassName[17];
extern char str_NotifyLeft[19];
extern char str_NotifyTop[18];
extern char str_NotifyWidth[20];
extern char str_NotifyHeight[21];

#define  SC_FL_KEEPSESSION                       0x0001
#define  SC_FL_USELASTSESSION            0x0002
#define  SC_FL_DIALOG                            0x0004

#define	WritePPInt(lpkey,lpsect,value)	(AmWritePrivateProfileInt((lpkey),(lpsect),(value),setup.szIniFile))
#define	GetPPInt(lpkey,lpsect,value)	(AmGetPrivateProfileInt((lpkey),(lpsect),(value),setup.szIniFile))

#undef GetMDIData
#undef SetMDIData
#undef InitMDIData
#undef FreeMDIData

#define GetMDIData(hDlg, DataType) \
    (DataType)GetWindowLong((hDlg), MOD_DATA)

#define SetMDIData(hDlg, lpData) \
    SetWindowLong((hDlg), MOD_DATA, (DWORD)(LPVOID)(lpData))

#define InitMDIData(hDlg, DataType) \
    (SetMDIData((hDlg), gmallocz(sizeof(DataType))), \
     GetMDIData((hDlg), DataType))

#define FreeMDIData(hDlg) \
    gfree((LPVOID)GetWindowLong((hDlg), MOD_DATA))


void InflateWnd( HWND hDlg, int dx, int dy );
void MoveWnd( HWND hDlg, int dx, int dy );
HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE hInstance,
									  LPCSTR lpTemplateName,
									  int x, int y, int cx, int cy, LPARAM lParam);
LPSTR addSig(LPContext lpContext, LPSTR lpszText);


static BOOL MDImessageEdit_OnMove(HWND hWnd, int x, int y)
{
RECT rc;

	if( !IsIconic( hWnd ) && !IsMaximized( hWnd ) ) 
	{
		GetWindowRect( hWnd, &rc );
		ScreenToClient( GetParent( hWnd ), (LPPOINT)&rc );
		WritePPInt( str_Window, str_NotifyLeft, rc.left );
		WritePPInt( str_Window, str_NotifyTop, rc.top );
	}
	return FALSE;
}

static BOOL MDImessageEdit_OnSize(HWND hWnd, UINT state, int x, int y)
{
RECT rc;

	if( !IsIconic( hWnd ) && !IsMaximized( hWnd ) ) 
	{
		GetWindowRect( hWnd, &rc );
		WritePPInt( str_Window, str_NotifyWidth, rc.right - rc.left );
		WritePPInt( str_Window, str_NotifyHeight, rc.bottom - rc.top );
	}
	return( (BOOL)DefAmeolMDIDlgProc( hWnd, WM_SIZE, (WPARAM)(UINT)(state), (LPARAM)MAKELPARAM((x), (y)) ) );
}


static BOOL MDImessageEdit_OnInitDialog(HWND hwnd, LPCREATESTRUCT lParam)
{
	LPARAM ltParam;
	LPMDICREATESTRUCT lpMDIdi;
	LPMessageEditData lpMessageEditData;
	LPMessageEditData lpMessageEditData1;
	LPWINDINFO lpWindInfo;

	lpMDIdi	 = (LPMDICREATESTRUCT)((LPCREATESTRUCT)lParam)->lpCreateParams;
	ltParam  = (LPARAM) ((LPCREATESTRUCT) lParam)->lpCreateParams ;

	lpMessageEditData1  = (LPMessageEditData)((LPMDICREATESTRUCT)ltParam)->lParam ;

	lpMessageEditData = (LPMessageEditData)gmalloc(sizeof(MessageEditData));

	memcpy(lpMessageEditData, lpMessageEditData1, sizeof(MessageEditData));

    lpMessageEditData->lpEditControl=(LPEditControl)gmallocz(sizeof(EditControl));

#ifndef WIN32
	
	SetWindowLong(hwnd, MOD_DATA, (LONG)(LPMessageEditData)lpMessageEditData);

#else

	SetLastError(0);
	if(!SetWindowLong(hwnd, MOD_DATA, (LONG)(LPMessageEditData)lpMessageEditData))
	{
		if(GetLastError() != 0)
		{
			alert(hwnd, "Error Setting Window Data (MDImessageEdit_OnInitDialog)");
			return FALSE;
		}
	}

#endif

	if( !( lpWindInfo = _fmalloc( sizeof( WINDINFO ) ) ) )
		return( -1 );

	_fmemset( lpWindInfo, 0, sizeof( WINDINFO ) );

	SetWindowLong( hwnd, GWL_WNDPTR, (LONG)lpWindInfo );

	StdMDIDialogBox( hwnd, "AMEOL_SAY", lpMDIdi );

	GetClientRect(hwnd, &lpMessageEditData->rcOrg);
	lpMessageEditData->fHasRect = TRUE;

    ComboBox_AddString(GetDlgItem(hwnd, CID_CONF_NAME), lpMessageEditData->lpcConfName);
    ComboBox_AddString(GetDlgItem(hwnd, CID_TOPIC_NAME), lpMessageEditData->lpcTopicName);
	ComboBox_SetCurSel(GetDlgItem(hwnd, CID_CONF_NAME), 0); 
	ComboBox_SetCurSel(GetDlgItem(hwnd, CID_TOPIC_NAME), 0);

	EnableWindow(GetDlgItem(hwnd, CID_CONF_NAME), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_SIG), FALSE);

	SetWindowText(hwnd, "Moderate - File Upload Notification");
	SetWindowFont(GetDlgItem(hwnd, CID_MESSAGE_TEXT), lpGlobals->hfMessageEdit, FALSE);
//	ShowWindow(GetDlgItem(hwnd, CID_IMPORT), lpMessageEditData->bImportEnabled ? SW_SHOW : SW_HIDE);

	lpMessageEditData->changed = FALSE;

//	Edit_FmtLines(GetDlgItem(hwnd, CID_MESSAGE_TITLE), TRUE);

    if (lpMessageEditData->hoobMessage) 
	{
        SAYOBJECT sayObject;

        GetOutBasketObjectData(lpMessageEditData->hoobMessage, &sayObject);

        if (*sayObject.szTitle)
		    SetDlgItemText(hwnd, CID_MESSAGE_TITLE, lpMessageEditData->szTitle);

		Edit_SetText(GetDlgItem(hwnd, CID_MESSAGE_TEXT), GetObjectText(lpMessageEditData->hoobMessage));
    }
	else
	{
		Edit_SetText(GetDlgItem(hwnd, CID_MESSAGE_TITLE), lpMessageEditData->szTitle);
		Edit_SetText(GetDlgItem(hwnd, CID_MESSAGE_TEXT), lpMessageEditData->lpText);
	}

	lpWindInfo->hwndFocus = GetDlgItem(hwnd, CID_MESSAGE_TEXT);

	SetFocus(GetDlgItem(hwnd, CID_MESSAGE_TEXT));
	
	PostMessage( hwnd, WM_MYSETFOCUS, (WPARAM)GetDlgItem(hwnd, CID_MESSAGE_TEXT), 0L );
    
	return(TRUE);
}


static LPCSTR MDImessageEdit_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_MESSAGE_EDIT, id));
}

static void MDImessageEdit_OnAmHelp(HWND hwnd)
{
    LPMessageEditData lpMessageEditData=GetMDIData(hwnd, LPMessageEditData);

    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, lpMessageEditData->wHelpTag);
}

//static void MDImessageEdit_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
BOOL MDImessageEdit_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) 
	{
	case CID_MESSAGE_TITLE:
	case CID_MESSAGE_TEXT:
		{
			LPMessageEditData lpMessageEditData = GetMDIData(hwnd, LPMessageEditData);
			if(codeNotify == EN_CHANGE)
			{
				if(lpMessageEditData)
				{
					lpMessageEditData->changed = TRUE;
				}
			}
			break;
		}
	case IDD_HELP:
		{
			MDImessageEdit_OnAmHelp(hwnd);
			break;
		}

    case CID_IMPORT:
		{
			break;
		}

    case CID_OK: 
		{
			SAYOBJECT sayObject;
			LPMessageEditData lpMessageEditData = GetMDIData(hwnd, LPMessageEditData);
			LPWINDINFO lpWindInfo;
			int length;
			
			if(lpMessageEditData)
			{
				if(GetPPInt("Spelling", "AutoCheck", 0) == 1)
				{
					int cCode;

//					if( SpellCheckDocument( hwnd, GetDlgItem(hwnd, CID_MESSAGE_TITLE), SC_FL_KEEPSESSION ) == SP_OK )
//						SpellCheckDocument( hwnd, GetDlgItem(hwnd, CID_MESSAGE_TEXT), SC_FL_USELASTSESSION|SC_FL_DIALOG );

					cCode = SpellCheckDocument( hwnd, GetDlgItem(hwnd, CID_MESSAGE_TITLE), SC_FL_KEEPSESSION );
					if(cCode == SP_CANCEL)
					{
						break;
					}

					if(cCode != SP_FINISH)
					{
						if(SpellCheckDocument( hwnd, GetDlgItem(hwnd, CID_MESSAGE_TEXT), SC_FL_USELASTSESSION|SC_FL_DIALOG) == SP_CANCEL)
							break;
					}

/*
					cCode = SpellCheckDocument( hwnd, GetDlgItem(hwnd, CID_MESSAGE_TITLE), FALSE );
					if(cCode == SP_CANCEL)
					{
						break;
					}

					if(cCode != SP_FINISH)
					{
						if(SpellCheckDocument( hwnd, GetDlgItem(hwnd, CID_MESSAGE_TEXT), FALSE ) == SP_CANCEL)
							break;
					}
*/
				}

				Edit_GetText(GetDlgItem(hwnd, CID_MESSAGE_TITLE), lpMessageEditData->szTitle, LEN_TITLE);
				
				length = Edit_GetTextLength(GetDlgItem(hwnd, CID_MESSAGE_TEXT))+10;
				
				lpMessageEditData->lpText = (LPSTR)gmalloc(length);

				if(lpMessageEditData->lpText)
				{
					Edit_GetText(GetDlgItem(hwnd, CID_MESSAGE_TEXT), lpMessageEditData->lpText, length);

					if (lpMessageEditData->hoobMessage)
						RemoveObject(lpMessageEditData->hoobMessage);

					InitObject(&sayObject, OT_SAY, SAYOBJECT);
					lstrcpy(sayObject.szConfName, lpMessageEditData->lpcConfName);
					lstrcpy(sayObject.szTopicName, lpMessageEditData->lpcTopicName);
					lstrcpy(sayObject.szTitle, lpMessageEditData->szTitle);
					lpMessageEditData->hoobMessage=PutObject(NULL, &sayObject, lpMessageEditData->lpText);
					gfree(lpMessageEditData->lpEditControl);
					lpMessageEditData->lpEditControl = NULL;
				}
			}

			lpWindInfo = (LPWINDINFO)GetWindowLong( hwnd, GWL_WNDPTR );
			if(lpWindInfo)
			{
				_ffree( lpWindInfo );
				lpWindInfo = NULL;
			}

			SetWindowLong( hwnd, GWL_WNDPTR, 0L );
			gfree(lpMessageEditData);
			lpMessageEditData = NULL;
			StdEndMDIDialog( hwnd );
			return TRUE;
		}

    case CID_CANCEL:
		{
			LPMessageEditData lpMessageEditData = GetMDIData(hwnd, LPMessageEditData);
			LPWINDINFO lpWindInfo;
			int confirm;

			if(lpMessageEditData->changed == TRUE)
				confirm = query(hwnd, MB_YESNOCANCEL, "Save Changes To Message");
			else
				confirm = IDNO;

			if(confirm == IDNO)
			{
				if(lpMessageEditData)
				{
					if(lpMessageEditData->lpEditControl)
					{
						gfree(lpMessageEditData->lpEditControl);
						lpMessageEditData->lpEditControl = NULL;
					}
					gfree(lpMessageEditData);
					lpMessageEditData = NULL;
				}

				lpWindInfo = (LPWINDINFO)GetWindowLong( hwnd, GWL_WNDPTR );
				if(lpWindInfo)
				{
					_ffree( lpWindInfo );
					lpWindInfo = NULL;
				}

				SetWindowLong( hwnd, GWL_WNDPTR, 0L );
				StdEndMDIDialog( hwnd );
				return TRUE;
			}
			
			if(confirm == IDYES)
			{
				FORWARD_WM_COMMAND(hwnd, CID_OK, 0, 0, SendMessage);
				break;
			}
			
			if(confirm == IDCANCEL)
			{
				break;
			}
			break;
		}
    }
	return FALSE;
}

BOOL _EXPORT CALLBACK MDImessageEditProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
BOOL fPassToDCP;

	fPassToDCP = FALSE;
	switch( uMsg )
	{

		HANDLE_MSG(hDlg, WM_CREATE,  MDImessageEdit_OnInitDialog);
		HANDLE_MSG(hDlg, WM_SIZE,    MDImessageEdit_OnSize);
		HANDLE_MSG(hDlg, WM_MOVE,    MDImessageEdit_OnMove);
//		HANDLE_MSG(hDlg, WM_COMMAND, MDImessageEdit_OnCommand);

		case WM_COMMAND:
			{
				if( !HANDLE_WM_COMMAND_EX( hDlg, wParam, lParam, MDImessageEdit_OnCommand ) )
					fPassToDCP = TRUE;
				break;
			}
		case WM_GETMINMAXINFO:
			{
				MINMAXINFO FAR * lpmmi = (MINMAXINFO FAR *) lParam; 
				POINT		 pt;
				LPMessageEditData lpMessageEditData=GetMDIData(hDlg, LPMessageEditData);

				if (lpMessageEditData && lpMessageEditData->fHasRect) 
				{
					pt.x = lpMessageEditData->rcOrg.right - lpMessageEditData->rcOrg.left;
					pt.y = lpMessageEditData->rcOrg.bottom - lpMessageEditData->rcOrg.top;
					lpmmi->ptMinTrackSize = pt; 
					DefAmeolMDIDlgProc(hDlg, uMsg, wParam, lParam);
					lpmmi->ptMinTrackSize = pt; 
				}
				break;
			}

		case WM_MYSETFOCUS: 
			{
				LPWINDINFO lpWindInfo;
				lpWindInfo = NULL;
				lpWindInfo = (LPWINDINFO)GetWindowLong( hDlg, GWL_WNDPTR );
				if(lpWindInfo != NULL)
				{
					if(IsWindow((HWND)wParam))
					{
						if(lpWindInfo != NULL)
						{
							lpWindInfo->hwndFocus = (HWND)wParam;
							SetFocus(lpWindInfo->hwndFocus);
						}
					}
				}
				break;
			}

		case WM_CLOSE:
			{
				SendMessage( hDlg, WM_COMMAND, CID_CANCEL, 0 );
				return( 0L );
			}

		case WM_MDIACTIVATE:
			{
				LPWINDINFO lpWindInfo;
				lpWindInfo = NULL;
				lpWindInfo = (LPWINDINFO)GetWindowLong( hDlg, GWL_WNDPTR );
				if(lpWindInfo != NULL)
				{
					if(hDlg == (HWND) lParam)
					{
						SetFocus(lpWindInfo->hwndFocus);
					}
					else
						SetFocus(GetDlgItem(hDlg, CID_MESSAGE_TEXT));
				}
				fPassToDCP = TRUE;
				break;
			}

		case WM_ADJUSTWINDOWS: 
			{
				int dx, dy;

				dx = (int)(short)LOWORD( lParam );
				dy = (int)(short)HIWORD( lParam );

				MoveWnd( GetDlgItem( hDlg, CID_OK ),               dx,  0 );
				MoveWnd( GetDlgItem( hDlg, CID_CANCEL ),           dx,  0 );
				MoveWnd( GetDlgItem( hDlg, IDD_HELP ),             dx,  0 );

				InflateWnd( GetDlgItem( hDlg, CID_MESSAGE_TEXT),     dx, dy );
				InflateWnd( GetDlgItem( hDlg, CID_MESSAGE_TITLE),    dx, 0 );

				fPassToDCP = TRUE;

				break;
			}
		default:	
			{
				fPassToDCP = TRUE;
				break;
			}
	}
	if( fPassToDCP )
	{
		return( (BOOL)DefAmeolMDIDlgProc( hDlg, uMsg, wParam, lParam ) );
	}
	return( FALSE );
}
