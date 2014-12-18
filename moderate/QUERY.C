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



static BOOL queryLists_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPContext lpContext=(LPContext)lParam;

    SetWindowFont(GetDlgItem(hwnd, CID_CONF_NAME), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_NAME), lpGlobals->hfNormal, FALSE);
    SetDlgItemText(hwnd, CID_CONF_NAME, lpContext->lpcConfName);
    SetDlgItemText(hwnd, CID_TOPIC_NAME, lpContext->lpcTopicName);
    RadioButton(hwnd, CID_CHANGE_TOPIC, CID_CHANGE_TOPIC);
    return(TRUE);
}



static LPCSTR queryLists_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_FLIST_MISSING, id));
}



static void queryLists_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_QUERY);
}



static void queryLists_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK:
	EndDialog(hwnd, RadioButton(hwnd, CID_CHANGE_TOPIC, 0));
	break;

    case CID_CANCEL:
	EndDialog(hwnd, CID_CANCEL);
	break;

    }
}



static LRESULT queryLists_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, queryLists_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, queryLists_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, queryLists_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, queryLists_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK queryListsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=queryLists_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
