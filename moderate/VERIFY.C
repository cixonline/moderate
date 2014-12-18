#include <windows.h>
#include <windowsx.h>
#include <ctype.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "globals.h"
#include "moderate.h"
#include "help.h"
#include "setup.h"
#include "verify.h"



typedef struct {
    LPSTR lpFileName;
    char szOldName[15];
} ValidateData, FAR *LPValidateData;



BOOL _EXPORT CALLBACK validateProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LPCSTR extractOne(LPCSTR, LPCSTR, LPSTR, LPSTR, BOOL );

BOOL checkName(LPCSTR lpcFileName, CheckDomain checkDomain)
{
    BOOL bValid=FALSE;

    switch (checkDomain) {

    case CHECK_DOS:
#if defined(_WIN32)
	bValid=lstrlen(lpcFileName)<=255;

	while (bValid && *lpcFileName) {
	    bValid=isascii(*lpcFileName) && isgraph(*lpcFileName) &&
		  *lpcFileName!='&' && *lpcFileName!='/' && *lpcFileName!='*' &&
		  *lpcFileName!='?' && *lpcFileName!='\\';

	    lpcFileName++;
	}
#else
	bValid=lstrlen(lpcFileName)<=12;

	if (bValid) {
	    LPCSTR lpPtr;
	    int nCount;

	    for (lpPtr=lpcFileName, nCount=0; nCount<8 && bValid && *lpPtr && *lpPtr!='.'; lpPtr++, nCount++)
		bValid=isascii(*lpPtr) && (isalnum(*lpPtr) || *lpPtr=='-' || *lpPtr=='_');

	    if (bValid) {
		if (*lpPtr=='.')
		    for (nCount=0; *(++lpPtr) && nCount<3 && bValid; nCount++)
			bValid=isascii(*lpPtr) && (isalnum(*lpPtr) || *lpPtr=='-' || *lpPtr=='_');

		if (bValid)
		    bValid= *lpPtr=='\0';

	    }
	}
#endif
	break;

    case CHECK_CIX:
	bValid=lstrlen(lpcFileName)<=14;

	while (bValid && *lpcFileName) {
	    bValid=isascii(*lpcFileName) && isgraph(*lpcFileName) &&
		  *lpcFileName!='&' && *lpcFileName!='/' && *lpcFileName!='*' &&
		  *lpcFileName!='?' && *lpcFileName!='\\';

	    lpcFileName++;
	}

	break;

    }

    return(bValid);
}



BOOL verifyName(HWND hwnd, LPSTR lpLocalName, LPCSTR lpcCixName)
{
    BOOL bOK=TRUE;
    BOOL bValid=checkName(lpcCixName, CHECK_DOS);

    lstrcpy(lpLocalName, lpcCixName);

    if (!bValid) {
	FARPROC lpProc;

	lpProc=MakeProcInstance(validateProc, lpGlobals->hInst);
	bOK=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_VALIDATE_DOWNLOAD", 
						  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpLocalName);
	FreeProcInstance(lpProc);
    }

    return(bOK);
}



static BOOL validate_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPValidateData lpValidateData=InitDlgData(hwnd, ValidateData);

    lpValidateData->lpFileName=(LPSTR)lParam;
    lstrcpy(lpValidateData->szOldName, lpValidateData->lpFileName);
    SetDlgItemText(hwnd, CID_OLDNAME, lpValidateData->szOldName);
    SetDlgItemText(hwnd, CID_NEWNAME, lpValidateData->szOldName);
#if defined(_WIN32)
    Edit_LimitText(GetDlgItem(hwnd, CID_NEWNAME), 12);
#else
    Edit_LimitText(GetDlgItem(hwnd, CID_NEWNAME), 14);
#endif
    return(TRUE);
}



static LPCSTR validate_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_VALIDATE_DOWNLOAD, id));
}



static void validate_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_VALIDATE_DOWNLOAD);
}



static void validate_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK: {
	LPValidateData lpValidateData=GetDlgData(hwnd, ValidateData);
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

	    if (!checkName(lpStart, CHECK_DOS))
		alert(hwnd, "Not a valid DOS filename!");
	    else {
		lstrcpy(lpValidateData->lpFileName, lpStart);
		EndDialog(hwnd, TRUE);
	    }
	}

	break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, FALSE);
	break;

    }
}



