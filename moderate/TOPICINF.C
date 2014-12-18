#include <windows.h>                    //  Windows API definitions
#include <windowsx.h>                   //  Windows API definitions
#include "ameolapi.h"                   //  Ameol API definitions
#include "winhorus.h"			//  Windows global memory allocation routines
#include "moderate.h"                   //  Resource definitions
#include "globals.h"                    //  Global variable and routine definitions
#include "confs.h"
#include "strftime.h"


void resetTopicInfo(LPTopicInfo lpTopicInfo)
{
    *lpTopicInfo->szConfName='\0';
    lpTopicInfo->topicHeader.fileVersion=CTI_REVISION;
    lpTopicInfo->topicHeader.lastUpdate=(time_t)0L;
    lpTopicInfo->topicHeader.updateSelect=CID_UPDATE_MANUAL;
    lpTopicInfo->topicHeader.updateDays=7;
    lpTopicInfo->topicHeader.updateFlags=TOPIC_UPDATESIZE|TOPIC_UPDATELIST|TOPIC_SIZEWARNING|TOPIC_GETNOTE;
    lpTopicInfo->topicHeader.topicWarn=DEFAULT_TOPIC_WARNING;
    lpTopicInfo->topicHeader.lastFlistUpdate=(time_t)0L;
    lpTopicInfo->topicHeader.flistSelect=CID_UPDATE_MANUAL;
    lpTopicInfo->topicHeader.flistDays=7;
    lpTopicInfo->currentTopic=NULL;
    lpTopicInfo->topicCount=0;
    lpTopicInfo->totalCount=0;
    lpTopicInfo->changed=FALSE;
}



void initTopicInfo(LPTopicInfo lpTopicInfo)
{
    int topicPtr;

    for (topicPtr=0; topicPtr<MAX_TOPICS; topicPtr++)
	lpTopicInfo->topics[topicPtr]=NULL;

    resetTopicInfo(lpTopicInfo);
}



void freeTopicInfo(LPTopicInfo lpTopicInfo)
{
    int topicPtr;

    for (topicPtr=0; topicPtr<MAX_TOPICS; topicPtr++)
        if (lpTopicInfo->topics[topicPtr])
		{
            gfree(lpTopicInfo->topics[topicPtr]);	lpTopicInfo->topics[topicPtr] = NULL;
		}

    initTopicInfo(lpTopicInfo);
}



void deleteSeedMessages(LPTopicInfo lpTopicInfo, BOOL bAll)
{
    int nTopicPtr;

    for (nTopicPtr=0; nTopicPtr<lpTopicInfo->totalCount; nTopicPtr++) {
        LPTopic lpTopic=lpTopicInfo->topics[nTopicPtr];

        if (lpTopic->hoobSeedMessage &&
	    (bAll || lpTopic->topicFlags&TOPIC_DELETE)) {
            RemoveObject(lpTopic->hoobSeedMessage);
            lpTopic->hoobSeedMessage=NULL;
        }
    }
}



void initTopic(LPTopic lpTopic, LPTopicHeader lpTopicHeader)
{
    lpTopic->topicIndex=LB_ERR;
    *lpTopic->topicName='\0';
    lpTopic->topicHandle=NULL;
    lpTopic->topicFlags=TOPIC_UPDATESIZE|(lpTopicHeader->updateFlags&TOPIC_SIZEWARNING);
    lpTopic->topicOldFlags=0;
    lpTopic->lastUpdate=(time_t)0L;
    lpTopic->updateSelect=lpTopicHeader->updateSelect ? lpTopicHeader->updateSelect : CID_UPDATE_MANUAL;
    lpTopic->updateDays=lpTopicHeader->updateDays ? lpTopicHeader->updateDays : 7;
    lpTopic->topicSize=0L;
    lpTopic->topicMax=2047000L;
    lpTopic->topicWarn=lpTopicHeader->topicWarn ? lpTopicHeader->topicWarn : DEFAULT_TOPIC_WARNING;
    lpTopic->lastFlistUpdate=(time_t)0L;
    lpTopic->flistSelect=lpTopicHeader->flistSelect ? lpTopicHeader->flistSelect : CID_UPDATE_MANUAL;
    lpTopic->flistDays=lpTopicHeader->flistDays ? lpTopicHeader->flistDays : 7;
    *lpTopic->topicDescription='\0';
    lpTopic->hoobSeedMessage=NULL;
}



