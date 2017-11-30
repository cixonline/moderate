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
#include "export.h"

void exportFdirEntry(LPContext lpContext, LPFdirEntry lpFdirEntry)
{
    HSCRIPT hScript=initScript("Moderate", FALSE);

    addToScript(hScript, "put `join %s/%s`¬"
			 "if waitfor(`R:`, `M:`) == 0¬"
			     "put `fexport %s`¬"
			     "waitfor `R:`¬"
			     "put `quit`¬"
			     "waitfor `M:`¬"
			 "endif¬",
		lpContext->lpcConfName, lpContext->lpcTopicName, lpFdirEntry->fdirName);

    actionScript(hScript, OT_PREINCLUDE, "export %s to filepool from %s/%s",
		 lpFdirEntry->fdirName, lpContext->lpcConfName, lpContext->lpcTopicName);

}



static BOOL exportFdir_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPExportFdirData lpExportFdirData=(LPExportFdirData)lParam;

    SetDlgData(hwnd, lpExportFdirData);
    SetDlgItemText(hwnd, CID_FILENAME, lpExportFdirData->lpFdirEntry->fdirName);
    ShowWindow(GetDlgItem(hwnd, CID_TRANSFORM), lpExportFdirData->bTransformOK);
    EnableWindow(GetDlgItem(hwnd, CID_EXPORT), lpExportFdirData->bExportOK);

    RadioButton(hwnd, CID_LOCAL,
                lpExportFdirData->lpFdirEntry->fdirFlags&FDIR_FLAG_EXPORT   ? CID_EXPORT    :
                lpExportFdirData->lpFdirEntry->fdirFlags&FDIR_FLAG_FILEPOOL ? CID_TRANSFORM :
									      CID_LOCAL);

    return(TRUE);
}



static LPCSTR exportFdir_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_EXPORT, id));
}



static void exportFdir_OnAmHelp(HWND hwnd)
{
    LPExportFdirData lpExportFdirData=GetDlgData(hwnd, ExportFdirData);

    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, lpExportFdirData->wHelpTag);
}



static void exportFdir_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPExportFdirData lpExportFdirData=GetDlgData(hwnd, ExportFdirData);

    switch (id) {

    case CID_OK:
        switch (RadioButton(hwnd, CID_LOCAL, 0)) {

        case CID_LOCAL:
            lpExportFdirData->lpFdirEntry->fdirFlags&= ~FDIR_FLAG_FILEPOOL;
            lpExportFdirData->lpFdirEntry->fdirFlags&= ~FDIR_FLAG_EXPORT;
            break;

        case CID_EXPORT:
            lpExportFdirData->lpFdirEntry->fdirFlags&= ~FDIR_FLAG_FILEPOOL;
			lpExportFdirData->lpFdirEntry->fdirFlags|=FDIR_FLAG_EXPORT;
			break;

        case CID_TRANSFORM:
            lpExportFdirData->lpFdirEntry->fdirFlags|=FDIR_FLAG_FILEPOOL;
            lpExportFdirData->lpFdirEntry->fdirFlags&= ~FDIR_FLAG_EXPORT;
            break;

        }

    case CID_CANCEL:
		EndDialog(hwnd, lpExportFdirData->lpFdirEntry->fdirFlags&(FDIR_FLAG_EXPORT|FDIR_FLAG_FILEPOOL));
        break;

    }
}



static LRESULT exportFdir_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, exportFdir_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, exportFdir_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, exportFdir_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, exportFdir_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK exportFdirProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=exportFdir_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



void exportFlistEntry(LPContext lpContext, LPFlistEntry lpFlistEntry)
{
    HSCRIPT hScript=initScript("Moderate", FALSE);

    addToScript(hScript, "put `join %s/%s`¬"
			 "if waitfor(`R:`, `M:`) == 0¬"
			     "put `fexport %s`¬"
			     "waitfor `R:`¬"
			     "put `quit`¬"
			     "waitfor `M:`¬"
			 "endif¬",
		lpContext->lpcConfName, lpContext->lpcTopicName, lpFlistEntry->flistName);

    actionScript(hScript, OT_PREINCLUDE, "export %s to filepool from %s/%s",
		 lpFlistEntry->flistName, lpContext->lpcConfName, lpContext->lpcTopicName);

}



static BOOL exportFlist_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPFlistEntry lpFlistEntry=(LPFlistEntry)lParam;

    SetDlgData(hwnd, lpFlistEntry);
    SetDlgItemText(hwnd, CID_FILENAME, lpFlistEntry->flistName);

    RadioButton(hwnd, CID_LOCAL,
                lpFlistEntry->flistFlags&FLIST_FLAG_EXPORT   ? CID_EXPORT    :
		lpFlistEntry->flistFlags&FLIST_FLAG_FILEPOOL ? CID_TRANSFORM :
                                                               CID_LOCAL);

    return(TRUE);
}



static LPCSTR exportFlist_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_EXPORT, id));
}



static void exportFlist_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_EXPORT);
}



static void exportFlist_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPFlistEntry lpFlistEntry=GetDlgData(hwnd, FlistEntry);

    switch (id) {

    case CID_OK:
        switch (RadioButton(hwnd, CID_LOCAL, 0)) {

        case CID_LOCAL:
			lpFlistEntry->flistFlags&= ~FLIST_FLAG_FILEPOOL;
            lpFlistEntry->flistFlags&= ~FLIST_FLAG_EXPORT;
            break;

        case CID_EXPORT:
            lpFlistEntry->flistFlags&= ~FLIST_FLAG_FILEPOOL;
            lpFlistEntry->flistFlags|=FLIST_FLAG_EXPORT;
            break;

        case CID_TRANSFORM:
            lpFlistEntry->flistFlags|=FLIST_FLAG_FILEPOOL;
            lpFlistEntry->flistFlags&= ~FLIST_FLAG_EXPORT;
            break;

	}

    case CID_CANCEL:
	EndDialog(hwnd, lpFlistEntry->flistFlags&(FLIST_FLAG_EXPORT|FLIST_FLAG_FILEPOOL));
        break;

    }
}


static LRESULT exportFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, exportFlist_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, exportFlist_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, exportFlist_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, exportFlist_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK exportFlistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=exportFlist_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

