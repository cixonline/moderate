#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#include "amctrls.h"
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "moderate.h"
#include "help.h"
#include "globals.h"
#include "setup.h"
#include "strftime.h"


Setup setup;

BOOL _EXPORT CALLBACK configureProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void getSetup(HWND hwnd)
{
	char szBuffer[128];
	LPSTR lpszToken;
	LPSTR lpPtr;
	
	GetInitialisationFile(setup.szIniFile);
	AmGetPrivateProfileString("Moderate", "dateformat", "%d%b%Y", setup.szDateFormat, 32, setup.szIniFile);
	GetProfileString("intl", "sThousand", "\",\"", szBuffer, 5);
	lpszToken=_fstrtok(szBuffer, "\"");
	
	if (!lpszToken)
		*setup.szThousandsSep='\0';
	else 
	{
		if (lstrlen(lpszToken)>3)
			lpszToken[3]='\0';
		
		lstrcpy(setup.szThousandsSep, lpszToken);
	}
	
	setup.nFdirSelect=AmGetPrivateProfileInt("Moderate", "fdirselect", 0, setup.szIniFile);
	AmGetPrivateProfileString("Moderate", "flistsort", "descending date", szBuffer, 128, setup.szIniFile);
	setup.sortFlist.sortDirection=CID_SORT_DESCENDING;
	setup.sortFlist.sortKey=CID_SORT_DATE;
	lpszToken=_fstrtok(szBuffer, " ,;");
	
	if (lpszToken)
	{
		if (lstrcmpi(lpszToken, "ascending")==0) 
		{
			setup.sortFlist.sortDirection=CID_SORT_ASCENDING;
			lpszToken=_fstrtok(NULL, " ,;");
		} 
		else if (lstrcmpi(lpszToken, "descending")==0) 
		{
			setup.sortFlist.sortDirection=CID_SORT_DESCENDING;
			lpszToken=_fstrtok(NULL, " ,;");
		}
	}
	
	if (lpszToken)
	{
		if (lstrcmpi(lpszToken, "name")==0)
			setup.sortFlist.sortKey=CID_SORT_NAME;
		else if (lstrcmpi(lpszToken, "date")==0)
			setup.sortFlist.sortKey=CID_SORT_DATE;
		else if (lstrcmpi(lpszToken, "size")==0)
			setup.sortFlist.sortKey=CID_SORT_SIZE;
	}	

	AmGetPrivateProfileString("Moderate", "fdirsort", "descending date", szBuffer, 128, setup.szIniFile);
	setup.sortFdir.sortDirection=CID_SORT_DESCENDING;
	setup.sortFdir.sortKey=CID_SORT_DATE;
	lpszToken=_fstrtok(szBuffer, " ,;");
	
	if (lpszToken)
	{
		if (lstrcmpi(lpszToken, "ascending")==0) 
		{
			setup.sortFdir.sortDirection=CID_SORT_ASCENDING;
			lpszToken=_fstrtok(NULL, " ,;");
		} 
		else if (lstrcmpi(lpszToken, "descending")==0) 
		{
			setup.sortFdir.sortDirection=CID_SORT_DESCENDING;
			lpszToken=_fstrtok(NULL, " ,;");
		}
	}	

	if (lpszToken)
	{
		if (lstrcmpi(lpszToken, "name")==0)
			setup.sortFdir.sortKey=CID_SORT_NAME;
		else if (lstrcmpi(lpszToken, "date")==0)
			setup.sortFdir.sortKey=CID_SORT_DATE;
		else if (lstrcmpi(lpszToken, "size")==0)
			setup.sortFdir.sortKey=CID_SORT_SIZE;
	}	

	setup.nConfirmDeletes=AmGetPrivateProfileInt("Moderate", "confirmdeletes", 0, setup.szIniFile);
	GetModuleFileName(InstanceFromWindow(hwnd), setup.szAmeolDir, LEN_PATHNAME);
	
	for (lpPtr=setup.szAmeolDir+lstrlen(setup.szAmeolDir);
		 lpPtr>setup.szAmeolDir && *lpPtr!='\\';
		 lpPtr--);
	
	*lpPtr='\0';
	
	wsprintf(setup.szDataDir, "%s\\data", (LPSTR)setup.szAmeolDir);
	AmGetPrivateProfileString("Directories", "data", setup.szDataDir, setup.szDataDir, LEN_PATHNAME, setup.szIniFile);
	
	wsprintf(setup.szDownloadDir, "%s\\download", (LPSTR)setup.szAmeolDir);
	AmGetPrivateProfileString("Directories", "download", setup.szDownloadDir, setup.szDownloadDir, LEN_PATHNAME, setup.szIniFile);
	AmGetPrivateProfileString("Moderate", "download", setup.szDownloadDir, setup.szDownloadDir, LEN_PATHNAME, setup.szIniFile);
	
	wsprintf(setup.szUploadDir, "%s\\upload", (LPSTR)setup.szAmeolDir);
	AmGetPrivateProfileString("Directories", "upload", setup.szUploadDir, setup.szUploadDir, LEN_PATHNAME, setup.szIniFile);
	AmGetPrivateProfileString("Moderate", "upload", setup.szUploadDir, setup.szUploadDir, LEN_PATHNAME, setup.szIniFile);
		  
	setup.bPrefixMenuCommands=AmGetPrivateProfileBool("Moderate", "menuprefix", FALSE, setup.szIniFile);
	setup.bNotifyAuto=AmGetPrivateProfileBool("Moderate", "autonotify", FALSE, setup.szIniFile);
	setup.bNotifyMethod=AmGetPrivateProfileInt("Moderate", "notifysingle", 0, setup.szIniFile);
	setup.bNotifySig=AmGetPrivateProfileBool("Moderate", "notifysignature", FALSE, setup.szIniFile);
	setup.bNotifySize=AmGetPrivateProfileBool("Moderate", "notifysize", TRUE, setup.szIniFile);
	setup.wNotifyCase=AmGetPrivateProfileInt("Moderate", "notifycase", 0, setup.szIniFile)%3;
	setup.bEditSeedMessages=AmGetPrivateProfileBool("Moderate", "editseed", FALSE, setup.szIniFile);
	setup.bCreateMissingTopics=AmGetPrivateProfileBool("Moderate", "createmissing", TRUE, setup.szIniFile);
	setup.bTopicsizeWarning=AmGetPrivateProfileBool("Moderate", "topicsizewarning", TRUE, setup.szIniFile);
	setup.bCreateConfHelp=AmGetPrivateProfileBool("Moderate", "createconfhelp", TRUE, setup.szIniFile);
	setup.bDebug=AmGetPrivateProfileBool("Moderate", "debug", FALSE, setup.szIniFile);
	setup.dwCommdlgBuffsize=AmGetPrivateProfileLong("Moderate", "commdlgbuffsize", 4096, setup.szIniFile);
	setup.rememberUp=AmGetPrivateProfileBool("Moderate", "RememberUp", TRUE, setup.szIniFile);
	setup.os_and_status=AmGetPrivateProfileBool("Moderate", "os and status", TRUE, setup.szIniFile);
	
	if (setup.dwCommdlgBuffsize<1024L || 32768L<setup.dwCommdlgBuffsize) 
	{
		alert(hwnd, "commdlgbuffsize=%ld in %s is out of range or illegal; "
			"set to default value of 4096",
			setup.dwCommdlgBuffsize, (LPSTR)setup.szIniFile);
		
		setup.dwCommdlgBuffsize=4096;
	}
	
	GetModuleFileName(lpGlobals->hInst, setup.szHelpFile, LEN_PATHNAME);
	strcpy(strrchr(setup.szHelpFile, '\\'), "\\moderate.chm");
	
	if (_access(setup.szHelpFile, 0)!=0) 
	{
		wsprintf(setup.szHelpFile, "%s\\moderate.chm", (LPSTR)setup.szAmeolDir);
		
		if (_access(setup.szHelpFile, 0)!=0) 
		{
			wsprintf(setup.szHelpFile, "%s\\addons", (LPSTR)setup.szAmeolDir);
			
			AmGetPrivateProfileString("Directories", "addons", setup.szHelpFile,
				setup.szHelpFile, LEN_PATHNAME, setup.szIniFile);
			
			lstrcat(setup.szHelpFile, "\\moderate.chm");
			
			if (_access(setup.szHelpFile, 0)!=0)
				strcpy(setup.szHelpFile, "moderate.chm");
		}
	}
}



