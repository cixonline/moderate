#include <windows.h>                    //  Windows API definitions
#include <windowsx.h>
#include <sys/stat.h>
#include <io.h>
#include <time.h>
#include "ameolapi.h"                   //  Ameol API definitions
#include "winhorus.h"                   //  Windows utility routines
#include "hctools.h"
#include "setup.h"                      //  Setup (ameol.ini) routines and variables
#include "moderate.h"                   //  Resource definitions
#include "help.h"             		//  Help resource definitions
#include "globals.h"                    //  Global variable and routine definitions
#include "confs.h"
#include "edit.h"
#include "strftime.h"


typedef struct {
    char szFileName[LEN_PATHNAME];
    LPTopicInfo lpTopicInfo;
    BOOL bNewConf;
    char szUpdate[128];
    BOOL bUpload, bDownload;
    LPEditControl lpEditControl;
} ConfNoteData, FAR *LPConfNoteData;



BOOL _EXPORT FAR PASCAL confNoteProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



void editConfNote(HWND hwnd, LPTopicInfo lpTopicInfo, LPCSTR lpcName)
{
    FARPROC lpProc=MakeProcInstance(confNoteProc, lpGlobals->hInst);
    ConfNoteData confNoteData;

    wsprintf(confNoteData.szFileName, "%s\\%s.CNO",
             (LPSTR)setup.szDataDir, lpcName ? lpcName : (LPSTR)"newconf");

    confNoteData.lpTopicInfo=lpTopicInfo;
    confNoteData.bNewConf=lpcName==NULL;

    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_CONF_NOTE", 
					  (HWND)hwnd, (DLGPROC)lpProc,
                      (LPARAM)(DWORD)(LPConfNoteData)&confNoteData);

    FreeProcInstance(lpProc);
}



static BOOL confNote_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPConfNoteData lpConfNoteData=(LPConfNoteData)lParam;
    char szFileName[LEN_PATHNAME];
    HSTREAM hStream;
    time_t timeUpdate=(time_t)0;
    time_t timeModify=(time_t)0;

    SetDlgData(hwnd, lpConfNoteData);
    *lpConfNoteData->szUpdate='\0';
    lpConfNoteData->bUpload=FALSE;
    lpConfNoteData->bDownload=FALSE;
    lpConfNoteData->lpEditControl=editPrelude(hwnd, CID_MESSAGE_TEXT, lpGlobals->hfMessageEdit, EDIT_KEEP_TABS);
    SetDlgItemText(hwnd, CID_CONF_NAME, lpConfNoteData->lpTopicInfo->szConfName);
    lstrcpy(szFileName, lpConfNoteData->szFileName);

    if (_access(szFileName, 0)==0) {
        struct stat statBuff;

        stat(szFileName, &statBuff);
        timeModify=(time_t)statBuff.st_mtime;
        hStream=openFile(lpConfNoteData->szFileName, OF_READ);

        if (hStream) {
            long lFileSize=fileSize(hStream);

            if (lFileSize>0) {
                LPSTR lpBuff=(LPSTR)gmalloc(lFileSize*2);

                if (lpBuff) {
                    LPSTR lpPtr=lpBuff;
                    int nCount;

                    if (readLine(hStream, lpConfNoteData->szUpdate, 127)>0)
                        timeUpdate=parseTime(lpConfNoteData->szUpdate);

                    while ((nCount=readLine(hStream, lpPtr, 255))>=0) {
                        lpPtr+=nCount;
                        *lpPtr++ ='\r';
                        *lpPtr++ ='\n';
                    }

                    if (lpPtr>lpBuff)
                        lpPtr-=2;

                    *lpPtr='\0';
                    editSetText(lpConfNoteData->lpEditControl, lpBuff);
                    gfree(lpBuff);
					lpBuff = NULL;
                }
            }

            closeFile(hStream);
        }
    }

    if (timeUpdate || lpConfNoteData->bNewConf) {
        char szUpdateDate[128];
        LPSTR lpUpdateDate=szUpdateDate;

        if (timeUpdate)
            lpUpdateDate+=strftime(szUpdateDate, 63, "last downloaded %I:%M %p on %d%b%Y", localtime(&timeUpdate));
        else {
            lstrcpy(szUpdateDate, "New forum note");
            lpUpdateDate+=lstrlen(szUpdateDate);
        }

        if (timeModify)
            strftime(lpUpdateDate, 63, ", last edited %I:%M %p on %d%b%Y", localtime(&timeModify));

        SetDlgItemText(hwnd, CID_UPDATE_DATE, szUpdateDate);
    } else {
        inform(hwnd, "Forum note for %s not found (or corrupt).\r\n"
                     "A copy will be downloaded from CIX on your next blink",
               lpConfNoteData->lpTopicInfo->szConfName);

        lpConfNoteData->lpTopicInfo->topicHeader.updateFlags|=TOPIC_GETNOTE;
        FORWARD_WM_CLOSE(hwnd, PostMessage);
    }

    Button_Enable(GetDlgItem(hwnd, CID_UNDO), FALSE);

    if (lpConfNoteData->bNewConf)
        ShowWindow(GetDlgItem(hwnd, CID_DOWNLOAD), SW_HIDE);

    return(TRUE);
}



