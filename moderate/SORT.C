#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include "ameolapi.h"
#include "hctools.h"
#include "winhorus.h"
#include "moderate.h"
#include "setup.h"
#include "help.h"
#include "globals.h"
#include "flist.h"



typedef struct {
    LPFlistEntry lpFirstEntry;
    LPFlistEntry lpFlistEntry;
    LPFlistEntry lpLastEntry;
} FlistSortData, FAR *LPFlistSortData, FAR * FAR *LPPFlistSortData;



static LPSortData lpFlistSort;
static LPSortData lpFdirSort;



static int compareFlist(const void FAR *a, const void FAR *b)
{
    LPFlistEntry aa, bb;
    LPFdirEntry aaSource, bbSource;

    switch (lpFlistSort->sortDirection) {

    case CID_SORT_ASCENDING:
	aa=((LPFlistSortData)a)->lpFlistEntry;
	bb=((LPFlistSortData)b)->lpFlistEntry;
	break;

    case CID_SORT_DESCENDING:
	aa=((LPFlistSortData)b)->lpFlistEntry;
	bb=((LPFlistSortData)a)->lpFlistEntry;
	break;

    }

    aaSource=aa->flistSource;

    if (aaSource && aaSource->fdirSize<0)
	aaSource=NULL;

    bbSource=bb->flistSource;

    if (bbSource && bbSource->fdirSize<0)
	bbSource=NULL;

    switch (lpFlistSort->sortKey) {

    case CID_SORT_NAME:
	return(lstrcmp(aa->flistName, bb->flistName));

    case CID_SORT_DATE:
	return(aaSource && bbSource ? aaSource->fdirTimestamp>bbSource->fdirTimestamp ? 1 :
				      aaSource->fdirTimestamp<bbSource->fdirTimestamp ? -1 :
				      lstrcmp(aa->flistName, bb->flistName) :
	       aaSource ? 1 :
	       bbSource ? -1 :
	       lstrcmp(aa->flistName, bb->flistName));

    case CID_SORT_SIZE:
	return(aaSource && bbSource ? aaSource->fdirSize>bbSource->fdirSize ? 1 :
				      aaSource->fdirSize<bbSource->fdirSize ? -1 :
				      lstrcmp(aa->flistName, bb->flistName) :
	       aaSource ? 1 :
	       bbSource ? -1 :
	       lstrcmp(aa->flistName, bb->flistName));

    default:
	return(0);

    }
}



static BOOL sortFlistChunk(HWND hFlist, LPPFlistEntry lppFlist, LPFlistSortData lpFlistSortData,
			   int nSort, int nFirstIndex, int nLastIndex)
{
    LPFlistEntry lpFlistBefore, lpFlistAfter;
    int nLinkPtr, nListPtr;
    LPFlistEntry lpNextEntry;

    if (nSort<=1)
	return(FALSE);

    lpFlistBefore=lpFlistSortData[0].lpFirstEntry->flistPrevious;
    lpFlistAfter=lpFlistSortData[nSort-1].lpLastEntry->flistNext;
    qsort(lpFlistSortData, nSort, sizeof(FlistSortData), compareFlist);

    if (lpFlistBefore)
        lpFlistBefore->flistNext=lpFlistSortData[0].lpFirstEntry;
    else
        *lppFlist=lpFlistSortData[0].lpFirstEntry;

    lpFlistSortData[0].lpFirstEntry->flistPrevious=lpFlistBefore;

    for (nLinkPtr=1; nLinkPtr<nSort; nLinkPtr++) {
        lpFlistSortData[nLinkPtr-1].lpLastEntry->flistNext=lpFlistSortData[nLinkPtr].lpFirstEntry;
        lpFlistSortData[nLinkPtr].lpFirstEntry->flistPrevious=lpFlistSortData[nLinkPtr-1].lpLastEntry;
    }

    if (lpFlistAfter)
        lpFlistAfter->flistPrevious=lpFlistSortData[nSort-1].lpLastEntry;

    lpFlistSortData[nSort-1].lpLastEntry->flistNext=lpFlistAfter;
    lpNextEntry=lpFlistSortData[0].lpFirstEntry;
    SetWindowRedraw(hFlist, FALSE);

    for (nListPtr=nFirstIndex; nListPtr<=nLastIndex; nListPtr++) {
        ListBox_DeleteString(hFlist, nListPtr);
        ListBox_InsertItemData(hFlist, lpNextEntry->flistNext ? nListPtr : -1, (DWORD)lpNextEntry);
        ListBox_SetSel(hFlist, TRUE, nListPtr);
        lpNextEntry=lpNextEntry->flistNext;
    }

    SetWindowRedraw(hFlist, TRUE);
    ListBox_SetCaretIndex(hFlist, nFirstIndex);
    return(TRUE);
}



