#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include "ameolapi.h"
#include "hctools.h"
#include "winhorus.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "flist.h"
#include "strftime.h"


typedef struct {
    LPFlistData lpFlistData;
    HMENU hMenuSort;
    HWND insertFdirHandle;
    LPPFdirEntry fdirHeader;
    int fdirEntries;
    int fdirAvailable;
} InsertFdirData, FAR *LPInsertFdirData;



static void drawFdirEntry(const DRAWITEMSTRUCT FAR * lpDis)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rcName, rcSize, rcDate, rcTime;
    LPFdirEntry thisEntry=(LPFdirEntry)lpDis->itemData;
    int width=lpDis->rcItem.right-lpDis->rcItem.left+1;
    char s[32];

    CopyRect(&rcName, &lpDis->rcItem);
    rcName.left+=10;
    rcName.right=rcName.left+(width*3)/10;

    CopyRect(&rcSize, &lpDis->rcItem);
    rcSize.left=rcName.right+1;
    rcSize.right=rcSize.left+width/5;

    CopyRect(&rcDate, &lpDis->rcItem);
    rcDate.left=rcSize.right+15;
    rcDate.right=rcDate.left+width/5;

    CopyRect(&rcTime, &lpDis->rcItem);
    rcTime.left=rcDate.right+5;
    rcTime.right-=2;

    if (lpDis->itemState&ODS_SELECTED) {
	newBg=GetSysColor(COLOR_HIGHLIGHT);
	newFg=GetSysColor(COLOR_HIGHLIGHTTEXT);
    } else {
	newBg=GetSysColor(COLOR_WINDOW);
	newFg=GetSysColor(COLOR_WINDOWTEXT);
    }

    oldBg=(COLORREF)SetBkColor(lpDis->hDC, newBg);
    oldFg=(COLORREF)SetTextColor(lpDis->hDC, newFg);
    hBrush=CreateSolidBrush(newBg);
    FillRect(lpDis->hDC, &lpDis->rcItem, hBrush);
    DeleteObject(hBrush);

    if (thisEntry) {
	SelectObject(lpDis->hDC, lpGlobals->hfBold);
	DrawText(lpDis->hDC, thisEntry->fdirName, -1, &rcName, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
	SelectObject(lpDis->hDC, lpGlobals->hfNormal);
	wsprintf(s, "%ld", thisEntry->fdirSize);
	DrawText(lpDis->hDC, s, -1, &rcSize, DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
	strftime(s, 31, "%d%b%Y", localtime(&thisEntry->fdirTimestamp));
	DrawText(lpDis->hDC, s, -1, &rcDate, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
	strftime(s, 31, "%I:%M %p", localtime(&thisEntry->fdirTimestamp));
	DrawText(lpDis->hDC, s, -1, &rcTime, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
    }

    if (lpDis->itemState&ODS_FOCUS)
	DrawFocusRect(lpDis->hDC, &lpDis->rcItem);

    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



static void insertFdirPrelude(LPInsertFdirData lpInsertFdirData)
{
    LPFdirEntry thisEntry= *lpInsertFdirData->fdirHeader;
    int count=0;

    ListBox_ResetContent(lpInsertFdirData->insertFdirHandle);

    while (thisEntry) {
	if (!thisEntry->fdirReferences) {
            ListBox_AddItemData(lpInsertFdirData->insertFdirHandle, thisEntry);
            count++;
        }

	thisEntry=thisEntry->fdirNext;
    }

    ListBox_SetSel(lpInsertFdirData->insertFdirHandle, TRUE, 0);
    lpInsertFdirData->fdirAvailable=count;
}



static BOOL insertFdir_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPInsertFdirData lpInsertFdirData=InitDlgData(hwnd, InsertFdirData);
    LPInsertControl lpInsertControl;

    lpInsertFdirData->lpFlistData=(LPFlistData)lParam;
    lpInsertFdirData->hMenuSort=LoadMenu(lpGlobals->hInst, "MID_SORT_FDIR");
    lpInsertFdirData->hMenuSort=GetSubMenu(lpInsertFdirData->hMenuSort, 0);
    SetWindowFont(GetDlgItem(hwnd, CID_INSERT_CAPTION), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_UPDATE_DATE), lpGlobals->hfNormal, FALSE);
    lpInsertControl= &lpInsertFdirData->lpFlistData->insertControl;

    switch (lpInsertControl->position) {

    case CID_INSERT:
	SetDlgItemText(hwnd, CID_OK, "&Insert");
	break;

    case CID_APPEND:
	SetDlgItemText(hwnd, CID_OK, "&Append");
	break;

    }

    switch (lpInsertControl->source) {

    case FLIST_FROM_FDIR:
	SetWindowText(hwnd, "File Directory");
	ShowWindow(GetDlgItem(hwnd, CID_UPDATE), SW_HIDE);

	switch (lpInsertControl->position) {

	case CID_INSERT:
	    SetDlgItemText(hwnd, CID_INSERT_CAPTION, "Select orphan &files to insert");
	    break;

	case CID_APPEND:
	    SetDlgItemText(hwnd, CID_INSERT_CAPTION, "Select orphan &files to append");
	    break;

	}

	if (lpInsertFdirData->lpFlistData->fdirOrphans>0) {
	    char s[64];

	    strftime(s, 63, "Last updated at %I:%M %p on %d%b%Y", localtime(&lpInsertFdirData->lpFlistData->fdirTime));
	    SetDlgItemText(hwnd, CID_UPDATE_DATE, s);
	    lpInsertFdirData->fdirHeader= &lpInsertFdirData->lpFlistData->fdir;
	    lpInsertFdirData->fdirEntries=lpInsertFdirData->lpFlistData->fdirFiles;
	} else {
	    inform(hwnd, "No orphan files in the file directory!");
	    FORWARD_WM_CLOSE(hwnd, PostMessage);
	    return(TRUE);
	}

	break;

    case FLIST_FROM_MAIL_DIR:
	SetWindowText(hwnd, "Mail Directory");
	ShowWindow(GetDlgItem(hwnd, CID_UPDATE), SW_SHOW);

	switch (lpInsertControl->position) {

	case CID_INSERT:
	    SetDlgItemText(hwnd, CID_INSERT_CAPTION, "Select mail directory &files to insert");
	    break;

	case CID_APPEND:
	    SetDlgItemText(hwnd, CID_INSERT_CAPTION, "Select mail directory &files to append");
	    break;

	}

	if (lpInsertFdirData->lpFlistData->maildir) {
	    char s[64];

	    strftime(s, 63, "Last updated at %I:%M %p on %d%b%Y", localtime(&lpInsertFdirData->lpFlistData->maildirTime));
	    SetDlgItemText(hwnd, CID_UPDATE_DATE, s);
	    lpInsertFdirData->fdirHeader= &lpInsertFdirData->lpFlistData->maildir;
	    lpInsertFdirData->fdirEntries=lpInsertFdirData->lpFlistData->maildirFiles;
	} else {
	    if (lpInsertFdirData->lpFlistData->maildirTime)
		inform(hwnd, "You do not have any files in your mail directory.");
	    else if (query(hwnd, MB_YESNO, "You don't have an offline copy of your mail directory - do you want to download one?")==IDYES)
		lpInsertFdirData->lpFlistData->maildirUpdate=TRUE;

	    FORWARD_WM_CLOSE(hwnd, PostMessage);
	    return(TRUE);
	}

	break;

    }

    lpInsertFdirData->insertFdirHandle=GetDlgItem(hwnd, CID_FDIR_LIST);
    sortFdir(lpInsertFdirData->fdirHeader, lpInsertFdirData->fdirEntries, &lpInsertFdirData->lpFlistData->fdirSort);
    insertFdirPrelude(lpInsertFdirData);

    if (lpInsertFdirData->fdirAvailable==0) {
	inform(hwnd, "All files already copied to the file list");
	FORWARD_WM_CLOSE(hwnd, PostMessage);
    }

    return(TRUE);
}



static void insertFdir_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void insertFdir_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
	drawFdirEntry(lpDrawItem);

}



static LPCSTR insertFdir_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_INSERT_FDIR, id));
}