LPTopic allocateTopic(LPTopicInfo lpTopicInfo, LPTopic lpTopicAssign)
{
    LPTopic lpTopic;

    if (!lpTopicInfo->topics[lpTopicInfo->totalCount])
        lpTopicInfo->topics[lpTopicInfo->totalCount]=(LPTopic)gmalloc(sizeof(Topic));

    lpTopic=lpTopicInfo->topics[lpTopicInfo->totalCount];

    if (!lpTopic)
        alert(NULL, "gmalloc() failed in allocateTopic()");
    else {
        if (lpTopicAssign)
            *lpTopic= *lpTopicAssign;
        else
            initTopic(lpTopic, &lpTopicInfo->topicHeader);

        lpTopic->topicIndex=lpTopicInfo->totalCount++;
        lpTopicInfo->topicCount++;
    }

    return(lpTopic);
}



void removeTopic(LPTopicInfo lpTopicInfo, LPTopic lpTopic)
{
    if (lpTopic && lpTopic->topicIndex!=LB_ERR) {
        int nTopic;

        if (lpTopic==lpTopicInfo->currentTopic)
            lpTopicInfo->currentTopic=NULL;

        if (!(lpTopic->topicFlags&TOPIC_DELETE))
            lpTopicInfo->topicCount--;

        for (nTopic=lpTopic->topicIndex; nTopic<lpTopicInfo->totalCount-1; nTopic++) {
            lpTopicInfo->topics[nTopic]=lpTopicInfo->topics[nTopic+1];
            lpTopicInfo->topics[nTopic]->topicIndex--;
        }

        lpTopicInfo->topics[--lpTopicInfo->totalCount]=lpTopic;
        initTopic(lpTopic, &lpTopicInfo->topicHeader);
    }
}



void saveTopic(HWND hwnd, LPTopic lpTopic)

//  Save topic info if required.

{
    if (lpTopic) {
        if (IsDlgButtonChecked(hwnd, CID_TOPIC_FLIST))
	    lpTopic->topicFlags|=TOPIC_FILES;

        if (IsDlgButtonChecked(hwnd, CID_TOPIC_READONLY))
            lpTopic->topicFlags|=TOPIC_READONLY;

        GetDlgItemText(hwnd, CID_TOPIC_DESCRIPTION, lpTopic->topicDescription, DESC_LENGTH);
    }
}



LPTopic findTopic(LPTopicInfo lpTopicInfo, LPSTR lpTopicName, int nTopicStart)
{
    int nTopicPtr;
    BOOL bFound=FALSE;

    if (lpTopicInfo->totalCount)
        if (nTopicStart<0) {
            for (nTopicPtr=0;
                 nTopicPtr<lpTopicInfo->totalCount &&
                 lstrcmp(lpTopicName, lpTopicInfo->topics[nTopicPtr]->topicName);
                 nTopicPtr++);

            bFound=nTopicPtr<lpTopicInfo->totalCount;
        } else {
            LPSTR lpCheck;

            nTopicPtr=nTopicStart;

	    do {
                LPSTR lpAgainst;

                lpCheck=lpTopicName;
                lpAgainst=lpTopicInfo->topics[nTopicPtr]->topicName;

                while (*lpCheck && *lpCheck== *lpAgainst) {
                    lpCheck++;
                    lpAgainst++;
                }

                if (*lpCheck && ++nTopicPtr>=lpTopicInfo->totalCount)
                    nTopicPtr=0;

            } while (*lpCheck && nTopicPtr!=nTopicStart);

            bFound= !(*lpCheck);
        }

    return(bFound ? lpTopicInfo->topics[nTopicPtr] : NULL);
}