void putSetup(void)
{
	AmWritePrivateProfileString("Moderate", "dateformat", setup.szDateFormat, setup.szIniFile);
	AmWritePrivateProfileInt("Moderate", "fdirselect", setup.nFdirSelect, setup.szIniFile);
	
	AmWritePrivateProfileItem("Moderate", "flistsort", setup.szIniFile, "%s %s",
		(LPSTR)(setup.sortFlist.sortDirection==CID_SORT_ASCENDING ? "ascending" : "descending"),
		(LPSTR)(setup.sortFlist.sortKey==CID_SORT_NAME ? "name" :
	setup.sortFlist.sortKey==CID_SORT_DATE ? "date" : "size"));
	
	AmWritePrivateProfileItem("Moderate", "fdirsort", setup.szIniFile, "%s %s",
		(LPSTR)(setup.sortFdir.sortDirection==CID_SORT_ASCENDING ? "ascending" : "descending"),
		(LPSTR)(setup.sortFdir.sortKey==CID_SORT_NAME ? "name" :
	setup.sortFdir.sortKey==CID_SORT_DATE ? "date" : "size"));
	
	AmWritePrivateProfileInt   ("Moderate", "confirmdeletes", setup.nConfirmDeletes, setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "download", setup.szDownloadDir, setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "upload", setup.szUploadDir, setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "menuprefix", setup.bPrefixMenuCommands ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "autonotify", setup.bNotifyAuto ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileInt   ("Moderate", "notifysingle", setup.bNotifyMethod, setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "notifysignature", setup.bNotifySig ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "notifysize", setup.bNotifySize ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileInt   ("Moderate", "notifycase", setup.wNotifyCase, setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "editseed", setup.bEditSeedMessages ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "createmissing", setup.bCreateMissingTopics ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "topicsizewarning", setup.bTopicsizeWarning ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "createconfhelp", setup.bCreateConfHelp ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "debug", setup.bDebug ? "on" : "off", setup.szIniFile);
	AmWritePrivateProfileLong  ("Moderate", "commdlgbuffsize", setup.dwCommdlgBuffsize, setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "RememberUp", setup.rememberUp ? "yes" : "no", setup.szIniFile);
	AmWritePrivateProfileString("Moderate", "os and status", setup.os_and_status ? "yes" : "no", setup.szIniFile);
}



static void exampleDate(HWND hwnd)
{
	time_t now=time(NULL);
	char szFormat[32];
	char szBuff[64];
	
	GetDlgItemText(hwnd, CID_DATE_FORMAT, szFormat, 32);
	strftime(szBuff, 63, szFormat, localtime(&now));
	SetDlgItemText(hwnd, CID_DATE_EXAMPLE, szBuff);
}



static BOOL configGeneral_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	Edit_LimitText(GetDlgItem(hwnd, CID_DOWNLOAD_DIR), LEN_PATHNAME-1);
	SetDlgItemText(hwnd, CID_DOWNLOAD_DIR, setup.szDownloadDir);
	Edit_LimitText(GetDlgItem(hwnd, CID_UPLOAD_DIR), LEN_PATHNAME-1);
	SetDlgItemText(hwnd, CID_UPLOAD_DIR, setup.szUploadDir);
	Edit_LimitText(GetDlgItem(hwnd, CID_DATE_FORMAT), 31);
	SetDlgItemText(hwnd, CID_DATE_FORMAT, setup.szDateFormat);
	SetWindowFont(GetDlgItem(hwnd, CID_DATE_EXAMPLE), lpGlobals->hfNormal, FALSE);
	exampleDate(hwnd);
	
	CheckDlgButton(hwnd, CID_MENU_PREFIX, setup.bPrefixMenuCommands);
	
	CheckDlgButton(hwnd, CID_REMEMBER_UP, setup.rememberUp);
//	CheckDlgButton(hwnd, CID_NOTIFY_OS_STATUS, setup.os_and_status);
	
	CheckDlgButton(hwnd, CID_EDIT_SEED, setup.bEditSeedMessages);
	CheckDlgButton(hwnd, CID_TOPICSIZE_WARNING, setup.bTopicsizeWarning);
	CheckDlgButton(hwnd, CID_LOCAL_TOPIC_DELETE, !setup.bCreateMissingTopics);
	
	return(TRUE);
}



static LPCSTR configGeneral_OnPopupHelp(HWND hwnd, int id)
{
	return(balloonHelp(DID_CONFIGURE, id));
}



static void configGeneral_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) 
	{
		
	case CID_DOWNLOAD_DIR:
	case CID_UPLOAD_DIR:
		{
			if (codeNotify==EN_CHANGE)
				PropSheet_Changed(GetParent(hwnd), hwnd);
			
			break;
		}
	case CID_DATE_FORMAT:
		{
			if (codeNotify==EN_CHANGE) 
			{
				exampleDate(hwnd);
				PropSheet_Changed(GetParent(hwnd), hwnd);
			}
			
			break;
		}
	case CID_DOWNLOAD_BROWSE: 
		{
			CHGDIR chgDir;
			
			chgDir.hwnd=hwnd;
			lstrcpy(chgDir.szTitle, "Moderate Download Directory");
			lstrcpy(chgDir.szPrompt, "&New directory for moderator downloads");
			GetDlgItemText(hwnd, CID_DOWNLOAD_DIR, chgDir.szPath, 144);
			
			if (ChangeDirectory(&chgDir))
				SetDlgItemText(hwnd, CID_DOWNLOAD_DIR, chgDir.szPath);
			
			break;
		}
		
	case CID_UPLOAD_BROWSE: 
		{
			CHGDIR chgDir;
			
			chgDir.hwnd=hwnd;
			lstrcpy(chgDir.szTitle, "Moderate Upload Directory");
			lstrcpy(chgDir.szPrompt, "&New directory for moderator uploads");
			GetDlgItemText(hwnd, CID_UPLOAD_DIR, chgDir.szPath, 144);
			
			if (ChangeDirectory(&chgDir))
				SetDlgItemText(hwnd, CID_UPLOAD_DIR, chgDir.szPath);
			
			break;
		}
		
	case CID_MENU_PREFIX:
	case CID_EDIT_SEED:
	case CID_TOPICSIZE_WARNING:
	case CID_LOCAL_TOPIC_DELETE:
	case CID_REMEMBER_UP:
		{
			if (codeNotify==BN_CLICKED)
				PropSheet_Changed(GetParent(hwnd), hwnd);
			
			break;
		}
		
	}
}



