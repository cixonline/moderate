#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "flist.h"
#include "msgedit.h"
#include "strftime.h"

#define MOD_DATA	GWL_USERDATA

extern char str_Window[9];
extern char str_NotifyClassName[17];
extern char str_NotifyLeft[19];
extern char str_NotifyTop[18];
extern char str_NotifyWidth[20];
extern char str_NotifyHeight[21];

enum {MAX_NOTIFY_SIZE=10240};

#define	WritePPInt(lpkey,lpsect,value)	(AmWritePrivateProfileInt((lpkey),(lpsect),(value),setup.szIniFile))
#define	GetPPInt(lpkey,lpsect,value)	(AmGetPrivateProfileInt((lpkey),(lpsect),(value),setup.szIniFile))


BOOL _EXPORT CALLBACK notifySelectProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WINAPI OpenNotifyWindow(LPSTR confName, LPSTR topicName, LPSTR subject, LPSTR msg);
HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE hInstance,
									  LPCSTR lpTemplateName,
									  int x, int y, int cx, int cy, LPARAM lParam);



static LPSTR notification(LPFlistData lpFlistData, LPSTR lpszText, LPSTR lpszTitle, int nFlistIndex)
{
    LPFlistEntry lpFlistEntry=flistGetEntry(lpFlistData, nFlistIndex);
    LPFdirEntry lpFdirEntry=lpFlistEntry->flistSource;
    char szFileName[LEN_CIXFILENAME];
    char szFileSize[64];
    char szFileDate[64];
    char szContributor[40];

    setCase(szFileName, lpFlistEntry->flistName, setup.wNotifyCase);

    if (lpFdirEntry && lpFdirEntry->fdirSize>=0) {
	formatLong(szFileSize, "", lpFdirEntry->fdirSize, " bytes");
	strftime(szFileDate, 63, "%A %d %B %Y", localtime(&lpFdirEntry->fdirTimestamp));

	if (lpszTitle)
            if (setup.bNotifySize)
                wsprintf(lpszTitle, "%s [%s]", (LPSTR)szFileName, (LPSTR)szFileSize);
            else
                lstrcpy(lpszTitle, szFileName);

    } else {
        *szFileSize= *szFileDate='\0';

        if (lpszTitle)
            lstrcpy(lpszTitle, szFileName);

    }

    switch (lpFlistEntry->flistSelect) {

    case FLIST_FROM_UPLOAD:
    case FLIST_FROM_MAIL_DIR:
		  lstrcpy( szContributor, "$CIX$" );
        GetRegistry(szContributor);
        break;

    case FLIST_FROM_FILEPOOL:
	lstrcpy(szContributor, "filepool");
	break;

    default:
	*szContributor='\0';
	break;

    }

    lpszText+=wsprintf(lpszText, "\r\n"
				 "   Filename: %s\r\n"
				 "    Hotlink: cixfile:%s/%s:%s\r\n"
				 "  File size: %s\r\n"
				 "Contributor: %s\r\n"
				 "       Date: %s\r\n"
				 "%s"
				 "\r\n"
				 "Description: \r\n\r\n"
				 "%s\r\n",
		       (LPSTR)szFileName, 
// SM - Changed this since when MDI the lpGlobals are no longer valid conf/topic names.
//			   lpGlobals->contextFlist.lpcConfName, lpGlobals->contextFlist.lpcTopicName,
				lpFlistData->context.lpcConfName, 
				lpFlistData->context.lpcTopicName,
			   (LPSTR)szFileName, 
			   (LPSTR)szFileSize,
		       (LPSTR)szContributor, 
			   (LPSTR)szFileDate,
			   setup.os_and_status?"     Status: \r\n        O/S: \r\n":"",
		       lpFlistEntry->flistDescription);

    while ((lpFlistEntry=lpFlistEntry->flistNext) &&
	   lpFlistEntry->flistSelect==FLIST_TABBED &&
	   !(lpFlistEntry->flistFlags&FLIST_FLAG_HOLD))
	lpszText+=wsprintf(lpszText, "%s\r\n", lpFlistEntry->flistDescription);

    return(lpszText);
}
           


