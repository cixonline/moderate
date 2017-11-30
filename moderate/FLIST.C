#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <sys/stat.h>
#include <commdlg.h>
#include "amctrls.h"
#include "ameolapi.h"
#include "winhorus.h"
#include "tabcntrl.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "verify.h"
#include "status.h"
#include "globals.h"
#include "flist.h"
#include "export.h"
#include "strftime.h"
#include "malloc.h"

extern char str_ClassName[15];
extern char str_Window[9];
extern char str_Left[14];
extern char str_Top[12];
extern char str_Width[14];
extern char str_Height[15];

//RECT	rcOrg;
//RECT	rcLast = {0,0,0,0};

BOOL FAR PASCAL IsAmeolQuitting( void );

#define	WritePPInt(lpkey,lpsect,value)	(AmWritePrivateProfileInt((lpkey),(lpsect),(value),setup.szIniFile))

#undef GetMDIData
#undef SetMDIData
#undef InitMDIData
#undef FreeMDIData

#define GetMDIData(hDlg, DataType) \
    (DataType)GetWindowLong((hDlg), MOD_DATA)

#define SetMDIData(hDlg, lpData) \
    SetWindowLong((hDlg), MOD_DATA, (DWORD)(LPVOID)(lpData))

#define InitMDIData(hDlg, DataType) \
    (SetMDIData((hDlg), gmallocz(sizeof(DataType))), \
     GetMDIData((hDlg), DataType))

#define FreeMDIData(hDlg) \
    gfree((LPVOID)GetWindowLong((hDlg), MOD_DATA))


LONG _EXPORT CALLBACK listboxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK tabFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK tabFdir_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WINAPI ResizeFlistDialog(HWND hWnd, int dx, int dy);
void WINAPI ResizeFDirDialog(HWND hDlg, int dx, int dy);
void WINAPI ResizeFrame(HWND hDlg, int x, int y);
void InflateWnd( HWND hDlg, int dx, int dy );
void MoveWnd( HWND hDlg, int dx, int dy );
long FAR PASCAL EXPORT NewListProc(HWND hwndList, 
                                     UINT message, 
                                     WPARAM wParam, 
                                     LPARAM lParam);
void FAR PASCAL _EXPORT ShowHideChildren(HWND hwnd, LPTabData lpTabData, BOOL show);

static TabDescription tabFlist[] = {
    {"Edit File List", "DID_FLIST", tabFlist_DlgProc},
    {"File Directory", "DID_FILE_DIRECTORY", tabFdir_DlgProc}
};

void WINAPI RePositionControl(HWND hDlg, int id)
{
int dx, dy;
RECT rc;
LPFlistData lpFlistData;
#ifndef WIN32
	lpFlistData=(LPFlistData)GetWindowLong(hDlg, MOD_DATA);
#else
	lpFlistData=(LPFlistData)GetWindowLong(hDlg, MOD_DATA);
#endif
	if(!lpFlistData)
		return;

	GetClientRect(GetDlgItem( hDlg, CID_FLIST_FRAME ), &rc);

	if(rc.right != rc.left)
	{
		dx = (rc.right - rc.left) - (lpFlistData->rcOrg.right-lpFlistData->rcOrg.left);
		dy = (rc.bottom - rc.top) - (lpFlistData->rcOrg.bottom-lpFlistData->rcOrg.top);
	}
	else
	{
		dx = (lpFlistData->rcOrg.right - lpFlistData->rcOrg.left) - (lpFlistData->rcLast.right-lpFlistData->rcLast.left);
		dy = (lpFlistData->rcOrg.bottom - lpFlistData->rcOrg.top) - (lpFlistData->rcLast.bottom-lpFlistData->rcLast.top);
	}

	ShowWindow( GetDlgItem( hDlg, IDT_STAT     ), SW_HIDE);
	ShowWindow( GetDlgItem( hDlg, CID_FLX_DATE ), SW_HIDE);

	switch(id)
	{
	case IDT_MOD:
		MoveWnd( GetDlgItem( hDlg, IDT_MOD ),              0,  dy );
		break;
	case IDT_FDIR:
		MoveWnd( GetDlgItem( hDlg, IDT_FDIR ),             0,  dy );
		break;
	case IDT_STAT:
		MoveWnd( GetDlgItem( hDlg, IDT_STAT ),             0,  dy );
		break;
	case CID_FLM_DATE:
		MoveWnd( GetDlgItem( hDlg, CID_FLM_DATE ),         0,  dy );
		break;
	case CID_FLD_DATE:
		MoveWnd( GetDlgItem( hDlg, CID_FLD_DATE ),         0,  dy );
		break;
	case CID_FLX_DATE:
		MoveWnd( GetDlgItem( hDlg, CID_FLX_DATE ),         0,  dy );
		break;
	case CID_OK:
		MoveWnd( GetDlgItem( hDlg, CID_OK ),               dx,  0 );
		break;
	case CID_CANCEL:
		MoveWnd( GetDlgItem( hDlg, CID_CANCEL ),           dx,  0 );
		break;
	case IDD_HELP:
		MoveWnd( GetDlgItem( hDlg, IDD_HELP ),             dx,  0 );
	case CID_CHANGE_TOPIC:
		MoveWnd( GetDlgItem( hDlg, CID_CHANGE_TOPIC ),     dx,  0 );
		break;
	case CID_DOWNLOAD_LISTS:
		MoveWnd( GetDlgItem( hDlg, CID_DOWNLOAD_LISTS ),   dx,  0 );
		break;
	case CID_INSERT:
		MoveWnd( GetDlgItem( hDlg, CID_INSERT ),           dx,  0 );
		break;
	case CID_APPEND:
		MoveWnd( GetDlgItem( hDlg, CID_APPEND ),           dx,  0 );
		break;
	case CID_UPDATE_FILE:
		MoveWnd( GetDlgItem( hDlg, CID_UPDATE_FILE ),      dx,  0 );
		break;
	case CID_EDIT:
		MoveWnd( GetDlgItem( hDlg, CID_EDIT ),             dx,  0 );
		break;
	case CID_IMPORT:
		MoveWnd( GetDlgItem( hDlg, CID_IMPORT ),           dx,  0 );
		break;
	case CID_MOVE_UP:
		MoveWnd( GetDlgItem( hDlg, CID_MOVE_UP ),          dx,  0 );
		break;
	case CID_MOVE_DOWN:
		MoveWnd( GetDlgItem( hDlg, CID_MOVE_DOWN ),        dx,  0 );
		break;
	case CID_ACTION:
		MoveWnd( GetDlgItem( hDlg, CID_ACTION ),           dx,  0 );
		break;
	case CID_SORT:
		MoveWnd( GetDlgItem( hDlg, CID_SORT ),             dx,  0 );
		break;
	case CID_UPLOAD_FLIST:
		MoveWnd( GetDlgItem( hDlg, CID_UPLOAD_FLIST ),     dx,  0 );
		break;
	case CID_FILE_LIST:
		InflateWnd( GetDlgItem( hDlg, CID_FILE_LIST ),     dx, dy );
		break;
	case CID_ORPHANS:
		MoveWnd( GetDlgItem( hDlg, CID_ORPHANS ),          dx,  0 );
		break;
	case CID_IN_FLIST:
		MoveWnd( GetDlgItem( hDlg, CID_IN_FLIST ),		   dx,  0 );
		break;
	case CID_ALL_FILES:
		MoveWnd( GetDlgItem( hDlg, CID_ALL_FILES ),        dx,  0 );
		break;
	case IDT_VIEW:
		MoveWnd( GetDlgItem( hDlg, IDT_VIEW ),             dx,  0 );
		break;
	case CID_FDIR_LIST:
		InflateWnd( GetDlgItem( hDlg, CID_FDIR_LIST ),     dx, dy );
		break;
	}
}

void insertSelection(LPINT lpItemsSelected, int nItem)
{
    int nFindPtr;

    for (nFindPtr=1; nFindPtr<= *lpItemsSelected && lpItemsSelected[nFindPtr]<nItem; nFindPtr++);

    if (nFindPtr<= *lpItemsSelected) 
	{
		int nMovePtr= *lpItemsSelected+1;

		while (nMovePtr>=nFindPtr) 
		{
			lpItemsSelected[nMovePtr+1]=lpItemsSelected[nMovePtr];
			nMovePtr--;
		}
    }

    lpItemsSelected[nFindPtr]=nItem;
    (*lpItemsSelected)++;
}



void removeSelection(LPINT lpItemsSelected, int nItem)
{
    int nPtr;

    for (nPtr=1; nPtr<= *lpItemsSelected && lpItemsSelected[nPtr]!=nItem; nPtr++);

    if (nPtr<= *lpItemsSelected) 
	{
		(*lpItemsSelected)--;

		while (nPtr<= *lpItemsSelected) 
		{
			lpItemsSelected[nPtr]=lpItemsSelected[nPtr+1];
			nPtr++;
		}
    }
}



static BOOL flistGetSelection(LPFlistData lpFlistData, BOOL bUpdateControls)
{
    LPINT lpSwap=lpFlistData->lpPrevSelected;
    BOOL bCheck=lpFlistData->bFlistSelCheck || bUpdateControls;

    lpFlistData->bFlistSelCheck=TRUE;
    lpFlistData->lpPrevSelected=lpFlistData->lpFlistSelected;
    lpFlistData->lpFlistSelected=lpSwap;
    *lpFlistData->lpFlistSelected=ListBox_GetSelItems(lpFlistData->hwndFlist, MAX_FLIST_SELECTIONS, lpFlistData->lpFlistSelected+1);

    if (!bCheck)
	if (*lpFlistData->lpFlistSelected!= *lpFlistData->lpPrevSelected)
	    bCheck=TRUE;
	else 
	{
	    int nPtr;

	    for (nPtr=1; nPtr<= *lpFlistData->lpFlistSelected && !bCheck; nPtr++)
		if (lpFlistData->lpFlistSelected[nPtr]!=lpFlistData->lpPrevSelected[nPtr])
		    bCheck=TRUE;

	}

    return(bCheck);
}



void flistClearSelection(LPFlistData lpFlistData)
{
    LPFlistEntry lpFlistEntry;
    BOOL bPrev=flistSuspendControlUpdates(lpFlistData);

    if (lpFlistData->hwndFlist)
		ListBox_SetSel(lpFlistData->hwndFlist, FALSE, -1);

    for (lpFlistEntry=lpFlistData->flist; lpFlistEntry; lpFlistEntry=lpFlistEntry->flistNext)
		lpFlistEntry->flistFlags&= ~FLIST_FLAG_SELECTED;

    flistResetSelection(lpFlistData, bPrev);
}



void flistControls(HWND hwnd, LPFlistData lpFlistData, BOOL bUpdateControls)
{
    if (lpFlistData->hwndFlist && flistGetSelection(lpFlistData, bUpdateControls)) 
	{
		FlistSelectCategory fsc=FSC_NONE;
		int nFileCount=0;
		LPFlistEntry lpFlistEntry=lpFlistData->flist;
		int nPtr=1, nIndex=0, nNextIndex= *lpFlistData->lpFlistSelected>0 ? lpFlistData->lpFlistSelected[1] : -1;

		FlistPresenceCategory fpc= !lpFlistData->flistTime	? FPC_MISSING :
				   lpFlistData->flistEntries==0 ? FPC_EMPTY   :
				   lpFlistData->flistFiles==0   ? FPC_NOFILES :
								  FPC_FILES;

		while (lpFlistEntry) 
		{
			if (nIndex!=nNextIndex)
				lpFlistEntry->flistFlags&= ~FLIST_FLAG_SELECTED;
			else 
			{
				lpFlistEntry->flistFlags|=FLIST_FLAG_SELECTED;

				switch (lpFlistEntry->flistSelect) 
				{

					case FLIST_MEMO:
					case FLIST_COMMENT:
					case FLIST_TABBED:
						if (fsc<FSC_STATIC)
							fsc=FSC_STATIC;

						break;

					case FLIST_FROM_FILEPOOL:
						if (fsc<FSC_FILEPOOL)
							fsc=FSC_FILEPOOL;

						nFileCount++;
						break;

					case FLIST_FROM_MAIL_DIR:
					case FLIST_FROM_UPLOAD:
						if (fsc<FSC_NEW)
							fsc=FSC_NEW;

						nFileCount++;
						break;

					case FLIST_FROM_FDIR:
					case FLIST_FROM_NOTIFICATION:
						if (fsc<FSC_LOCAL)
							fsc=FSC_LOCAL;

						nFileCount++;
						break;

				}

				nNextIndex=nPtr< *lpFlistData->lpFlistSelected ? lpFlistData->lpFlistSelected[++nPtr] : -1;
			}

			lpFlistEntry=lpFlistEntry->flistNext;
			nIndex++;
		}

		Button_Enable(GetDlgItem(hwnd, CID_DOWNLOAD_LISTS), !lpFlistData->bCreating);
		Button_Enable(GetDlgItem(hwnd, CID_INSERT), fpc>FPC_MISSING);
		Button_Enable(GetDlgItem(hwnd, CID_APPEND), fpc>FPC_MISSING);
		Button_Enable(GetDlgItem(hwnd, CID_IMPORT), fpc>FPC_NOFILES);
		Button_Enable(GetDlgItem(hwnd, CID_UPLOAD_FLIST), fpc>FPC_EMPTY);
		Button_Enable(GetDlgItem(hwnd, CID_FILE_LIST), fpc>FPC_EMPTY);

		if (bUpdateControls || xor(fsc==FSC_LOCAL, lpFlistData->flistSelectCategory==FSC_LOCAL))
			Button_Enable(GetDlgItem(hwnd, CID_UPDATE_FILE), fsc==FSC_LOCAL);

		if (bUpdateControls || xor(fsc==FSC_NONE, lpFlistData->flistSelectCategory==FSC_NONE)) 
		{
			Button_Enable(GetDlgItem(hwnd, CID_EDIT), fsc!=FSC_NONE);
			Button_Enable(GetDlgItem(hwnd, CID_MOVE_UP), fsc!=FSC_NONE);
			Button_Enable(GetDlgItem(hwnd, CID_MOVE_DOWN), fsc!=FSC_NONE);
			Button_Enable(GetDlgItem(hwnd, CID_ACTION), fsc!=FSC_NONE);
		}

//		if (bUpdateControls ||
//			(xor(fsc>=FSC_FILEPOOL, lpFlistData->flistSelectCategory>=FSC_FILEPOOL) &&
//			xor(nFileCount>1, lpFlistData->bFlistMultipleFiles))//)
//				Button_Enable(GetDlgItem(hwnd, CID_SORT), fsc>=FSC_FILEPOOL);
		if(nFileCount>1)
				Button_Enable(GetDlgItem(hwnd, CID_SORT), TRUE);
		else
				Button_Enable(GetDlgItem(hwnd, CID_SORT), FALSE);

		lpFlistData->flistSelectCategory=fsc;
		lpFlistData->bFlistMultipleFiles=nFileCount>1;
	}
}



static BOOL fdirGetSelection(LPFlistData lpFlistData, BOOL bUpdateControls)
{
    LPINT lpSwap=lpFlistData->lpPrevSelected;
    BOOL bCheck= !lpFlistData->bFdirSelCheck || bUpdateControls;

    lpFlistData->bFdirSelCheck=TRUE;
    lpFlistData->lpPrevSelected=lpFlistData->lpFdirSelected;
    lpFlistData->lpFdirSelected=lpSwap;
    *lpFlistData->lpFdirSelected=ListBox_GetSelItems(lpFlistData->hwndFdir, MAX_FLIST_SELECTIONS, lpFlistData->lpFdirSelected+1);

    if (!bCheck)
	{
		if (*lpFlistData->lpFdirSelected!= *lpFlistData->lpPrevSelected)
		{
			bCheck=TRUE;
		}
		else 
		{
			int nPtr;

			for (nPtr=1; nPtr<= *lpFlistData->lpFdirSelected && !bCheck; nPtr++)
				if (lpFlistData->lpFdirSelected[nPtr]!=lpFlistData->lpPrevSelected[nPtr])
					bCheck=TRUE;
		}
	}

    return(bCheck);
}



void fdirClearSelection(LPFlistData lpFlistData)
{
    LPFdirEntry lpFdirEntry;
    BOOL bPrev=fdirSuspendControlUpdates(lpFlistData);

    if (lpFlistData->hwndFdir)
		ListBox_SetSel(lpFlistData->hwndFdir, FALSE, -1);

    for (lpFdirEntry=lpFlistData->fdir; lpFdirEntry; lpFdirEntry=lpFdirEntry->fdirNext)
		lpFdirEntry->fdirFlags&= ~FDIR_FLAG_SELECTED;

    fdirResetSelection(lpFlistData, bPrev);
}



static void fdirControls(HWND hwnd, LPFlistData lpFlistData, BOOL bUpdateControls)
{
    BOOL bOldSelected= *lpFlistData->lpFdirSelected>0;

    if (lpFlistData->hwndFdir && fdirGetSelection(lpFlistData, bUpdateControls)) 
	{
		BOOL bSelected;
		LPFdirEntry lpFdirEntry=lpFlistData->fdir;
		int nPtr=1, nIndex=0, nNextIndex= *lpFlistData->lpFdirSelected>0 ? lpFlistData->lpFdirSelected[1] : -1;

		while (lpFdirEntry) 
		{
			BOOL bVisible=fdirEntryVisible(lpFlistData, lpFdirEntry);

			if (!bVisible || nIndex!=nNextIndex)
			{
				lpFdirEntry->fdirFlags&= ~FDIR_FLAG_SELECTED;
			}
			else 
			{
				lpFdirEntry->fdirFlags|=FDIR_FLAG_SELECTED;
				nNextIndex=nPtr< *lpFlistData->lpFlistSelected ? lpFlistData->lpFlistSelected[++nPtr] : -1;
			}

			lpFdirEntry=lpFdirEntry->fdirNext;

			if (bVisible)
				nIndex++;

		}

		bSelected= *lpFlistData->lpFdirSelected>0;
		Button_Enable(GetDlgItem(hwnd, CID_DOWNLOAD_LISTS), !lpFlistData->bCreating);

		if (bUpdateControls || xor(bOldSelected, bSelected)) 
		{
			Button_Enable(GetDlgItem(hwnd, CID_INSERT), bSelected);
			Button_Enable(GetDlgItem(hwnd, CID_APPEND), bSelected);
			Button_Enable(GetDlgItem(hwnd, CID_UPDATE_FILE), bSelected);
			Button_Enable(GetDlgItem(hwnd, CID_ACTION), bSelected);
		}

		Button_Enable(GetDlgItem(hwnd, CID_SORT), fdirDisplayCount(lpFlistData)>1);
    }
}



static void updateLists(HWND hwnd, LPFlistData lpFlistData)
{
    FARPROC lpProc;

    lpProc=MakeProcInstance((FARPROC)updateListsProc, lpGlobals->hInst);
    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_UPDATE_LISTS", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
    FreeProcInstance(lpProc);
}



