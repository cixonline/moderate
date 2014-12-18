#include <windows.h>
#include <windowsx.h>
#include "winhorus.h"
#include "edit.h"



LONG _EXPORT CALLBACK editSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



static LPEditControl lpEditStack=NULL;



LPEditControl PASCAL editPrelude(HWND hdlg, WORD wEditControl, HFONT hfont, WORD wFlags)
{
    LPEditControl lpEditControl=(LPEditControl)gmallocz(sizeof(EditControl));

    lpEditControl->lpEditPrevious=lpEditStack;
    lpEditControl->wEditControl=wEditControl;
    lpEditControl->hwnd=GetDlgItem(hdlg, wEditControl);
    lpEditControl->hfont=hfont;
    lpEditControl->bFocus=GetFocus()==lpEditControl->hwnd;;
    lpEditControl->pointPosition.x=lpEditControl->pointPosition.y=0;
    lpEditControl->rangeSelection.wStart=lpEditControl->rangeSelection.wEnd=0;
    lpEditControl->bChanged=FALSE;
    lpEditControl->lpBuff=NULL;
    lpEditControl->wPtr=0;
    SetWindowFont(lpEditControl->hwnd, hfont, FALSE);
    lpEditControl->bTab=FALSE;
    lpEditControl->wFlags=wFlags;
    lpEditControl->lpfnEditProc=MakeProcInstance((FARPROC)editSubclassProc, GetWindowWord(hdlg, GWW_HINSTANCE));
    lpEditControl->lpfnEditOldProc=SubclassWindow(lpEditControl->hwnd, lpEditControl->lpfnEditProc);
    lpEditStack=lpEditControl;
    return(lpEditControl);
}



void PASCAL editPostlude(LPEditControl lpEditControl)
{
    LPEditControl lpEditSearch;
    LPEditControl lpEditParent=NULL;

    for (lpEditSearch=lpEditStack;
         lpEditSearch && lpEditSearch!=lpEditControl;
         lpEditParent=lpEditSearch, lpEditSearch=lpEditSearch->lpEditPrevious);

    if (lpEditSearch) {
        if (lpEditParent)
            lpEditParent->lpEditPrevious=lpEditControl->lpEditPrevious;
        else
            lpEditStack=lpEditControl->lpEditPrevious;
                      
        if (lpEditControl->lpfnEditOldProc) {
            SubclassWindow(lpEditControl->hwnd, lpEditControl->lpfnEditOldProc);
            FreeProcInstance(lpEditControl->lpfnEditProc);
        }

	gfree(lpEditControl);
	lpEditControl = NULL;
    }
}



WORD PASCAL editPointToOffset(LPEditControl lpEditControl, POINT pointPosition)
{
    return((WORD) (Edit_LineIndex(lpEditControl->hwnd, pointPosition.y)+pointPosition.x));
}



POINT PASCAL editOffsetToPoint(LPEditControl lpEditControl, WORD wOffset)
{
    POINT pointPosition;

    pointPosition.y=Edit_LineFromChar(lpEditControl->hwnd, wOffset);
    pointPosition.x=wOffset-Edit_LineIndex(lpEditControl->hwnd, pointPosition.y);
    return(pointPosition);
}



POINT PASCAL editGetPosition(LPEditControl lpEditControl)
{
    if (lpEditControl->bFocus) {
	HFONT hOldFont;
	TEXTMETRIC tm;
	HDC hDC;
	POINT pointCaret;
	RECT rect;
	int nLineOffset;

	GetCaretPos(&pointCaret);
	hDC=GetDC(lpEditControl->hwnd);
	hOldFont=SelectFont(hDC, lpEditControl->hfont);
	GetTextMetrics(hDC, &tm);
	Edit_GetRect(lpEditControl->hwnd, &rect);
	nLineOffset=Edit_GetFirstVisibleLine(lpEditControl->hwnd);
	lpEditControl->pointPosition.y=nLineOffset+(pointCaret.y-rect.top)/tm.tmHeight;

	if (pointCaret.x<=rect.left)
	    lpEditControl->pointPosition.x=0;
	else {
	    WORD wSize=GetWindowTextLength(lpEditControl->hwnd)+1;
	    int nLineCount=Edit_GetLineCount(lpEditControl->hwnd);
	    int nStart=Edit_LineIndex(lpEditControl->hwnd, lpEditControl->pointPosition.y);
	    int nEnd=nStart>=nLineCount-1 ? wSize : Edit_LineIndex(lpEditControl->hwnd, lpEditControl->pointPosition.y+1);
	    LPSTR lpText=(LPSTR)gmalloc(wSize);
	    BOOL bFinished=FALSE;
	    int nPtr;

	    Edit_GetText(lpEditControl->hwnd, lpText, wSize);

	    for (nPtr=nStart; nPtr<nEnd && !bFinished; nPtr++) {
		SIZE size;

		GetTextExtentPoint(hDC, lpText+nStart, (nPtr-nStart)+1, &size);
		bFinished=pointCaret.x<=rect.left+size.cx;
	    }

	    gfree(lpText);
		lpText = NULL;
	    lpEditControl->pointPosition.x=nPtr-nStart;
	}

	SelectFont(hDC, hOldFont);
	ReleaseDC(lpEditControl->hwnd, hDC);
    }

    return(lpEditControl->pointPosition);
}