static BOOL sortFlist(HWND hFlist, LPPFlistEntry lppFlist, LPINT lpFlistSelected, LPSortData lpSort)
{
    LPFlistSortData lpFlistSortData;
    int nFlist=1;
    BOOL changed=FALSE;
    BOOL finished=FALSE;

    if (*lpFlistSelected<=1)
	return(FALSE);

    lpFlistSortData=(LPFlistSortData)gmalloc(*lpFlistSelected*sizeof(FlistSortData));

    if (!lpFlistSortData)
        return(FALSE);

    lpFlistSort=lpSort;

    while (!finished) {
        int nSort=0;
        int nFlistIndex;
        int nPrevIndex;
        LPFlistEntry lpFlistEntry;
        int nFirstIndex, nLastIndex;
        BOOL continueSearch=TRUE;
        BOOL finished=FALSE;

        while (nFlist<= *lpFlistSelected && continueSearch) {
            nFlistIndex=lpFlistSelected[nFlist++];
            lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(hFlist, nFlistIndex);
            continueSearch=isComment(lpFlistEntry);
        }

        if (continueSearch)
            break;

        nPrevIndex=nFlistIndex-1;

        do {
            if (nSort==0)
                nFirstIndex=nFlistIndex;

            lpFlistSortData[nSort].lpFirstEntry=lpFlistEntry;

            if (isComment(lpFlistEntry)) {
                continueSearch=TRUE;

		while (continueSearch && nFlist<= *lpFlistSelected) {
                    nPrevIndex=nFlistIndex;
                    nFlistIndex=lpFlistSelected[nFlist++];
                    lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(hFlist, nFlistIndex);
                    continueSearch=nFlistIndex==nPrevIndex+1 && isComment(lpFlistEntry);
                }

                if (continueSearch)
		    finished=TRUE;

            }

            if (nFlistIndex==nPrevIndex+1 && isFile(lpFlistEntry)) {
                lpFlistSortData[nSort].lpFlistEntry=lpFlistEntry;

                if (nFlist> *lpFlistSelected) {
                    nLastIndex=nFlistIndex;
                    lpFlistSortData[nSort].lpLastEntry=lpFlistEntry;
                    finished=TRUE;
                } else {
                    LPFlistEntry lpPrevEntry;

                    do {
                        nPrevIndex=nFlistIndex;
                        lpPrevEntry=lpFlistEntry;
                        nFlistIndex=lpFlistSelected[nFlist++];
                        lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(hFlist, nFlistIndex);
                        continueSearch=nFlistIndex==nPrevIndex+1 && isContinuation(lpFlistEntry);
		    } while (continueSearch && nFlist<= *lpFlistSelected);

                    if (continueSearch) {
                        nLastIndex=nFlistIndex;
                        lpFlistSortData[nSort].lpLastEntry=lpFlistEntry;
                        finished=TRUE;
                    } else {
                        nLastIndex=nPrevIndex;
                        lpFlistSortData[nSort].lpLastEntry=lpPrevEntry;
                    }
                }

                nSort++;
            }

            if (nSort>0 && (nFlistIndex!=nPrevIndex+1 || finished)) {
                if (sortFlistChunk(hFlist, lppFlist, lpFlistSortData, nSort, nFirstIndex, nLastIndex))
                    changed=TRUE;

                nSort=0;
            }
        } while (nFlistIndex==nPrevIndex+1 && !finished);
    }

    gfree(lpFlistSortData);
	lpFlistSortData = NULL;
    return(changed);
}