static void flistClear(HWND hwnd, LPFlistData lpFlistData)
{
    if (lpFlistData->hwndFlist) 
	{
		ListBox_ResetContent(lpFlistData->hwndFlist);
		SetDlgItemText(hwnd, CID_FLM_DATE, "");
		SetDlgItemText(hwnd, CID_FLD_DATE, "");
		SetDlgItemText(hwnd, CID_FLX_DATE, "");
    }

    if (lpFlistData->hwndFdir) 
	{
		ListBox_ResetContent(lpFlistData->hwndFdir);
    }

    freeFlist(&lpFlistData->flist);
    flistResetSelection(lpFlistData, TRUE);
    freeFdir(&lpFlistData->fdir);
    fdirResetSelection(lpFlistData, TRUE);
    freeFdir(&lpFlistData->filepooldir);
    freeFdir(&lpFlistData->maildir);
    freeFdir(&lpFlistData->localdir);
}



LPFlistEntry flistGetEntry(LPFlistData lpFlistData, int nIndex)
{
    LPFlistEntry lpFlistEntry=lpFlistData->flist;

    while (nIndex>0 && lpFlistEntry) 
	{
		lpFlistEntry=lpFlistEntry->flistNext;
		nIndex--;
    }

    return(lpFlistEntry);
}



int flistFindEntry(LPFlistData lpFlistData, LPFlistEntry lpFlistEntry)
{
    if (!lpFlistEntry)
	{
		return(-1);
	}
    else 
	{
		LPFlistEntry lpFlistSearch;
		int nIndex=0;

		for (lpFlistSearch=lpFlistData->flist;
			 lpFlistSearch && lpFlistSearch!=lpFlistEntry;
			 lpFlistSearch=lpFlistSearch->flistNext)
				nIndex++;

		return(lpFlistSearch ? nIndex : -1);
    }
}



static void deleteFdirFile(LPFlistData lpFlistData, LPFdirEntry lpFdirEntry)
{
    HSCRIPT hScript=initScript("Moderate", FALSE);

    addToScript(hScript, "put `join %s/%s`¬"
			 "if waitfor(`R:`, `M:`) == 0¬"
			     "put `era %s`¬"
			     "if waitfor(`(y/n)?`, `R:`) == 0¬"
			     "put `y`¬"
			     "waitfor `R:`¬"
			     "endif¬"
			     "put `quit`¬"
			     "waitfor `M:`¬"
			 "endif¬",
		lpFlistData->context.lpcConfName, lpFlistData->context.lpcTopicName,
		lpFdirEntry->fdirName);

    lpFlistData->outbasketActions[lpFlistData->actionCount++]=
	actionScript(hScript, OT_PREINCLUDE, "delete fdir file %s in %s/%s",
		     lpFdirEntry->fdirName, lpFlistData->context.lpcConfName,
		     lpFlistData->context.lpcTopicName);

    if (*lpFlistData->lpFdirSelected && fdirEntryVisible(lpFlistData, lpFdirEntry)) 
	{
		LPFdirEntry lpThisEntry=lpFlistData->fdir;
		int nIndex=0, nPtr=1, nNextIndex=lpFlistData->lpFdirSelected[1];

		while (lpThisEntry && lpThisEntry!=lpFdirEntry && nNextIndex>=0) 
		{
			if (fdirEntryVisible(lpFlistData, lpThisEntry)) 
			{
				if (nIndex==nNextIndex)
					nNextIndex=nPtr< *lpFlistData->lpFdirSelected ? lpFlistData->lpFdirSelected[++nPtr] : -1;
				nIndex++;
			}

			lpThisEntry=lpThisEntry->fdirNext;
		}

		if (nIndex==nNextIndex)
			removeSelection(lpFlistData->lpFdirSelected, nIndex);

    }

    lpFlistData->fdirFiles--;
    lpFlistData->fdirChanged=TRUE;
}



static void deleteFdirEntry(LPFlistData lpFlistData, LPFdirEntry lpFdirEntry)
{
    lpFdirEntry->fdirReferences=0;

    if (*lpFdirEntry->fdirHead==lpFlistData->fdir)
	{
		deleteFdirFile(lpFlistData, lpFdirEntry);
	}
    else if (*lpFdirEntry->fdirHead==lpFlistData->maildir)
	{
		lpFlistData->maildirExported--;
	}
    else if (*lpFdirEntry->fdirHead==lpFlistData->localdir)
	{
		lpFlistData->localdirUploaded--;
	}
    if (*lpFdirEntry->fdirHead!=lpFlistData->maildir) 
	{
		if (lpFdirEntry== *lpFdirEntry->fdirHead)
		{
			*lpFdirEntry->fdirHead=lpFdirEntry->fdirNext;
		}
		else 
		{
			LPFdirEntry searchEntry= *lpFdirEntry->fdirHead;

			while (searchEntry->fdirNext && searchEntry->fdirNext!=lpFdirEntry)
			{
				searchEntry=searchEntry->fdirNext;
			}

			if (searchEntry->fdirNext)
			{
				searchEntry->fdirNext=lpFdirEntry->fdirNext;
			}
		}

		if (lpFdirEntry->fdirLocalPath)
		{
			gfree(lpFdirEntry->fdirLocalPath);
			lpFdirEntry->fdirLocalPath = NULL;
		}

		gfree(lpFdirEntry);
		lpFdirEntry = NULL;
    }
}



static BOOL unlinkFdirEntry(LPFlistData lpFlistData, LPFlistEntry lpFlistEntry, BOOL bDelete)
{
	LPFdirEntry lpFdirEntry=lpFlistEntry->flistSource;
	BOOL entryDeleted=FALSE;
	
	if (lpFdirEntry)
	{
		if (lpFdirEntry->fdirReferences>1) 
		{
			int thisReference=0;
			
			while (thisReference<lpFdirEntry->fdirReferences &&
				lpFdirEntry->fdirFlistEntry[thisReference]!=lpFlistEntry)
				thisReference++;
			
			if (thisReference<lpFdirEntry->fdirReferences) 
			{
				lpFdirEntry->fdirReferences--;
				
				while (thisReference<lpFdirEntry->fdirReferences) 
				{
					lpFdirEntry->fdirFlistEntry[thisReference]=lpFdirEntry->fdirFlistEntry[thisReference+1];
					thisReference++;
				}
			}
		} 
		else if (bDelete) 
		{
			deleteFdirEntry(lpFlistData, lpFdirEntry);
			entryDeleted=TRUE;
		} 
		else 
		{
			lpFdirEntry->fdirReferences=0;
			
			if (*lpFdirEntry->fdirHead==lpFlistData->fdir)
				lpFlistData->fdirOrphans++;
		}
		return(entryDeleted);
	}
	return FALSE;
}