static DWORD configGeneral_OnNotify(HWND hwnd, int idFrom, LPNMHDR lpNmhdr)
{
	switch (lpNmhdr->code) 
	{
		
	case PSN_HELP:
		HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_SETTINGS);
		return(PSNRET_NOERROR);
		
	case PSN_APPLY: 
		{
			BOOL bOldPrefixState=setup.bPrefixMenuCommands;
			
			GetDlgItemText(hwnd, CID_DOWNLOAD_DIR, setup.szDownloadDir, LEN_PATHNAME);
			GetDlgItemText(hwnd, CID_UPLOAD_DIR, setup.szUploadDir, LEN_PATHNAME);
			GetDlgItemText(hwnd, CID_DATE_FORMAT, setup.szDateFormat, 32);
			
			setup.bPrefixMenuCommands=IsDlgButtonChecked(hwnd, CID_MENU_PREFIX);
			
			setup.bEditSeedMessages=IsDlgButtonChecked(hwnd, CID_EDIT_SEED);
			setup.bTopicsizeWarning=IsDlgButtonChecked(hwnd, CID_TOPICSIZE_WARNING);
			setup.bCreateMissingTopics= !IsDlgButtonChecked(hwnd, CID_LOCAL_TOPIC_DELETE);
			
			setup.rememberUp   = IsDlgButtonChecked(hwnd, CID_REMEMBER_UP);
			
			putSetup();
			
			if (xor(setup.bPrefixMenuCommands, bOldPrefixState)) 
			{
				removeAmeolMenus();
				addAmeolMenus();
			}
			
			PropSheet_UnChanged(GetParent(hwnd), hwnd);
			return(PSNRET_NOERROR);
		}
		
	default:
		return(PSNRET_NOERROR);
		
	}
}



