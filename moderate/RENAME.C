#include <windows.h>
#include <windowsx.h>
#include <ctype.h>
#include "ameolapi.h"
#include "hctools.h"
#include "winhorus.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "flist.h"



typedef struct {
    LPSTR lpFileName;
    char szOldName[15];
} RenameData, FAR *LPRenameData;



static BOOL rename_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPRenameData lpRenameData=InitDlgData(hwnd, RenameData);

    lpRenameData->lpFileName=(LPSTR)lParam;
    lstrcpy(lpRenameData->szOldName, lpRenameData->lpFileName);
    SetDlgItemText(hwnd, CID_OLDNAME, lpRenameData->szOldName);
    SetDlgItemText(hwnd, CID_NEWNAME, lpRenameData->szOldName);
    Edit_LimitText(GetDlgItem(hwnd, CID_NEWNAME), 14);
    return(TRUE);
}



static LPCSTR rename_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_RENAME, id));
}



static void rename_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_RENAME);
}



static void rename_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK: {
	LPRenameData lpRenameData=GetDlgData(hwnd, RenameData);
	char szNewName[15];
	LPSTR lpStart=szNewName;

	GetDlgItemText(hwnd, CID_NEWNAME, szNewName, 15);

	while (*lpStart && *lpStart==' ')
	    lpStart++;

	if (! *lpStart)
	    alert(hwnd, "New name cannot be empty!");
	else {
	    LPSTR lpEnd;

	    for (lpEnd=lpStart+lstrlen(lpStart)-1; *lpEnd==' ' && lpEnd>lpStart; lpEnd--);
	    *(lpEnd+1)='\0';

	    if (lstrcmp(lpStart, lpRenameData->szOldName)==0)
		alert(hwnd, "New name cannot be the same as the old name!");
	    else {
		while (lpEnd>=lpStart && isascii(*lpEnd) && isgraph(*lpEnd))
		    lpEnd--;

		if (lpEnd>=lpStart)
		    alert(hwnd, "Illegal character '%c' in new name", *lpEnd);
		else {
		    lstrcpy(lpRenameData->lpFileName, lpStart);
		    EndDialog(hwnd, IDYES);
		}
	    }
	}

	break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, IDNO);
	break;

    case CID_CANCEL_ALL:
	EndDialog(hwnd, IDCANCEL);
	break;

    }
}



static void rename_OnDestroy(HWND hwnd)
{
    FreeDlgData(hwnd);
}



static LRESULT rename_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, rename_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, rename_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, rename_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, rename_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, rename_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK renameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=rename_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