static void insertFdir_OnAmHelp(HWND hwnd)
{
    LPInsertFdirData lpInsertFdirData=GetDlgData(hwnd, InsertFdirData);
    LPInsertControl lpInsertControl= &lpInsertFdirData->lpFlistData->insertControl;

    switch (lpInsertControl->source) {

    case FLIST_FROM_FDIR:
	switch (lpInsertControl->position) {

	case CID_INSERT:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_INSERT_FLIST_FDIR);
	    break;

	case CID_APPEND:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_APPEND_FLIST_FDIR);
	    break;

	}

	break;

    case FLIST_FROM_MAIL_DIR:
	switch (lpInsertControl->position) {

	case CID_INSERT:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_INSERT_FLIST_MAIL);
	    break;

	case CID_APPEND:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_APPEND_FLIST_MAIL);
	    break;

	}

	break;

    }
}



static void insertFdir_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPInsertFdirData lpInsertFdirData=GetDlgData(hwnd, InsertFdirData);

    switch (id) {

    case CID_FDIR_LIST:
	if (codeNotify==LBN_DBLCLK)
	    FORWARD_WM_COMMAND(hwnd, CID_OK, GetDlgItem(hwnd, CID_OK),
			       BN_CLICKED, PostMessage);

	break;

    case CID_SORT: {
	RECT rect;
	POINT point;

	GetClientRect(hwndCtl, &rect);
	point.x=rect.right>>1;
	point.y=rect.bottom>>1;
	ClientToScreen(hwndCtl, &point);
	TrackPopupMenu(lpInsertFdirData->hMenuSort, 0, point.x, point.y, 0, hwnd, NULL);
	break;
    }

    case CID_SORT_ASCENDING:
    case CID_SORT_DESCENDING: {
	LPSortData lpSortData=&lpInsertFdirData->lpFlistData->fdirSort;

	if (lpSortData->sortDirection!=id) {
	    CheckMenuItem(lpInsertFdirData->hMenuSort, lpSortData->sortDirection, MF_UNCHECKED);
	    CheckMenuItem(lpInsertFdirData->hMenuSort, id, MF_CHECKED);
	    lpSortData->sortDirection=id;
	    sortFdir(lpInsertFdirData->fdirHeader, lpInsertFdirData->fdirEntries, lpSortData);
	    insertFdirPrelude(lpInsertFdirData);
	}

	break;
    }

    case CID_SORT_NAME:
    case CID_SORT_DATE:
    case CID_SORT_SIZE: {
	LPSortData lpSortData=&lpInsertFdirData->lpFlistData->fdirSort;

	if (lpSortData->sortKey!=id) {
	    CheckMenuItem(lpInsertFdirData->hMenuSort, lpSortData->sortKey, MF_UNCHECKED);
	    CheckMenuItem(lpInsertFdirData->hMenuSort, id, MF_CHECKED);
	    lpSortData->sortKey=id;
	    sortFdir(lpInsertFdirData->fdirHeader, lpInsertFdirData->fdirEntries, lpSortData);
	    insertFdirPrelude(lpInsertFdirData);
	}

	break;
    }

    case CID_UPDATE:
	switch (lpInsertFdirData->lpFlistData->insertControl.source) {

	case FLIST_FROM_FDIR:
	    inform(hwnd, "Offline file directory marked for update");
	    lpInsertFdirData->lpFlistData->fdirUpdate=TRUE;
	    break;

	case FLIST_FROM_MAIL_DIR:
	    inform(hwnd, "Offline mail directory marked for update");
	    lpInsertFdirData->lpFlistData->maildirUpdate=TRUE;
	    break;

	}

	break;

    case CID_OK: {
	LPINT lpFdirSelected=(LPINT)gcallocz((WORD) sizeof(int), (WORD) (lpInsertFdirData->fdirAvailable+1));
	LPINT lpFlistSelected=lpInsertFdirData->lpFlistData->lpFlistSelected;
	int fdirPtr;
	int nFileCount=0;

	*lpFdirSelected=ListBox_GetSelItems(lpInsertFdirData->insertFdirHandle,
					    lpInsertFdirData->fdirAvailable, lpFdirSelected+1);
					    
	flistSuspendControlUpdates(lpInsertFdirData->lpFlistData);
	flistClearSelection(lpInsertFdirData->lpFlistData);

	for (fdirPtr=1; fdirPtr-1< *lpFdirSelected; fdirPtr++) {
	    int itemIndex=lpFdirSelected[fdirPtr];
	    LPFdirEntry fdirEntry=(LPFdirEntry)ListBox_GetItemData(lpInsertFdirData->insertFdirHandle, itemIndex);
	    LPFdirEntry oldEntry;

	    CheckReplace status=checkReplace(hwnd, lpInsertFdirData->lpFlistData,
					     lpInsertFdirData->lpFlistData->insertControl.source, fdirEntry, &oldEntry);

	    if (status==CR_OK || status==CR_DUPLICATE)
		addFlistItem(hwnd, lpInsertFdirData->lpFlistData, fdirEntry);

	    if (status==CR_OK || status==CR_REPLACE) {
		nFileCount++;

		switch (lpInsertFdirData->lpFlistData->insertControl.source) {

		case FLIST_FROM_FDIR:
		    lpInsertFdirData->lpFlistData->fdirOrphans--;
		    break;

		case FLIST_FROM_MAIL_DIR:
		    lpInsertFdirData->lpFlistData->maildirExported++;
		    break;

		}
	    }
	}

	flistControls(hwnd, lpInsertFdirData->lpFlistData, FALSE);

	if (*lpFlistSelected>0 &&
	    lpInsertFdirData->lpFlistData->insertControl.bEdit) {
	    FARPROC lpProc=MakeProcInstance((FARPROC)editProc, lpGlobals->hInst);

	    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FILE_DESCRIPTION", 
						  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpInsertFdirData->lpFlistData);
	    FreeProcInstance(lpProc);
	}

	gfree(lpFdirSelected);
	lpFdirSelected = NULL;
	EndDialog(hwnd, nFileCount);
	break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, 0);
	break;

    }
}



static void insertFdir_OnDestroy(HWND hwnd)
{
    FreeDlgData(hwnd);
}



static LRESULT insertFdir_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, insertFdir_OnInitDialog);
    HANDLE_MSG(hwnd, WM_MEASUREITEM, insertFdir_OnMeasureItem);
    HANDLE_MSG(hwnd, WM_DRAWITEM, insertFdir_OnDrawItem);
    HANDLE_MSG(hwnd, WM_POPUPHELP, insertFdir_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, insertFdir_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, insertFdir_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, insertFdir_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK insertFdirProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=insertFdir_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