static LRESULT configGeneral_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		
		HANDLE_MSG(hwnd, WM_INITDIALOG, configGeneral_OnInitDialog);
		HANDLE_MSG(hwnd, WM_POPUPHELP, configGeneral_OnPopupHelp);
		HANDLE_MSG(hwnd, WM_COMMAND, configGeneral_OnCommand);
		HANDLE_MSG(hwnd, WM_NOTIFY, configGeneral_OnNotify);
		
	default:
		return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));
		
	}
}



BOOL _EXPORT CALLBACK configGeneralProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	
	CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
	lResult=configGeneral_DlgProc(hwnd, uMsg, wParam, lParam);
	return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static BOOL configFlist_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	int flistConfirm=setup.nConfirmDeletes%10;
	int flistFdirConfirm=(setup.nConfirmDeletes/10)%10;
	
	RadioButton(hwnd, CID_FLIST_SORT_NAME,
		setup.sortFlist.sortKey==CID_SORT_NAME ? CID_FLIST_SORT_NAME :
	setup.sortFlist.sortKey==CID_SORT_DATE ? CID_FLIST_SORT_DATE :
	CID_FLIST_SORT_SIZE);
	
	RadioButton(hwnd, CID_FLIST_SORT_ASCENDING,
		setup.sortFlist.sortDirection==CID_SORT_ASCENDING ? CID_FLIST_SORT_ASCENDING :
	CID_FLIST_SORT_DESCENDING);
	
	CheckDlgButton(hwnd, CID_AUTO_NOTIFY, setup.bNotifyAuto);
	CheckDlgButton(hwnd, CID_NOTIFY_SIGNATURE, setup.bNotifySig);
	CheckDlgButton(hwnd, CID_NOTIFY_SIZE, setup.bNotifySize);
	CheckDlgButton(hwnd, CID_NOTIFY_UPPERCASE, setup.wNotifyCase);
	CheckDlgButton(hwnd, CID_REMEMBER_UP, setup.rememberUp);
	CheckDlgButton(hwnd, CID_NOTIFY_OS_STATUS, setup.os_and_status);

	
	RadioButton(hwnd, CID_NOTIFY_PROMPT,
		setup.bNotifyMethod==0 ? CID_NOTIFY_PROMPT :
	setup.bNotifyMethod==1 ? CID_NOTIFY_MULTIPLE : CID_NOTIFY_SINGLE);
	
	RadioButton(hwnd, CID_CONFIRM_FDIR,
		flistFdirConfirm==0 ? CID_CONFIRM_FDIR :
	flistFdirConfirm==1 ? CID_LEAVE_FDIR : CID_DELETE_ALL_FDIR);
	
	CheckDlgButton(hwnd, CID_CONFIRM_FLIST, flistConfirm==0);
	
	return(TRUE);
}



