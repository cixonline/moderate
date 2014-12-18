#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "flist.h"
#include "edit.h"
#include "strftime.h"


typedef struct {
    LPFlistData lpFlistData;
    int nPtr;
    int nCount;
    LPEditRange lpEditRange;
    LPFlistEntry lpFlistEntry;
    WORD wEditControl;
    LPEditControl lpEditControl;
} EditData, FAR *LPEditData;

void InflateWnd( HWND, int, int );
void MoveWnd( HWND, int, int );

static int initEdit(HWND hwnd, LPEditData lpEditData)
{
    LPINT lpFlistSelected=lpEditData->lpFlistData->lpFlistSelected;
    int nFlistPtr=1;
    int nRangePtr=0;

    do {
	int nFlistIndex=lpFlistSelected[nFlistPtr++];
	LPFlistEntry lpFlistEntry=flistGetEntry(lpEditData->lpFlistData, nFlistIndex);
	WORD bHoldFlag=lpFlistEntry->flistFlags&FLIST_FLAG_HOLD;

	lpEditData->lpEditRange[nRangePtr].first=nFlistIndex;

	switch (lpFlistEntry->flistSelect) {

	case FLIST_FROM_FILEPOOL:
	case FLIST_FROM_FDIR:
	case FLIST_FROM_MAIL_DIR:
	case FLIST_FROM_UPLOAD:
	case FLIST_FROM_NOTIFICATION: {
	    LPFlistEntry lpNextEntry;

	    for (lpNextEntry=lpFlistEntry->flistNext;
		 lpNextEntry &&
		 lpNextEntry->flistSelect==FLIST_TABBED &&
		 (lpNextEntry->flistFlags&FLIST_FLAG_HOLD)==bHoldFlag;
		 lpNextEntry=lpNextEntry->flistNext) {
		if (nFlistPtr<= *lpFlistSelected && lpFlistSelected[nFlistPtr]==nFlistIndex+1)
		    nFlistPtr++;

		nFlistIndex++;
	    }

	    lpEditData->lpEditRange[nRangePtr].last=nFlistIndex;
	    break;
	}

	case FLIST_MEMO:
	case FLIST_COMMENT:
	case FLIST_TABBED: {
	    LPFlistEntry lpNextEntry;

	    for (lpNextEntry=lpFlistEntry->flistNext;
		 lpNextEntry &&
		 nFlistPtr<= *lpFlistSelected && lpFlistSelected[nFlistPtr]==nFlistIndex+1 &&
		 lpNextEntry->flistSelect==lpFlistEntry->flistSelect &&
		 (lpNextEntry->flistFlags&FLIST_FLAG_HOLD)==bHoldFlag;
		 lpNextEntry=lpNextEntry->flistNext) {
		nFlistPtr++;
		nFlistIndex++;
	    }

	    lpEditData->lpEditRange[nRangePtr].last=nFlistIndex;
	    break;
	}

	}

	nRangePtr++;
    } while (nFlistPtr<= *lpFlistSelected);

    return(nRangePtr);
}