static LPCSTR confNote_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_CONF_NOTE, id));
}



static void confNote_OnAmHelp(HWND hwnd)
{
    LPConfNoteData lpConfNoteData=GetDlgData(hwnd, ConfNoteData);

    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT,
            lpConfNoteData->bNewConf ? HID_CREATE_CONF_NOTE : HID_TOPICS_CONF_NOTE);

    editFocus(lpConfNoteData->lpEditControl);
}



static void confNote_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPConfNoteData lpConfNoteData=GetDlgData(hwnd, ConfNoteData);

    switch (id) {

    case CID_MESSAGE_TEXT:
        if (codeNotify==EN_CHANGE &&
            !editChanged(lpConfNoteData->lpEditControl, IDYES))
            Button_Enable(GetDlgItem(hwnd, CID_UNDO), TRUE);

        break;

    case CID_UNDO:
        editUndo(lpConfNoteData->lpEditControl);
        Button_Enable(GetDlgItem(hwnd, CID_UNDO), FALSE);
        editFocus(lpConfNoteData->lpEditControl);
        break;

    case CID_DOWNLOAD:
        if (lpConfNoteData->lpTopicInfo->topicHeader.updateFlags&TOPIC_GETNOTE ||
            lpConfNoteData->bDownload)
            inform(hwnd, "The forum note for %s is already marked for refreshing from CIX",
                   lpConfNoteData->lpTopicInfo->szConfName);

        else {
            inform(hwnd, "Your offline copy of the forum note for %s will be refreshed from CIX on your next blink",
                   lpConfNoteData->lpTopicInfo->szConfName);

            lpConfNoteData->bDownload=TRUE;
        }

        editFocus(lpConfNoteData->lpEditControl);
        break;

    case CID_OK: {
        HWND hwndEdit=GetDlgItem(hwnd, CID_MESSAGE_TEXT);

        if (Edit_GetModify(hwndEdit)) {
            HSTREAM hStream=openFile(lpConfNoteData->szFileName, OF_WRITE);

            if (hStream) {
                int nLength;
                LPSTR lpBuff;
                char szLine[128];

                nLength=Edit_GetTextLength(hwndEdit)+Edit_GetLineCount(hwndEdit)*3;
                lpBuff=(LPSTR)gmalloc(nLength+1);
                writeLine(hStream, "%s\n", lpConfNoteData->szUpdate);
                editFormat(lpConfNoteData->lpEditControl, TRUE);
                editExtract(lpConfNoteData->lpEditControl, lpBuff, nLength);

                while (editExtract(lpConfNoteData->lpEditControl, szLine, 0)>=0)
                    writeLine(hStream, "%s\n", (LPSTR)szLine);

                gfree(lpBuff);
				lpBuff = NULL;
                closeFile(hStream);
                lpConfNoteData->bUpload=TRUE;
            } else
                alert(hwnd, "Error writing the forum note file (%s) for %s",
                      lpConfNoteData->szFileName, lpConfNoteData->lpTopicInfo->szConfName);

        }

        if (lpConfNoteData->bUpload)
            lpConfNoteData->lpTopicInfo->topicHeader.updateFlags|=TOPIC_PUTNOTE|TOPIC_GETNOTE;
        else if (lpConfNoteData->bDownload)
            lpConfNoteData->lpTopicInfo->topicHeader.updateFlags|=TOPIC_GETNOTE;

        EndDialog(hwnd, TRUE);
        break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, FALSE);
        break;

    }
}



static void confNote_OnDestroy(HWND hwnd)
{
    LPConfNoteData lpConfNoteData=GetDlgData(hwnd, ConfNoteData);

    editPostlude(lpConfNoteData->lpEditControl);
}



static LRESULT confNote_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, confNote_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, confNote_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, confNote_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, confNote_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, confNote_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK confNoteProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=confNote_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