void PASCAL editResetPosition(LPEditControl lpEditControl)
{
    HFONT hEditFont;
    HFONT hOldFont;
    TEXTMETRIC tm;
    HDC hDC;
    POINT pointCaret;
    RECT rect;
    int nLineOffset, nLineCount;
    WORD wSize;
    LPSTR lpText;
    int nStart;
    SIZE size;

    if (!lpEditControl->bFocus)
        return;

    hEditFont=GetWindowFont(lpEditControl->hwnd);
    hDC=GetDC(lpEditControl->hwnd);
    hOldFont=SelectFont(hDC, hEditFont);
    GetTextMetrics(hDC, &tm);
    Edit_GetRect(lpEditControl->hwnd, &rect);
    nLineOffset=Edit_GetFirstVisibleLine(lpEditControl->hwnd);
    nLineCount=(rect.bottom-rect.top+1)/tm.tmHeight;

    if (lpEditControl->pointPosition.y>=nLineOffset+nLineCount) {
        int nScroll=lpEditControl->pointPosition.y-(nLineOffset+nLineCount)+1;

        Edit_Scroll(lpEditControl->hwnd, nScroll, 0);
        nLineOffset+=nScroll;
    }

    pointCaret.y=(lpEditControl->pointPosition.y-nLineOffset)*tm.tmHeight+rect.top;
    nStart=Edit_LineIndex(lpEditControl->hwnd, lpEditControl->pointPosition.y);
    wSize=GetWindowTextLength(lpEditControl->hwnd)+1;
    lpText=(LPSTR)gmalloc(wSize);
    Edit_GetText(lpEditControl->hwnd, lpText, wSize);
    GetTextExtentPoint(hDC, lpText+nStart, lpEditControl->pointPosition.x, &size);
    pointCaret.x=size.cx+rect.left;
    gfree(lpText);
	lpText = NULL;
    SetCaretPos(pointCaret.x, pointCaret.y);
    SelectFont(hDC, hOldFont);
    ReleaseDC(lpEditControl->hwnd, hDC);
}



void PASCAL editSetPosition(LPEditControl lpEditControl, POINT pointPosition)
{
    lpEditControl->pointPosition=pointPosition;
    editResetPosition(lpEditControl);
}



RANGE PASCAL editGetSelection(LPEditControl lpEditControl)
{
    DWORD dwSelection=Edit_GetSel(lpEditControl->hwnd);

    lpEditControl->rangeSelection.wStart=LOWORD(dwSelection);
    lpEditControl->rangeSelection.wEnd=HIWORD(dwSelection);
    return(lpEditControl->rangeSelection);
}



void PASCAL editResetSelection(LPEditControl lpEditControl)
{
    Edit_SetSel(lpEditControl->hwnd, lpEditControl->rangeSelection.wStart, lpEditControl->rangeSelection.wEnd);
}



void PASCAL editSetSelection(LPEditControl lpEditControl, RANGE rangeSelection)
{
    lpEditControl->rangeSelection=rangeSelection;
    editResetSelection(lpEditControl);
}



RANGE PASCAL editClearSelection(LPEditControl lpEditControl)
{
    POINT pointPosition=editGetPosition(lpEditControl);
    RANGE rangeSelection;

    rangeSelection.wStart=rangeSelection.wEnd=editPointToOffset(lpEditControl, pointPosition);
    editSetSelection(lpEditControl, rangeSelection);
    return(rangeSelection);
}



void PASCAL editInsertText(LPEditControl lpEditControl, LPCSTR lpcText)
{
    RANGE rangeSelection=editGetSelection(lpEditControl);
    POINT pointPosition;

    if (rangeSelection.wStart==rangeSelection.wEnd)
        rangeSelection=editClearSelection(lpEditControl);

    Edit_ReplaceSel(lpEditControl->hwnd, lpcText);
    pointPosition = editOffsetToPoint(lpEditControl, (WORD) (rangeSelection.wStart + lstrlen(lpcText)) );
    editSetPosition(lpEditControl, pointPosition);
    editClearSelection(lpEditControl);
    lpEditControl->bChanged=TRUE;
}



BOOL PASCAL editChanged(LPEditControl lpEditControl, WORD wChanged)
{
    BOOL bPrevChanged=lpEditControl->bChanged;

    switch (wChanged) {

    case IDYES:    lpEditControl->bChanged=TRUE;    break;
    case IDNO:     lpEditControl->bChanged=FALSE;   break;

    }

    return(bPrevChanged);
}