static void descPrelude(HWND hwnd, LPEditData lpEditData)
{
    int nFlistIndex=lpEditData->lpEditRange[lpEditData->nPtr].first;
    FlistSelect flistSelect;
    char szFlistText[1024];
    LPCSTR lpcHoldFlag;
    char szDesc[32];
    LPFlistEntry lpNextEntry=flistGetEntry(lpEditData->lpFlistData, nFlistIndex);

    lpEditData->lpFlistEntry=lpNextEntry;
    flistSelect=lpNextEntry->flistSelect;
    lpcHoldFlag=lpNextEntry->flistFlags&FLIST_FLAG_HOLD ? " - hold" : "";
    EnableWindow(GetDlgItem(hwnd, CID_PREVIOUS), lpEditData->nPtr>0);
    EnableWindow(GetDlgItem(hwnd, CID_NEXT), lpEditData->nPtr<lpEditData->nCount-1);
    EnableWindow(GetDlgItem(hwnd, CID_UNDO), FALSE);
    lstrcpy(szFlistText, lpNextEntry->flistDescription);

    while ((WORD) ++nFlistIndex <= lpEditData->lpEditRange[lpEditData->nPtr].last) 
	{
		lpNextEntry=lpNextEntry->flistNext;
		if(lstrlen(szFlistText)+2 < 1023)
			lstrcat(szFlistText, "\r\n");
		if(lstrlen(szFlistText)+lstrlen(lpNextEntry->flistDescription) < 1023)
			lstrcat(szFlistText, lpNextEntry->flistDescription);
    }

    switch (flistSelect) 
	{

		case FLIST_FROM_FILEPOOL:
		case FLIST_FROM_FDIR:
		case FLIST_FROM_MAIL_DIR:
		case FLIST_FROM_UPLOAD:
		case FLIST_FROM_NOTIFICATION: 
			{
				ShowWindow(GetDlgItem(hwnd, CID_COMMENT_TEXT), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_COMMENT_TEXT), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_COMMENT_TYPE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TABBED_TEXT), SW_HIDE);

				ShowWindow(GetDlgItem(hwnd, CID_FILENAME), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_FILENAME), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_SIZE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_SIZE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_DATE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_DATE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_SOURCE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_SOURCE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_FDESCRIPTION), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_DESCRIPTION), SW_SHOW);

				lpEditData->wEditControl=CID_FDESCRIPTION;
				SetDlgItemText(hwnd, CID_FDESCRIPTION, szFlistText);
				SetDlgItemText(hwnd, CID_FILENAME, lpEditData->lpFlistEntry->flistName);

				if (lpEditData->lpFlistEntry->flistSource &&
				lpEditData->lpFlistEntry->flistSource->fdirSize>=0) 
				{
					char szBuff[32];

					EnableWindow(GetDlgItem(hwnd, CID_FILE_DATE), TRUE);
					wsprintf(szBuff, "%ld", lpEditData->lpFlistEntry->flistSource->fdirSize);
					SetDlgItemText(hwnd, CID_SIZE, szBuff);

					strftime(szBuff, 31, "%d%b%Y %I:%M %p",
						 localtime(&lpEditData->lpFlistEntry->flistSource->fdirTimestamp));

					SetDlgItemText(hwnd, CID_DATE, szBuff);
				} 
				else 
				{
					EnableWindow(GetDlgItem(hwnd, CID_FILE_DATE), FALSE);
					SetDlgItemText(hwnd, CID_SIZE, "?");
					SetDlgItemText(hwnd, CID_DATE, "?");
				}

				switch (flistSelect) 
				{

					case FLIST_FROM_FILEPOOL:
						wsprintf(szDesc, "filepool%s", lpcHoldFlag);
						break;

					case FLIST_FROM_FDIR:
						wsprintf(szDesc, "file directory%s", lpcHoldFlag);
						break;

					case FLIST_FROM_MAIL_DIR:
						wsprintf(szDesc, "mail directory%s", lpcHoldFlag);
						break;

					case FLIST_FROM_UPLOAD:
						wsprintf(szDesc, "upload%s", lpcHoldFlag);
						break;

					case FLIST_FROM_NOTIFICATION:
						wsprintf(szDesc, "notification%s", lpcHoldFlag);
						break;

				}

				SetDlgItemText(hwnd, CID_SOURCE, szDesc);
				break;
			}

		case FLIST_MEMO:
		case FLIST_COMMENT:
			{
				ShowWindow(GetDlgItem(hwnd, CID_FILENAME), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_FILENAME), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_SIZE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_SIZE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_DATE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_DATE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_SOURCE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_SOURCE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_FDESCRIPTION), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_DESCRIPTION), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TABBED_TEXT), SW_HIDE);

				ShowWindow(GetDlgItem(hwnd, CID_COMMENT_TEXT), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_COMMENT_TEXT), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_COMMENT_TYPE), SW_SHOW);

				lpEditData->wEditControl=CID_COMMENT_TEXT;
				SetDlgItemText(hwnd, CID_COMMENT_TEXT, szFlistText);
				EnableWindow(GetDlgItem(hwnd, CID_FILE_DATE), FALSE);

				switch (flistSelect) 
				{

					case FLIST_MEMO:
						SetDlgItemText(hwnd, CID_COMMENT_TYPE, "memo");
						break;

					case FLIST_COMMENT:
						wsprintf(szDesc, "comment%s", lpcHoldFlag);
						SetDlgItemText(hwnd, CID_COMMENT_TYPE, szDesc);
						break;

				}

				break;
			}
		case FLIST_TABBED:
			{
				ShowWindow(GetDlgItem(hwnd, CID_FILENAME), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_FILENAME), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_SIZE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_SIZE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_DATE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_DATE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_SOURCE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_SOURCE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_FDESCRIPTION), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_TAG_DESCRIPTION), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CID_COMMENT_TEXT), SW_HIDE);

				ShowWindow(GetDlgItem(hwnd, CID_TAG_COMMENT_TEXT), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_COMMENT_TYPE), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CID_TABBED_TEXT), SW_SHOW);

				lpEditData->wEditControl=CID_TABBED_TEXT;
				SetDlgItemText(hwnd, CID_TABBED_TEXT, szFlistText);
				EnableWindow(GetDlgItem(hwnd, CID_FILE_DATE), FALSE);
				wsprintf(szDesc, "tabbed comment%s", lpcHoldFlag);
				SetDlgItemText(hwnd, CID_COMMENT_TYPE, szDesc);
				break;
			}
	}
	lpEditData->lpEditControl=editPrelude(hwnd, lpEditData->wEditControl, lpGlobals->hfDescEdit, 0);
}



