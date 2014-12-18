#include <windows.h>
#include <windowsx.h>
#include "ameolapi.h"
#include "hctools.h"
#include "winhorus.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "flist.h"



static BOOL insertFilepool_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPFlistData lpFlistData=(LPFlistData)lParam;

    SetDlgData(hwnd, lpFlistData);
    SetWindowFont(GetDlgItem(hwnd, CID_INSERT_CAPTION), lpGlobals->hfNormal, FALSE);

    switch (lpFlistData->insertControl.source) {

    case FLIST_FROM_FILEPOOL:
	SetWindowText(hwnd, "Filepool");
	break;

    case FLIST_FROM_NOTIFICATION:
	lpFlistData->fdirUpdate=TRUE;
	SetWindowText(hwnd, "Notification");
	break;

    }

    switch (lpFlistData->insertControl.position) {

    case CID_INSERT:
	SetDlgItemText(hwnd, CID_INSERT_CAPTION, "Enter &filename to insert");
	SetDlgItemText(hwnd, CID_OK, "&Insert");
	break;

    case CID_APPEND:
	SetDlgItemText(hwnd, CID_INSERT_CAPTION, "Enter &filename to append");
	SetDlgItemText(hwnd, CID_OK, "&Append");
	break;

    }

    Edit_LimitText(GetDlgItem(hwnd, CID_FILENAME), LEN_CIXFILENAME-1);

    if (lpFlistData->flistInsertPoint==LB_ERR)
	lpFlistData->flistInsertPoint=0;

    return(TRUE);
}



static LPCSTR insertFilepool_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_INSERT_FILEPOOL, id));
}



static void insertFilepool_OnAmHelp(HWND hwnd)
{
    LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);

    switch (lpFlistData->insertControl.source) {

    case FLIST_FROM_FILEPOOL:
	switch (lpFlistData->insertControl.position) {

	case CID_INSERT:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_INSERT_FLIST_FILEPOOL);
	    break;

	case CID_APPEND:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_APPEND_FLIST_FILEPOOL);
	    break;

	}

	break;

    case FLIST_FROM_NOTIFICATION:
	switch (lpFlistData->insertControl.position) {

	case CID_INSERT:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_INSERT_FLIST_NOTIFICATION);
	    break;

	case CID_APPEND:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_APPEND_FLIST_NOTIFICATION);
	    break;

	}

	break;

    }
}



static void insertFilepool_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK: {
	LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);
	char fileName[15];
	LPSTR ptrStart=fileName;
	LPSTR ptrEnd;
	int nFileCount=0;

	GetDlgItemText(hwnd, CID_FILENAME, fileName, LEN_CIXFILENAME);

	while (*ptrStart && *ptrStart==' ')
	    ptrStart++;

	for (ptrEnd=ptrStart+lstrlen(ptrStart)-1; *ptrEnd==' ' && ptrEnd>ptrStart; ptrEnd--);
	*(ptrEnd+1)='\0';

	if (! *ptrStart)
	    alert(hwnd, "Filename cannot be empty!");
	else {
	    LPFdirEntry newEntry;
	    LPFdirEntry oldEntry;
	    CheckReplace status;
	    BOOL allocated=FALSE;

	    AnsiLower(ptrStart);
	    newEntry=lpFlistData->insertControl.source==FLIST_FROM_FILEPOOL ? lpFlistData->filepooldir : lpFlistData->fdir;

	    while (newEntry && lstrcmp(newEntry->fdirName, ptrStart))
		newEntry=newEntry->fdirNext;

	    if (!newEntry) {
		allocated=TRUE;
		newEntry=(LPFdirEntry)gmalloc(sizeof(FdirEntry));
		newEntry->fdirHead=lpFlistData->insertControl.source==FLIST_FROM_FILEPOOL ? &lpFlistData->filepooldir : &lpFlistData->fdir;
		newEntry->fdirNext=NULL;
		lstrcpy(newEntry->fdirName, ptrStart);
		newEntry->fdirLocalPath=NULL;
		newEntry->fdirSize= -1;
		newEntry->fdirTimestamp=(time_t)0;
		newEntry->fdirReferences=0;
		newEntry->fdirFlags=FDIR_DEFAULT_FLAGS;
	    }

	    status=checkReplace(hwnd, lpFlistData, lpFlistData->insertControl.source, newEntry, &oldEntry);

	    if (status!=CR_CANCEL) {
		flistSuspendControlUpdates(lpFlistData);
		flistClearSelection(lpFlistData);

		if (status==CR_OK || status==CR_REPLACE) {
		    nFileCount++;

		    if (allocated) {
			newEntry->fdirNext= *newEntry->fdirHead;
			*newEntry->fdirHead=newEntry;

			if (lpFlistData->insertControl.source==FLIST_FROM_NOTIFICATION) {
			    lpFlistData->fdirFiles++;
			    sortFdir(&lpFlistData->fdir, lpFlistData->fdirFiles, &lpFlistData->fdirSort);
			}
		    }

		    if (status!=CR_REPLACE)
			addFlistItem(hwnd, lpFlistData, newEntry);

		} else {
		    addFlistItem(hwnd, lpFlistData, oldEntry);

		    if (allocated)
			{
				gfree(newEntry);
				newEntry = NULL;
			}

		}

		flistControls(hwnd, lpFlistData, FALSE);

		if (lpFlistData->insertControl.bEdit) {
		    FARPROC lpProc=MakeProcInstance((FARPROC)editProc, lpGlobals->hInst);

		    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FILE_DESCRIPTION", 
							  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
		    FreeProcInstance(lpProc);
		}
	    } 
		else if (allocated)
		{
			gfree(newEntry);
			newEntry = NULL;
		}

	}

	EndDialog(hwnd, nFileCount);
	break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, 0);
	break;

    }
}



static LRESULT insertFilepool_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, insertFilepool_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, insertFilepool_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, insertFilepool_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, insertFilepool_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK insertFilepoolProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=insertFilepool_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
