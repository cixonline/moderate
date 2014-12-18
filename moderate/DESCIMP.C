#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "moderate.h"
#include "help.h"
#include "globals.h"
#include "flist.h"



BOOL _EXPORT CALLBACK importDescProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



static void saveDescription(HWND hwndList, LPFlistEntry lpFlistEntry, LPSTR lpDescription)
{
    LPSTR lpDescPtr=lpDescription;
    int nFlistIndex=ListBox_FindString(hwndList, -1, lpFlistEntry);

    while (*lpDescPtr) {
	LPSTR lpStartPtr=lpDescPtr;
	LPSTR lpBreakPtr=NULL;
	LPSTR lpCopyPtr=lpFlistEntry->flistDescription;
	int nDescCount=0;

	while (*lpDescPtr && ++nDescCount<54) {
	    if (isspace(*lpDescPtr))
		lpBreakPtr=lpDescPtr;

	    lpDescPtr++;
	}

	if (*lpDescPtr && lpBreakPtr)
	    lpDescPtr=lpBreakPtr+1;

	while (lpStartPtr<lpDescPtr)
	    *lpCopyPtr++ = *lpStartPtr++;

	*lpCopyPtr='\0';
	ListBox_SetSel(hwndList, TRUE, nFlistIndex);

	if (*lpDescPtr) {
	    nFlistIndex++;

	    if (lpFlistEntry->flistNext && isContinuation(lpFlistEntry->flistNext))
		lpFlistEntry=lpFlistEntry->flistNext;
	    else {
		LPFlistEntry lpPrevEntry=lpFlistEntry;
		int nAddPosition;

		lpFlistEntry=(LPFlistEntry)gmalloc(sizeof(FlistEntry));
		lpFlistEntry->flistPrevious=lpPrevEntry;
		lpFlistEntry->flistNext=lpPrevEntry->flistNext;

		if (lpFlistEntry->flistNext) {
		    lpFlistEntry->flistNext->flistPrevious=lpFlistEntry;
                    nAddPosition=nFlistIndex;
                } else
                    nAddPosition= -1;

                lpPrevEntry->flistNext=lpFlistEntry;
                lpFlistEntry->flistFlags=lpPrevEntry->flistFlags;
                lpFlistEntry->flistSelect=FLIST_TABBED;
		*lpFlistEntry->flistDescription='\0';
		ListBox_InsertItemData(hwndList, nAddPosition, lpFlistEntry);
            }
        }
    }

    while (lpFlistEntry->flistNext && isContinuation(lpFlistEntry->flistNext)) {
        lpFlistEntry=lpFlistEntry->flistNext;
        ListBox_DeleteString(hwndList, ++nFlistIndex);
	lpFlistEntry->flistPrevious->flistNext=lpFlistEntry->flistNext;

	if (lpFlistEntry->flistNext)
            lpFlistEntry->flistNext->flistPrevious=lpFlistEntry->flistPrevious;

        gfree(lpFlistEntry);
		lpFlistEntry = NULL;
    }
}