static LPCSTR configFlist_OnPopupHelp(HWND hwnd, int id)
{
	return(balloonHelp(DID_CONFIGURE, id));
}



static void configFlist_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) 
	{
		
	case CID_FLIST_SORT_NAME:
	case CID_FLIST_SORT_DATE:
	case CID_FLIST_SORT_SIZE:
	case CID_FLIST_SORT_ASCENDING:
	case CID_FLIST_SORT_DESCENDING:
	case CID_AUTO_NOTIFY:
	case CID_NOTIFY_SIGNATURE:
	case CID_NOTIFY_MULTIPLE:
	case CID_NOTIFY_SIZE:
	case CID_NOTIFY_UPPERCASE:
	case CID_CONFIRM_FDIR:
	case CID_LEAVE_FDIR:
	case CID_DELETE_ALL_FDIR:
	case CID_CONFIRM_FLIST:
	case CID_NOTIFY_OS_STATUS:
	case CID_NOTIFY_PROMPT:	
	case CID_NOTIFY_SINGLE:
		{
			if (codeNotify==BN_CLICKED)
				PropSheet_Changed(GetParent(hwnd), hwnd);
			
			break;
		}
	}
}



static DWORD configFlist_OnNotify(HWND hwnd, int idFrom, LPNMHDR lpNmhdr)
{
	switch (lpNmhdr->code) 
	{
		
	case PSN_HELP:
		HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_SETTINGS_FLIST);
		return(PSNRET_NOERROR);
		
	case PSN_APPLY: 
		{
			int flistConfirm=0;
			int flistFdirConfirm=0;
			
			switch (RadioButton(hwnd, CID_FLIST_SORT_NAME, 0)) 
			{
				
			case CID_FLIST_SORT_NAME:		setup.sortFlist.sortKey=CID_SORT_NAME;	break;
			case CID_FLIST_SORT_DATE:		setup.sortFlist.sortKey=CID_SORT_DATE;	break;
			case CID_FLIST_SORT_SIZE:		setup.sortFlist.sortKey=CID_SORT_SIZE;	break;
				
			}
			
			switch (RadioButton(hwnd, CID_FLIST_SORT_ASCENDING, 0)) 
			{
				
			case CID_FLIST_SORT_ASCENDING:	setup.sortFlist.sortDirection=CID_SORT_ASCENDING;  break;
			case CID_FLIST_SORT_DESCENDING: setup.sortFlist.sortDirection=CID_SORT_DESCENDING; break;
				
			}
			
			setup.bNotifyAuto   = IsDlgButtonChecked(hwnd, CID_AUTO_NOTIFY);
			setup.bNotifySig    = IsDlgButtonChecked(hwnd, CID_NOTIFY_SIGNATURE);
			setup.bNotifySize   = IsDlgButtonChecked(hwnd, CID_NOTIFY_SIZE);
			setup.wNotifyCase   = IsDlgButtonChecked(hwnd, CID_NOTIFY_UPPERCASE);
			setup.os_and_status = IsDlgButtonChecked(hwnd, CID_NOTIFY_OS_STATUS);
			
			switch (RadioButton(hwnd, CID_CONFIRM_FDIR, 0)) 
			{
				
			case CID_CONFIRM_FDIR:			flistFdirConfirm=0;   break;
			case CID_LEAVE_FDIR:			flistFdirConfirm=1;   break;
			case CID_DELETE_ALL_FDIR:		flistFdirConfirm=2;   break;
				
			}
			switch (RadioButton(hwnd, CID_NOTIFY_PROMPT, 0)) 
			{
				
			case CID_NOTIFY_PROMPT:			setup.bNotifyMethod=0;   break;
			case CID_NOTIFY_MULTIPLE:		setup.bNotifyMethod=1;   break;
			case CID_NOTIFY_SINGLE:			setup.bNotifyMethod=2;   break;
				
			}
			
			flistConfirm=IsDlgButtonChecked(hwnd, CID_CONFIRM_FLIST) ? 0 : 1;
			setup.nConfirmDeletes=(setup.nConfirmDeletes/100)*100+flistFdirConfirm*10+flistConfirm;
			putSetup();
			PropSheet_UnChanged(GetParent(hwnd), hwnd);
			return(PSNRET_NOERROR);
		}
		
	default:
		return(PSNRET_NOERROR);
		
	}
}



