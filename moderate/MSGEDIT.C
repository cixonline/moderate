#include <windows.h>
#include <windowsx.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "edit.h"
#include "msgedit.h"



static BOOL messageEdit_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPMessageEditData lpMessageEditData=(LPMessageEditData)lParam;

    SetDlgData(hwnd, lpMessageEditData);
    SetDlgItemText(hwnd, CID_CONF_NAME, lpMessageEditData->lpcConfName);
    SetDlgItemText(hwnd, CID_TOPIC_NAME, lpMessageEditData->lpcTopicName);

    if (lpMessageEditData->hoobMessage) {
        SAYOBJECT sayObject;

        GetObjectInfo(lpMessageEditData->hoobMessage, &sayObject);

        if (*sayObject.szTitle)
            lstrcpy(lpMessageEditData->szTitle, sayObject.szTitle);

        lstrcpy(lpMessageEditData->lpText, GetObjectText(lpMessageEditData->hoobMessage));
    }

    SetDlgItemText(hwnd, CID_MESSAGE_TITLE, lpMessageEditData->szTitle);
    lpMessageEditData->lpEditControl=editPrelude(hwnd, CID_MESSAGE_TEXT, lpGlobals->hfMessageEdit, EDIT_KEEP_TABS);
    editSetText(lpMessageEditData->lpEditControl, lpMessageEditData->lpText);
    ShowWindow(GetDlgItem(hwnd, CID_IMPORT), lpMessageEditData->bImportEnabled ? SW_SHOW : SW_HIDE);
    Button_Enable(GetDlgItem(hwnd, CID_UNDO), FALSE);
    return(TRUE);
}



static LPCSTR messageEdit_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_MESSAGE_EDIT, id));
}



static void messageEdit_OnAmHelp(HWND hwnd)
{
    LPMessageEditData lpMessageEditData=GetDlgData(hwnd, MessageEditData);

    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, lpMessageEditData->wHelpTag);
}



static void messageEdit_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPMessageEditData lpMessageEditData=GetDlgData(hwnd, MessageEditData);

    switch (id) {

    case CID_MESSAGE_TEXT:
        if (codeNotify==EN_CHANGE &&
            !editChanged(lpMessageEditData->lpEditControl, IDYES))
            Button_Enable(GetDlgItem(hwnd, CID_UNDO), TRUE);

        break;

    case CID_UNDO:
        editUndo(lpMessageEditData->lpEditControl);
        Button_Enable(hwndCtl, FALSE);
        editFocus(lpMessageEditData->lpEditControl);
        break;

    case CID_IMPORT:
        break;

    case CID_OK: {
        SAYOBJECT sayObject;

        GetDlgItemText(hwnd, CID_MESSAGE_TITLE, lpMessageEditData->szTitle, LEN_TITLE);
        editExtract(lpMessageEditData->lpEditControl, lpMessageEditData->lpText, lpMessageEditData->nTextLength);
        editPostlude(lpMessageEditData->lpEditControl);

        if (lpMessageEditData->hoobMessage)
            RemoveObject(lpMessageEditData->hoobMessage);

        InitObject(&sayObject, OT_SAY, SAYOBJECT);
        lstrcpy(sayObject.szConfName, lpMessageEditData->lpcConfName);
        lstrcpy(sayObject.szTopicName, lpMessageEditData->lpcTopicName);
        lstrcpy(sayObject.szTitle, lpMessageEditData->szTitle);
        lpMessageEditData->hoobMessage=PutObject(NULL, &sayObject, lpMessageEditData->lpText);
        EndDialog(hwnd, TRUE);
        break;
    }

    case CID_CANCEL:
        editPostlude(lpMessageEditData->lpEditControl);
	EndDialog(hwnd, FALSE);
        break;

    }
}



static LRESULT messageEdit_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, messageEdit_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, messageEdit_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, messageEdit_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, messageEdit_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK messageEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=messageEdit_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

