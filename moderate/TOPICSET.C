#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include "ameolapi.h"                   //  Ameol API definitions
#include "winhorus.h"			//  Windows utility routines
#include "hctools.h"
#include "moderate.h"                   //  Resource definitions
#include "help.h"                       //  Balloon help routine definition
#include "setup.h"
#include "globals.h"                    //  Global variable and routine definitions



typedef struct tagTopicEntry {
    HTOPIC hTopic;
    LPCSTR lpcTopicName;
    BOOL bTopicFlist;
    struct tagTopicEntry FAR *lpNextEntry;
} TopicEntry, FAR *LPTopicEntry;



typedef struct {
    LPContext lpContext;
    Context saveContext;
    LPTopicEntry lpTopicEntryList;
} TopicData, FAR *LPTopicData;



static void drawConfName(LPDRAWITEMSTRUCT lpDis)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rc;
    HCONF hConf=(HCONF)lpDis->itemData;
    LPCSTR confName=hConf ? GetConferenceName(hConf) : "";

    CopyRect(&rc, &lpDis->rcItem);
    InflateRect(&rc, -2, 0);

    if (lpDis->itemState&ODS_FOCUS || lpDis->itemState&ODS_SELECTED) {
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
    DrawText(lpDis->hDC, confName, -1, &rc, DT_LEFT|DT_SINGLELINE);
    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



static void drawTopicName(LPDRAWITEMSTRUCT lpDis)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rc1, rc2;
    LPTopicEntry lpTopicEntry=(LPTopicEntry)lpDis->itemData;

    CopyRect(&rc1, &lpDis->rcItem);    rc1.left+=2;             rc1.right-=22;
    CopyRect(&rc2, &lpDis->rcItem);    rc2.left=rc2.right-20;   rc2.right-=2;

    if (lpDis->itemState&ODS_FOCUS || lpDis->itemState&ODS_SELECTED) {
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

    if (lpTopicEntry) {
	DrawText(lpDis->hDC, lpTopicEntry->lpcTopicName, -1, &rc1, DT_LEFT|DT_SINGLELINE);

	if (lpTopicEntry->bTopicFlist)
	    DrawText(lpDis->hDC, "(F)", -1, &rc2, DT_LEFT|DT_SINGLELINE);

    }

    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



void adjustTopic(HWND hwnd, LPTopicData lpTopicData)
{
    HTOPIC hTopic;
    HWND hTopicList=GetDlgItem(hwnd, CID_TOPIC);
    LPContext lpContext=lpTopicData->lpContext;
    LPTopicEntry lpTopicEntry=lpTopicData->lpTopicEntryList;
    LPTopicEntry lpNextTopicEntry=lpTopicEntry;

    ComboBox_ResetContent(hTopicList);

    if (lpContext->hConference==NULL) {
	lpContext->hTopic=NULL;
	lpContext->lpcTopicName=NULL;
    } else {
	int topicCount=0;
	int topicSelect= -1;

	for (hTopic=GetTopic(lpContext->hConference, NULL); hTopic; hTopic=GetTopic(NULL, hTopic)) {
	    TOPICINFO tInfo;

	    GetTopicInfo(hTopic, &tInfo);

	    if (!(tInfo.wFlags&TF_LOCAL)) {
		if (!lpNextTopicEntry) {
		    lpNextTopicEntry=(LPTopicEntry)gmalloc(sizeof(TopicEntry));
		    assert(lpNextTopicEntry != NULL );
		    lpNextTopicEntry->lpNextEntry=NULL;
		    lpTopicEntry->lpNextEntry=lpNextTopicEntry;
		}

		lpTopicEntry=lpNextTopicEntry;
		lpNextTopicEntry=lpTopicEntry->lpNextEntry;
		lpTopicEntry->hTopic=hTopic;
		lpTopicEntry->lpcTopicName=GetTopicName(hTopic);
		assert(lpTopicEntry->lpcTopicName != NULL);
		lpTopicEntry->bTopicFlist=tInfo.wFlags&TF_HASFILES;
		ComboBox_AddItemData(hTopicList, lpTopicEntry);

		if (hTopic==lpContext->hTopic)
		    topicSelect=topicCount;

		topicCount++;
	    }
	}

	if (topicSelect>=0) {
	    ComboBox_SetCurSel(hTopicList, topicSelect);
	} else if (topicCount>0) {
	    lpTopicEntry=(LPTopicEntry)ComboBox_GetItemData(hTopicList, 0);
	    assert(lpTopicEntry != NULL);
	    if (lpTopicEntry && lpTopicEntry->hTopic) {
		lpContext->hTopic=lpTopicEntry->hTopic;
		lpContext->lpcTopicName=lpTopicEntry->lpcTopicName;;
	    } else {
		MessageBeep(0);
		lpContext->hTopic=NULL;
		lpContext->lpcTopicName=NULL;
	    }

	    ComboBox_SetCurSel(hTopicList, 0);
	} else {
	    lpContext->hTopic=NULL;
	    lpContext->lpcTopicName=NULL;
	}
    }
}



static BOOL topic_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPTopicData lpTopicData=InitDlgData(hwnd, TopicData);
    HCONF hConf;
    HWND hConfList=GetDlgItem(hwnd, CID_CONFERENCE);
    int confCount=0;
    int confSelect= -1;

    lpTopicData->lpContext=(LPContext)lParam;
    lpTopicData->saveContext= *lpTopicData->lpContext;
    lpTopicData->lpTopicEntryList=(LPTopicEntry)gmalloc(sizeof(TopicEntry));
    lpTopicData->lpTopicEntryList->lpNextEntry=NULL;

    if (!lpTopicData->lpContext->hConference) {
	CURMSG currentMessage;

	if (GetCurrentMsg(&currentMessage)) {
	    lpTopicData->lpContext->hConference=currentMessage.pcl;
	    lpTopicData->lpContext->lpcConfName=GetConferenceName(lpTopicData->lpContext->hConference);
	    lpTopicData->lpContext->hTopic=currentMessage.ptl;
	    lpTopicData->lpContext->lpcTopicName=GetTopicName(lpTopicData->lpContext->hTopic);
	}
    }

    for (hConf=GetConference(NULL); hConf; hConf=GetConference(hConf)) {
	LPCSTR confName=GetConferenceName(hConf);

	if (lstrcmp(confName, "mail")!=0 &&
	    lstrcmp(confName, "Usenet")!=0 &&
	    lstrcmp(confName, "UseNet")!=0 &&
	    IsModerator(hConf, NULL)) {
	    ComboBox_AddItemData(hConfList, hConf);

	    if (hConf==lpTopicData->lpContext->hConference)
		confSelect=confCount;

	    confCount++;
	}
    }

    if (confSelect>=0) {
	ComboBox_SetCurSel(hConfList, confSelect);
    } else if (confCount>0) {
	lpTopicData->lpContext->hConference=(HCONF)ComboBox_GetItemData(hConfList, 0);
	lpTopicData->lpContext->lpcConfName=GetConferenceName(lpTopicData->lpContext->hConference);
	ComboBox_SetCurSel(hConfList, 0);
    } else {
	lpTopicData->lpContext->hConference=NULL;
	lpTopicData->lpContext->lpcConfName=NULL;
    }

    adjustTopic(hwnd, lpTopicData);
    return(TRUE);
}



static void topic_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void topic_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
	if (lpDrawItem->CtlID==CID_CONFERENCE)
	    drawConfName((LPDRAWITEMSTRUCT)lpDrawItem);
	else
	    drawTopicName((LPDRAWITEMSTRUCT)lpDrawItem);

}