static LRESULT configFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		
		HANDLE_MSG(hwnd, WM_INITDIALOG, configFlist_OnInitDialog);
		HANDLE_MSG(hwnd, WM_POPUPHELP, configFlist_OnPopupHelp);
		HANDLE_MSG(hwnd, WM_COMMAND, configFlist_OnCommand);
		HANDLE_MSG(hwnd, WM_NOTIFY, configFlist_OnNotify);
		
	default:
		return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));
		
	}
}



BOOL _EXPORT CALLBACK configFlistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	
	CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
	lResult=configFlist_DlgProc(hwnd, uMsg, wParam, lParam);
	return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static BOOL configFdir_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	int fdirConfirm=(setup.nConfirmDeletes/100)%10;
	
	RadioButton(hwnd, CID_ORPHANS,
		setup.nFdirSelect==0 ? CID_ORPHANS :
	setup.nFdirSelect==1 ? CID_IN_FLIST : CID_ALL_FILES);
	
	RadioButton(hwnd, CID_SORT_NAME, setup.sortFdir.sortKey);
	RadioButton(hwnd, CID_SORT_ASCENDING, setup.sortFdir.sortDirection);
	
	RadioButton(hwnd, CID_CONFIRM_ALL_FDIR,
		fdirConfirm==0 ? CID_CONFIRM_ALL_FDIR :
	fdirConfirm==1 ? CID_CONFIRM_FLIST_FDIR : CID_CONFIRM_NONE_FDIR);
	
	return(TRUE);
}



