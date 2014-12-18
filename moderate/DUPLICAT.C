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



static BOOL duplicate_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    SetDlgItemText(hwnd, CID_DUP_TEXT, (LPSTR)lParam);
    RadioButton(hwnd, CID_DUP_CANCEL, CID_DUP_CANCEL);
    return(TRUE);
}



static LPCSTR duplicate_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_DUPLICATE, id));
}



static void duplicate_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_DUPLICATE);
}



static void duplicate_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_CANCEL:
	EndDialog(hwnd, CR_CANCEL);
        break;

    case CID_OK:
        switch (RadioButton(hwnd, CID_DUP_CANCEL, 0)) {

	case CID_DUP_CANCEL:
	    EndDialog(hwnd, CR_CANCEL);
            break;

        case CID_DUP_REPLACE:
	    EndDialog(hwnd, CR_REPLACE);
            break;

	case CID_DUP_DUPLICATE:
	    EndDialog(hwnd, CR_DUPLICATE);
            break;

        }

        break;

    }
}



static LRESULT duplicate_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, duplicate_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, duplicate_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, duplicate_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, duplicate_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK duplicateProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=duplicate_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