static LPCSTR topic_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_SELECT_TOPIC, id));
}



static void topic_OnAmHelp(HWND hwnd)
{
    LPTopicData lpTopicData=GetDlgData(hwnd, TopicData);

    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT,
	    lpTopicData->lpContext->bInEditor ? HID_FLIST_CHANGE_TOPIC :
						HID_FLIST_CHOOSE_TOPIC);

}



static void topic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPTopicData lpTopicData=GetDlgData(hwnd, TopicData);

    switch (id) {

    case CID_CONFERENCE:
	if (codeNotify==CBN_SELCHANGE) {
	    HCONF hOldConference=lpTopicData->lpContext->hConference;
	    int iConf=ComboBox_GetCurSel(hwndCtl);
	    assert( iConf != -1 );

	    lpTopicData->lpContext->hConference=(HCONF)ComboBox_GetItemData(hwndCtl, iConf);
	    assert(lpTopicData->lpContext->hConference != NULL );

	    if (lpTopicData->lpContext->hConference!=hOldConference)
		if (!lpTopicData->lpContext->hConference) {
		    MessageBeep(0);
		    lpTopicData->lpContext->lpcConfName=NULL;
		} else {
		    lpTopicData->lpContext->lpcConfName=GetConferenceName(lpTopicData->lpContext->hConference);
		    adjustTopic(hwnd, lpTopicData);
	    }
	}

	break;

    case CID_TOPIC:
	if (codeNotify==CBN_SELCHANGE) {
	    int iTopic=ComboBox_GetCurSel(hwndCtl);
	    LPTopicEntry lpTopicEntry=(LPTopicEntry)ComboBox_GetItemData(hwndCtl, iTopic);

	    if (lpTopicEntry && lpTopicEntry->hTopic) {
		lpTopicData->lpContext->hTopic=lpTopicEntry->hTopic;
		lpTopicData->lpContext->lpcTopicName=lpTopicEntry->lpcTopicName;;
	    } else {
		MessageBeep(0);
		lpTopicData->lpContext->hTopic=NULL;
		lpTopicData->lpContext->lpcTopicName=NULL;
	    }
	}

	break;

    case CID_OK:
	EndDialog(hwnd, TRUE);
	break;

    case CID_CANCEL:
	*lpTopicData->lpContext=lpTopicData->saveContext;
	EndDialog(hwnd, FALSE);
	break;

    }
}



static void topic_OnDestroy(HWND hwnd)
{
    LPTopicData lpTopicData=GetDlgData(hwnd, TopicData);
    LPTopicEntry lpTopicEntry=lpTopicData->lpTopicEntryList;

    while (lpTopicEntry) 
	{
		LPTopicEntry lpNextTopicEntry=lpTopicEntry->lpNextEntry;

		gfree(lpTopicEntry);	lpTopicEntry = NULL;
		lpTopicEntry=lpNextTopicEntry;
    }

    FreeDlgData(hwnd);
}



static LRESULT topicProc_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, topic_OnInitDialog);
    HANDLE_MSG(hwnd, WM_MEASUREITEM, topic_OnMeasureItem);
    HANDLE_MSG(hwnd, WM_DRAWITEM, topic_OnDrawItem);
    HANDLE_MSG(hwnd, WM_POPUPHELP, topic_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, topic_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, topic_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, topic_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK topicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=topicProc_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}