static void validate_OnDestroy(HWND hwnd)
{
    FreeDlgData(hwnd);
}



static LRESULT validate_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, validate_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, validate_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, validate_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, validate_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, validate_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK validateProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=validate_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static LPCSTR extractOne(LPCSTR lpcBuffer, LPCSTR lpcDefaultPath, LPSTR lpPath, LPSTR lpName, BOOL fExplorerStyle)
{
	LPCSTR lpcStart=lpcBuffer;
	LPCSTR lpcLastSlash=NULL;
	LPSTR lpCopy;
	char chSeparator;

	chSeparator = fExplorerStyle ? '\0' : ' ';
	while (*lpcBuffer && *lpcBuffer!=chSeparator)
		{
		if (*lpcBuffer=='\\')
			lpcLastSlash=lpcBuffer;
		lpcBuffer++;
		}
	if (lpcLastSlash)
		{
		BOOL bRelative=FALSE;
		LPCSTR lpcPathPtr=lpcDefaultPath+lstrlen(lpcDefaultPath);

		while (lpcStart[0]=='.' && lpcStart[1]=='.' && lpcStart[2]=='\\')
			{
			bRelative=TRUE;
			while (--lpcPathPtr>=lpcDefaultPath && *lpcPathPtr!='\\')
				lpcStart+=3;
			}
		lpCopy=lpPath;
		if (bRelative)
			while (lpcDefaultPath<=lpcPathPtr)
				*lpCopy++ = *lpcDefaultPath++;

		while (lpcStart<lpcLastSlash)
			*lpCopy++ = *lpcStart++;

		*lpCopy='\0';
		lpcStart=lpcLastSlash+1;
		}
	else
		lstrcpy(lpPath, lpcDefaultPath);

	lpCopy=lpName;
	while (lpcStart<lpcBuffer)
		*lpCopy++ = *lpcStart++;
	*lpCopy='\0';
	AnsiLower(lpPath);
	AnsiLower(lpName);

	while (*lpcBuffer && *lpcBuffer==chSeparator)
		lpcBuffer++;
	if( fExplorerStyle && *lpcBuffer == '\0' )
		++lpcBuffer;

	return(*lpcBuffer ? lpcBuffer : NULL);
}



BOOL extractName(LPCSTR lpcBuff, LPSTR lpPath, LPSTR lpName, BOOL fExplorerStyle)
{
	static LPCSTR lpcBuffer=NULL;
	static char szDefaultPath[256];
	char chSeparator;

	chSeparator = fExplorerStyle ? '\0' : ' ';
	if (lpcBuff)
		{
		LPSTR lpLastSlash=NULL;
		LPSTR defaultPtr=szDefaultPath;

		lpcBuffer=lpcBuff;
		while (*lpcBuffer && *lpcBuffer!=chSeparator)
			{
			if (*lpcBuffer=='\\')
				lpLastSlash=defaultPtr;
			*defaultPtr++ = *lpcBuffer++;
			}
		if( fExplorerStyle && *(lpcBuffer+1) == '\0' )
			{
			*lpLastSlash='\0';
			lstrcpy(lpPath, szDefaultPath);
			AnsiLower(lpPath);
			lstrcpy(lpName, lpcBuff+(lpLastSlash-(LPSTR)szDefaultPath+1));
			AnsiLower(lpName);
			lpcBuffer=NULL;
			return(TRUE);
			}
		else if (*lpcBuffer == chSeparator)
			{
			*defaultPtr='\0';
			while (*(++lpcBuffer)==chSeparator);

			if (*lpcBuffer)
				lpcBuffer=extractOne(lpcBuffer, szDefaultPath, lpPath, lpName, fExplorerStyle);
			return(TRUE);
			}
		else if (lpLastSlash)
			{
			*lpLastSlash='\0';
			lstrcpy(lpPath, szDefaultPath);
			AnsiLower(lpPath);
			lstrcpy(lpName, lpcBuff+(lpLastSlash-(LPSTR)szDefaultPath+1));
			AnsiLower(lpName);
			lpcBuffer=NULL;
			return(TRUE);
			}
		else
			{
			lpcBuffer=NULL;
			return(FALSE);
			}
		}
	else if (lpcBuffer)
		{
		lpcBuffer=extractOne(lpcBuffer, szDefaultPath, lpPath, lpName, fExplorerStyle);
		return(TRUE);
		}
	else
		return(FALSE);
}