static void descPostlude(HWND hwnd, LPEditData lpEditData, BOOL bSave)
{
    if (bSave && editChanged(lpEditData->lpEditControl, 0)) 
	{
		int nFlistIndex=lpEditData->lpEditRange[lpEditData->nPtr].first;
		LPFlistEntry lpFlistEntry=flistGetEntry(lpEditData->lpFlistData, nFlistIndex);
		FlistSelect flistSelect=lpFlistEntry->flistSelect;
		HWND hwndFlist=lpEditData->lpFlistData->hwndFlist;
		static char szFlistText[1024];
		char szDescription[128];
		int nIncrement=0;
		RECT rect;

		editFormat(lpEditData->lpEditControl, TRUE);
		editExtract(lpEditData->lpEditControl, szFlistText, 1023);
		editExtract(lpEditData->lpEditControl, szDescription, 0);

		for(;;) 
		{
			lstrcpy(lpFlistEntry->flistDescription, szDescription);

			if (editExtract(lpEditData->lpEditControl, szDescription, 0)<0)
				break;

			if ((WORD) ++nFlistIndex <= lpEditData->lpEditRange[lpEditData->nPtr].last)
			{
				lpFlistEntry=flistGetEntry(lpEditData->lpFlistData, nFlistIndex);
			}
			else 
			{
				LPFlistEntry lpPrevEntry=lpFlistEntry;
				int nAddPosition;

				nIncrement++;
				lpFlistEntry=(LPFlistEntry)gmalloc(sizeof(FlistEntry));
				lpFlistEntry->flistPrevious=lpPrevEntry;
				lpFlistEntry->flistNext=lpPrevEntry->flistNext;

				if (lpFlistEntry->flistNext) 
				{
					lpFlistEntry->flistNext->flistPrevious=lpFlistEntry;
					nAddPosition=nFlistIndex;
				} 
				else
					nAddPosition= -1;

				lpPrevEntry->flistNext=lpFlistEntry;
				lpFlistEntry->flistFlags=lpPrevEntry->flistFlags;

				switch (flistSelect) 
				{
					case FLIST_FROM_FILEPOOL:
					case FLIST_FROM_FDIR:
					case FLIST_FROM_MAIL_DIR:
					case FLIST_FROM_UPLOAD:
					case FLIST_FROM_NOTIFICATION:
					case FLIST_TABBED:
						lpFlistEntry->flistSelect=FLIST_TABBED;
						break;

					case FLIST_MEMO:
					case FLIST_COMMENT:
						lpFlistEntry->flistSelect=flistSelect;
						break;

				}

				*lpFlistEntry->flistDescription='\0';

				if (hwndFlist) 
				{
					ListBox_InsertItemData(hwndFlist, nAddPosition, lpFlistEntry);
					ListBox_SetSel(hwndFlist, TRUE, nFlistIndex);
				}
			}
		}

		if ((WORD) ++nFlistIndex <= lpEditData->lpEditRange[lpEditData->nPtr].last) 
		{
			int nPtr;

			for (nPtr=nFlistIndex; (WORD) nPtr <= lpEditData->lpEditRange[lpEditData->nPtr].last; nPtr++) 
			{
				nIncrement--;
				lpFlistEntry=flistGetEntry(lpEditData->lpFlistData, nFlistIndex);

				if (hwndFlist)
					ListBox_DeleteString(hwndFlist, nFlistIndex);

				lpFlistEntry->flistPrevious->flistNext=lpFlistEntry->flistNext;

				if (lpFlistEntry->flistNext)
					lpFlistEntry->flistNext->flistPrevious=lpFlistEntry->flistPrevious;

				gfree(lpFlistEntry);
				lpFlistEntry = NULL;
			}
		}

		if (hwndFlist)
			ListBox_SetCaretIndex(hwndFlist, lpEditData->lpEditRange[lpEditData->nPtr].first);

		if (nIncrement) 
		{
			int nPtr=lpEditData->nPtr;

			lpEditData->lpEditRange[nPtr].last+=nIncrement;

			while (++nPtr<lpEditData->nCount) 
			{
				lpEditData->lpEditRange[nPtr].first+=nIncrement;
				lpEditData->lpEditRange[nPtr].last+=nIncrement;
			}

			lpEditData->lpFlistData->flistEntries+=nIncrement;
		}

		if (hwndFlist) 
		{
			GetWindowRect(hwndFlist, &rect);
			InvalidateRect(hwndFlist, &rect, TRUE);
		}

		lpEditData->lpFlistData->flistChanged=TRUE;
    }

    editPostlude(lpEditData->lpEditControl);
}