static void drawFlistEntry(const DRAWITEMSTRUCT FAR * lpDis, BOOL bStatsDisplay)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rcFlag, rcName, rcSize, rcDescription, rcComment;
    LPFlistEntry thisEntry=(LPFlistEntry)lpDis->itemData;
    int width=lpDis->rcItem.right-lpDis->rcItem.left+1;

    CopyRect(&rcFlag, &lpDis->rcItem);
    rcFlag.left+=2;
    rcFlag.right=rcFlag.left+width/30;

    CopyRect(&rcName, &lpDis->rcItem);
    rcName.left=rcFlag.right+4;
    rcName.right=rcName.left+width/6;

    CopyRect(&rcSize, &lpDis->rcItem);
    rcSize.left=rcName.right+1;
    rcSize.right=rcSize.left+width/9;

    CopyRect(&rcDescription, &lpDis->rcItem);
    rcDescription.left=rcSize.right+8;
    rcDescription.right-=2;

    CopyRect(&rcComment, &lpDis->rcItem);
    rcComment.left=rcName.left;
    rcComment.right-=2;

    if (lpDis->itemState&ODS_SELECTED)
		newBg=GetSysColor(COLOR_HIGHLIGHT);
    else
		newBg=GetSysColor(COLOR_WINDOW);

    if (thisEntry && (thisEntry->flistSelect==FLIST_MEMO || thisEntry->flistFlags&FLIST_FLAG_HOLD))
		newFg=GetSysColor(COLOR_GRAYTEXT);
    else if (lpDis->itemState&ODS_SELECTED)
		newFg=GetSysColor(COLOR_HIGHLIGHTTEXT);
    else
		newFg=GetSysColor(COLOR_WINDOWTEXT);

    oldBg=(COLORREF)SetBkColor(lpDis->hDC, newBg);
    oldFg=(COLORREF)SetTextColor(lpDis->hDC, newFg);
    hBrush=CreateSolidBrush(newBg);
    FillRect(lpDis->hDC, &lpDis->rcItem, hBrush);
    DeleteObject(hBrush);

    if (thisEntry)
	{
		switch (thisEntry->flistSelect) 
		{

		case FLIST_COMMENT:
			SelectObject(lpDis->hDC, lpGlobals->hfBold);

			if (thisEntry->flistFlags&FLIST_FLAG_HOLD)
				DrawText(lpDis->hDC, "h", -1, &rcFlag, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);

			DrawText(lpDis->hDC, thisEntry->flistDescription, -1, &rcComment, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			break;

		case FLIST_MEMO:
			SelectObject(lpDis->hDC, lpGlobals->hfBold);
			DrawText(lpDis->hDC, "#", -1, &rcFlag, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			DrawText(lpDis->hDC, thisEntry->flistDescription, -1, &rcComment, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			break;

		case FLIST_FROM_FDIR:
		case FLIST_FROM_MAIL_DIR:
		case FLIST_FROM_FILEPOOL:
		case FLIST_FROM_UPLOAD:
		case FLIST_FROM_NOTIFICATION: 
			{
			char flag[2];

			SelectObject(lpDis->hDC, lpGlobals->hfNormal);

			if (!(thisEntry->flistFlags&(FLIST_FLAG_EXPORT|FLIST_FLAG_FILEPOOL)) &&
				thisEntry->flistSource && thisEntry->flistSource->fdirSize>=0) 
			{
				char s[16];

				wsprintf(s, "%ld", thisEntry->flistSource->fdirSize);
				DrawText(lpDis->hDC, s, -1, &rcSize, DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
			} 
			else
			{
				DrawText(lpDis->hDC, "?", -1, &rcSize, DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
			}
			SelectObject(lpDis->hDC, lpGlobals->hfBold);

			if (thisEntry->flistFlags&(FLIST_FLAG_EXPORT|FLIST_FLAG_FILEPOOL))
				lstrcpy(flag, "f");
			else
			{
				switch (thisEntry->flistSelect) 
				{

					case FLIST_FROM_FDIR:           lstrcpy(flag, "c");     break;
					case FLIST_FROM_MAIL_DIR:       lstrcpy(flag, "m");     break;
					case FLIST_FROM_FILEPOOL:       lstrcpy(flag, "f");     break;
					case FLIST_FROM_UPLOAD:         lstrcpy(flag, "u");     break;
					case FLIST_FROM_NOTIFICATION:   lstrcpy(flag, "n");     break;
				}
			}

			DrawText(lpDis->hDC, flag, -1, &rcFlag, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			DrawText(lpDis->hDC, thisEntry->flistName, -1, &rcName, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			SelectObject(lpDis->hDC, lpGlobals->hfNormal);
			DrawText(lpDis->hDC, thisEntry->flistDescription, -1, &rcDescription, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			break;
		}

		case FLIST_TABBED:
			SelectObject(lpDis->hDC, lpGlobals->hfNormal);

			if (thisEntry->flistFlags&FLIST_FLAG_HOLD)
				DrawText(lpDis->hDC, "h", -1, &rcFlag, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);

			DrawText(lpDis->hDC, thisEntry->flistDescription, -1, &rcDescription, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
			break;

		}
	}

    if (lpDis->itemState&ODS_FOCUS)
		DrawFocusRect(lpDis->hDC, &lpDis->rcItem);

    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



static void redrawFlistEntry(HWND hwnd, LPFlistData lpFlistData, LPFlistEntry lpFlistEntry)
{
    if (lpFlistData->hwndFlist) 
	{
		int flistIndex=ListBox_FindItemData(lpFlistData->hwndFlist, -1, lpFlistEntry);
		RECT rect;

		if ((long)flistIndex==LB_ERR)
			alert(hwnd, "Internal error (redrawFlistEntry): file %s not found in file list - please report to CIX Support (support@cix.uk)",
				lpFlistEntry->flistName);

		else if (ListBox_GetItemRect(lpFlistData->hwndFlist, flistIndex, &rect)!=LB_ERR)
			InvalidateRect(lpFlistData->hwndFlist, &rect, FALSE);

    }
}



CheckReplace checkReplace(HWND hwnd, LPFlistData lpFlistData, FlistSelect flistSelect, LPFdirEntry newEntry, LPPFdirEntry oldEntry)

//   Check if a name is already in the flist. If it is, we have two basic situations:
//
//   1. The new entry and the existing entry refer to the same file (whether it be already
//      in the fdir, an upload or an export from the maildir). In this case, the options
//      are to abandon the insertion or to create a duplicate entry.
//
//   2. The entries refer to different files. In this case, the options are to abandon the
//      insertion or to replace the existing entry/entries with the new file. In the latter
//      case, we can either just alter the existing entries or add a new duplicate entry as
//      well. Note that whilst it is legal to have two entries with the same name, one
//      referring to the fdir and one to the filepool, we disallow this to save confusion.

{
    LPFlistEntry searchEntry=lpFlistData->flist;
    CheckReplace status=CR_OK;

    *oldEntry=NULL;

    while (searchEntry &&
	   !((searchEntry->flistSelect==FLIST_FROM_FDIR ||
	      searchEntry->flistSelect==FLIST_FROM_FILEPOOL ||
	      searchEntry->flistSelect==FLIST_FROM_MAIL_DIR ||
	      searchEntry->flistSelect==FLIST_FROM_UPLOAD ||
	      searchEntry->flistSelect==FLIST_FROM_NOTIFICATION) &&
	     lstrcmp(searchEntry->flistName, newEntry->fdirName)==0))
	searchEntry=searchEntry->flistNext;

    if (searchEntry) {
	//  We have a duplication...

	if ((searchEntry->flistSelect==FLIST_FROM_FDIR ||
	     searchEntry->flistSelect==FLIST_FROM_NOTIFICATION) &&
	    (flistSelect==FLIST_FROM_FDIR || flistSelect==FLIST_FROM_NOTIFICATION) ||
	    searchEntry->flistSelect==FLIST_FROM_FILEPOOL && flistSelect==FLIST_FROM_FILEPOOL ||
	    searchEntry->flistSelect==FLIST_FROM_MAIL_DIR && flistSelect==FLIST_FROM_MAIL_DIR ||
	    searchEntry->flistSelect==FLIST_FROM_UPLOAD && flistSelect==FLIST_FROM_UPLOAD &&
	    searchEntry->flistSource &&
	    (!searchEntry->flistSource->fdirLocalPath && !newEntry->fdirLocalPath ||
	     lstrcmp(searchEntry->flistSource->fdirLocalPath, newEntry->fdirLocalPath)==0)) {

	    if (query(hwnd, MB_YESNO, "File %s is already in the file list, create duplicate entry?",
		      newEntry->fdirName)!=IDYES)
		status=CR_CANCEL;
	    else if (searchEntry->flistSource) {
		status=CR_DUPLICATE;
		*oldEntry=searchEntry->flistSource;
	    } else {
		status=CR_OK;
		newEntry->fdirReferences=1;
		newEntry->fdirFlistEntry[0]=searchEntry;
		searchEntry->flistSource=newEntry;
		searchEntry->flistSelect=flistSelect;
	    }
	} else if (searchEntry->flistSelect==FLIST_FROM_FILEPOOL) {
	    inform(hwnd, "File %s is already in the file list imported from the filepool; "
			 "please remove or transform this entry before attempting to replace it",
		   newEntry->fdirName);

	    status=CR_CANCEL;
	} else if (flistSelect==FLIST_FROM_FILEPOOL) {
	    inform(hwnd, "File %s is already in the file list; use transform to convert this "
			 "entry to an import from the filepool",
		   newEntry->fdirName);

	    status=CR_CANCEL;
	} else {
	    FARPROC lpProc=MakeProcInstance(duplicateProc, lpGlobals->hInst);

	    status=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_DUPLICATE", 
								 (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPSTR)newEntry->fdirName);
	    FreeProcInstance(lpProc);

	    switch (status) {

	    case CR_REPLACE: {
		LPFdirEntry replaceEntry=searchEntry->flistSource;

		if (replaceEntry) {
		    int dup;

		    for (dup=0; dup<replaceEntry->fdirReferences && dup<MAX_REFERENCES; dup++) {
			LPFlistEntry lpFlistEntry=replaceEntry->fdirFlistEntry[dup];
			int nFlistIndex=flistFindEntry(lpFlistData, lpFlistEntry);

			newEntry->fdirFlistEntry[dup]=lpFlistEntry;
			lpFlistEntry->flistSource=newEntry;
			lpFlistEntry->flistSelect=flistSelect;
			lpFlistEntry->flistFlags|=FLIST_FLAG_SELECTED;

			if (nFlistIndex>=0)
			    insertSelection(lpFlistData->lpFlistSelected, nFlistIndex);

			if (lpFlistData->hwndFlist)
			    ListBox_SetSel(lpFlistData->hwndFlist, TRUE, nFlistIndex);

		    }

		    newEntry->fdirReferences=replaceEntry->fdirReferences;
		    replaceEntry->fdirReferences=0;

		    if (searchEntry->flistSelect==FLIST_FROM_FDIR ||
			searchEntry->flistSelect==FLIST_FROM_NOTIFICATION)
			inform(hwnd, "Old copy of %s marked for deletion from file directory",
			       searchEntry->flistName);

		    deleteFdirEntry(lpFlistData, replaceEntry);
		} else {
		    newEntry->fdirReferences=1;
		    newEntry->fdirFlistEntry[0]=searchEntry;
		    searchEntry->flistSource=newEntry;
		    searchEntry->flistSelect=flistSelect;
		}

		break;
	    }

	    case CID_DUP_DUPLICATE:
		status=CR_DUPLICATE;
		*oldEntry=searchEntry->flistSource;
		break;

	    }
	}
    } else if (!checkName(newEntry->fdirName, CHECK_CIX)) {
	alert(hwnd, "%s is not a valid CIX filename, entry not added", newEntry->fdirName);
	status=CR_CANCEL;
    }

    return(status);
}



static BOOL flistPostlude(HWND hwnd, LPFlistData lpFlistData, LPContext lpContext, FlistExit flistExit)
{
    LPFlistEntry lpFlistEntry;
    LPFdirEntry lpFdirEntry;

    if (flistExit!=EXIT_OK) {
	BOOL cancelRequest=flistExit==EXIT_CANCEL;
	BOOL abortExit=FALSE;

	if (lpFlistData->flistChanged || lpFlistData->fdirChanged)
		switch (query(hwnd, (WORD)(IsAmeolQuitting()?MB_YESNO:MB_YESNOCANCEL),
			  "File list has been changed: "
			  "do you want to keep the changes you have made?")) {

	    case IDYES:
		cancelRequest=FALSE;
		break;

	    case IDCANCEL:
		cancelRequest=FALSE;
		abortExit=TRUE;
		break;

	    }

	if (cancelRequest) {
	    int ptr;

	    for (ptr=0; ptr<lpFlistData->actionCount; ptr++)
		RemoveObject(lpFlistData->outbasketActions[ptr]);

	    lpFlistData->actionCount=0;
	    flistClear(hwnd, lpFlistData);
	    return(FALSE);
	} else if (abortExit)
	    return(TRUE);

    } else if (lpFlistData->flistChanged && !lpFlistData->flistUploaded) {
	int nSelect=lpFlistData->flistMustUpload ?
		    query(hwnd, MB_OKCANCEL|MB_ICONINFORMATION,
			  "File list marked for upload to reflect changes made to file directory") :
		    query(hwnd, MB_YESNOCANCEL,
			  "The offline file list has been changed: "
			  "do you want to update the file list on CIX?");

	switch (nSelect) {

	case IDYES:
	case IDOK:
	    lpFlistData->flistUploaded=TRUE;
	    break;

	case IDCANCEL:
	    return(TRUE);

	}
    }

    if (lpFlistData->maildirExported>0) {
	LPFdirEntry maildirEntry=lpFlistData->maildir;

	while (maildirEntry) {
	    if (maildirEntry->fdirReferences) {
		HSCRIPT hScript=initScript("Moderate", FALSE);

		addToScript(hScript, "put `join %s/%s`¬"
				     "if waitfor(`R:`, `M:`) == 0¬"
					 "put `mail`¬"
					 "waitfor `Ml:`¬"
					 "put `export %s`¬"
					 "waitfor `Ml:`¬"
					 "put `quit`¬"
					 "waitfor `R:`¬"
					 "put `quit`¬"
					 "waitfor `M:`¬"
				     "endif¬",
			    lpContext->lpcConfName, lpContext->lpcTopicName, maildirEntry->fdirName);

		actionScript(hScript, OT_PREINCLUDE, "export %s to %s/%s",
			     maildirEntry->fdirName, lpContext->lpcConfName, lpContext->lpcTopicName);

		lpFlistData->fdirUpdate=TRUE;
	    }

	    maildirEntry=maildirEntry->fdirNext;
	}
    }

    if (lpFlistData->localdirUploaded>0) {
	LPFdirEntry localdirEntry=lpFlistData->localdir;

	while (localdirEntry) {
	    if (localdirEntry->fdirReferences) {
		HSCRIPT hScript=initScript("Moderate", FALSE);

		if (localdirEntry->fdirFlags&FDIR_FLAG_UPLOAD_RENAME) {
		    LPSTR lpLocalName=_fstrrchr(localdirEntry->fdirLocalPath, '\\')+1;

		    addToScript(hScript, "put `join %s/%s`¬"
					 "if waitfor(`R:`, `M:`) == 0¬"
					     "put `ful`¬"
					     "upload `%s`¬"
					     "put `ren %s %s`¬"
					     "waitfor `R:`¬"
					     "put `quit`¬"
					     "waitfor `M:`¬"
					 "endif¬",
				lpContext->lpcConfName, lpContext->lpcTopicName,
				localdirEntry->fdirLocalPath,
				lpLocalName, localdirEntry->fdirName);

		    actionScript(hScript, OT_PREINCLUDE, "upload %s to %s/%s as %s",
				 lpLocalName,
				 lpContext->lpcConfName, lpContext->lpcTopicName,
				 localdirEntry->fdirName);

		} else {
		    LPSTR lpUploadDir=localdirEntry->fdirLocalPath ? localdirEntry->fdirLocalPath : (LPSTR)setup.szUploadDir;

		    addToScript(hScript, "put `join %s/%s`¬"
					 "if waitfor(`R:`, `M:`) == 0¬"
					     "put `ful`¬"
					     "upload `%s\\%s`¬"
					     "put `quit`¬"
					     "waitfor `M:`¬"
					 "endif¬",
				lpContext->lpcConfName, lpContext->lpcTopicName,
				lpUploadDir, localdirEntry->fdirName);

		    actionScript(hScript, OT_PREINCLUDE, "upload %s to %s/%s",
				 localdirEntry->fdirName,
				 lpContext->lpcConfName, lpContext->lpcTopicName);

		}

		lpFlistData->fdirUpdate=TRUE;
	    }

	    localdirEntry=localdirEntry->fdirNext;
	}
    }

    for (lpFlistEntry=lpFlistData->flist; lpFlistEntry; lpFlistEntry=lpFlistEntry->flistNext)
	if (isFile(lpFlistEntry) && !lpFlistEntry->flistSource && lpFlistEntry->flistFlags&FLIST_FLAG_EXPORT)
	    exportFlistEntry(lpContext, lpFlistEntry);

    for (lpFdirEntry=lpFlistData->fdir; lpFdirEntry; lpFdirEntry=lpFdirEntry->fdirNext)
	if (lpFdirEntry->fdirFlags&FDIR_FLAG_EXPORT)
	    exportFdirEntry(lpContext, lpFdirEntry);

    if (lpFlistData->flistChanged) {
	int status=writeFlist(lpContext->hTopic, lpFlistData->flist);

	if (status)
	    alert(hwnd, "Error writing Moderator's file list - status %d", status);

    }

    if (lpFlistData->flistUploaded) {
	uploadFlist(lpContext->hTopic);
	lpFlistData->fuserUpdate=TRUE;
    }

    if (lpFlistData->flistUpdate)
	downloadFlist(lpContext->hTopic, TRUE);

    if (lpFlistData->fdirUpdate || lpFlistData->fdirChanged)
	downloadFdir(lpContext->hTopic, TRUE);

    if (lpFlistData->maildirUpdate)
	downloadMaildir(TRUE);

    if (lpFlistData->fuserUpdate)
	downloadFuser(lpContext->hTopic, TRUE);

    lpFlistData->actionCount=0;
    flistClear(hwnd, lpFlistData);
    return(FALSE);
}



static void parseError(HWND hwnd, LPCSTR fileDescription, LPCSTR fileName, LPCSTR fileExtension)
{
    alert(hwnd, "The moderator addon was unable to parse the %s file - please check the file "
		"%s.%s in an editor. If it appears to be correct, please binmail it to CIX Support (support@cix.uk) "
		"with a cover note describing the problem.",
	  fileDescription, fileName, fileExtension);

}



static BOOL readFlistFiles(HWND hwnd, LPFlistData lpFlistData, LPContext lpContext)
{
    BOOL bCancel=FALSE;
    BOOL bAgain;
    int nStatus;

    flistResetSelection(lpFlistData, TRUE);
    fdirResetSelection(lpFlistData, TRUE);
    lpFlistData->bCreating=FALSE;

    do {
	char szTitle[64];

	bAgain=FALSE;
	wsprintf(szTitle, "File List Maintenance - %s/%s", lpContext->lpcConfName, lpContext->lpcTopicName);
	SetWindowText(hwnd, szTitle);

	nStatus=readFlist(hwnd, lpContext->hTopic,
				&lpFlistData->flist, &lpFlistData->flistTime, &lpFlistData->flistEntries, &lpFlistData->flistFiles,
				&lpFlistData->fdir, &lpFlistData->fdirTime, &lpFlistData->fdirFiles, &lpFlistData->fdirOrphans,
				&lpFlistData->fuserTime, &lpFlistData->filepooldir);

	if (nStatus==ERR_FLIST_CHANGED)
	    lpFlistData->flistChanged=TRUE;
	else if (nStatus!=OK) {
	    TOPICINFO topicInfo;

	    GetTopicInfo(lpContext->hTopic, &topicInfo);

	    if (nStatus==ERR_FILE_MISSING) {
		if (topicInfo.wFlags&TF_HASFILES) {
		    updateLists(hwnd, lpFlistData);
		    flistPostlude(hwnd, lpFlistData, lpContext, EXIT_OK);
		    bCancel=TRUE;
		} else {
		    FARPROC lpProc=MakeProcInstance((FARPROC)queryListsProc, lpGlobals->hInst);

		    switch (StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FLIST_MISSING", 
									  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpContext)) {

		    case CID_CHANGE_TOPIC: {
			FARPROC lpProc=MakeProcInstance((FARPROC)topicProc, lpGlobals->hInst);

			bAgain=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_SELECT_TOPIC", 
									 (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpContext);
			FreeProcInstance(lpProc);
			bCancel= !bAgain;
			break;
		    }

		    case CID_CREATE_FLIST:
			lpFlistData->flistTime=lpFlistData->fdirTime=time(NULL);
			lpFlistData->bCreating=TRUE;
			lpFlistData->flistUploaded=TRUE;
			break;

		    case CID_ASSUME:
			lpFlistData->flistUpdate=TRUE;
			lpFlistData->fdirUpdate=TRUE;
			inform(hwnd, "File lists marked for download");
			flistPostlude(hwnd, lpFlistData, lpContext, EXIT_OK);
			bCancel=TRUE;
			break;

		    case CID_CANCEL:
			bCancel=TRUE;
			break;

		    }

		    FreeProcInstance(lpProc);
		}
	    } else {
		parseError(hwnd, nStatus==ERR_BAD_FLM ? "moderator's file list" : "file directory",
			   topicInfo.szFileName, nStatus==ERR_BAD_FLM ? "flm" : "fld");

		bCancel=TRUE;
	    }
	}
    } while (bAgain && !bCancel);

    return(bCancel);
}



static BOOL flistPrelude(HWND hwnd, LPFlistData lpFlistData, LPContext lpContext, LPContext lpOldContext)
{
    BOOL bCancel=FALSE;

    if (lpOldContext && flistPostlude(hwnd, lpFlistData, lpOldContext, EXIT_SWITCHTOPIC))
	return(TRUE);

    lpFlistData->flistChanged=FALSE;
    lpFlistData->flistUploaded=FALSE;
    lpFlistData->flistMustUpload=FALSE;
    lpFlistData->flistUpdate=FALSE;
    lpFlistData->fdirChanged=FALSE;
    lpFlistData->fdirUpdate=FALSE;
    lpFlistData->fuserUpdate=FALSE;
    lpFlistData->maildirUpdate=FALSE;
    lpFlistData->maildirExported=0;
    lpFlistData->localdirUploaded=0;
    lpFlistData->actionCount=0;

    bCancel=readFlistFiles(hwnd, lpFlistData, lpContext);

    if (!bCancel) {
	int nStatus=readMaildir(&lpFlistData->maildir, &lpFlistData->maildirTime, &lpFlistData->maildirFiles);

	if (nStatus!=OK && nStatus!=ERR_FILE_MISSING)
	    parseError(hwnd, "mail directory", "mail", "lst");

    }

    return(bCancel);
}



static void footerDisplay(HWND hwnd, LPFlistData lpFlistData)
{
    SetWindowFont(GetDlgItem(hwnd, CID_FLM_DATE), lpGlobals->hfNormal, FALSE);

    if (lpFlistData->flistTime) {
	char s[64];

	strftime(s, 63, "%I:%M %p on %d%b%Y", localtime(&lpFlistData->flistTime));
	SetDlgItemText(hwnd, CID_FLM_DATE, s);
    } else
	SetDlgItemText(hwnd, CID_FLM_DATE, "not yet downloaded");

    SetWindowFont(GetDlgItem(hwnd, CID_FLD_DATE), lpGlobals->hfNormal, FALSE);

    if (lpFlistData->fdirTime) {
	char s[64];

	strftime(s, 63, "%I:%M %p on %d%b%Y", localtime(&lpFlistData->fdirTime));
	SetDlgItemText(hwnd, CID_FLD_DATE, s);
    } else
	SetDlgItemText(hwnd, CID_FLD_DATE, "not yet downloaded");

    SetWindowFont(GetDlgItem(hwnd, CID_FLX_DATE), lpGlobals->hfNormal, FALSE);

    if (lpFlistData->fstatTime) {
	char s[64];

	strftime(s, 63, "%I:%M %p on %d%b%Y", localtime(&lpFlistData->fstatTime));
	SetDlgItemText(hwnd, CID_FLX_DATE, s);
    } else
	SetDlgItemText(hwnd, CID_FLX_DATE, "not yet downloaded");

}



static void flistDisplay(HWND hwnd, LPFlistData lpFlistData)
{
    LPFlistEntry lpFlistEntry=lpFlistData->flist;

    flistSuspendControlUpdates(lpFlistData);
	SendMessage(lpFlistData->hwndFlist, WM_SETREDRAW, 0, 0);
    while (lpFlistEntry) 
	{
		int nIndex=ListBox_AddItemData(lpFlistData->hwndFlist, lpFlistEntry);

		if (lpFlistEntry->flistFlags&FLIST_FLAG_SELECTED)
			ListBox_SetSel(lpFlistData->hwndFlist, TRUE, nIndex);

		lpFlistEntry=lpFlistEntry->flistNext;
    }

    if (lpFlistData->flistEntries>0)
		NextDlgCtl(hwnd, CID_FILE_LIST);
	
	SendMessage(lpFlistData->hwndFlist, WM_SETREDRAW, 1, 0);
	InvalidateRect(lpFlistData->hwndFlist, 0, 0);
    
	flistControls(hwnd, lpFlistData, TRUE);
    footerDisplay(hwnd, lpFlistData);
}



static LPFlistEntry updateFlistFile(LPFlistData lpFlistData, BOOL bControlSelected)
{
    static int flistIndex=0;

    if (bControlSelected || !lpFlistData->lpFlistSelected || ++flistIndex> *lpFlistData->lpFlistSelected) {
	flistIndex=0;
	return(NULL);
    } else
	return((LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, lpFlistData->lpFlistSelected[flistIndex]));

}



static LPFdirEntry updateFdirFile(LPFlistData lpFlistData, BOOL bControlSelected)
{
    static int fdirIndex=0;

    if (bControlSelected || !lpFlistData->lpFdirSelected || ++fdirIndex> *lpFlistData->lpFdirSelected) {
	fdirIndex=0;
	return(NULL);
    } else
	return((LPFdirEntry)ListBox_GetItemData(lpFlistData->hwndFdir, lpFlistData->lpFdirSelected[fdirIndex]));

}



static int updateFromMailDir(HWND hwnd, LPFlistData lpFlistData, LPFdirEntry lpFdirEntry, BOOL bEdit)
{
    int nInitialCount=lpFlistData->maildirExported;

    if (lpFdirEntry) {
	LPFdirEntry lpSearchEntry=lpFlistData->maildir;

	while (lpSearchEntry && lstrcmp(lpSearchEntry->fdirName, lpFdirEntry->fdirName))
	    lpSearchEntry=lpSearchEntry->fdirNext;

	if (lpSearchEntry) {
	    int dup;

	    for (dup=0; dup<lpFdirEntry->fdirReferences && dup<MAX_REFERENCES; dup++) {
		 lpSearchEntry->fdirFlistEntry[dup]=lpFdirEntry->fdirFlistEntry[dup];
		 lpSearchEntry->fdirFlistEntry[dup]->flistSource=lpSearchEntry;
		 lpSearchEntry->fdirFlistEntry[dup]->flistSelect=FLIST_FROM_MAIL_DIR;
		 redrawFlistEntry(hwnd, lpFlistData, lpSearchEntry->fdirFlistEntry[dup]);
	    }

	    lpSearchEntry->fdirReferences=lpFdirEntry->fdirReferences;

	    if (lpFlistData->hwndFdir) {
		int itemIndex=ListBox_FindItemData(lpFlistData->hwndFdir, -1, lpFdirEntry);

		if (itemIndex!=LB_ERR)
		    ListBox_DeleteString(lpFlistData->hwndFdir, itemIndex);

	    }

	    deleteFdirEntry(lpFlistData, lpFdirEntry);
	    lpFlistData->maildirExported++;
	} else
	    inform(hwnd, "File %s not found in mail directory - no update performed on this file",
		   lpFdirEntry->fdirName);

    }

    return(lpFlistData->maildirExported-nInitialCount);
}



static int updateFromUpload(HWND hwnd, LPFlistData lpFlistData, LPFdirEntry lpFdirEntry, BOOL bEdit)
{
    int nInitialCount=lpFlistData->localdirUploaded;

    if (lpFdirEntry) {
	LPOPENFILENAME lpOfn=(LPOPENFILENAME)gmallocz(sizeof(OPENFILENAME));
	char szTitle[64];
	char szFilename[LEN_PATHNAME];
	LPSTR lpFilename;
	struct stat statBuff;
	BOOL bOK, bRename;
	char szFilter[32];
	LPCSTR lpcExt;

	lpcExt=lpFdirEntry->fdirName;
	lstrcpy(szFilename, lpFdirEntry->fdirName);

	lpOfn->lStructSize=sizeof(OPENFILENAME);
	lpOfn->hwndOwner=hwnd;
	lpOfn->lpstrFile=szFilename;
	lpOfn->nMaxFile=LEN_PATHNAME-1;
	lpOfn->lpstrInitialDir=setup.szUploadDir;
	while (*lpcExt && *lpcExt++ !='.');
	memset((char *)&szFilter, 0, 31);

	wsprintf(szFilter, "*.%s%c*.%s%cAll files (*.*)%c*.*%c", lpcExt, '\0', lpcExt, '\0', '\0', '\0');
	lpOfn->lpstrFilter=szFilter;
	lpOfn->nFilterIndex=1;
	wsprintf(szTitle, "Select file to use to update %s", (LPSTR)szFilename);
	lpOfn->lpstrTitle=szTitle;
	lpOfn->Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_SHOWHELP;
	lpOfn->hInstance=lpGlobals->hInst;
	lpFlistData->wHelpContext=CID_UPDATE_FILE;

	if (bOK=GetOpenFileName(lpOfn))
	    if (stat(szFilename, &statBuff)) {
		inform(hwnd, "Can't find file %s", (LPSTR)szFilename);
		bOK=FALSE;
	    }

	if (bOK) {

		lpFilename=szFilename+lpOfn->nFileOffset;

		if (bRename=lstrcmpi(lpFilename, lpFdirEntry->fdirName)!=0)
		{
			if (!checkName(lpFilename, CHECK_CIX)) {
				alert(hwnd, "%s is not a valid CIX filename so it cannot be uploaded and renamed to %s",
				  lpFilename, lpFdirEntry->fdirName);
				  
				bOK=FALSE;
			} else {
				LPFdirEntry lpSearchEntry=lpFlistData->fdir;
					while (lpSearchEntry && lstrcmp(lpSearchEntry->fdirName, lpFilename))
						lpSearchEntry=lpSearchEntry->fdirNext;

				if (lpSearchEntry) {
					inform(hwnd, "The file you have chosen to upload (%s) has the same name as another "
							 "file in the fdir so it cannot be uploaded and renamed to %s",
						   (LPSTR)szFilename, lpFdirEntry->fdirName);

					bOK=FALSE;
				}
			}
		}

		if (bOK) {
			LPFdirEntry lpNewEntry=(LPFdirEntry)gmalloc(sizeof(FdirEntry));
			int nDup;

			lpNewEntry->fdirHead= &lpFlistData->localdir;
			lpNewEntry->fdirNext=NULL;
			lstrcpy(lpNewEntry->fdirName, lpFdirEntry->fdirName);
			lpNewEntry->fdirFlags=FDIR_DEFAULT_FLAGS;

			if (bRename) 
			{
				lpNewEntry->fdirFlags|=FDIR_FLAG_UPLOAD_RENAME;
				lpNewEntry->fdirLocalPath=gmalloc(lstrlen(szFilename)+1);
				lstrcpy(lpNewEntry->fdirLocalPath, szFilename);
			} 
			else 
			{
				szFilename[lpOfn->nFileOffset-1]='\0';

				if (lstrcmpi(szFilename, setup.szUploadDir)==0)
					lpNewEntry->fdirLocalPath=NULL;
				else 
				{
					lpNewEntry->fdirLocalPath=gmalloc(lstrlen(szFilename)+1);
					lstrcpy(lpNewEntry->fdirLocalPath, szFilename);
				}
			}

			lpNewEntry->fdirSize=statBuff.st_size;
			lpNewEntry->fdirTimestamp=(time_t)statBuff.st_mtime;
			lpNewEntry->fdirReferences=0;

			for (nDup=0; nDup<lpFdirEntry->fdirReferences && nDup<MAX_REFERENCES; nDup++) 
			{
				lpNewEntry->fdirFlistEntry[nDup]=lpFdirEntry->fdirFlistEntry[nDup];
				lpNewEntry->fdirFlistEntry[nDup]->flistSource=lpNewEntry;
				lpNewEntry->fdirFlistEntry[nDup]->flistSelect=FLIST_FROM_UPLOAD;
				redrawFlistEntry(hwnd, lpFlistData, lpNewEntry->fdirFlistEntry[nDup]);
			}

			lpNewEntry->fdirReferences=lpFdirEntry->fdirReferences;
			lpNewEntry->fdirNext=lpFlistData->localdir;
			lpFlistData->localdir=lpNewEntry;

			if (lpFlistData->hwndFdir) {
			int itemIndex=ListBox_FindItemData(lpFlistData->hwndFdir, -1, lpFdirEntry);

			if (itemIndex!=LB_ERR)
				ListBox_DeleteString(lpFlistData->hwndFdir, itemIndex);

			}

			deleteFdirEntry(lpFlistData, lpFdirEntry);
			lpFlistData->localdirUploaded++;
		}
	}
	gfree(lpOfn);
	lpOfn = NULL;
    }

    return(lpFlistData->localdirUploaded-nInitialCount);
}



void addFlistItem(HWND hwnd, LPFlistData lpFlistData, LPFdirEntry lpFdirEntry)
{
    LPFlistEntry lpFlistEntry=(LPFlistEntry)gmalloc(sizeof(FlistEntry));
    int nInsertPoint=0;

    lpFlistEntry->flistSelect=lpFlistData->insertControl.source;

    if (!lpFdirEntry)
	lpFlistEntry->flistSource=NULL;
    else {
	if (lpFdirEntry->fdirReferences<MAX_REFERENCES)
	    lpFdirEntry->fdirFlistEntry[lpFdirEntry->fdirReferences]=lpFlistEntry;

	lpFdirEntry->fdirReferences++;
	lstrcpy(lpFlistEntry->flistName, lpFdirEntry->fdirName);
	lpFlistEntry->flistSource=lpFdirEntry;
    }

    lpFlistEntry->flistFlags=FLIST_DEFAULT_FLAGS|FLIST_FLAG_SELECTED;
    *lpFlistEntry->flistDescription='\0';

    if (lpFlistData->insertControl.position==CID_APPEND)
	nInsertPoint=lpFlistData->flistEntries;
    else
	nInsertPoint=lpFlistData->flistInsertPoint++;

    if (nInsertPoint>lpFlistData->flistEntries) {
	alert(hwnd, "Internal error in addFlistItem(): insertPoint=%d, flistEntries=%d - please report to CIX Support (support@cix.uk)",
	      nInsertPoint, lpFlistData->flistEntries);

	nInsertPoint=lpFlistData->flistEntries;
    }

    if (nInsertPoint==0) {
	lpFlistEntry->flistPrevious=NULL;
	lpFlistEntry->flistNext=lpFlistData->flist;

	if (lpFlistData->flist)
	    lpFlistData->flist->flistPrevious=lpFlistEntry;

	lpFlistData->flist=lpFlistEntry;
    } else {
	LPFlistEntry lpInsertEntry=flistGetEntry(lpFlistData, nInsertPoint-1);

	lpFlistEntry->flistPrevious=lpInsertEntry;
	lpFlistEntry->flistNext=lpInsertEntry->flistNext;

	if (lpInsertEntry->flistNext)
	    lpInsertEntry->flistNext->flistPrevious=lpFlistEntry;

	lpInsertEntry->flistNext=lpFlistEntry;
    }

    lpFlistData->flistEntries++;
    lpFlistData->flistChanged=TRUE;

    if (lpFlistData->hwndFlist) {
	if (nInsertPoint==lpFlistData->flistEntries-1)
	    ListBox_AddItemData(lpFlistData->hwndFlist, lpFlistEntry);
	else
	    ListBox_InsertItemData(lpFlistData->hwndFlist, nInsertPoint, lpFlistEntry);

	if (lpFlistEntry->flistFlags&FLIST_FLAG_SELECTED)
	    ListBox_SetSel(lpFlistData->hwndFlist, TRUE, nInsertPoint);

    }

    if (lpFdirEntry)
	lpFlistData->flistFiles++;

    insertSelection(lpFlistData->lpFlistSelected, nInsertPoint);
}



static void addTextItem(HWND hwnd, LPFlistData lpFlistData)
{
    flistSuspendControlUpdates(lpFlistData);
    flistClearSelection(lpFlistData);
    addFlistItem(hwnd, lpFlistData, NULL);
    flistControls(hwnd, lpFlistData, FALSE);

    if (lpFlistData->insertControl.bEdit) {
	FARPROC lpProc=MakeProcInstance((FARPROC)editProc, lpGlobals->hInst);

	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FILE_DESCRIPTION", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
	FreeProcInstance(lpProc);
    }
}



static int addLocalItem(HWND hwnd, LPFlistData lpFlistData)
{
	LPOPENFILENAME lpOfn=(LPOPENFILENAME)gmallocz(sizeof(OPENFILENAME));
	char szTitle[64];
	LPSTR lpFileList=gmalloc(setup.dwCommdlgBuffsize);
	int nInitialCount=lpFlistData->localdirUploaded;
	BOOL fExplorerStyle = FALSE;

	static LPCSTR szFilter=
		"Archived files\0*.zip;*.lzh;*.zoo;*.lha;*.arc\0"
		"Executable files\0*.exe;*.dll;*.vbx;*.com\0"
		"Text files\0*.doc;*.wri;*.txt;*.asc\0"
		"All files (*.*)\0*.*\0";

	*lpFileList='\0';
	lpOfn->lStructSize=sizeof(OPENFILENAME);
	lpOfn->hwndOwner=hwnd;
	lpOfn->lpstrFilter=szFilter;
	lpOfn->nFilterIndex=1;
	lpOfn->lpstrFile=lpFileList;
	lpOfn->nMaxFile=setup.dwCommdlgBuffsize;
	lpOfn->lpstrInitialDir=setup.szUploadDir;

	wsprintf(szTitle, "%s Files to Flist (Multi-Select)",
	(LPSTR)(lpFlistData->insertControl.position==CID_APPEND ? "Append" : "Insert"));

	lpOfn->lpstrTitle=szTitle;

	lpOfn->Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT|
	OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_SHOWHELP;

	#ifdef WIN32
	if( lpGlobals->wWinVer >= 0x35F )
		{
		lpOfn->Flags |= OFN_EXPLORER;
		fExplorerStyle = TRUE;
		}
	#endif

	lpFlistData->wHelpContext=lpFlistData->insertControl.position;

	if (GetOpenFileName(lpOfn))
		{
		char szPath[LEN_PATHNAME];
		char szName[LEN_FILENAME];

		if (extractName(lpFileList, szPath, szName, fExplorerStyle))
			{
			flistSuspendControlUpdates(lpFlistData);
			flistClearSelection(lpFlistData);

			if(setup.rememberUp)
			{
				strcpy(setup.szUploadDir, szPath);
				AmWritePrivateProfileString("Moderate", "upload", setup.szUploadDir, setup.szIniFile);
			}
			
			do {
				char szFullName[LEN_PATHNAME+LEN_FILENAME];
				struct stat statBuff;

				wsprintf(szFullName, "%s\\%s", (LPSTR)szPath, (LPSTR)szName);

				if (stat(szFullName, &statBuff))
					inform(hwnd, "Can't find file %s", (LPSTR)szFullName);
				else if (!checkName(szName, CHECK_CIX))
					alert(hwnd, "%s is not a valid CIX filename, entry not added", (LPSTR)szName);
				else
					{
					LPFdirEntry lpNewEntry=(LPFdirEntry)gmalloc(sizeof(FdirEntry));
					LPFdirEntry lpOldEntry;
					CheckReplace crStatus;

					lpNewEntry->fdirHead= &lpFlistData->localdir;
					lpNewEntry->fdirNext=NULL;
					lstrcpy(lpNewEntry->fdirName, szName);

					if (lstrcmp(szPath, setup.szUploadDir)==0)
						lpNewEntry->fdirLocalPath=NULL;
					else
						{
						lpNewEntry->fdirLocalPath=gmalloc(lstrlen(szPath)+1);
						lstrcpy(lpNewEntry->fdirLocalPath, szPath);
						}

					lpNewEntry->fdirSize=statBuff.st_size;
					lpNewEntry->fdirTimestamp=(time_t)statBuff.st_mtime;
					lpNewEntry->fdirReferences=0;
					lpNewEntry->fdirFlags=FDIR_DEFAULT_FLAGS;
					crStatus=checkReplace(hwnd, lpFlistData, FLIST_FROM_UPLOAD, lpNewEntry, &lpOldEntry);

					if (crStatus!=CR_CANCEL)
						{
						lpNewEntry->fdirNext=lpFlistData->localdir;
						lpFlistData->localdir=lpNewEntry;

						switch (crStatus)
							{
							case CR_OK:
								addFlistItem(hwnd, lpFlistData, lpNewEntry);
								lpFlistData->localdirUploaded++;
								break;

							case CR_REPLACE:
								lpFlistData->localdirUploaded++;
								break;

							case CR_DUPLICATE:
//								memcpy(lpNewEntry, lpOldEntry, sizeof(LPFdirEntry));
								addFlistItem(hwnd, lpFlistData, lpNewEntry);//lpOldEntry);
//								gfree(lpNewEntry);
//								lpNewEntry = NULL;
								break;
							}
						}
					else
					{
						gfree(lpNewEntry);
						lpNewEntry = NULL;
					}

					}
				}
			while (extractName(NULL, szPath, szName, fExplorerStyle));
			flistControls(hwnd, lpFlistData, FALSE);
			if (*lpFlistData->lpFlistSelected>0 && lpFlistData->insertControl.bEdit)
				{
				FARPROC lpProc=MakeProcInstance((FARPROC)editProc, lpGlobals->hInst);

				StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FILE_DESCRIPTION", 
								  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
				FreeProcInstance(lpProc);
				}
			}
		}
	else
		{
		DWORD dwStatus=CommDlgExtendedError();

		if (dwStatus)
			alert(hwnd, "GetOpenFileName() error: %lx", dwStatus);
		}
	gfree(lpFileList);
	lpFileList = NULL;
	gfree(lpOfn);
	lpOfn = NULL;
	return(lpFlistData->localdirUploaded-nInitialCount);
}



static void addItem(HWND hwnd, LPFlistData lpFlistData, WORD position, FlistSelect source, BOOL bEdit)
{
    int nFileCount=0;

    if (position==CID_UPDATE_FILE) 
	{
		LPFlistEntry nextUpdate;

		while (nextUpdate=updateFlistFile(lpFlistData, FALSE))
			if (nextUpdate->flistSource)
			switch (source) 
			{

				case FLIST_FROM_MAIL_DIR:
					nFileCount+=updateFromMailDir(hwnd, lpFlistData, nextUpdate->flistSource, bEdit);
					break;

				case FLIST_FROM_UPLOAD:
					nFileCount+=updateFromUpload(hwnd, lpFlistData, nextUpdate->flistSource, bEdit);
					break;

			}
			else
				inform(hwnd, "No file directory information for file %s "
						 "(File directory out of date or file missing from directory) - "
						 "No update performed on this file", nextUpdate->flistName);

		flistControls(hwnd, lpFlistData, FALSE);
	} 
	else 
	{
		if (position==CID_INSERT) 
		{
			lpFlistData->flistInsertPoint=ListBox_GetCaretIndex(lpFlistData->hwndFlist);

			if (lpFlistData->flistInsertPoint==LB_ERR)
				lpFlistData->flistInsertPoint=0;

		}

		lpFlistData->insertControl.position=position;
		lpFlistData->insertControl.source=source;
		lpFlistData->insertControl.bEdit=bEdit;

		switch (source) 
		{

			case FLIST_FROM_FDIR:
			case FLIST_FROM_MAIL_DIR: 
				{
					FARPROC lpProc;

					lpProc=MakeProcInstance((FARPROC)insertFdirProc, lpGlobals->hInst);
					nFileCount+=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_INSERT_FDIR", 
												  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
					FreeProcInstance(lpProc);
					break;
				}

			case FLIST_FROM_FILEPOOL:
			case FLIST_FROM_NOTIFICATION: 
				{
					FARPROC lpProc;

					lpProc=MakeProcInstance((FARPROC)insertFilepoolProc, lpGlobals->hInst);
					nFileCount+=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_INSERT_FILEPOOL", 
												  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
					FreeProcInstance(lpProc);
					break;
				}

			case FLIST_FROM_UPLOAD:
				{
					nFileCount+=addLocalItem(hwnd, lpFlistData);
					break;
				}
			case FLIST_COMMENT:
			case FLIST_TABBED:
			case FLIST_MEMO:
				{
					addTextItem(hwnd, lpFlistData);
					break;
				}
		}
    }

    if (nFileCount && setup.bNotifyAuto)
		notify(hwnd, lpFlistData);
	SetFocus(GetDlgItem(hwnd, CID_FILE_LIST));
	FORWARD_WM_COMMAND(hwnd, CID_FILE_LIST, 0, LBN_SELCHANGE, SendMessage);
//	InvalidateRect(GetDlgItem(hwnd, CID_FDIR_LIST), 0, 0);
	InvalidateRect(GetDlgItem(hwnd, CID_FILE_LIST), 0, 0);
}



//static BOOL tabFlist_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
static BOOL tabFlist_OnInitDialog(HWND hwnd, LPCREATESTRUCT lParam)
{
    LPFlistData lpFlistData= GetMDIData(hwnd, LPFlistData);

	if(lpFlistData)
	{
		lpFlistData->hwndFlist=GetDlgItem(hwnd, CID_FILE_LIST);
		lpFlistData->subclassData.lpfnOldProc=SubclassWindow(lpFlistData->hwndFlist, lpFlistData->subclassData.lpfnNewProc);
		lpFlistData->wHelpContext = 0;
		flistDisplay(hwnd, lpFlistData);
		return(TRUE);
	}
	else
	{
		return FALSE;
	}
}



static void tabFlist_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void tabFlist_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
		drawFlistEntry(lpDrawItem, FALSE);

}



static LPCSTR tabFlist_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_FLIST, id));
}



static void tabFlist_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_EDIT_FLIST);
}



static void tabFlist_OnPopupMenu(HWND hwnd, HWND hwndCtl, int x, int y)
{
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

    TrackPopupMenu(lpFlistData->hMenuFlistPopup, 0, x, y, 0, hwnd, NULL);
}


static void tabFlist_OnInitMenuPopup(HWND hwnd, HMENU hMenu, int item, BOOL fSystemMenu)
{
    if (!fSystemMenu) 
	{
		
		LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);
	    LPFlistEntry flistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, ListBox_GetCurSel(lpFlistData->hwndFlist));
		
		if (hMenu==lpFlistData->hMenuFlistPopup) {
			WORD wLocal = (WORD) (lpFlistData->flistSelectCategory == FSC_LOCAL ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
			WORD wExists = (WORD) (lpFlistData->flistSelectCategory != FSC_NONE ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
			WORD wFile = (WORD) (lpFlistData->flistSelectCategory >= FSC_FILEPOOL ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
			
			EnableMenuItem(hMenu, 2, MF_BYPOSITION|wLocal);
			EnableMenuItem(hMenu, CID_EDIT, wExists);
			EnableMenuItem(hMenu, CID_MOVE_UP, wExists);
			EnableMenuItem(hMenu, CID_MOVE_DOWN, wExists);
			EnableMenuItem(hMenu, CID_NOTIFY, wFile);
			EnableMenuItem(hMenu, CID_DOWNLOAD, wFile);
			EnableMenuItem(hMenu, CID_EXPORT, wFile);
			EnableMenuItem(hMenu, CID_HOLD, wExists);
			EnableMenuItem(hMenu, CID_DELETE, wExists);

			if(ListBox_GetSelCount(lpFlistData->hwndFlist) > 1)
			{
				ModifyMenu( hMenu, CID_HOLD, MF_BYCOMMAND|MF_STRING, CID_HOLD,  "&Hold/Release");
			}
			else if(flistEntry->flistFlags & FLIST_FLAG_HOLD)
			{
				ModifyMenu( hMenu, CID_HOLD, MF_BYCOMMAND|MF_STRING, CID_HOLD,  "&Release");
			}
			else
			{
				ModifyMenu( hMenu, CID_HOLD, MF_BYCOMMAND|MF_STRING, CID_HOLD,  "&Hold");
			}
		} 
		else if (hMenu==lpFlistData->hMenuFlistAction) {
			WORD wFile = (WORD) (lpFlistData->flistSelectCategory>=FSC_FILEPOOL ? MF_ENABLED : MF_DISABLED|MF_GRAYED);
			
			EnableMenuItem(hMenu, CID_NOTIFY, wFile);
			EnableMenuItem(hMenu, CID_DOWNLOAD, wFile);
			EnableMenuItem(hMenu, CID_EXPORT, wFile);
		}
    }
}



static void tabFlist_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

    switch (id) {
	case IDD_HELP:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_EDIT_FLIST);
		break;

    case CID_FILE_LIST:
	switch (codeNotify) {

	case LBN_SELCHANGE:
	    if (lpFlistData->bFlistSelCheck)
		flistControls(hwnd, lpFlistData, FALSE);

	    break;

	case LBN_DBLCLK:
	    FORWARD_WM_COMMAND(hwnd, CID_EDIT, GetDlgItem(hwnd, CID_EDIT),
			       BN_CLICKED, PostMessage);

	    break;

	}

	break;

    case CID_OK:
	if (!flistPostlude(hwnd, lpFlistData, &lpFlistData->context, EXIT_OK))
		StdEndMDIDialog( hwnd );
//		EndDialog(hwnd, 0);

	break;

//	case IDCANCEL:
    case CID_CANCEL:
	if (!flistPostlude(hwnd, lpFlistData, &lpFlistData->context, EXIT_CANCEL))
		StdEndMDIDialog( hwnd );
//		EndDialog(hwnd, 0);

	break;

    case CID_CHANGE_TOPIC: {
	FARPROC lpProc=MakeProcInstance((FARPROC)topicProc, lpGlobals->hInst);
	Context newContext=lpFlistData->context;

	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_SELECT_TOPIC", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPContext)&newContext);
	FreeProcInstance(lpProc);

	if (lpFlistData->context.hTopic!=newContext.hTopic) {
	    flistPrelude(hwnd, lpFlistData, &newContext, &lpFlistData->context);
	    flistDisplay(hwnd, lpFlistData);
	    lpFlistData->context=newContext;
	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;
    }

    case CID_DOWNLOAD_LISTS:
	updateLists(hwnd, lpFlistData);
	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_INSERT:
    case CID_APPEND: {
	POINT point;

	if (hwndCtl) {
	    RECT rect;

	    GetClientRect(hwndCtl, &rect);
	    point.x=rect.right>>1;
	    point.y=rect.bottom>>1;
	    ClientToScreen(hwndCtl, &point);
	} else
	    GetCursorPos(&point);

	TrackPopupMenu(id==CID_INSERT ? lpFlistData->hMenuFlistInsert : lpFlistData->hMenuFlistAppend,
		       0, point.x, point.y, 0, hwnd, NULL);

	break;
    }

    case CID_INSERT_FDIR:	case CID_INSERT_FILEPOOL:	case CID_INSERT_MAIL_DIR:
    case CID_INSERT_UPLOAD:    	case CID_INSERT_NOTIFICATION:   case CID_INSERT_BLANK_LINE:
    case CID_INSERT_COMMENT:    case CID_INSERT_INDENTED:    	case CID_INSERT_MEMO:
    case CID_APPEND_FDIR:    	case CID_APPEND_FILEPOOL:    	case CID_APPEND_MAIL_DIR:
    case CID_APPEND_UPLOAD:    	case CID_APPEND_NOTIFICATION:   case CID_APPEND_BLANK_LINE:
    case CID_APPEND_COMMENT:    case CID_APPEND_INDENTED:    	case CID_APPEND_MEMO:
	switch (id) {

	case CID_INSERT_FDIR:	  	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_FROM_FDIR, TRUE);		break;
	case CID_INSERT_FILEPOOL:	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_FROM_FILEPOOL, TRUE);	break;
	case CID_INSERT_MAIL_DIR:	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_FROM_MAIL_DIR, TRUE);	break;
	case CID_INSERT_UPLOAD:	  	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_FROM_UPLOAD, TRUE);	break;
	case CID_INSERT_NOTIFICATION: 	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_FROM_NOTIFICATION, TRUE);	break;
	case CID_INSERT_BLANK_LINE:	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_COMMENT, FALSE);		break;
	case CID_INSERT_COMMENT:	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_COMMENT, TRUE);		break;
	case CID_INSERT_INDENTED:	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_TABBED, TRUE);		break;
	case CID_INSERT_MEMO:	  	addItem(hwnd, lpFlistData, CID_INSERT, FLIST_MEMO, TRUE);		break;
	case CID_APPEND_FDIR:	  	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_FROM_FDIR, TRUE);		break;
	case CID_APPEND_FILEPOOL:	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_FROM_FILEPOOL, TRUE);	break;
	case CID_APPEND_MAIL_DIR:	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_FROM_MAIL_DIR, TRUE);	break;
	case CID_APPEND_UPLOAD:	  	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_FROM_UPLOAD, TRUE);	break;
	case CID_APPEND_NOTIFICATION: 	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_FROM_NOTIFICATION, TRUE);	break;
	case CID_APPEND_BLANK_LINE:	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_COMMENT, FALSE);		break;
	case CID_APPEND_COMMENT:	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_COMMENT, TRUE);		break;
	case CID_APPEND_INDENTED:	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_TABBED, TRUE);		break;
	case CID_APPEND_MEMO:	  	addItem(hwnd, lpFlistData, CID_APPEND, FLIST_MEMO, TRUE);		break;

	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_UPDATE_FILE: {
	int nSelected, ptrIn, ptrOut;

	nSelected=0;
	ptrIn=ptrOut=1;

	while (ptrIn<= *lpFlistData->lpFlistSelected) {
	    LPFlistEntry itemEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, lpFlistData->lpFlistSelected[ptrIn]);

	    if (itemEntry->flistSelect==FLIST_FROM_FDIR) {
		lpFlistData->lpFlistSelected[ptrOut++]=lpFlistData->lpFlistSelected[ptrIn];
		nSelected++;
	    }

	    ptrIn++;
	}

	if (nSelected>0) {
	    POINT point;

	    updateFlistFile(lpFlistData, TRUE);

	    if (hwndCtl) {
		RECT rect;

		GetClientRect(hwndCtl, &rect);
		point.x=rect.right>>1;
		point.y=rect.bottom>>1;
		ClientToScreen(hwndCtl, &point);
	    } else
		GetCursorPos(&point);

	    TrackPopupMenu(lpFlistData->hMenuFlistUpdate, 0, point.x, point.y, 0, hwnd, NULL);
	} else
	    NextDlgCtl(hwnd, CID_FILE_LIST);

	break;
    }

    case CID_UPDATE_MAIL_DIR:
	addItem(hwnd, lpFlistData, CID_UPDATE_FILE, FLIST_FROM_MAIL_DIR, TRUE);
	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_UPDATE_UPLOAD:
	addItem(hwnd, lpFlistData, CID_UPDATE_FILE, FLIST_FROM_UPLOAD, TRUE);
	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_EDIT:
	if (*lpFlistData->lpFlistSelected>0) {
	    FARPROC lpProc=MakeProcInstance((FARPROC)editProc, lpGlobals->hInst);

	    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FILE_DESCRIPTION", 
						  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
	    FreeProcInstance(lpProc);
		SetFocus(GetDlgItem(hwnd, CID_FILE_LIST));
	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_IMPORT:
	if (!importDescriptions(hwnd, lpFlistData)) {
	    NextDlgCtl(hwnd, CID_FILE_LIST);
	    break;
	}

    case CID_MOVE_UP:
	if (*lpFlistData->lpFlistSelected>0 && lpFlistData->lpFlistSelected[1]>0) {
	    int nPtr;

	    flistSuspendControlUpdates(lpFlistData);

	    for (nPtr=1; nPtr<= *lpFlistData->lpFlistSelected; nPtr++) {
		int nItemIndex=lpFlistData->lpFlistSelected[nPtr];
		LPFlistEntry lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, nItemIndex);
		LPFlistEntry lpFlistJump=lpFlistEntry->flistPrevious;

		ListBox_DeleteString(lpFlistData->hwndFlist, nItemIndex);
		ListBox_InsertItemData(lpFlistData->hwndFlist, nItemIndex-1, lpFlistEntry);
		ListBox_SetSel(lpFlistData->hwndFlist, TRUE, nItemIndex-1);

		if (lpFlistEntry->flistNext)
		    lpFlistEntry->flistNext->flistPrevious=lpFlistJump;

		if (lpFlistJump->flistPrevious)
		    lpFlistJump->flistPrevious->flistNext=lpFlistEntry;
		else
		    lpFlistData->flist=lpFlistEntry;

		lpFlistEntry->flistPrevious=lpFlistJump->flistPrevious;
		lpFlistJump->flistNext=lpFlistEntry->flistNext;
		lpFlistEntry->flistNext=lpFlistJump;
		lpFlistJump->flistPrevious=lpFlistEntry;
		lpFlistData->flistChanged=TRUE;
	    }

	    flistControls(hwnd, lpFlistData, FALSE);
	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_MOVE_DOWN:
	if (*lpFlistData->lpFlistSelected>0 &&
	    lpFlistData->lpFlistSelected[*lpFlistData->lpFlistSelected]<lpFlistData->flistEntries-1) {
	    int nPtr;

	    flistSuspendControlUpdates(lpFlistData);

	    for (nPtr= *lpFlistData->lpFlistSelected; nPtr>=1; nPtr--) {
		int nItemIndex=lpFlistData->lpFlistSelected[nPtr];
		LPFlistEntry lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, nItemIndex);
		LPFlistEntry lpFlistJump=lpFlistEntry->flistNext;

		ListBox_DeleteString(lpFlistData->hwndFlist, nItemIndex);
		ListBox_InsertItemData(lpFlistData->hwndFlist, nItemIndex==lpFlistData->flistEntries-2 ? -1 : nItemIndex+1, lpFlistEntry);
		ListBox_SetSel(lpFlistData->hwndFlist, TRUE, nItemIndex+1);

		if (lpFlistEntry->flistPrevious)
		    lpFlistEntry->flistPrevious->flistNext=lpFlistJump;
		else
		    lpFlistData->flist=lpFlistJump;

		if (lpFlistJump->flistNext)
		    lpFlistJump->flistNext->flistPrevious=lpFlistEntry;

		lpFlistEntry->flistNext=lpFlistJump->flistNext;
		lpFlistJump->flistPrevious=lpFlistEntry->flistPrevious;
		lpFlistEntry->flistPrevious=lpFlistJump;
		lpFlistJump->flistNext=lpFlistEntry;
		lpFlistData->flistChanged=TRUE;
	    }

	    flistControls(hwnd, lpFlistData, FALSE);
	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_ACTION: {
	POINT point;

	if (hwndCtl) {
	    RECT rect;

	    GetClientRect(hwndCtl, &rect);
	    point.x=rect.right>>1;
	    point.y=rect.bottom>>1;
	    ClientToScreen(hwndCtl, &point);
	} else
	    GetCursorPos(&point);

	TrackPopupMenu(lpFlistData->hMenuFlistAction, 0, point.x, point.y, 0, hwnd, NULL);
	break;
    }

    case CID_NOTIFY:
	notify(hwnd, lpFlistData);
	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    case CID_DOWNLOAD: {
	int flistPtr;
	static char s[1024];
	LPSTR sPtr=s;
	int count=0;

	for (flistPtr=1; flistPtr-1< *lpFlistData->lpFlistSelected; flistPtr++) {
	    int itemIndex=lpFlistData->lpFlistSelected[flistPtr];
	    LPFlistEntry flistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, itemIndex);

	    if (flistEntry->flistSelect==FLIST_FROM_FDIR ||
		flistEntry->flistSelect==FLIST_FROM_NOTIFICATION ||
		flistEntry->flistSelect==FLIST_FROM_FILEPOOL) {
		char downloadName[16];
		BOOL ok=verifyName(hwnd, downloadName, flistEntry->flistName);

		if (ok) {
		    HSCRIPT hScript=initScript("Moderate", FALSE);

		    if (lstrcmp(downloadName, flistEntry->flistName)!=0)
			sPtr+=wsprintf(sPtr, "%s -> ", flistEntry->flistName);

		    sPtr+=wsprintf(sPtr, "%s\r\n", (LPSTR)downloadName);

		    addToScript(hScript, "put `join %s/%s`¬"
					 "if waitfor(`R:`, `M:`) == 0¬"
					     "put `fdl %s`¬"
					     "download `%s\\%s`¬"
					     "put `quit`¬"
					     "waitfor `M:`¬"
					 "endif¬",
				lpFlistData->context.lpcConfName, lpFlistData->context.lpcTopicName,
				flistEntry->flistName, (LPSTR)setup.szDownloadDir, (LPSTR)downloadName);

		    lpFlistData->outbasketActions[lpFlistData->actionCount++]=
			actionScript(hScript, OT_INCLUDE, "download %s from flist in %s/%s",
				     flistEntry->flistName, lpFlistData->context.lpcConfName,
				     lpFlistData->context.lpcTopicName);

		    count++;
		}
	    }
	}

	if (count>0) {
	    wsprintf(sPtr, "\r\nmarked for download to %s", (LPSTR)setup.szDownloadDir);
	    inform(hwnd, s);
	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;
    }

    case CID_EXPORT: {
	int nFlistPtr=1;

	while (nFlistPtr<= *lpFlistData->lpFlistSelected) 
	{
	    int nItemIndex=lpFlistData->lpFlistSelected[nFlistPtr++];

	    LPFlistEntry lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, nItemIndex);
	    FlistSelect flistSelect=lpFlistEntry->flistSelect;

	    switch (flistSelect) {

	    case FLIST_FROM_FILEPOOL: {
		LPFdirEntry lpFdirEntry;

		for (lpFdirEntry=lpFlistData->fdir;
		     lpFdirEntry && lstrcmp(lpFlistEntry->flistName, lpFdirEntry->fdirName);
		     lpFdirEntry=lpFdirEntry->fdirNext);

		if (!lpFdirEntry)
		    inform(hwnd, "File %s is not in the fdir - cannot transform from filepool",
			   lpFlistEntry->flistName);

		else {
		    ExportFdirData exportFdirData;
		    FARPROC lpProc=MakeProcInstance((FARPROC)exportFdirProc, lpGlobals->hInst);

		    exportFdirData.lpFdirEntry=lpFdirEntry;
		    exportFdirData.bTransformOK=TRUE;
		    exportFdirData.bExportOK=FALSE;
		    exportFdirData.wHelpTag=HID_FLIST_EXPORT;
		    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_EXPORT", 
							  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPExportFdirData)&exportFdirData);
		    FreeProcInstance(lpProc);

		    if (lpFdirEntry->fdirFlags&FDIR_FLAG_FILEPOOL)
			lpFdirEntry->fdirFlags&= ~FDIR_FLAG_FILEPOOL;
		    else {
			if (!lpFdirEntry->fdirReferences)
			    lpFlistData->fdirOrphans--;
			else if (lpFdirEntry->fdirReferences<MAX_REFERENCES)
			    lpFdirEntry->fdirFlistEntry[lpFdirEntry->fdirReferences++]=lpFlistEntry;

			lpFlistEntry->flistSource=lpFdirEntry;
			lpFlistEntry->flistSelect=FLIST_FROM_FDIR;
			redrawFlistEntry(hwnd, lpFlistData, lpFlistEntry);
			lpFlistData->flistChanged=TRUE;
		    }
		}

		break;
	    }

	    case FLIST_FROM_FDIR:
	    case FLIST_FROM_MAIL_DIR:
	    case FLIST_FROM_UPLOAD:
	    case FLIST_FROM_NOTIFICATION: {
		LPFdirEntry lpFdirEntry=lpFlistEntry->flistSource;

		if (lpFdirEntry) {
		    ExportFdirData exportFdirData;
		    FARPROC lpProc=MakeProcInstance((FARPROC)exportFdirProc, lpGlobals->hInst);
		    WORD wOldState=lpFdirEntry->fdirFlags&(FDIR_FLAG_EXPORT|FDIR_FLAG_FILEPOOL);
		    WORD wNewState;

		    exportFdirData.lpFdirEntry=lpFdirEntry;
		    exportFdirData.bTransformOK=TRUE;
		    exportFdirData.bExportOK=TRUE;
		    exportFdirData.wHelpTag=HID_FLIST_EXPORT;
		    wNewState=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_EXPORT", 
									    (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPExportFdirData)&exportFdirData);
		    FreeProcInstance(lpProc);

		    if (wOldState!=wNewState) {
			int nThisReference;

			for (nThisReference=0; nThisReference<lpFdirEntry->fdirReferences; nThisReference++) {
			    int nFlistIndex;

			    lpFlistEntry=lpFdirEntry->fdirFlistEntry[nThisReference];
			    nFlistIndex = (int)SendMessage(lpFlistData->hwndFlist, LB_FINDSTRING, (WPARAM) -1, (LPARAM)lpFlistEntry);

			    if (wNewState&FDIR_FLAG_EXPORT) {
				lpFlistEntry->flistFlags|=FLIST_FLAG_EXPORT;
				lpFlistEntry->flistFlags&= ~FLIST_FLAG_FILEPOOL;
			    } else if (wNewState&FDIR_FLAG_FILEPOOL) {
				lpFlistEntry->flistFlags&= ~FLIST_FLAG_EXPORT;
				lpFlistEntry->flistFlags|=FLIST_FLAG_FILEPOOL;
			    } else {
				lpFlistEntry->flistFlags&= ~FLIST_FLAG_EXPORT;
				lpFlistEntry->flistFlags&= ~FLIST_FLAG_FILEPOOL;
			    }

			    redrawFlistEntry(hwnd, lpFlistData, lpFlistEntry);
			    removeSelection(lpFlistData->lpFlistSelected, nFlistIndex);
				nFlistPtr--;
			}
		    }
		} else {
		    FARPROC lpProc=MakeProcInstance((FARPROC)exportFlistProc, lpGlobals->hInst);
		    WORD wOldState=lpFlistEntry->flistFlags&(FLIST_FLAG_EXPORT|FLIST_FLAG_FILEPOOL);
		    WORD wNewState=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_EXPORT", 
											 (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistEntry);

		    FreeProcInstance(lpProc);

		    if (wOldState!=wNewState)
			redrawFlistEntry(hwnd, lpFlistData, lpFlistEntry);

		}

		lpFlistData->flistChanged=TRUE;
		break;
	    }

	    }
	}

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;
    }

    case CID_HOLD: {
	int flistPtr=1;

	while (flistPtr-1< *lpFlistData->lpFlistSelected) {
	    int itemIndex=lpFlistData->lpFlistSelected[flistPtr++];
	    LPFlistEntry flistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, itemIndex);
	    FlistSelect select=flistEntry->flistSelect;
	    WORD flags=flistEntry->flistFlags;

	    if (select!=FLIST_MEMO)
		for (;;) {
		    RECT rect;

		    flistEntry->flistFlags^=FLIST_FLAG_HOLD;

		    if (ListBox_GetItemRect(lpFlistData->hwndFlist, itemIndex, &rect)!=LB_ERR)
			InvalidateRect(lpFlistData->hwndFlist, &rect, FALSE);

		    if (select==FLIST_COMMENT || select==FLIST_TABBED)
			break;

		    if (++itemIndex>=lpFlistData->flistEntries)
			break;

		    flistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, itemIndex);

		    if (flistEntry->flistSelect!=FLIST_TABBED ||
			flistEntry->flistFlags&FLIST_FLAG_HOLD^flags&FLIST_FLAG_HOLD)
			break;

		    if (flistPtr-1< *lpFlistData->lpFlistSelected && lpFlistData->lpFlistSelected[flistPtr]==itemIndex)
			flistPtr++;

		}

	}

	if (*lpFlistData->lpFlistSelected>0)
	    lpFlistData->flistChanged=TRUE;

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;
    }

    case CID_DELETE: {
	int flistPtr;
	int deletingFiles=0;
	FlistConfirm deleteConfirm;
	BOOL ok=TRUE;

	for (flistPtr=1; flistPtr-1< *lpFlistData->lpFlistSelected && deletingFiles<2; flistPtr++) {
	    int itemIndex=lpFlistData->lpFlistSelected[flistPtr];
	    LPFlistEntry flistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, itemIndex);

	    if (isFile(flistEntry))
		deletingFiles++;

	}

	deleteConfirm=lpFlistData->flistConfirm;

	if (deletingFiles>1) {
	    FARPROC lpProc=MakeProcInstance((FARPROC)confirmProc, lpGlobals->hInst);

	    ok=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_DELETE_FLIST", 
						     (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPFlistConfirm)&deleteConfirm);
	    FreeProcInstance(lpProc);
	}

	if (ok && *lpFlistData->lpFlistSelected>0) {
	    int flistIndex=lpFlistData->lpFlistSelected[1];

	    flistSuspendControlUpdates(lpFlistData);

	    for (flistPtr=1; flistPtr-1< *lpFlistData->lpFlistSelected; flistPtr++) {
		int itemIndex=lpFlistData->lpFlistSelected[flistPtr];
		LPFlistEntry flistEntry=(LPFlistEntry)ListBox_GetItemData(lpFlistData->hwndFlist, itemIndex);
		int confirm=IDYES;

		if (deleteConfirm.flistConfirm==FLIST_DELETE_CONFIRM && isFile(flistEntry))
		    confirm=query(hwnd, MB_YESNO/*CANCEL*/, "Delete file %s from flist?", flistEntry->flistName);
		else if(isComment(flistEntry))
		    confirm=query(hwnd, MB_YESNO/*CANCEL*/, "Delete comment from flist?");

		if (confirm==IDCANCEL)
		    break;
		else if (confirm==IDYES) {
		    int ptr=flistPtr;

		    switch (flistEntry->flistSelect) {

		    case FLIST_FROM_FDIR:
			if (flistEntry->flistSource) {
			    int fdirDelete;
			    LPFdirEntry fdirEntry=flistEntry->flistSource;

			    switch (deleteConfirm.flistFdirConfirm) {

			    case FDIR_DELETE_CONFIRM:
				fdirDelete=query(hwnd, MB_YESNO,
						 "Delete file %s from fdir as well as flist?",
						 fdirEntry->fdirName);

				break;

			    case FDIR_DELETE_LEAVE:
				fdirDelete=IDNO;
				break;

			    case FDIR_DELETE_ALL:
				fdirDelete=IDYES;
				break;

			    }

			    if (unlinkFdirEntry(lpFlistData, flistEntry, fdirDelete==IDYES))
				lpFlistData->flistMustUpload=TRUE;

			}

			lpFlistData->flistFiles--;
			break;

		    case FLIST_FROM_MAIL_DIR:
		    case FLIST_FROM_FILEPOOL:
		    case FLIST_FROM_UPLOAD:
			unlinkFdirEntry(lpFlistData, flistEntry, TRUE);
			lpFlistData->flistFiles--;
			break;

		    case FLIST_FROM_NOTIFICATION:
			lpFlistData->flistFiles--;
			break;

		    case FLIST_COMMENT:
		    case FLIST_TABBED:
		    case FLIST_MEMO:
			break;

		    }

		    ListBox_DeleteString(lpFlistData->hwndFlist, itemIndex);

		    if (flistEntry->flistPrevious)
			flistEntry->flistPrevious->flistNext=flistEntry->flistNext;
		    else
			lpFlistData->flist=flistEntry->flistNext;

		    if (flistEntry->flistNext)
			flistEntry->flistNext->flistPrevious=flistEntry->flistPrevious;

		    gfree(flistEntry);
			flistEntry = NULL;
		    lpFlistData->flistEntries--;
		    lpFlistData->flistChanged=TRUE;

		    while (ptr++ < *lpFlistData->lpFlistSelected)
			lpFlistData->lpFlistSelected[ptr]--;

		}
	    }

	    if (lpFlistData->flistEntries>0) {
		if (flistIndex>=lpFlistData->flistEntries)
		    flistIndex=lpFlistData->flistEntries-1;

		ListBox_SetSel(lpFlistData->hwndFlist, TRUE, flistIndex);
		ListBox_SetCaretIndex(lpFlistData->hwndFlist, flistIndex);
		SetFocus(GetDlgItem(hwnd, CID_FILE_LIST));
		FORWARD_WM_COMMAND(hwnd, CID_FILE_LIST, 0, LBN_SELCHANGE, SendMessage);
	    }

	    flistControls(hwnd, lpFlistData, FALSE);
	}

	if (lpFlistData->flistEntries>0)
	    NextDlgCtl(hwnd, CID_FILE_LIST);

	break;
    }

    case CID_SORT: {
	FARPROC lpProc;
	BOOL saveChanged=lpFlistData->flistChanged;

	lpFlistData->flistChanged=FALSE;
	lpProc=MakeProcInstance((FARPROC)sortFlistProc, lpGlobals->hInst);
	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_SORT_FLIST", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
	FreeProcInstance(lpProc);

	if (lpFlistData->flistChanged) {
	    RECT rect;

	    GetWindowRect(lpFlistData->hwndFlist, &rect);
	    InvalidateRect(lpFlistData->hwndFlist, &rect, FALSE);
	} else
	    lpFlistData->flistChanged=saveChanged;

	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;
    }

    case CID_UPLOAD_FLIST:
	lpFlistData->flistUploaded=TRUE;
	inform(hwnd, "Moderator's file list marked for upload");
	NextDlgCtl(hwnd, CID_FILE_LIST);
	break;

    }
}