static LPCSTR configFdir_OnPopupHelp(HWND hwnd, int id)
{
	return(balloonHelp(DID_CONFIGURE, id));
}



static void configFdir_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) 
	{
		
	case CID_ORPHANS:
	case CID_IN_FLIST:
	case CID_ALL_FILES:
	case CID_SORT_NAME:
	case CID_SORT_DATE:
	case CID_SORT_SIZE:
	case CID_SORT_ASCENDING:
	case CID_SORT_DESCENDING:
	case CID_CONFIRM_ALL_FDIR:
	case CID_CONFIRM_FLIST_FDIR:
	case CID_CONFIRM_NONE_FDIR:
		{
			if (codeNotify==BN_CLICKED)
				PropSheet_Changed(GetParent(hwnd), hwnd);
			
			break;
		}		
	}
}



static DWORD configFdir_OnNotify(HWND hwnd, int idFrom, LPNMHDR lpNmhdr)
{
	switch (lpNmhdr->code) 
	{
		
	case PSN_HELP:
		{
			HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_SETTINGS_FDIR);
			return(PSNRET_NOERROR);
		}
		
	case PSN_APPLY: 
		{
			int fdirConfirm=0;
			
			switch (RadioButton(hwnd, CID_ORPHANS, 0)) 
			{
			case CID_ORPHANS:			 setup.nFdirSelect=0;  break;
			case CID_IN_FLIST:			 setup.nFdirSelect=1;  break;
			case CID_ALL_FILES: 		 setup.nFdirSelect=2;  break;
			}
			
			setup.sortFdir.sortKey=RadioButton(hwnd, CID_SORT_NAME, 0);
			setup.sortFdir.sortDirection=RadioButton(hwnd, CID_SORT_ASCENDING, 0);
			
			switch (RadioButton(hwnd, CID_CONFIRM_ALL_FDIR, 0)) 
			{
			case CID_CONFIRM_ALL_FDIR:	 fdirConfirm=0; 	   break;
			case CID_CONFIRM_FLIST_FDIR: fdirConfirm=1; 	   break;
			case CID_CONFIRM_NONE_FDIR:  fdirConfirm=2; 	   break;
			}
			
			setup.nConfirmDeletes=setup.nConfirmDeletes%100+fdirConfirm*100;
			putSetup();
			PropSheet_UnChanged(GetParent(hwnd), hwnd);
			return(PSNRET_NOERROR);
		}
		
	default:
		return(PSNRET_NOERROR);
		
	}
}



