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


static BOOL updateLists_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPFlistData lpFlistData=(LPFlistData)lParam;

    SetDlgData(hwnd, lpFlistData);
    SetWindowFont(GetDlgItem(hwnd, CID_CONF_NAME), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_NAME), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_FLM_DATE), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_FLD_DATE), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_FLS_DATE), lpGlobals->hfNormal, FALSE);
    SetDlgItemText(hwnd, CID_CONF_NAME, lpFlistData->context.lpcConfName);
    SetDlgItemText(hwnd, CID_TOPIC_NAME, lpFlistData->context.lpcTopicName);
    CheckDlgButton(hwnd, CID_FLM_UPDATE, TRUE);

    if (lpFlistData->flistTime) {
	char s[64];

	strftime(s, 63, "last updated %I:%M %p on %d%b%Y", localtime(&lpFlistData->flistTime));
	SetDlgItemText(hwnd, CID_FLM_DATE, s);
    } else {
	SetDlgItemText(hwnd, CID_FLM_DATE, "not yet downloaded");
	EnableWindow(GetDlgItem(hwnd, CID_FLM_UPDATE), FALSE);
    }

    CheckDlgButton(hwnd, CID_FLD_UPDATE, TRUE);

    if (lpFlistData->fdirTime) {
	char s[64];

	strftime(s, 63, "last updated %I:%m %p on %d%b%Y", localtime(&lpFlistData->fdirTime));
	SetDlgItemText(hwnd, CID_FLD_DATE, s);
    } else {
	SetDlgItemText(hwnd, CID_FLD_DATE, "not yet downloaded");
	EnableWindow(GetDlgItem(hwnd, CID_FLD_UPDATE), FALSE);
    }

    CheckDlgButton(hwnd, CID_FLS_UPDATE, TRUE);

    if (lpFlistData->fuserTime) {
	char s[64];

	strftime(s, 63, "last updated %I:%M %p on %d%b%Y", localtime(&lpFlistData->fuserTime));
	SetDlgItemText(hwnd, CID_FLS_DATE, s);
    } else
	SetDlgItemText(hwnd, CID_FLS_DATE, "not yet downloaded");

    return(TRUE);
}



static LPCSTR updateLists_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_UPDATE_LISTS, id));
}



static void updateLists_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_DOWNLOAD_LISTS);    
//	HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_QUERY);
}



static void updateLists_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK: {
	LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);

	if (IsDlgButtonChecked(hwnd, CID_FLM_UPDATE))
	    lpFlistData->flistUpdate=TRUE;

	if (IsDlgButtonChecked(hwnd, CID_FLD_UPDATE))
	    lpFlistData->fdirUpdate=TRUE;

	if (IsDlgButtonChecked(hwnd, CID_FLS_UPDATE))
	    lpFlistData->fuserUpdate=TRUE;

	EndDialog(hwnd, TRUE);
	break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, FALSE);
	break;

    }
}



static LRESULT updateLists_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, updateLists_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, updateLists_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, updateLists_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, updateLists_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK updateListsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=updateLists_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