static void tabFlist_OnDestroy(HWND hwnd)
{
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

    if (lpFlistData->subclassData.lpfnOldProc)
	SubclassWindow(lpFlistData->hwndFlist, lpFlistData->subclassData.lpfnOldProc);

    lpFlistData->flistInsertPoint=ListBox_GetCaretIndex(lpFlistData->hwndFlist);
    lpFlistData->hwndFlist=NULL;
}



static LRESULT CALLBACK tabFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		//	  HANDLE_MSG(hwnd, WM_INITDIALOG, tabFlist_OnInitDialog);
		HANDLE_MSG(hwnd, WM_CREATE, tabFlist_OnInitDialog);
		HANDLE_MSG(hwnd, WM_MEASUREITEM, tabFlist_OnMeasureItem);
		HANDLE_MSG(hwnd, WM_DRAWITEM, tabFlist_OnDrawItem);
		HANDLE_MSG(hwnd, WM_POPUPHELP, tabFlist_OnPopupHelp);
		HANDLE_MSG(hwnd, WM_AMHELP, tabFlist_OnAmHelp);
		HANDLE_MSG(hwnd, UM_POPUPMENU, tabFlist_OnPopupMenu);
		HANDLE_MSG(hwnd, WM_INITMENUPOPUP, tabFlist_OnInitMenuPopup);
		HANDLE_MSG(hwnd, WM_COMMAND, tabFlist_OnCommand);
		HANDLE_MSG(hwnd, WM_DESTROY, tabFlist_OnDestroy);

	case WM_ADJUSTWINDOWS: 
		{
	//		ResizeFrame(hwnd, (int)(short)LOWORD( lParam ), (int)(short)HIWORD( lParam ));
			ResizeFlistDialog(hwnd, (int)(short)LOWORD( lParam ), (int)(short)HIWORD( lParam ));
			break;
		}
		
	case WM_CLOSE:
		{
			SendMessage( hwnd, WM_COMMAND, CID_CANCEL, 0 );
			return( 0L );
		}
		
	default:
		{
			if (uMsg==lpGlobals->wHelpMessageID) 
			{
				LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);
				
				switch (lpFlistData->wHelpContext) 
				{
					
				case CID_INSERT:
					HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_INSERT_FLIST_UPLOAD);
					break;
					
				case CID_APPEND:
					HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_APPEND_FLIST_UPLOAD);
					break;
					
				case CID_UPDATE_FILE:
					HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_UPDATE_UPLOAD);
					break;
					
				case CID_IMPORT:
					HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_IMPORT);
					break;
				}
			} 
			else
			{
				return( DefAmeolMDIDlgProc( hwnd, uMsg, wParam, lParam ) );
		//		return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));
			}
		}
	}
	return ( 0L );
}