int importDescriptions(HWND hwnd, LPFlistData lpFlistData)
{
    HWND hwndList=lpFlistData->hwndFlist;
    LPContext lpContext= &lpFlistData->context;
    LPFlistEntry lpFlist=lpFlistData->flist;
    LPOPENFILENAME lpOfn=(LPOPENFILENAME)gmallocz(sizeof(OPENFILENAME));
    TOPICINFO topicInfo;
    char szFilename[LEN_PATHNAME];
    char szInitialDir[LEN_PATHNAME];
    int nFieldSelect=3;
    int nImportCount=0;

    static LPCSTR szFilter=
	"Users' file lists (*.fls)\0*.fls\0"
	"Text files (*.txt)\0*.txt\0"
	"BBS files (*.bbs)\0*.bbs\0"
	"All files (*.*)\0*.*\0";

    GetTopicInfo(lpContext->hTopic, &topicInfo);
    wsprintf(szFilename, "%s.fls", (LPSTR)topicInfo.szFileName);
    wsprintf(szInitialDir, "%s\\flist", (LPSTR)setup.szDataDir);
    lpOfn->lStructSize=sizeof(OPENFILENAME);
    lpOfn->hwndOwner=hwnd;
    lpOfn->lpstrFilter=szFilter;
    lpOfn->nFilterIndex=1;
    lpOfn->lpstrFile=szFilename;
    lpOfn->nMaxFile=LEN_PATHNAME-1;
    lpOfn->lpstrInitialDir=szInitialDir;
    lpOfn->lpstrTitle="Import File Descriptions";
    lpOfn->Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|
		 OFN_SHOWHELP|OFN_ENABLEHOOK|OFN_ENABLETEMPLATE;
    lpOfn->lCustData=(LPARAM)(LPINT)&nFieldSelect;
    lpOfn->hInstance=lpGlobals->hInst;
#ifdef WIN32
    lpOfn->lpfnHook=(LPOFNHOOKPROC) MakeProcInstance((FARPROC)importDescProc, lpGlobals->hInst);
#else
    lpOfn->lpfnHook= (UINT (CALLBACK *)(HWND, UINT, WPARAM, LPARAM)) MakeProcInstance((FARPROC)importDescProc, lpGlobals->hInst);
#endif
    lpOfn->lpTemplateName="DID_IMPORT_SELECT";
    lpFlistData->wHelpContext=CID_IMPORT;

    if (GetOpenFileName(lpOfn)) {
	HSTREAM hStream=openFile(szFilename, OF_READ);

	if (!hStream)
	    alert(hwnd, "Cannot open %s", (LPSTR)szFilename);
	else {
	    char szBuff[256];
	    int nPrevCount=256;
	    LPSTR lpDescription=(LPSTR)gmalloc(2048);
	    LPFlistEntry lpFlistEntry=NULL;
	    BOOL bFirst=TRUE;

	    *lpDescription='\0';

	    while (readLine(hStream, szBuff, 256)>=0) {
		LPSTR lpPtr=szBuff;
		int nCount=0;

		while (*lpPtr && isspace(*lpPtr)) {
		    nCount++;
		    lpPtr++;
		}

		if (nCount>nPrevCount) {
		    lstrcat(lpDescription, " ");
		    lstrcat(lpDescription, lpPtr);
		} else {
		    LPSTR lpToken=_fstrtok(szBuff, " \t");

		    if (lpToken) {
			if (lpFlistEntry) {
			    if (bFirst) {
				bFirst=FALSE;
				ListBox_SetSel(hwndList, FALSE, -1);
			    }

			    saveDescription(hwndList, lpFlistEntry, lpDescription);
			}

			lpFlistEntry=lpFlist;
			*lpDescription='\0';

			while (lpFlistEntry &&
			       !(isFile(lpFlistEntry) &&
				 lstrcmpi(lpFlistEntry->flistName, lpToken)==0))
			    lpFlistEntry=lpFlistEntry->flistNext;

			if (lpFlistEntry) {
			    int nFieldPtr=1;

			    nImportCount++;
			    while (++nFieldPtr<nFieldSelect && (lpToken=_fstrtok(NULL, " \t")));

			    if (lpToken && (lpToken=_fstrtok(NULL, "\r\n"))) {
				while (*lpToken && isspace(*lpToken))
				    lpToken++;

				if (*lpToken)
				    lstrcpy(lpDescription, lpToken);

			    }
			}
		    }
		}
	    }

	    if (lpFlistEntry) {
		if (bFirst)
		    ListBox_SetSel(hwndList, FALSE, -1);

		saveDescription(hwndList, lpFlistEntry, lpDescription);
	    }

	    gfree(lpDescription);
		lpDescription = NULL;
	    closeFile(hStream);
	}
    }

    FreeProcInstance((FARPROC)lpOfn->lpfnHook);
    gfree(lpOfn);
	lpOfn = NULL;
    return(nImportCount);
}



static BOOL importDesc_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPINT lpFieldSelect;
    int nFieldSelect;
    HWND hwndFieldSelect=GetDlgItem(hwnd, CID_FIELD_SELECT);

    lpFieldSelect=(LPINT)((LPOPENFILENAME)lParam)->lCustData;
    setWindowData(hwnd, "fs", lpFieldSelect);

    for (nFieldSelect=2; nFieldSelect<7; nFieldSelect++) {
	char szFieldSelect[2];

	szFieldSelect[0]=nFieldSelect+'0';
	szFieldSelect[1]='\0';
	ComboBox_AddString(hwndFieldSelect, szFieldSelect);
    }

    ComboBox_SetCurSel(hwndFieldSelect, *lpFieldSelect-2);
    return(TRUE);
}



static void importDesc_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_IMPORT);
}



static void importDesc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id==CID_OK) {
        LPINT lpFieldSelect=getWindowData(hwnd, "fs");

        if (lpFieldSelect)
            *lpFieldSelect=ComboBox_GetCurSel(GetDlgItem(hwnd, CID_FIELD_SELECT))+2;

    }
}



static void importDesc_OnDestroy(HWND hwnd)
{
    freeWindowData(hwnd, "fs");
}



static LRESULT importDesc_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, importDesc_OnInitDialog);
    HANDLE_MSG(hwnd, WM_AMHELP, importDesc_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, importDesc_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, importDesc_OnDestroy);

    default:
	return(FALSE);

    }
}



BOOL _EXPORT CALLBACK importDescProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    lResult=importDesc_DlgProc(hwnd, uMsg, wParam, lParam);
    return((BOOL)lResult);
}

