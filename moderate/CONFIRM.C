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



static BOOL confirm_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPFlistConfirm lpFlistConfirm=(LPFlistConfirm)lParam;

    SetDlgData(hwnd, lpFlistConfirm);
    CheckDlgButton(hwnd, CID_CONFIRM_FLIST, lpFlistConfirm->flistConfirm==FLIST_DELETE_CONFIRM);

    switch (lpFlistConfirm->flistFdirConfirm) {

    case FDIR_DELETE_LEAVE:
	RadioButton(hwnd, CID_LEAVE_FDIR, CID_LEAVE_FDIR);
	break;

    case FDIR_DELETE_CONFIRM:
	RadioButton(hwnd, CID_LEAVE_FDIR, CID_CONFIRM_FDIR);
	break;

    case FDIR_DELETE_ALL:
	RadioButton(hwnd, CID_LEAVE_FDIR, CID_DELETE_ALL_FDIR);
	break;

    }

    return(TRUE);
}



static LPCSTR confirm_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_DELETE_FLIST, id));
}



static void confirm_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_DELETE);
}



static void confirm_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPFlistConfirm lpFlistConfirm=GetDlgData(hwnd, FlistConfirm);

    switch (id) {

    case CID_OK: {
	lpFlistConfirm->flistConfirm=IsDlgButtonChecked(hwnd, CID_CONFIRM_FLIST)
	    ? FLIST_DELETE_CONFIRM : FLIST_DELETE_ALL;

	switch (RadioButton(hwnd, CID_LEAVE_FDIR, 0)) {

	case CID_DELETE_ALL_FDIR:
	    lpFlistConfirm->flistFdirConfirm=FDIR_DELETE_ALL;
	    break;

	case CID_CONFIRM_FDIR:
	    lpFlistConfirm->flistFdirConfirm=FDIR_DELETE_CONFIRM;
	    break;

	case CID_LEAVE_FDIR:
	    lpFlistConfirm->flistFdirConfirm=FDIR_DELETE_LEAVE;
	    break;

	}

	EndDialog(hwnd, TRUE);
        break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, FALSE);
        break;

    }
}



static LRESULT confirm_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, confirm_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, confirm_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, confirm_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, confirm_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK confirmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=confirm_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