static void fdirPrelude(HWND hwnd, LPFlistData lpFlistData)
{
    LPFdirEntry lpFdirEntry=lpFlistData->fdir;
    int nCount=0;

    fdirSuspendControlUpdates(lpFlistData);
	
	SendMessage(lpFlistData->hwndFdir, WM_SETREDRAW, 0, 0);

    ListBox_ResetContent(lpFlistData->hwndFdir);

    while (lpFdirEntry) {
	if (fdirEntryVisible(lpFlistData, lpFdirEntry)) {
	    int nIndex=ListBox_AddItemData(lpFlistData->hwndFdir, lpFdirEntry);

	    if (lpFdirEntry->fdirFlags&FDIR_FLAG_SELECTED)
		ListBox_SetSel(lpFlistData->hwndFdir, TRUE, nIndex);

	    nCount++;
	}

	lpFdirEntry=lpFdirEntry->fdirNext;
    }

    if (nCount>0)
	NextDlgCtl(hwnd, CID_FDIR_LIST);

	SendMessage(lpFlistData->hwndFdir, WM_SETREDRAW, 1, 0);
	InvalidateRect(lpFlistData->hwndFdir, 0, 0);

    fdirControls(hwnd, lpFlistData, TRUE);
}



static void drawFdirEntry(const DRAWITEMSTRUCT FAR * lpDis)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rcFlag, rcName, rcSize, rcDate, rcTime;
    LPFdirEntry thisEntry=(LPFdirEntry)lpDis->itemData;
    int width=lpDis->rcItem.right-lpDis->rcItem.left+1;
    char s[32];

	if(!IsWindow(lpDis->hwndItem))
		return;

    CopyRect(&rcFlag, &lpDis->rcItem);
    rcFlag.left+=2;
    rcFlag.right=rcFlag.left+width/20;

    CopyRect(&rcName, &lpDis->rcItem);
    rcName.left=rcFlag.right+4;
    rcName.right=rcName.left+(width*3)/10;

    CopyRect(&rcSize, &lpDis->rcItem);
    rcSize.left=rcName.right+1;
    rcSize.right=rcSize.left+width/5;

    CopyRect(&rcDate, &lpDis->rcItem);
    rcDate.left=rcSize.right+15;
    rcDate.right=rcDate.left+width/5;

    CopyRect(&rcTime, &lpDis->rcItem);
    rcTime.left=rcDate.right+5;
    rcTime.right-=2;

    if (lpDis->itemState&ODS_SELECTED) {
		newBg=GetSysColor(COLOR_HIGHLIGHT);
		newFg=GetSysColor(COLOR_HIGHLIGHTTEXT);
    } else {
		newBg=GetSysColor(COLOR_WINDOW);
		newFg=GetSysColor(COLOR_WINDOWTEXT);
    }

    oldBg=(COLORREF)SetBkColor(lpDis->hDC, newBg);
    oldFg=(COLORREF)SetTextColor(lpDis->hDC, newFg);
    hBrush=CreateSolidBrush(newBg);
    FillRect(lpDis->hDC, &lpDis->rcItem, hBrush);
    DeleteObject(hBrush);

    if (thisEntry) {
	SelectObject(lpDis->hDC, lpGlobals->hfBold);

	if (!thisEntry->fdirReferences || thisEntry->fdirFlags&(FDIR_FLAG_EXPORT|FDIR_FLAG_FILEPOOL))
	    DrawText(lpDis->hDC, "o", -1, &rcFlag, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);

	DrawText(lpDis->hDC, thisEntry->fdirName, -1, &rcName, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
	SelectObject(lpDis->hDC, lpGlobals->hfNormal);
	wsprintf(s, "%ld", thisEntry->fdirSize);
	DrawText(lpDis->hDC, s, -1, &rcSize, DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
	if(thisEntry->fdirTimestamp != -1)
		strftime(s, 31, "%d%b%Y", localtime(&thisEntry->fdirTimestamp));
	else
		wsprintf(s, "Error");
	DrawText(lpDis->hDC, s, -1, &rcDate, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
	if(thisEntry->fdirTimestamp != -1)
		strftime(s, 31, "%I:%M %p", localtime(&thisEntry->fdirTimestamp));
	else
		wsprintf(s, "00:00:00");
	DrawText(lpDis->hDC, s, -1, &rcTime, DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
    }

    if (lpDis->itemState&ODS_FOCUS)
	DrawFocusRect(lpDis->hDC, &lpDis->rcItem);

    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



//static BOOL tabFdir_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
static BOOL tabFdir_OnInitDialog(HWND hwnd, LPCREATESTRUCT lParam)
{
//    LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

	if(lpFlistData)
	{
		lpFlistData->hwndFdir=GetDlgItem(hwnd, CID_FDIR_LIST);
		lpFlistData->subclassData.lpfnOldProc=SubclassWindow(lpFlistData->hwndFdir, lpFlistData->subclassData.lpfnNewProc);
		lpFlistData->wHelpContext = 0;
		CheckMenuItem(lpFlistData->hMenuFdirSort, lpFlistData->fdirSort.sortDirection, MF_CHECKED);
		CheckMenuItem(lpFlistData->hMenuFdirSort, lpFlistData->fdirSort.sortKey, MF_CHECKED);

		RadioButton(hwnd, CID_ORPHANS, lpFlistData->fdirSelect);
		footerDisplay(hwnd, lpFlistData);

		if (lpFlistData->flistInsertPoint==LB_ERR)
		lpFlistData->flistInsertPoint=0;

		sortFdir(&lpFlistData->fdir, lpFlistData->fdirFiles, &lpFlistData->fdirSort);
		lpFlistData->bFdirFlistChanged=FALSE;
		fdirPrelude(hwnd, lpFlistData);
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}



static void tabFdir_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void tabFdir_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
	drawFdirEntry(lpDrawItem);

}



static LPCSTR tabFdir_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_FILE_DIRECTORY, id));
}



static void tabFdir_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_GROUP_4);
}