static BOOL edit_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	LPEditData lpEditData=InitDlgData(hwnd, EditData);
	TEXTMETRIC tm;
	HFONT hOldFont;
	HWND hwndEdit;
	int cxEdit;
	RECT rc;
	HDC hdc;

	lpEditData->lpFlistData=(LPFlistData)lParam;
	SetWindowFont(GetDlgItem(hwnd, CID_FILENAME), lpGlobals->hfNormal, FALSE);
	SetWindowFont(GetDlgItem(hwnd, CID_SIZE), lpGlobals->hfNormal, FALSE);
	SetWindowFont(GetDlgItem(hwnd, CID_DATE), lpGlobals->hfNormal, FALSE);
	SetWindowFont(GetDlgItem(hwnd, CID_SOURCE), lpGlobals->hfNormal, FALSE);
	SetWindowFont(GetDlgItem(hwnd, CID_COMMENT_TYPE), lpGlobals->hfNormal, FALSE);
	SetWindowFont(GetDlgItem(hwnd, CID_FDESCRIPTION),lpGlobals->hfDescEdit, TRUE);
//	SetWindowFont(GetDlgItem(hwnd, CID_FDESCRIPTION),lpGlobals->hfNormal, TRUE);
	lpEditData->lpEditRange=(LPEditRange)gmalloc(sizeof(EditRange)*lpEditData->lpFlistData->flistEntries);
	lpEditData->nCount=initEdit(hwnd, lpEditData);
	lpEditData->nPtr=0;
	descPrelude(hwnd, lpEditData);

	/* Make sure the description field is wide enough for 50 characters.
	 * 1. Determine required width.
	 */
	hdc = GetDC( hwnd );
	hOldFont = SelectFont( hdc, lpGlobals->hfDescEdit );
	GetTextMetrics( hdc, &tm );
	SelectFont( hdc, hOldFont );
	ReleaseDC( hwnd, hdc );
	cxEdit = ( tm.tmAveCharWidth * 53 ) + 6;

	/* 2. Find out current width
	 */
	hwndEdit = GetDlgItem( hwnd, CID_FDESCRIPTION );
	GetClientRect( hwndEdit, &rc );
	if( ( rc.right - rc.left ) < cxEdit )
		{
		int diff;

		/* 3. Poo, we're too short. Now extend the edit control
		 *    AND the dialog.
		 */
		diff = cxEdit - ( rc.right - rc.left );
		InflateWnd( hwnd, diff, 0 );
		InflateWnd( hwndEdit, diff, 0 );
		MoveWnd( GetDlgItem( hwnd, IDOK ), diff, 0 );
		}
	return(TRUE);
}