void displayTopic(HWND hwnd, LPTopic lpTopic, BOOL bSetName)
{
    HWND hwndTopic=GetDlgItem(hwnd, CID_TOPIC);

    if (lpTopic) {
	int topicIndex=ListBox_GetCurSel(hwndTopic);

        if (topicIndex!=lpTopic->topicIndex)
            ListBox_SetCurSel(hwndTopic, lpTopic->topicIndex);

        if (bSetName)
            SetDlgItemText(hwnd, CID_TOPIC_NAME, lpTopic->topicName);

        SetDlgItemText(hwnd, CID_TOPIC_DESCRIPTION, lpTopic->topicDescription);
        CheckDlgButton(hwnd, CID_TOPIC_FLIST, lpTopic->topicFlags&TOPIC_FILES);
        CheckDlgButton(hwnd, CID_TOPIC_READONLY, lpTopic->topicFlags&TOPIC_READONLY);

        if (GetDlgItem(hwnd, CID_TOPIC_SIZE)) {
            char szBuff[64];

            if (lpTopic->lastUpdate > 0)
                strftime(szBuff, 63, "Topic size (updated: %A, %d %B %Y at %I:%M %p)", localtime(&lpTopic->lastUpdate));
            else
                lstrcpy(szBuff, "Topic size (never updated)");

            SetDlgItemText(hwnd, CID_TOPIC_UPDATED, szBuff);
            formatLong(szBuff, "", lpTopic->topicSize, " bytes");
            SetDlgItemText(hwnd, CID_TOPIC_SIZE, szBuff);
            formatLong(szBuff, "", lpTopic->topicMax, " bytes");
            SetDlgItemText(hwnd, CID_TOPIC_MAX, szBuff);
            formatLong(szBuff, "", lpTopic->topicMax-lpTopic->topicSize, " bytes");
            SetDlgItemText(hwnd, CID_TOPIC_FREE, szBuff);
            SetDlgItemInt(hwnd, CID_TOPIC_PERCENT_FULL, (UINT)((lpTopic->topicSize*100L)/lpTopic->topicMax), FALSE);
        }
    } else {
        ListBox_SetCurSel(hwndTopic, LB_ERR);

        if (bSetName)
            SetDlgItemText(hwnd, CID_TOPIC_NAME, "");

        SetDlgItemText(hwnd, CID_TOPIC_DESCRIPTION, "");
        CheckDlgButton(hwnd, CID_TOPIC_FLIST, FALSE);
        CheckDlgButton(hwnd, CID_TOPIC_READONLY, FALSE);

        if (GetDlgItem(hwnd, CID_TOPIC_SIZE)) {
            SetDlgItemText(hwnd, CID_TOPIC_UPDATED, "Topic size");
            SetDlgItemText(hwnd, CID_TOPIC_SIZE, "");
            SetDlgItemText(hwnd, CID_TOPIC_MAX, "");
            SetDlgItemText(hwnd, CID_TOPIC_FREE, "");
            SetDlgItemText(hwnd, CID_TOPIC_PERCENT_FULL, "");
        }
    }
}



LPTopic selectTopic(HWND hwnd, LPTopicInfo lpTopicInfo, int topicIndex, BOOL bSetName)

//  Select a topic from the list box and set the controls to suit.
//  Return a pointer to the selected topic (or NULL).

{
    saveTopic(hwnd, lpTopicInfo->currentTopic);

    lpTopicInfo->currentTopic=topicIndex<0 || topicIndex>=lpTopicInfo->totalCount ?
                              NULL :
                              lpTopicInfo->topics[topicIndex];

    displayTopic(hwnd, lpTopicInfo->currentTopic, bSetName);
    return(lpTopicInfo->currentTopic);
}