static void tabFdir_OnPopupMenu(HWND hwnd, HWND hwndCtl, int x, int y)
{
//    LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

	if(lpFlistData)
		TrackPopupMenu(lpFlistData->hMenuFdirPopup, 0, x, y, 0, hwnd, NULL);
}



static void tabFdir_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
//    LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

    switch (id) {
	case IDD_HELP:
	    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_GROUP_4);
		break;

    case CID_FDIR_LIST:
	if (codeNotify==LBN_SELCHANGE && lpFlistData->bFdirSelCheck)
	    fdirControls(hwnd, lpFlistData, FALSE);

	break;

    case CID_OK:
	if (!flistPostlude(hwnd, lpFlistData, &lpFlistData->context, EXIT_OK))
		StdEndMDIDialog( hwnd );
//	    EndDialog(hwnd, 0);

	break;

    case CID_CANCEL:
	if (!flistPostlude(hwnd, lpFlistData, &lpFlistData->context, EXIT_CANCEL))
		StdEndMDIDialog( hwnd );
//		EndDialog(hwnd, 0);

	break;

    case CID_CHANGE_TOPIC: {
	FARPROC lpProc=MakeProcInstance((FARPROC)topicProc, lpGlobals->hInst);
	Context newContext=lpFlistData->context;

	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_SELECT_TOPIC", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPContext)&newContext);
	FreeProcInstance(lpProc);

	if (lpFlistData->context.hTopic!=newContext.hTopic) {
	    flistPrelude(hwnd, lpFlistData, &newContext, &lpFlistData->context);
	    footerDisplay(hwnd, lpFlistData);
	    sortFdir(&lpFlistData->fdir, lpFlistData->fdirFiles, &lpFlistData->fdirSort);
	    lpFlistData->bFdirFlistChanged=FALSE;
	    fdirPrelude(hwnd, lpFlistData);
	    lpFlistData->context=newContext;
	}

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    case CID_DOWNLOAD_LISTS:
	updateLists(hwnd, lpFlistData);
	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;

    case CID_INSERT:
    case CID_APPEND: {
	int fdirPtr;

	flistClearSelection(lpFlistData);

	for (fdirPtr=1; fdirPtr<= *lpFlistData->lpFdirSelected; fdirPtr++) {
	    int itemIndex=lpFlistData->lpFdirSelected[fdirPtr];
	    LPFdirEntry fdirEntry=(LPFdirEntry)ListBox_GetItemData(lpFlistData->hwndFdir, itemIndex);
	    LPFdirEntry oldEntry;
	    CheckReplace status;

	    status=checkReplace(hwnd, lpFlistData, FLIST_FROM_FDIR, fdirEntry, &oldEntry);

	    if (status==CR_OK || status==CR_DUPLICATE) {
		if (lpFlistData->bFdirFlistChanged) {
		    flistClearSelection(lpFlistData);
		    lpFlistData->bFdirFlistChanged=TRUE;
		}

		lpFlistData->insertControl.source=FLIST_FROM_FDIR;
		lpFlistData->insertControl.position=id;
		lpFlistData->insertControl.bEdit=TRUE;
		addFlistItem(hwnd, lpFlistData, fdirEntry);

		if (status==CR_OK) {
		    lpFlistData->fdirOrphans--;

		    if (lpFlistData->fdirSelect==CID_ORPHANS) {
			int nPtr;

			ListBox_DeleteString(lpFlistData->hwndFdir, itemIndex);

			for (nPtr=fdirPtr+1; nPtr<= *lpFlistData->lpFdirSelected; nPtr++)
			    lpFlistData->lpFdirSelected[nPtr]--;

		    }
		}
	    }
	}

	if (*lpFlistData->lpFlistSelected>0) {
	    FARPROC lpProc=MakeProcInstance((FARPROC)editProc, lpGlobals->hInst);
	    RECT rect;

	    GetWindowRect(lpFlistData->hwndFdir, &rect);
	    InvalidateRect(lpFlistData->hwndFdir, &rect, FALSE);
	    StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FILE_DESCRIPTION", 
						  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistData);
	    FreeProcInstance(lpProc);

	    if (setup.bNotifyAuto)
		notify(hwnd, lpFlistData);

		
	}

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    case CID_UPDATE_FILE:
	if (*lpFlistData->lpFdirSelected>0) {
	    POINT point;

	    updateFdirFile(lpFlistData, TRUE);

	    if (hwndCtl) {
		RECT rect;

		GetClientRect(hwndCtl, &rect);
		point.x=rect.right>>1;
		point.y=rect.bottom>>1;
		ClientToScreen(hwndCtl, &point);
	    } else
		GetCursorPos(&point);

	    TrackPopupMenu(lpFlistData->hMenuFdirUpdate, 0, point.x, point.y, 0, hwnd, NULL);
	} else
	    NextDlgCtl(hwnd, CID_FDIR_LIST);

	break;

    case CID_UPDATE_MAIL_DIR:
    case CID_UPDATE_UPLOAD: {
	LPFdirEntry nextUpdate;

	if (lpFlistData->bFdirFlistChanged) {
	    flistClearSelection(lpFlistData);
	    lpFlistData->bFdirFlistChanged=TRUE;
	}

	while (nextUpdate=updateFdirFile(lpFlistData, FALSE))
	    switch (id) {

	    case CID_UPDATE_MAIL_DIR:
		updateFromMailDir(hwnd, lpFlistData, nextUpdate, FALSE);
		break;

	    case CID_UPDATE_UPLOAD:
		updateFromUpload(hwnd, lpFlistData, nextUpdate, FALSE);
		break;

	    }

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    case CID_ACTION: {
	POINT point;

	if (hwndCtl) {
	    RECT rect;

	    GetClientRect(hwndCtl, &rect);
	    point.x=rect.right>>1;
	    point.y=rect.bottom>>1;
	    ClientToScreen(hwndCtl, &point);
	} else
	    GetCursorPos(&point);

	TrackPopupMenu(lpFlistData->hMenuFdirAction, 0, point.x, point.y, 0, hwnd, NULL);
	break;
    }

    case CID_DOWNLOAD: {
	int fdirPtr;
	static char s[1024];
	LPSTR sPtr=s;
	int count=0;

	for (fdirPtr=1; fdirPtr-1< *lpFlistData->lpFdirSelected; fdirPtr++) {
	    int itemIndex=lpFlistData->lpFdirSelected[fdirPtr];
	    LPFdirEntry fdirEntry=(LPFdirEntry)ListBox_GetItemData(lpFlistData->hwndFdir, itemIndex);
	    char downloadName[16];
	    BOOL ok=verifyName(hwnd, downloadName, fdirEntry->fdirName);

	    if (ok) {
		HSCRIPT hScript=initScript("Moderate", FALSE);

		if (lstrcmp(downloadName, fdirEntry->fdirName)!=0)
                    sPtr+=wsprintf(sPtr, "%s -> ", fdirEntry->fdirName);

		sPtr+=wsprintf(sPtr, "%s\r\n", (LPSTR)downloadName);

		addToScript(hScript, "put `join %s/%s`¬"
				     "if waitfor(`R:`, `M:`) == 0¬"
					 "put `fdl %s`¬"
					 "download `%s\\%s`¬"
					 "put `quit`¬"
					 "waitfor `M:`¬"
				     "endif¬",
			    lpFlistData->context.lpcConfName,
			    lpFlistData->context.lpcTopicName, fdirEntry->fdirName,
                            (LPSTR)setup.szDownloadDir, (LPSTR)downloadName);

		lpFlistData->outbasketActions[lpFlistData->actionCount++]=
		    actionScript(hScript, OT_INCLUDE, "download %s from fdir in %s/%s",
				 fdirEntry->fdirName,
				 lpFlistData->context.lpcConfName,
				 lpFlistData->context.lpcTopicName);

		count++;
	    }
	}

	if (count>0) {
	    wsprintf(sPtr, "\r\nmarked for download to %s", (LPSTR)setup.szDownloadDir);
	    inform(hwnd, s);
        }

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    case CID_EXPORT: {
	int nFdirPtr;

	for (nFdirPtr=1; nFdirPtr-1< *lpFlistData->lpFdirSelected; nFdirPtr++) {
	    int nItemIndex=lpFlistData->lpFdirSelected[nFdirPtr];
            LPFdirEntry lpFdirEntry=(LPFdirEntry)ListBox_GetItemData(lpFlistData->hwndFdir, nItemIndex);
	    ExportFdirData exportFdirData;
	    FARPROC lpProc=MakeProcInstance((FARPROC)exportFdirProc, lpGlobals->hInst);
	    WORD wOldState=lpFdirEntry->fdirFlags&(FDIR_FLAG_EXPORT|FDIR_FLAG_FILEPOOL);
	    WORD wNewState;

            exportFdirData.lpFdirEntry=lpFdirEntry;
            exportFdirData.bTransformOK=FALSE;
	    exportFdirData.bExportOK=TRUE;
	    exportFdirData.wHelpTag=HID_FDIR_EXPORT;
            wNewState=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_EXPORT", 
										(HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)(LPExportFdirData)&exportFdirData);
	    FreeProcInstance(lpProc);

	    if (wOldState!=wNewState && lpFdirEntry->fdirReferences) {
		int nThisReference;

		for (nThisReference=0; nThisReference<lpFdirEntry->fdirReferences; nThisReference++) {
                    LPFlistEntry lpFlistEntry=lpFdirEntry->fdirFlistEntry[nThisReference];

                    if (wNewState&FDIR_FLAG_EXPORT) {
                        lpFlistEntry->flistFlags|=FLIST_FLAG_EXPORT;
                        lpFlistEntry->flistFlags&= ~FLIST_FLAG_FILEPOOL;
		    } else if (wNewState&FDIR_FLAG_FILEPOOL) {
			lpFlistEntry->flistFlags&= ~FLIST_FLAG_EXPORT;
			lpFlistEntry->flistFlags|=FLIST_FLAG_FILEPOOL;
		    } else {
                        lpFlistEntry->flistFlags&= ~FLIST_FLAG_EXPORT;
			lpFlistEntry->flistFlags&= ~FLIST_FLAG_FILEPOOL;
		    }
		}

		lpFlistData->flistChanged=TRUE;
		lpFlistData->flistMustUpload=TRUE;
	    }
	}

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    case CID_RENAME: {
	int fdirPtr;

	for (fdirPtr=1; fdirPtr-1< *lpFlistData->lpFdirSelected; fdirPtr++) {
	    int itemIndex=lpFlistData->lpFdirSelected[fdirPtr];
	    LPFdirEntry fdirEntry=(LPFdirEntry)ListBox_GetItemData(lpFlistData->hwndFdir, itemIndex);
	    int ok;
	    FARPROC lpProc=MakeProcInstance((FARPROC)renameProc, lpGlobals->hInst);
	    char oldName[15];

	    lstrcpy(oldName, fdirEntry->fdirName);
	    ok=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_RENAME", 
							 (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)fdirEntry->fdirName);
	    FreeProcInstance(lpProc);

	    if (ok==IDCANCEL)
		break;
	    else if (ok==IDYES) {
		RECT rect;
		HSCRIPT hScript=initScript("Moderate", FALSE);

		if (ListBox_GetItemRect(lpFlistData->hwndFdir, itemIndex, &rect)!=LB_ERR)
		    InvalidateRect(lpFlistData->hwndFdir, &rect, FALSE);

		if (fdirEntry->fdirReferences) {
		    int ptr;

		    for (ptr=0; ptr<fdirEntry->fdirReferences; ptr++)
			lstrcpy(fdirEntry->fdirFlistEntry[ptr]->flistName, fdirEntry->fdirName);

		    lpFlistData->flistChanged=TRUE;
		    lpFlistData->flistMustUpload=TRUE;
		}

		addToScript(hScript, "put `join %s/%s`¬"
				     "if waitfor(`R:`, `M:`) == 0¬"
					 "put `ren %s %s`¬"
					 "waitfor `R:`¬"
					 "put `quit`¬"
					 "waitfor `M:`¬"
				     "endif¬",
			    lpFlistData->context.lpcConfName,
			    lpFlistData->context.lpcTopicName,
			    (LPSTR)oldName, fdirEntry->fdirName);

		lpFlistData->outbasketActions[lpFlistData->actionCount++]=
		    actionScript(hScript, OT_PREINCLUDE, "rename %s to %s in %s/%s",
				 (LPSTR)oldName, fdirEntry->fdirName,
				 lpFlistData->context.lpcConfName,
				 lpFlistData->context.lpcTopicName);

		lpFlistData->fdirChanged=TRUE;
	    }
	}

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    case CID_DELETE: {
	enum {FDIR_CONFIRM_ALL, FDIR_CONFIRM_FLIST, FDIR_NO_CONFIRM} fdirConfirm=(setup.nConfirmDeletes/100)%10;
	int nFdirCount;
	int nFdirPtr=1;
	int selCount=0;
	BOOL bCancel=FALSE;

	fdirSuspendControlUpdates(lpFlistData);

	selCount = *lpFlistData->lpFdirSelected;

	while (!bCancel && nFdirPtr<= *lpFlistData->lpFdirSelected && selCount) 
	{
	    int nItemIndex=lpFlistData->lpFdirSelected[nFdirPtr];
	    LPFdirEntry lpFdirEntry=(LPFdirEntry)ListBox_GetItemData(lpFlistData->hwndFdir, nItemIndex);
	    int nStatus=IDYES;

#ifdef WIN32
		if((INT)lpFdirEntry == LB_ERR)
			break;
#else
		if((int)lpFdirEntry == LB_ERR)
			break;
#endif
		selCount--;

	    if (lpFdirEntry->fdirReferences) 
		{
			if (fdirConfirm<FDIR_NO_CONFIRM)
			{
				nStatus=query(hwnd, MB_YESNOCANCEL,
					"File %s is in the file list: do you want to remove it?",
					lpFdirEntry->fdirName);
			}
			if (nStatus==IDYES) 
			{
				int nPtr;

				for (nPtr=0; nPtr<lpFdirEntry->fdirReferences; nPtr++) 
				{
					lpFlistData->flistEntries--;
					lpFlistData->flistFiles--;

					if (lpFdirEntry->fdirFlistEntry[nPtr]->flistPrevious)
						lpFdirEntry->fdirFlistEntry[nPtr]->flistPrevious->flistNext=lpFdirEntry->fdirFlistEntry[nPtr]->flistNext;
					else
						lpFlistData->flist=lpFdirEntry->fdirFlistEntry[nPtr]->flistNext;

					if (lpFdirEntry->fdirFlistEntry[nPtr]->flistNext)
						lpFdirEntry->fdirFlistEntry[nPtr]->flistNext->flistPrevious=lpFdirEntry->fdirFlistEntry[nPtr]->flistPrevious;

					gfree(lpFdirEntry->fdirFlistEntry[nPtr]);
					lpFdirEntry->fdirFlistEntry[nPtr] = NULL;
				}

				lpFlistData->flistChanged=TRUE;
				lpFlistData->flistMustUpload=TRUE;
			}
	    } 
		else if (fdirConfirm==FDIR_CONFIRM_ALL)
		{
			nStatus=query(hwnd, MB_YESNOCANCEL, "Delete %s from file directory?", lpFdirEntry->fdirName);
		}
		
		switch (nStatus) 
		{

			case IDYES: 
				{
//					int nPtr;

					if (!lpFdirEntry->fdirReferences)
						lpFlistData->fdirOrphans--;

					ListBox_DeleteString(lpFlistData->hwndFdir, nItemIndex);
					deleteFdirEntry(lpFlistData, lpFdirEntry);

//					for (nPtr=nFdirPtr; nPtr<= *lpFlistData->lpFdirSelected; nPtr++)
//						lpFlistData->lpFdirSelected[nPtr]--;
					break;
				}

		    case IDNO:
				{
					nFdirPtr++;
					break;
				}

			case IDCANCEL:
			default:
				bCancel=TRUE;
				break;

	    }
	}

	nFdirCount=fdirDisplayCount(lpFlistData);

	if (nFdirCount>0) {
	    if (*lpFlistData->lpFdirSelected>0) {
		int nFdirIndex=lpFlistData->lpFdirSelected[1];

		if (nFdirIndex>=nFdirCount)
		    nFdirIndex=nFdirCount-1;

		ListBox_SetCaretIndex(lpFlistData->hwndFdir, nFdirIndex);
	    }

	    NextDlgCtl(hwnd, CID_FDIR_LIST);
	}

	fdirControls(hwnd, lpFlistData, FALSE);
	break;
    }

    case CID_SORT: {
	RECT rect;
	POINT point;

	GetClientRect(hwndCtl, &rect);
	point.x=rect.right>>1;
	point.y=rect.bottom>>1;
	ClientToScreen(hwndCtl, &point);
	TrackPopupMenu(lpFlistData->hMenuFdirSort, 0, point.x, point.y, 0, hwnd, NULL);
	break;
    }

    case CID_SORT_ASCENDING:
    case CID_SORT_DESCENDING:
	if (lpFlistData->fdirSort.sortDirection!=id) {
	    CheckMenuItem(lpFlistData->hMenuFdirSort, lpFlistData->fdirSort.sortDirection, MF_UNCHECKED);
            CheckMenuItem(lpFlistData->hMenuFdirSort, id, MF_CHECKED);
	    lpFlistData->fdirSort.sortDirection=id;
	    sortFdir(&lpFlistData->fdir, lpFlistData->fdirFiles, &lpFlistData->fdirSort);
	    fdirPrelude(hwnd, lpFlistData);
	}

        NextDlgCtl(hwnd, CID_FDIR_LIST);
        break;

    case CID_SORT_NAME:
    case CID_SORT_DATE:
    case CID_SORT_SIZE:
	if (lpFlistData->fdirSort.sortKey!=id) {
            CheckMenuItem(lpFlistData->hMenuFdirSort, lpFlistData->fdirSort.sortKey, MF_UNCHECKED);
	    CheckMenuItem(lpFlistData->hMenuFdirSort, id, MF_CHECKED);
	    lpFlistData->fdirSort.sortKey=id;
	    sortFdir(&lpFlistData->fdir, lpFlistData->fdirFiles, &lpFlistData->fdirSort);
	    fdirPrelude(hwnd, lpFlistData);
	}

	NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;

    case CID_ORPHANS:
    case CID_IN_FLIST:
    case CID_ALL_FILES: {
	int fdirOld=lpFlistData->fdirSelect;

	lpFlistData->fdirSelect=RadioButton(hwnd, CID_ORPHANS, 0);

	if (lpFlistData->fdirSelect!=fdirOld)
	    fdirPrelude(hwnd, lpFlistData);

        NextDlgCtl(hwnd, CID_FDIR_LIST);
	break;
    }

    }
}



static void tabFdir_OnDestroy(HWND hwnd)
{
//    LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

    if (lpFlistData->subclassData.lpfnOldProc)
	SubclassWindow(lpFlistData->hwndFdir, lpFlistData->subclassData.lpfnOldProc);

    lpFlistData->hwndFdir=NULL;
}



static LRESULT CALLBACK tabFdir_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		
		//	  HANDLE_MSG(hwnd, WM_INITDIALOG, tabFdir_OnInitDialog);
		HANDLE_MSG(hwnd, WM_CREATE, 		tabFdir_OnInitDialog);
		HANDLE_MSG(hwnd, WM_MEASUREITEM,	tabFdir_OnMeasureItem);
		HANDLE_MSG(hwnd, WM_DRAWITEM,		tabFdir_OnDrawItem);
		HANDLE_MSG(hwnd, WM_POPUPHELP,		tabFdir_OnPopupHelp);
		HANDLE_MSG(hwnd, WM_AMHELP, 		tabFdir_OnAmHelp);
		HANDLE_MSG(hwnd, UM_POPUPMENU,		tabFdir_OnPopupMenu);
		HANDLE_MSG(hwnd, WM_COMMAND,		tabFdir_OnCommand);
		HANDLE_MSG(hwnd, WM_DESTROY,		tabFdir_OnDestroy);
	case WM_ADJUSTWINDOWS: 
		{
			//		ResizeFrame(hwnd, (int)(short)LOWORD( lParam ), (int)(short)HIWORD( lParam ));
			ResizeFDirDialog(hwnd, (int)(short)LOWORD( lParam ), (int)(short)HIWORD( lParam ));
			break;
		}
		
	case WM_CLOSE:
		{
			SendMessage( hwnd, WM_COMMAND, CID_CANCEL, 0 );
			return( 0L );
		}
	default:
		{
			if (uMsg==lpGlobals->wHelpMessageID)
			{
				//		LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);
				LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);
				
				switch (lpFlistData->wHelpContext) 
				{
				case CID_UPDATE_FILE:
					HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FDIR_UPDATE_UPLOAD);
					break;
					
				}
			} 
			else
				return( DefAmeolMDIDlgProc( hwnd, uMsg, wParam, lParam ) );
		//		return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));
		}
	}
	
	return ( 0L );
}