LPSTR addSig(LPContext lpContext, LPSTR lpszText)
{
    TOPICINFO topicInfo;
    char fnSig[128];
    HSTREAM streamSig;

    GetTopicInfo(lpContext->hTopic, &topicInfo);
    wsprintf(fnSig, "%s\\sig\\%s.SIG", (LPSTR)setup.szDataDir, *topicInfo.szSigFile ? (LPSTR)topicInfo.szSigFile : (LPSTR)"GLOBAL");

    if (streamSig=openFile(fnSig, OF_READ)) {
	char lineBuff[80];

	while (readLine(streamSig, lineBuff, 79)>=0)
	    lpszText+=wsprintf(lpszText, "\r\n%s", (LPSTR)lineBuff);

	closeFile(streamSig);
    }

    return(lpszText);
}


BOOL WINAPI MDINotifyDialog(HWND hwnd, LPMessageEditData messageEditData)
{
int x, y;
int cx, cy;
HWND hWnd;

	x  = GetPPInt( str_Window, str_NotifyLeft, 10 );
	y  = GetPPInt( str_Window, str_NotifyTop, 10 );
	cx = GetPPInt( str_Window, str_NotifyWidth, 500 );
	cy = GetPPInt( str_Window, str_NotifyHeight, 340 );
	hWnd = CreateMDIDialogParam( lpGlobals->hInst, str_NotifyClassName, x, y, cx, cy, (LPARAM)(LPMessageEditData)messageEditData);
	if(IsWindow(hWnd))
	{
		UpdateWindow( hWnd );
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void notify(HWND hwnd, LPFlistData lpFlistData)
{
int			iCount, iType;
HOOB		hWork;
BOOL		bNotFound = TRUE;
SAYOBJECT	sayObject;

    if (! *lpFlistData->lpFlistSelected)
	{
		inform(hwnd, "No file entries selected for notification! ");
	}
    else 
	{
	LPINT lpFlistSelected=(LPINT)gcallocz((WORD) sizeof(int), (WORD) (*lpFlistData->lpFlistSelected+1));
	int nFlist;

	for (nFlist=1; nFlist<= *lpFlistData->lpFlistSelected; nFlist++) {
	    int nFlistIndex=lpFlistData->lpFlistSelected[nFlist];
	    LPFlistEntry lpFlistEntry=flistGetEntry(lpFlistData, nFlistIndex);

	    if (!(lpFlistEntry->flistFlags&FLIST_FLAG_HOLD) && isFile(lpFlistEntry)) {
		(*lpFlistSelected)++;
		lpFlistSelected[*lpFlistSelected]=nFlistIndex;
	    }
	}

	if (! *lpFlistSelected)
	    inform(hwnd, "No file entries selected for notification!");
	else {
	    BOOL bSingleMessage=FALSE;
	    MessageEditData messageEditData;
	    FARPROC lpEditProc=MakeProcInstance((FARPROC)messageEditProc, lpGlobals->hInst);

	    if (*lpFlistSelected>1) 
		{
			if(setup.bNotifyMethod == 0)
			{
				FARPROC lpProc=MakeProcInstance((FARPROC)notifySelectProc, lpGlobals->hInst);
				int nResult=StdDialogBox((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_NOTIFY_SELECT", 
										 (HWND)hwnd, (DLGPROC)lpProc);

				FreeProcInstance(lpProc);

				if (nResult==CID_CANCEL) 
				{
					FreeProcInstance(lpEditProc);
					return;
				}
				bSingleMessage=nResult==CID_NOTIFY_SINGLE;
			}
			else
			{
				bSingleMessage=setup.bNotifyMethod==1?0:1;
			}
	    }

	    messageEditData.lpcConfName=lpFlistData->context.lpcConfName;
	    messageEditData.lpcTopicName=lpFlistData->context.lpcTopicName;
	    messageEditData.lpText=(LPSTR)gmalloc(MAX_NOTIFY_SIZE);
	    messageEditData.nTextLength=MAX_NOTIFY_SIZE;
	    messageEditData.bImportEnabled=FALSE;
	    messageEditData.wHelpTag=HID_FLIST_NOTIFY;
		messageEditData.fHasRect=FALSE;

	    if (bSingleMessage) {

		LPSTR		lpPtr=messageEditData.lpText;

		for (nFlist=1; nFlist<= *lpFlistSelected; nFlist++)
		    lpPtr=notification(lpFlistData, lpPtr, NULL, lpFlistSelected[nFlist]);

		if (setup.bNotifySig)
		    addSig(&lpFlistData->context, lpPtr);

		wsprintf(messageEditData.szTitle, "%d new files available", *lpFlistSelected);
		messageEditData.hoobMessage=NULL;

		for (	iCount = GetObjectCount(),	hWork = GetOutBasketObject(NULL);
			bNotFound && (iCount > 0);
			iCount--,						hWork = GetOutBasketObject(hWork))
		{
			iType = GetOutBasketObjectType(hWork);
			if (iType == OT_SAY)
			{
				GetOutBasketObjectData(hWork,(SAYOBJECT FAR *)&sayObject);
				if (strcmp(sayObject.szConfName,messageEditData.lpcConfName) == 0)
				{
					if (strcmp(sayObject.szTopicName,messageEditData.lpcTopicName) == 0)
					{
						if (strcmp(sayObject.szTitle,messageEditData.szTitle) == 0)
						{
							messageEditData.hoobMessage = hWork;
							bNotFound = FALSE;
						}
					}
				}
			}
		}

//		if (StdDialogBoxParam(lpGlobals->hInst, "DID_MESSAGE_EDIT", hwnd, lpEditProc, (DWORD)(LPMessageEditData)&messageEditData))
		MDINotifyDialog(hwnd, (LPMessageEditData)&messageEditData);
//		    lpFlistData->outbasketActions[lpFlistData->actionCount++]=messageEditData.hoobMessage;
//		OpenNotifyWindow((LPSTR)lpFlistData->context.lpcConfName, 
//							 (LPSTR)lpFlistData->context.lpcTopicName, 
//							 messageEditData.szTitle, 
//							 messageEditData.lpText);

	    } else
		for (nFlist=1; nFlist<= *lpFlistSelected; nFlist++) {
		    LPSTR lpPtr=notification(lpFlistData, messageEditData.lpText, messageEditData.szTitle, lpFlistSelected[nFlist]);

		    if (setup.bNotifySig)
			addSig(&lpFlistData->context, lpPtr);

		    messageEditData.hoobMessage=NULL;
			for (	iCount = GetObjectCount(),	hWork = GetOutBasketObject(NULL);
				bNotFound && (iCount > 0);
				iCount--,						hWork = GetOutBasketObject(hWork))
			{
				iType = GetOutBasketObjectType(hWork);
				if (iType == OT_SAY)
				{
					GetOutBasketObjectData(hWork,(SAYOBJECT FAR *)&sayObject);
					if (strcmp(sayObject.szConfName,messageEditData.lpcConfName) == 0)
					{
						if (strcmp(sayObject.szTopicName,messageEditData.lpcTopicName) == 0)
						{
							if (strcmp(sayObject.szTitle,messageEditData.szTitle) == 0)
							{
								messageEditData.hoobMessage = hWork;
								bNotFound = FALSE;
							}
						}
					}
				}
			}

//		    if (StdDialogBoxParam(lpGlobals->hInst, "DID_MESSAGE_EDIT", hwnd, lpEditProc, (DWORD)(LPMessageEditData)&messageEditData))
			MDINotifyDialog(hwnd, (LPMessageEditData)&messageEditData);
//				lpFlistData->outbasketActions[lpFlistData->actionCount++]=messageEditData.hoobMessage;
			
//			OpenNotifyWindow((LPSTR)lpFlistData->context.lpcConfName, 
//							 (LPSTR)lpFlistData->context.lpcTopicName, 
//							 messageEditData.szTitle, 
//							 messageEditData.lpText);
		}

	    FreeProcInstance(lpEditProc);
	    gfree(messageEditData.lpText);
		messageEditData.lpText = NULL;
	}

	gfree(lpFlistSelected);
	lpFlistSelected = NULL;
    }
}



static BOOL notifySelect_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    RadioButton(hwnd, CID_NOTIFY_SINGLE, setup.bNotifyMethod==1 ? CID_NOTIFY_SINGLE : CID_NOTIFY_MULTIPLE);
    return(TRUE);
}


static LPCSTR notifySelect_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_NOTIFY_SELECT, id));
}



static void notifySelect_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_NOTIFY_SELECT);
}



static void notifySelect_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK:
        EndDialog(hwnd, RadioButton(hwnd, CID_NOTIFY_SINGLE, 0));
        break;

    case CID_CANCEL:
		EndDialog(hwnd, CID_CANCEL);
        break;

    }
}

static LRESULT notifySelect_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, notifySelect_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, notifySelect_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, notifySelect_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, notifySelect_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
} 



BOOL _EXPORT CALLBACK notifySelectProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=notifySelect_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