/* This function expands the specified window by the given amounts,
 * where negative values reduce the window size.
 */
void InflateWnd( HWND hwnd, int dx, int dy )
{
	RECT rc;

	GetWindowRect( hwnd, &rc );
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	SetWindowPos( hwnd, NULL, 0, 0, rc.right + dx, rc.bottom + dy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
}

/* This function moves the specified window by the given amounts,
 * where negative values are to the left and up.
 */
void MoveWnd( HWND hwnd, int dx, int dy )
{
	RECT rc;

	GetWindowRect( hwnd, &rc );
	ScreenToClient( GetParent( hwnd ), (LPPOINT)&rc );
	SetWindowPos( hwnd, NULL, rc.left + dx, rc.top + dy, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
}

static LPCSTR edit_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_FILE_DESCRIPTION, id));
}



static void edit_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_EDIT);
}



static void edit_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPEditData lpEditData=GetDlgData(hwnd, EditData);

    switch (id) {

    case CID_FDESCRIPTION:
    case CID_COMMENT_TEXT:
    case CID_TABBED_TEXT:
        if (codeNotify==EN_CHANGE &&
            !editChanged(lpEditData->lpEditControl, IDYES))
            EnableWindow(GetDlgItem(hwnd, CID_UNDO), TRUE);

        break;

    case CID_INSERT_DATE: {
        time_t now=time(NULL);
        char szBuff[64];

        strftime(szBuff, 63, setup.szDateFormat, localtime(&now));
        editInsertText(lpEditData->lpEditControl, szBuff);
        editFocus(lpEditData->lpEditControl);
        break;
    }

    case CID_FILE_DATE:
        if (lpEditData->lpFlistEntry->flistSource) {
            char szBuff[64];

            strftime(szBuff, 63, setup.szDateFormat,
                     localtime(&lpEditData->lpFlistEntry->flistSource->fdirTimestamp));

            editInsertText(lpEditData->lpEditControl, szBuff);
            editFocus(lpEditData->lpEditControl);
        }

        break;

    case CID_PREVIOUS:
        if (lpEditData->nPtr>0) {
            descPostlude(hwnd, lpEditData, TRUE);
            lpEditData->nPtr--;
            descPrelude(hwnd, lpEditData);
            editFocus(lpEditData->lpEditControl);
	}

        break;

    case CID_NEXT:
        if (lpEditData->nPtr<lpEditData->nCount-1) {
            descPostlude(hwnd, lpEditData, TRUE);
            lpEditData->nPtr++;
            descPrelude(hwnd, lpEditData);
            editFocus(lpEditData->lpEditControl);
        }

        break;

    case CID_UNDO:
        descPostlude(hwnd, lpEditData, FALSE);
        descPrelude(hwnd, lpEditData);
        editFocus(lpEditData->lpEditControl);
        break;

    case CID_OK:
        descPostlude(hwnd, lpEditData, TRUE);
		EndDialog(hwnd, 0);
		SetFocus(GetDlgItem(GetParent(hwnd), CID_FILE_LIST));
        break;

    case CID_CANCEL:
        descPostlude(hwnd, lpEditData, FALSE);
		EndDialog(hwnd, 0);
		SetFocus(GetDlgItem(GetParent(hwnd), CID_FILE_LIST));
        break;

    }
}



static void edit_OnDestroy(HWND hwnd)
{
    LPEditData lpEditData=GetDlgData(hwnd, EditData);

    gfree(lpEditData->lpEditRange);
	lpEditData->lpEditRange = NULL;
    FreeDlgData(hwnd);
}


static LRESULT edit_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, edit_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, edit_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, edit_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, edit_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, edit_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK editProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=edit_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