void WINAPI InitModerateMDIDialog(HWND hWnd, LPCREATESTRUCT lParam)
{
LPMDICREATESTRUCT lpMDICreateStruct;

	lpMDICreateStruct = (LPMDICREATESTRUCT)((LPCREATESTRUCT)lParam)->lpCreateParams;
	StdMDIDialogBox( hWnd, "DID_FLIST_FRAME", lpMDICreateStruct );
}

//static BOOL flist_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
static BOOL flist_OnInitDialog(HWND hwnd, LPCREATESTRUCT lParam)
{
//    LPFlistData lpFlistData=InitDlgData(hwnd, FlistData);
    LPFlistData lpFlistData;//=InitMDIData(hwnd, LPFlistData);
	LPWINDINFO lpWindInfo;

	if( !( lpWindInfo = _fmalloc( sizeof( WINDINFO ) ) ) )
		return( -1 );
	_fmemset( lpWindInfo, 0, sizeof( WINDINFO ) );
	SetWindowLong( hwnd, GWL_WNDPTR, (LONG)lpWindInfo );

	lpFlistData = gmallocz(sizeof(FlistData));
	if(!lpFlistData)
		return FALSE;
#ifndef WIN32
	SetWindowLong(hwnd, MOD_DATA, (LONG)(LPFlistData)lpFlistData);
#else
	SetLastError(0);
	if(!SetWindowLong(hwnd, MOD_DATA, (LONG)(LPFlistData)lpFlistData))
	{
		if(GetLastError() != 0)
		{
			alert(hwnd, "Error Setting Window Data (flist_OnInitDialog)");
			return FALSE;
		}
	}
#endif

	InitModerateMDIDialog(hwnd, lParam);

	GetClientRect(GetDlgItem( hwnd, CID_FLIST_FRAME ), &lpFlistData->rcOrg);
	InflateRect(&lpFlistData->rcOrg, 5, 5);//GetSystemMetrics(SM_CYCAPTION));
	CopyRect(&lpFlistData->rcLast, &lpFlistData->rcOrg);

    lpFlistData->context= lpGlobals->contextFlist; // *(LPContext)lParam;
    lpFlistData->context.bInEditor=TRUE;
    lpFlistData->actionCount=0;
    lpFlistData->subclassData.lpfnNewProc=(WNDPROC)MakeProcInstance((FARPROC)listboxProc, lpGlobals->hInst);
    lpFlistData->subclassData.hwndDialog=hwnd;
    lpFlistData->lpPrevSelected=(LPINT)gcallocz(sizeof(int), MAX_FLIST_SELECTIONS+1);

    lpFlistData->hMenuFlistPopup=LoadMenu(lpGlobals->hInst, "MID_FLIST_POPUP");
    lpFlistData->hMenuFlistPopup=GetSubMenu(lpFlistData->hMenuFlistPopup, 0);
    lpFlistData->hMenuFlistInsert=LoadMenu(lpGlobals->hInst, "MID_INSERT");
    lpFlistData->hMenuFlistInsert=GetSubMenu(lpFlistData->hMenuFlistInsert, 0);
    lpFlistData->hMenuFlistAppend=LoadMenu(lpGlobals->hInst, "MID_APPEND");
    lpFlistData->hMenuFlistAppend=GetSubMenu(lpFlistData->hMenuFlistAppend, 0);
    lpFlistData->hMenuFlistUpdate=LoadMenu(lpGlobals->hInst, "MID_UPDATE");
    lpFlistData->hMenuFlistUpdate=GetSubMenu(lpFlistData->hMenuFlistUpdate, 0);
    lpFlistData->hMenuFlistAction=LoadMenu(lpGlobals->hInst, "MID_FLIST_ACTION");
    lpFlistData->hMenuFlistAction=GetSubMenu(lpFlistData->hMenuFlistAction, 0);
    lpFlistData->hwndFlist=NULL;
    lpFlistData->flist=NULL;
    lpFlistData->lpFlistSelected=(LPINT)gcallocz(sizeof(int), MAX_FLIST_SELECTIONS+1);
    flistResetSelection(lpFlistData, TRUE);
    lpFlistData->flistSort=setup.sortFlist;
    lpFlistData->bCreating=FALSE;
    lpFlistData->flistTime=(time_t)0;
    lpFlistData->flistEntries=0;
    lpFlistData->flistFiles=0;
    lpFlistData->flistChanged=FALSE;
    lpFlistData->flistMustUpload=FALSE;
    lpFlistData->flistUploaded=FALSE;
    lpFlistData->flistUpdate=FALSE;
    lpFlistData->flistInsertPoint=0;
    lpFlistData->flistConfirm.flistConfirm=setup.nConfirmDeletes%10;
    lpFlistData->flistConfirm.flistFdirConfirm=(setup.nConfirmDeletes/10)%10;

    lpFlistData->hMenuFdirPopup=LoadMenu(lpGlobals->hInst, "MID_FDIR_POPUP");
    lpFlistData->hMenuFdirPopup=GetSubMenu(lpFlistData->hMenuFdirPopup, 0);
    lpFlistData->hMenuFdirSort=LoadMenu(lpGlobals->hInst, "MID_SORT_FDIR");
    lpFlistData->hMenuFdirSort=GetSubMenu(lpFlistData->hMenuFdirSort, 0);
    lpFlistData->hMenuFdirUpdate=LoadMenu(lpGlobals->hInst, "MID_UPDATE");
    lpFlistData->hMenuFdirUpdate=GetSubMenu(lpFlistData->hMenuFdirUpdate, 0);
    lpFlistData->hMenuFdirAction=LoadMenu(lpGlobals->hInst, "MID_FDIR_ACTION");
    lpFlistData->hMenuFdirAction=GetSubMenu(lpFlistData->hMenuFdirAction, 0);
    lpFlistData->hwndFdir=NULL;
    lpFlistData->fdir=NULL;
    lpFlistData->lpFdirSelected=(LPINT)gcallocz(sizeof(int), MAX_FLIST_SELECTIONS+1);
    fdirResetSelection(lpFlistData, TRUE);
    lpFlistData->bFdirFlistChanged=FALSE;

    lpFlistData->fdirSelect=setup.nFdirSelect==0 ? CID_ORPHANS :
			    setup.nFdirSelect==1 ? CID_IN_FLIST :
						   CID_ALL_FILES;

    lpFlistData->fdirSort=setup.sortFdir;
    lpFlistData->fdirTime=0;
    lpFlistData->fdirFiles=0;
    lpFlistData->fdirOrphans=0;
    lpFlistData->fdirChanged=FALSE;
    lpFlistData->fdirUpdate=FALSE;

//    lpFlistData->fstats=???;
    lpFlistData->fstatTime=0;

    lpFlistData->fuserTime=0;
    lpFlistData->fuserUpdate=FALSE;

    lpFlistData->filepooldir=NULL;

    lpFlistData->maildir=NULL;
    lpFlistData->maildirTime=0;
    lpFlistData->maildirFiles=0;
    lpFlistData->maildirExported=0;
    lpFlistData->maildirUpdate=FALSE;

    lpFlistData->localdir=NULL;
    lpFlistData->localdirUploaded=0;

    lpFlistData->lpTabData=initMDITabs(hwnd, CID_FLIST_FRAME, 2, tabFlist, 0);

    if (flistPrelude(hwnd, lpFlistData, &lpFlistData->context, NULL)) 
	{
		StdEndMDIDialog( hwnd );
		return(FALSE);
    } else
		flistDisplay(hwnd, lpFlistData);

	PostMessage( hwnd, WM_MYSETFOCUS, (WPARAM)GetDlgItem(hwnd, CID_FILE_LIST), 0L );

    return(TRUE);
}