static BOOL sortFlist_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPFlistData lpFlistData=(LPFlistData)lParam;

    SetDlgData(hwnd, lpFlistData);
    RadioButton(hwnd, CID_SORT_ASCENDING, lpFlistData->flistSort.sortDirection);
    RadioButton(hwnd, CID_SORT_NAME, lpFlistData->flistSort.sortKey);
    return(TRUE);
}



static LPCSTR sortFlist_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_SORT_FLIST, id));
}



static void sortFlist_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_SORT_FILE_LIST);
}



static void sortFlist_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_OK: {
	LPFlistData lpFlistData=GetDlgData(hwnd, FlistData);

	lpFlistData->flistSort.sortDirection=RadioButton(hwnd, CID_SORT_ASCENDING, 0);
	lpFlistData->flistSort.sortKey=RadioButton(hwnd, CID_SORT_NAME, 0);

	if (sortFlist(lpFlistData->hwndFlist, &lpFlistData->flist, lpFlistData->lpFlistSelected, &lpFlistData->flistSort))
	    lpFlistData->flistChanged=TRUE;

	EndDialog(hwnd, TRUE);
	break;
    }

    case CID_CANCEL:
	EndDialog(hwnd, FALSE);
	break;

    }
}



static LRESULT sortFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, sortFlist_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, sortFlist_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, sortFlist_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, sortFlist_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK sortFlistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=sortFlist_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static int compareFdir(const void FAR *a, const void FAR *b)
{
    LPFdirEntry aa, bb;

    switch (lpFdirSort->sortDirection) {

    case CID_SORT_ASCENDING:
        aa= *(LPPFdirEntry)a;
        bb= *(LPPFdirEntry)b;
        break;

    case CID_SORT_DESCENDING:
        aa= *(LPPFdirEntry)b;
        bb= *(LPPFdirEntry)a;
        break;

    }

    switch (lpFdirSort->sortKey) {

    case CID_SORT_NAME:
        return(lstrcmp(aa->fdirName, bb->fdirName));

    case CID_SORT_DATE:
        return(aa->fdirTimestamp>bb->fdirTimestamp ? 1 :
               aa->fdirTimestamp<bb->fdirTimestamp ? -1 :
               lstrcmp(aa->fdirName, bb->fdirName));

    case CID_SORT_SIZE:
	return(aa->fdirSize>bb->fdirSize ? 1 :
	       aa->fdirSize<bb->fdirSize ? -1 :
	       lstrcmp(aa->fdirName, bb->fdirName));

    default:
	return(0);

    }
}



void sortFdir(LPPFdirEntry lppFdirHeader, int nFdirCount, LPSortData lpSort)
{
    if (nFdirCount>1) {
	LPPFdirEntry lppFdirArray=(LPPFdirEntry)gmalloc(sizeof(LPFdirEntry)*nFdirCount);
	LPFdirEntry lpThisFdir;
	int nLinkPtr=0;

	if (!lppFdirArray) {
	    alert(NULL, "Memory allocation failed in sortFdir()!");
	    return;
	}

	for (lpThisFdir= *lppFdirHeader; lpThisFdir; lpThisFdir=lpThisFdir->fdirNext) {
	    if (nLinkPtr>=nFdirCount) {
		alert(NULL, "List overrun in sortFdir()!");
		break;
	    }

	    lppFdirArray[nLinkPtr++]=lpThisFdir;
	}

	lpFdirSort=lpSort;
	qsort(lppFdirArray, nFdirCount, sizeof(LPFdirEntry), compareFdir);

	*lppFdirHeader=lppFdirArray[0];

	for (nLinkPtr=0; nLinkPtr<nFdirCount-1; nLinkPtr++)
	    lppFdirArray[nLinkPtr]->fdirNext=lppFdirArray[nLinkPtr+1];

	lppFdirArray[nFdirCount-1]->fdirNext=NULL;
	gfree(lppFdirArray);
	lppFdirArray = NULL;
    }
}