BOOL PASCAL editUndo(LPEditControl lpEditControl)
{
    BOOL bStatus=Edit_Undo(lpEditControl->hwnd);

    lpEditControl->pointPosition.x=lpEditControl->pointPosition.y=0;
    lpEditControl->rangeSelection.wStart=lpEditControl->rangeSelection.wEnd=0;
    editChanged(lpEditControl, IDNO);
    return(bStatus);
}



void PASCAL editFocus(LPEditControl lpEditControl)
{
    if (!lpEditControl->bFocus)
        NextDlgCtl(GetParent(lpEditControl->hwnd), lpEditControl->wEditControl);

}



int PASCAL editExtract(LPEditControl lpEditControl, LPSTR lpBuff, int nSize)
{
    if (nSize) {
        Edit_GetText(lpEditControl->hwnd, lpBuff, nSize);
        lpEditControl->lpBuff=lpBuff;
        lpEditControl->wPtr=0;
        return(0);
    } else if (lpEditControl->lpBuff) {
        int nCount=0;

        while (lpEditControl->lpBuff[lpEditControl->wPtr] &&
	       lpEditControl->lpBuff[lpEditControl->wPtr]!='\r') {
            *lpBuff++ =lpEditControl->lpBuff[lpEditControl->wPtr++];
            nCount++;
        }

        *lpBuff='\0';

        if (lpEditControl->lpBuff[lpEditControl->wPtr]=='\r')
            lpEditControl->wPtr++;

        if (lpEditControl->lpBuff[lpEditControl->wPtr]=='\r')
            lpEditControl->wPtr++;

        if (lpEditControl->lpBuff[lpEditControl->wPtr]=='\n')
            lpEditControl->wPtr++;

        return(nCount || lpEditControl->lpBuff[lpEditControl->wPtr] ? nCount : -1);
    } else
        return(-1);

}



static LPEditControl editFindData(HWND hwnd)
{
    LPEditControl lpEditControl;

    for (lpEditControl=lpEditStack;
         lpEditControl && lpEditControl->hwnd!=hwnd;
         lpEditControl=lpEditControl->lpEditPrevious);

    return(lpEditControl);
}



static void editSubclass_OnSetFocus(HWND hwnd, HWND hwndOldFocus)
{
    LPEditControl lpEditControl=editFindData(hwnd);

    lpEditControl->bFocus=TRUE;
    editResetSelection(lpEditControl);
    editResetPosition(lpEditControl);
    lpEditControl->bTab=FALSE;
    WFORWARD_WM_SETFOCUS(hwnd, hwndOldFocus, lpEditControl->lpfnEditOldProc);
}



static void editSubclass_OnKillFocus(HWND hwnd, HWND hwndNewFocus)
{
    LPEditControl lpEditControl=editFindData(hwnd);

    editGetPosition(lpEditControl);
    editGetSelection(lpEditControl);
    lpEditControl->bFocus=FALSE;
    WFORWARD_WM_KILLFOCUS(hwnd, hwndNewFocus, lpEditControl->lpfnEditOldProc);
}



static void editSubclass_OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    LPEditControl lpEditControl=editFindData(hwnd);

    if (lpEditControl->wFlags&EDIT_KEEP_TABS &&
        MapVirtualKey(LOBYTE(flags), 1)==VK_TAB &&
        GetKeyState(VK_CONTROL)>=0 && GetKeyState(VK_SHIFT)>=0) {
        int nSpaceCount;

        lpEditControl->bTab=TRUE;
        editGetPosition(lpEditControl);
        nSpaceCount=8-lpEditControl->pointPosition.x%8;

        if (nSpaceCount>0)
            WFORWARD_WM_CHAR(hwnd, VK_SPACE, nSpaceCount, lpEditControl->lpfnEditOldProc);

    } else
        WFORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, lpEditControl->lpfnEditOldProc);
}



static void editSubclass_OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    LPEditControl lpEditControl=editFindData(hwnd);

    if (MapVirtualKey(LOBYTE(flags), 1)==VK_TAB &&
        lpEditControl->bTab)
        lpEditControl->bTab=FALSE;
    else
        WFORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags, lpEditControl->lpfnEditOldProc);

}



static void editSubclass_OnDestroy(HWND hwnd)
{
    LPEditControl lpEditControl=editFindData(hwnd);

    editPostlude(lpEditControl);
    WFORWARD_WM_DESTROY(hwnd, lpEditControl->lpfnEditOldProc);
}



LONG _EXPORT CALLBACK editSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_SETFOCUS, editSubclass_OnSetFocus);
    HANDLE_MSG(hwnd, WM_KILLFOCUS, editSubclass_OnKillFocus);
    HANDLE_MSG(hwnd, WM_KEYDOWN, editSubclass_OnKeyDown);
    HANDLE_MSG(hwnd, WM_KEYUP, editSubclass_OnKeyUp);
    HANDLE_MSG(hwnd, WM_DESTROY, editSubclass_OnDestroy);

    default: {
        LPEditControl lpEditControl=editFindData(hwnd);

        return(WFORWARD_MESSAGE(hwnd, uMsg, wParam, lParam, lpEditControl->lpfnEditOldProc));
    }

    }
}