static BOOL flist_OnSelChanging(HWND hwnd)
{
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);
	
	if(lpFlistData)
	{
		closeTab(hwnd, lpFlistData->lpTabData);
	}
	else
		alert(hwnd, "Internal error (flist_OnSelChanging)");

	return(FALSE);
}



static void flist_OnSelChange(HWND hwnd)
{
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

	if(lpFlistData)
	{
	    openMDITab(hwnd, lpFlistData->lpTabData);
	}
	else
		alert(hwnd, "Internal error (flist_OnSelChange)");
}



static DWORD flist_OnNotify(HWND hwnd, int idFrom, LPNMHDR lpNmhdr)
{
    switch (lpNmhdr->code) 
	{
		case TCN_SELCHANGING:
			return(flist_OnSelChanging(hwnd));

		case TCN_SELCHANGE:
			flist_OnSelChange(hwnd);
			return(FALSE);

		default:
		{
			LPFlistData lpFlistData;
			LPTabData lpTabData;
			lpFlistData=(LPFlistData)GetWindowLong(hwnd, MOD_DATA);
			if(lpFlistData)
				lpTabData=lpFlistData->lpTabData;
			else
				lpTabData=NULL;
			return( DefMDIProcTab(hwnd, WM_NOTIFY, (WPARAM)(int)(idFrom), (LPARAM)(NMHDR FAR*)(lpNmhdr), lpTabData, &lpGlobals->bRecursionFlag));
		}
    }
}



static void flist_OnDestroy(HWND hwnd)
{
    LPFlistData lpFlistData=GetMDIData(hwnd, LPFlistData);

    if (lpFlistData) 
	{
		FreeProcInstance((FARPROC)lpFlistData->subclassData.lpfnNewProc);
		freeTabs(hwnd, lpFlistData->lpTabData);
		gfree(lpFlistData->lpFdirSelected);  lpFlistData->lpFdirSelected  = NULL;
		gfree(lpFlistData->lpFlistSelected); lpFlistData->lpFlistSelected = NULL;
		gfree(lpFlistData->lpPrevSelected);  lpFlistData->lpPrevSelected  = NULL;
		FreeMDIData(hwnd);
    }
}

static BOOL flist_OnMove(HWND hWnd, int x, int y)
{
RECT rc;

	if( !IsIconic( hWnd ) && !IsMaximized( hWnd ) ) 
	{
		GetWindowRect( hWnd, &rc );
		ScreenToClient( GetParent( hWnd ), (LPPOINT)&rc );
		WritePPInt( str_Window, str_Left, rc.left );
		WritePPInt( str_Window, str_Top, rc.top );
	}
	return FALSE;
}

static BOOL flist_OnSize(HWND hWnd, UINT state, int x, int y)
{
RECT rc;
LPFlistData lpFlistData;
LPTabData lpTabData;

	if( !IsIconic( hWnd ) && !IsMaximized( hWnd ) ) 
	{
		GetWindowRect( hWnd, &rc );
		WritePPInt( str_Window, str_Width, rc.right - rc.left );
		WritePPInt( str_Window, str_Height, rc.bottom - rc.top );
	}
	if(!IsIconic( hWnd ))
	{
		ResizeFrame(hWnd, x, y);
		SendMessage(hWnd, WM_ADJUSTWINDOWS, 0, 0L);
	}
	lpFlistData=(LPFlistData)GetWindowLong(hWnd, MOD_DATA);
	if(lpFlistData)
		lpTabData=lpFlistData->lpTabData;
	else
		lpTabData=NULL;
	return( (BOOL)DefMDIProcTab(hWnd, WM_SIZE, (WPARAM)(UINT)(state), (LPARAM)MAKELPARAM((x), (y)), lpTabData, &lpGlobals->bRecursionFlag));
}


static BOOL ModProc_OnCommand( HWND hDlg, int id, HWND hwndCtl, UINT codeNotify )
{
	switch(id)
	{
		case IDCANCEL:
		case IDOK:
		{
			LPWINDINFO lpWindInfo;

			lpWindInfo = (LPWINDINFO)GetWindowLong( hDlg, GWL_WNDPTR );
			_ffree( lpWindInfo );
			SetWindowLong( hDlg, GWL_WNDPTR, 0L );

			StdEndMDIDialog( hDlg );
			break; 
		}
	}
	return TRUE;
}

void WINAPI ResizeFrame(HWND hDlg, int x, int y)
{
RECT rc;

	GetClientRect( hDlg, &rc );

	SetWindowPos( GetDlgItem( hDlg, CID_FLIST_FRAME ), NULL, 0, 0, 
				  rc.right-10, rc.bottom-10, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
}

void WINAPI ResizeFlistDialog(HWND hDlg, int x, int y)
{
int dx, dy;
RECT rc;
LPFlistData lpFlistData;

	lpFlistData=(LPFlistData)GetWindowLong(hDlg, MOD_DATA);
	if(!lpFlistData)
		return;

	GetClientRect(GetDlgItem( hDlg, CID_FLIST_FRAME ), &rc);

	dx = (short)((rc.right  - rc.left) - (lpFlistData->rcLast.right-lpFlistData->rcLast.left));
	dy = (short)((rc.bottom - rc.top)  - (lpFlistData->rcLast.bottom-lpFlistData->rcLast.top));

	InflateWnd( GetDlgItem( hDlg, CID_FILE_LIST ),     dx, dy );

	MoveWnd( GetDlgItem( hDlg, IDT_MOD ),              0,  dy );
	MoveWnd( GetDlgItem( hDlg, IDT_FDIR ),             0,  dy );
	MoveWnd( GetDlgItem( hDlg, IDT_STAT ),             0,  dy );
	MoveWnd( GetDlgItem( hDlg, CID_FLM_DATE ),         0,  dy );
	MoveWnd( GetDlgItem( hDlg, CID_FLD_DATE ),         0,  dy );
	MoveWnd( GetDlgItem( hDlg, CID_FLX_DATE ),         0,  dy );
	
	MoveWnd( GetDlgItem( hDlg, CID_OK ),               dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_CANCEL ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, IDD_HELP ),             dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_CHANGE_TOPIC ),     dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_DOWNLOAD_LISTS ),   dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_INSERT ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_APPEND ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_UPDATE_FILE ),      dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_EDIT ),             dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_IMPORT ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_MOVE_UP ),          dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_MOVE_DOWN ),        dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_ACTION ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_SORT ),             dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_UPLOAD_FLIST ),     dx,  0 );
	
//	ShowWindow(GetDlgItem( hDlg, CID_FILE_LIST ), SW_SHOW);

	CopyRect( &lpFlistData->rcLast, &rc );
}

void WINAPI ResizeFDirDialog(HWND hDlg, int x, int y)
{
int dx, dy;
RECT rc;
LPFlistData lpFlistData;

	lpFlistData=(LPFlistData)GetWindowLong(hDlg, MOD_DATA);
	if(!lpFlistData)
		return;

	GetClientRect(GetDlgItem( hDlg, CID_FLIST_FRAME ), &rc);

	dx = (short)((rc.right - rc.left) - (lpFlistData->rcLast.right-lpFlistData->rcLast.left));
	dy = (short)((rc.bottom - rc.top) - (lpFlistData->rcLast.bottom-lpFlistData->rcLast.top));

	InflateWnd( GetDlgItem( hDlg, CID_FDIR_LIST ),     dx, dy );

	MoveWnd( GetDlgItem( hDlg, CID_FLM_DATE ),         0,  dy );
	MoveWnd( GetDlgItem( hDlg, CID_FLD_DATE ),         0,  dy );
	MoveWnd( GetDlgItem( hDlg, CID_FLX_DATE ),         0,  dy );
	MoveWnd( GetDlgItem( hDlg, IDT_MOD ),              0,  dy );
	MoveWnd( GetDlgItem( hDlg, IDT_FDIR ),             0,  dy );
	MoveWnd( GetDlgItem( hDlg, IDT_STAT ),             0,  dy );

	MoveWnd( GetDlgItem( hDlg, CID_OK ),               dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_CANCEL ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, IDD_HELP ),             dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_CHANGE_TOPIC ),     dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_DOWNLOAD_LISTS ),   dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_INSERT ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_APPEND ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_UPDATE_FILE ),      dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_ACTION ),           dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_SORT ),             dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_ORPHANS ),          dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_IN_FLIST ),		   dx,  0 );
	MoveWnd( GetDlgItem( hDlg, CID_ALL_FILES ),        dx,  0 );
	MoveWnd( GetDlgItem( hDlg, IDT_VIEW ),             dx,  0 );

//	ShowWindow(GetDlgItem( hDlg, CID_FDIR_LIST ), SW_SHOW);
	
	CopyRect( &lpFlistData->rcLast, &rc );
}

flist_OnKeyInput(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{

	LPSubclassData lpSubclassData=GetMDIData(GetParent(hwnd), LPSubclassData);

	switch(vk)
	{
		case VK_ESCAPE:
			{
				SendMessage (hwnd, WM_COMMAND, CID_CANCEL, 0);
	    		return FALSE;
			}
		default:
			if(lpSubclassData)
				return((int)WFORWARD_MESSAGE(hwnd, WM_KEYDOWN, vk, MAKELPARAM((cRepeat), (flags)), lpSubclassData->lpfnOldProc));
			else
				return 1;
			break;
	}
}

BOOL _EXPORT CALLBACK flistProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
BOOL fPassToDCP;
LPFlistData lpFlistData;
LPTabData lpTabData;

	fPassToDCP = FALSE;
	switch( uMsg )
	{
		HANDLE_MSG(hDlg, WM_CREATE,  flist_OnInitDialog);
	    HANDLE_MSG(hDlg, WM_NOTIFY,  flist_OnNotify);
		HANDLE_MSG(hDlg, WM_DESTROY, flist_OnDestroy);
		HANDLE_MSG(hDlg, WM_SIZE,    flist_OnSize);
		HANDLE_MSG(hDlg, WM_MOVE,    flist_OnMove);
 
		case WM_GETMINMAXINFO:
			{
				MINMAXINFO FAR * lpmmi = (MINMAXINFO FAR *) lParam; 
				POINT		 pt;
				LPFlistData lpFlistData=(LPFlistData)GetMDIData(hDlg, LPFlistData);

				if (lpFlistData) 
				{
					pt.x = (lpFlistData->rcOrg.right - lpFlistData->rcOrg.left) / 2;
					pt.y = lpFlistData->rcOrg.bottom - lpFlistData->rcOrg.top;
					lpmmi->ptMinTrackSize = pt; 
					DefAmeolMDIDlgProc(hDlg, uMsg, wParam, lParam);
					lpmmi->ptMinTrackSize = pt; 
				}
				break;
			}

		case WM_CLOSE:
			SendMessage( hDlg, WM_COMMAND, CID_CANCEL, 0 );
			return( 0L );

		case WM_MYSETFOCUS: 
			{
				LPWINDINFO lpWindInfo;
				lpWindInfo = NULL;
				lpWindInfo = (LPWINDINFO)GetWindowLong( hDlg, GWL_WNDPTR );
				if(lpWindInfo != NULL)
				{
					if(IsWindow((HWND)wParam))
					{
						if(lpWindInfo != NULL)
						{
							lpWindInfo->hwndFocus = (HWND)wParam;
							SetFocus(lpWindInfo->hwndFocus);
						}
					}
				}
				break;
			}
		case WM_MDIACTIVATE:
			{
				LPWINDINFO lpWindInfo;
				lpWindInfo = NULL;
				lpWindInfo = (LPWINDINFO)GetWindowLong( hDlg, GWL_WNDPTR );
				if(lpWindInfo != NULL)
				{
					if(hDlg != (HWND) lParam)
					{
						lpWindInfo->hwndFocus = GetFocus();//GetDlgItem(hDlg, CID_FILE_LIST);
					}
					else
					{
						LPFlistData lpFlistData=(LPFlistData)GetMDIData(hDlg, LPFlistData);
						if (lpFlistData) 
						{
							lpGlobals->contextFlist = lpGlobals->context;
							lpGlobals->context  = lpFlistData->context;
						}
						SetFocus(lpWindInfo->hwndFocus);
					}
				}
				fPassToDCP = TRUE;
				break;
			}

		default:	
			{
				fPassToDCP = TRUE;
				break;
			}
	}
	if( fPassToDCP )
	{
		lpFlistData=(LPFlistData)GetWindowLong(hDlg, MOD_DATA);
		if(lpFlistData)
			lpTabData=lpFlistData->lpTabData;
		else
			lpTabData=NULL;
		return( (BOOL)DefMDIProcTab(hDlg, uMsg, wParam, lParam, lpTabData, &lpGlobals->bRecursionFlag));
	}
	return( FALSE );
}



/*BOOL _EXPORT CALLBACK flistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=flist_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
} */



static void listbox_OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
//    LPSubclassData lpSubclassData=GetDlgData(GetParent(hwnd), SubclassData);
    LPSubclassData lpSubclassData=GetMDIData(GetParent(hwnd), LPSubclassData);
    POINT point;

    FORWARD_WM_LBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, PostMessage);
    FORWARD_WM_LBUTTONUP(hwnd, x, y, keyFlags, PostMessage);
    point.x=x;
    point.y=y;
    ClientToScreen(hwnd, &point);
	
	BringWindowToTop(GetParent(hwnd));

    Post_Moderate_PopupMenu(lpSubclassData->hwndDialog, hwnd, point.x, point.y);
}

listbox_OnKeyInput(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{

	LPSubclassData lpSubclassData=GetMDIData(GetParent(hwnd), LPSubclassData);
	
	switch(vk)
	{
	case VK_RETURN:
		{
			SendMessage(GetParent(hwnd), WM_COMMAND, CID_EDIT, 0);
			return FALSE;
		}
	case VK_DELETE:
		{
			int cCode;
			cCode = ListBox_GetCurSel(hwnd);
			if(cCode != LB_ERR )
				SendMessage (GetParent(hwnd), WM_COMMAND, CID_DELETE, 0);
			return FALSE;
		}
	case VK_INSERT:
		{
			int cCode;
			cCode = ListBox_GetCurSel(hwnd);
			if(cCode != LB_ERR )
				SendMessage (GetParent(hwnd), WM_COMMAND, CID_INSERT_BLANK_LINE, 0);
			return FALSE;
		}
	default:
		{
			if(lpSubclassData)
				return((int)WFORWARD_MESSAGE(hwnd, WM_KEYDOWN, vk, MAKELPARAM((cRepeat), (flags)), lpSubclassData->lpfnOldProc));
			return FALSE;
		}
	}
}

LONG _EXPORT CALLBACK listboxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) 
	{
	HANDLE_MSG(hwnd, WM_RBUTTONDOWN, listbox_OnRButtonDown);
	HANDLE_MSG(hwnd, WM_KEYDOWN, listbox_OnKeyInput);	
	default: 
		{
			if(TabCtrl_GetCurSel(GetDlgItem(GetParent(hwnd), CID_FLIST_FRAME)) == 0)
				return NewListProc(hwnd, uMsg, wParam, lParam);
			else
			{
				LPSubclassData lpSubclassData= (LPSubclassData)GetWindowLong(GetParent(hwnd), MOD_DATA);
				if(!lpSubclassData)
					return FALSE;
				return WFORWARD_MESSAGE(hwnd, uMsg, wParam, lParam, lpSubclassData->lpfnOldProc);
			}
		}
    }
}