static LRESULT configFdir_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		
		HANDLE_MSG(hwnd, WM_INITDIALOG, configFdir_OnInitDialog);
		HANDLE_MSG(hwnd, WM_POPUPHELP, configFdir_OnPopupHelp);
		HANDLE_MSG(hwnd, WM_COMMAND, configFdir_OnCommand);
		HANDLE_MSG(hwnd, WM_NOTIFY, configFdir_OnNotify);
		
	default:
		return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));
		
	}
}



BOOL _EXPORT CALLBACK configFdirProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	
	CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
	lResult=configFdir_DlgProc(hwnd, uMsg, wParam, lParam);
	return(SetDlgMsgResult(hwnd, uMsg, lResult));
}

void newConfigureSetup(HWND hwnd)
{
	static struct {
		LPCSTR pszTitle;
		LPCSTR pszTemplate;
		DLGPROC pfnDlgProc;
	} pspInfo[]={
		{"Mod: General", "DID_CONFIG_GENERAL_CA", configGeneralProc},
		{"Mod: Flist", "DID_CONFIG_FLIST_CA", configFlistProc},
		{"Mod: FDir", "DID_CONFIG_FDIR_CA", configFdirProc}
	};
	
	AMPROPSHEETPAGE psp[3];
	int nSheet;
	
	for (nSheet=0; nSheet<3; nSheet++) 
	{
		memset(&psp[nSheet], 0, sizeof(AMPROPSHEETPAGE));
		psp[nSheet].dwSize=sizeof(AMPROPSHEETPAGE);
		psp[nSheet].dwFlags=PSP_USETITLE|PSP_HASHELP;
		psp[nSheet].hInstance=lpGlobals->hInst;
		psp[nSheet].pszTemplate=pspInfo[nSheet].pszTemplate;
		psp[nSheet].pfnDlgProc=pspInfo[nSheet].pfnDlgProc;
		psp[nSheet].pszTitle=pspInfo[nSheet].pszTitle;
		psp[nSheet].lParam=0;
		psp[nSheet].pfnCallback = NULL;
		
		PropSheet_AddPage( hwnd, &psp[nSheet]);
	}
}


void configureSetup(HWND hwnd)
{
	static struct {
		LPCSTR pszTitle;
		LPCSTR pszTemplate;
		DLGPROC pfnDlgProc;
	} pspInfo[]={
		{"General", "DID_CONFIG_GENERAL", configGeneralProc},
		{"Flist", "DID_CONFIG_FLIST", configFlistProc},
		{"FDir", "DID_CONFIG_FDIR", configFdirProc}
	};
	
	AMPROPSHEETPAGE psp[3];
	AMPROPSHEETHEADER psh;
	int nSheet;
	
	for (nSheet=0; nSheet<3; nSheet++) 
	{
		memset(&psp[nSheet], 0, sizeof(AMPROPSHEETPAGE));
		psp[nSheet].dwSize=sizeof(AMPROPSHEETPAGE);
		psp[nSheet].dwFlags=PSP_USETITLE|PSP_HASHELP;
		psp[nSheet].hInstance=lpGlobals->hInst;
		psp[nSheet].pszTemplate=pspInfo[nSheet].pszTemplate;
		psp[nSheet].pfnDlgProc=pspInfo[nSheet].pfnDlgProc;
		psp[nSheet].pszTitle=pspInfo[nSheet].pszTitle;
		psp[nSheet].lParam=0;
		psp[nSheet].pfnCallback = NULL;
	}
	
	memset(&psh, 0, sizeof(AMPROPSHEETHEADER));
	psh.dwSize=sizeof(AMPROPSHEETHEADER);
	psh.dwFlags=PSH_PROPSHEETPAGE|PSH_HASHELP;
	psh.hwndParent=hwnd;
	psh.hInstance=lpGlobals->hInst;
	psh.pszCaption="Moderate Settings";
	psh.nPages=3;
	psh.nStartPage=0;
	psh.ppsp=psp;
	psh.pfnCallback = NULL;
	
	Amctl_PropertySheet(&psh);
}
