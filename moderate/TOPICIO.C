#include <windows.h>                    //  Windows API definitions
#include <windowsx.h>                   //  Windows API definitions
#include <stdio.h>                      //  For sscanf()
#include <stdlib.h>                     //  For atol()
#include <time.h>
#include <string.h>                     //  For strtok()
#include <ctype.h>                      //  Character classification
#include "ameolapi.h"                   //  Ameol API definitions
#include "winhorus.h"			//  Windows global memory and serial io routines
#include "hctools.h"                    //  Ameol script routines
#include "setup.h"                      //  Setup (ameol.ini) routines and variables
#include "moderate.h"                   //  Resource definitions
#include "help.h"
#include "globals.h"                    //  Global variable and routine definitions
#include "flist.h"
#include "confs.h"
#include "msgedit.h"

UINT FAR PASCAL _EXPORT alertLevel(UINT newLevel);

static int importSelect(int updateSelect)
{
    switch (updateSelect) {

    case 0:  return(CID_UPDATE_MANUAL);
    case 1:  return(CID_UPDATE_PERIODIC);
    case 2:  return(CID_UPDATE_ALWAYS);
    case -1: return(0);

    default: return(CID_UPDATE_MANUAL);

    }
}



static int exportSelect(int updateSelect)
{
    switch (updateSelect) {

    case CID_UPDATE_MANUAL:    return(0);
    case CID_UPDATE_PERIODIC:  return(1);
    case CID_UPDATE_ALWAYS:    return(2);
    case 0:						    return(-1);

    default:                   return(0);

    }
}



static int checkDays(int updateDays)
{
    return(updateDays<=0 || 100<updateDays ? 7 : updateDays);
}



#define headerToken(lpBuffer, lpToken)      \
    lpToken=_fstrtok(lpBuffer, " ,");       \
                                            \
    if (!lpToken)                           \
	return(FALSE);




BOOL readTopicHeader(HSTREAM hStream, LPTopicHeader lpTopicHeader)
{
    char buff[256];

    lpTopicHeader->updateFlags=0;
    readSeek(hStream, 0L);

    if (readLine(hStream, buff, 256)>0) {
	LPSTR lpToken;

	headerToken(buff, lpToken);    lpTopicHeader->fileVersion=atoi(lpToken);
	headerToken(NULL, lpToken);    lpTopicHeader->lastUpdate=(time_t)atol(lpToken);
	headerToken(NULL, lpToken);    lpTopicHeader->updateSelect=importSelect(atoi(lpToken));
	headerToken(NULL, lpToken);    lpTopicHeader->updateDays=checkDays(atoi(lpToken));
	headerToken(NULL, lpToken);    lpTopicHeader->updateFlags=atoi(lpToken);

	if (lpTopicHeader->fileVersion>1) {
	    headerToken(NULL, lpToken);	   lpTopicHeader->topicWarn=atol(lpToken);

	    if (lpTopicHeader->fileVersion>2) {
		headerToken(NULL, lpToken);    lpTopicHeader->lastFlistUpdate=(time_t)atol(lpToken);
		headerToken(NULL, lpToken);    lpTopicHeader->flistSelect=importSelect(atoi(lpToken));
		headerToken(NULL, lpToken);    lpTopicHeader->flistDays=checkDays(atoi(lpToken));
	    } else {
		lpTopicHeader->lastFlistUpdate=(time_t)0;
		lpTopicHeader->flistSelect=CID_UPDATE_MANUAL;
		lpTopicHeader->flistDays=7;
	    }
	} else {
	    lpTopicHeader->topicWarn=DEFAULT_TOPIC_WARNING;
	    lpTopicHeader->lastFlistUpdate=(time_t)0;
	    lpTopicHeader->flistSelect=CID_UPDATE_MANUAL;
	    lpTopicHeader->flistDays=7;
	}

	return(TRUE);
    }

    return(FALSE);
}



void writeTopicHeader(HSTREAM hStream, LPTopicHeader lpTopicHeader)
{
    writeSeek(hStream, 0L);

    writeLine(hStream, "%d %ld %d %d %d %ld %ld %d %d\n",
	      CTI_REVISION,
	      (long)lpTopicHeader->lastUpdate,
	      exportSelect(lpTopicHeader->updateSelect),
	      (int)lpTopicHeader->updateDays,
	      (int)lpTopicHeader->updateFlags,
	      lpTopicHeader->topicWarn,
	      (long)lpTopicHeader->lastFlistUpdate,
	      exportSelect(lpTopicHeader->flistSelect),
	      (int)lpTopicHeader->flistDays);

}



static void buildTopicInfo(HCONF hConf, LPSTR lpFileInfo, LPTopicInfo lpTopicInfo)
{
    HSTREAM hStreamInfo=openFile(lpFileInfo, OF_WRITE);

    resetTopicInfo(lpTopicInfo);

    if (hStreamInfo) {
	HTOPIC hTopic;

	writeTopicHeader(hStreamInfo, &lpTopicInfo->topicHeader);
	lstrcpy(lpTopicInfo->szConfName, GetConferenceName(hConf));

	for (hTopic=GetTopic(hConf, NULL); hTopic; hTopic=GetTopic(hConf, hTopic)) {
	    TOPICINFO ameolInfo;

	    GetTopicInfo(hTopic, &ameolInfo);

	    if (!(ameolInfo.wFlags&TF_LOCAL))
		writeLine(hStreamInfo, "%s 0 0 7 %d 0 2047000 %ld 0 0 7 \n",
			  GetTopicName(hTopic),
			  TOPIC_UPDATESIZE,
			  DEFAULT_TOPIC_WARNING);

	}

	closeFile(hStreamInfo);
    } else
	alert(NULL, "Can't open topic information file %s for write", lpFileInfo);

}



#define topicToken(lpBuffer, lpToken)       \
    lpToken=_fstrtok(lpBuffer, " ,");       \
					    \
    if (!lpToken) {                         \
	ok=FALSE;                           \
	break;                              \
    }



BOOL loadTopicInfo(HWND hwnd, HCONF hConf, LPTopicInfo lpTopicInfo)
{
    CONFERENCEINFO confInfo;
    LPCSTR lpcConfName;
    HSTREAM hStreamInfo;
    char szFileInfo[LEN_PATHNAME];
    BOOL ok=FALSE;

    if (hConf) {
	GetConferenceInfo(hConf, &confInfo);
	lpcConfName=GetConferenceName(hConf);
	wsprintf(szFileInfo, "%s\\%s.CTI", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
	hStreamInfo=openFile(szFileInfo, OF_READ);

	if (!hStreamInfo) {
	    inform(hwnd, "Topic information file %s for %s doesn't exist - "
			 "building from messagebase - information will be "
			 "loaded from CIX on your next blink",
		   (LPSTR)szFileInfo, lpcConfName);

	    buildTopicInfo(hConf, szFileInfo, lpTopicInfo);
	    hStreamInfo=openFile(szFileInfo, OF_READ);
	}

	if (hStreamInfo) {
	    int nRetry=0;

	    do {
		char buff[256];
		TopicHeader topicHeader;

		nRetry++;

		if (readTopicHeader(hStreamInfo, &topicHeader))
		    if (topicHeader.fileVersion>CTI_REVISION) {
			alert(hwnd, "Topic information file %s for forum %s was written by a more "
				    "recent version of Moderate - file version is %d",
			      (LPSTR)szFileInfo, lpcConfName, topicHeader.fileVersion);

			nRetry=2;
		    } else {
			resetTopicInfo(lpTopicInfo);
			lstrcpy(lpTopicInfo->szConfName, lpcConfName);
			lpTopicInfo->topicHeader=topicHeader;
			ok=TRUE;

			while (readLine(hStreamInfo, buff, 256)>0) 
			{
                LPTopic lpTopic=allocateTopic(lpTopicInfo, NULL);
                LPSTR lpToken;

                topicToken(buff, lpToken);    lstrcpy(lpTopic->topicName, lpToken);
                topicToken(NULL, lpToken);    lpTopic->lastUpdate=(time_t)atol(lpToken);
                topicToken(NULL, lpToken);    lpTopic->updateSelect=importSelect(atoi(lpToken));
                topicToken(NULL, lpToken);    lpTopic->updateDays=checkDays(atoi(lpToken));
                topicToken(NULL, lpToken);    lpTopic->topicFlags=atoi(lpToken)&TOPIC_SAVEMASK;
                topicToken(NULL, lpToken);    lpTopic->topicSize=atol(lpToken);
				topicToken(NULL, lpToken);    lpTopic->topicMax=atol(lpToken);

				if (topicHeader.fileVersion>1) 
				{
					topicToken(NULL, lpToken);    lpTopic->topicWarn=atol(lpToken);

					if (topicHeader.fileVersion>2) 
					{
						topicToken(NULL, lpToken);    lpTopic->lastFlistUpdate=(time_t)atol(lpToken);
						topicToken(NULL, lpToken);    lpTopic->flistSelect=importSelect(atoi(lpToken));
						topicToken(NULL, lpToken);    lpTopic->flistDays=checkDays(atoi(lpToken));
					} 
					else 
					{
						lpTopic->lastFlistUpdate=topicHeader.lastFlistUpdate;
						lpTopic->flistSelect=topicHeader.flistSelect;
						lpTopic->flistDays=topicHeader.flistDays;
					}
			    } 
				else 
				{
				lpTopic->topicWarn=topicHeader.topicWarn;
				lpTopic->lastFlistUpdate=topicHeader.lastFlistUpdate;
				lpTopic->flistSelect=topicHeader.flistSelect;
				lpTopic->flistDays=topicHeader.flistDays;
			    }

			    lpToken=_fstrtok(NULL, "");

			    if (lpToken)
				lstrcpy(lpTopic->topicDescription, lpToken);
			    else
				*lpTopic->topicDescription='\0';

                        }
                    }

                closeFile(hStreamInfo);

                if (!ok && nRetry<2) {
                    inform(hwnd, "Topic information file %s for forum %s is corrupt - "
                                 "rebuilding from messagebase - information will be reloaded from CIX",
			   (LPSTR)szFileInfo, lpcConfName);

		    buildTopicInfo(hConf, szFileInfo, lpTopicInfo);
		    hStreamInfo=openFile(szFileInfo, OF_READ);
		}
	    } while (!ok && hStreamInfo && nRetry<2);
	}
    }

    if (ok) {
	int topicPtr;

        for (topicPtr=0; topicPtr<lpTopicInfo->totalCount; topicPtr++) {
	    LPTopic lpTopic=lpTopicInfo->topics[topicPtr];

	    lpTopic->topicHandle=OpenTopic(hConf, lpTopic->topicName);

	    if (!lpTopic->topicHandle && setup.bCreateMissingTopics) {
		lpTopic->topicHandle=CreateTopic(hConf, lpTopic->topicName);
		SetTopicFlags(lpTopic->topicHandle, TF_RESIGNED, TRUE);

		alert(hwnd, "Topic %s/%s did not exist offline and has been created - "
			    "resigned, read only and flist flags may be incorrect",
			    lpcConfName, lpTopic->topicName);

	    }

	    if (lpTopic->topicHandle) {
		TOPICINFO ameolInfo;

		GetTopicInfo(lpTopic->topicHandle, &ameolInfo);

		if (ameolInfo.wFlags&TF_HASFILES)
		    lpTopic->topicFlags|=TOPIC_FILES;

		if (ameolInfo.wFlags&TF_READONLY)
		    lpTopic->topicFlags|=TOPIC_READONLY;

	    }

            lpTopic->topicOldFlags=lpTopic->topicFlags;
        }
    } else
        resetTopicInfo(lpTopicInfo);

    return(ok);
}



BOOL getTopicInfo(HWND hwnd, HCONF hConf, LPTopicInfo lpTopicInfo)
{
    HWND hwndTopic=GetDlgItem(hwnd, CID_TOPIC);
    BOOL ok=loadTopicInfo(hwnd, hConf, lpTopicInfo);

    ListBox_ResetContent(hwndTopic);

    if (ok) {
	int nTopicIndex;

        for (nTopicIndex=0; nTopicIndex<lpTopicInfo->totalCount; nTopicIndex++)
            ListBox_AddItemData(hwndTopic, lpTopicInfo->topics[nTopicIndex]);

    }

    return(ok);
}



void editSeedMessage(HWND hwnd, LPTopicInfo lpTopicInfo)
{
    LPTopic lpTopic=lpTopicInfo->currentTopic;
    LPSTR lpszText=(LPSTR)gmalloc(2048);
    MessageEditData messageEditData;
    FARPROC lpEditProc=MakeProcInstance((FARPROC)messageEditProc, lpGlobals->hInst);

    messageEditData.lpcConfName=lpTopicInfo->szConfName;
    messageEditData.lpcTopicName=lpTopic->topicName;
    messageEditData.hoobMessage=lpTopic->hoobSeedMessage;
    lstrcpy(messageEditData.szTitle, lpTopic->topicDescription);
    messageEditData.lpText=lpszText;
    messageEditData.nTextLength=2048;
    messageEditData.bImportEnabled=FALSE;
    messageEditData.wHelpTag=HID_EDIT_SEED;
	messageEditData.fHasRect=FALSE;

    if (StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_MESSAGE_EDIT", 
						  (HWND)hwnd, (DLGPROC)lpEditProc, (LPARAM)(DWORD)(LPMessageEditData)&messageEditData))
	lpTopic->hoobSeedMessage=messageEditData.hoobMessage;

    gfree(lpszText);	lpszText = NULL;
}



void writeSeedMessage(HWND hwnd, LPTopicInfo lpTopicInfo, BOOL bEdit)
{
    LPTopic lpTopic=lpTopicInfo->currentTopic;
    SAYOBJECT sayObject;
    LPSTR lpszText=(LPSTR)gmalloc(2048);
    LPSTR lpPtr=lpszText;

    InitObject(&sayObject, OT_SAY, SAYOBJECT);
    lstrcpy(sayObject.szConfName, lpTopicInfo->szConfName);
    lstrcpy(sayObject.szTopicName, lpTopic->topicName);
    lstrcpy(sayObject.szTitle, lpTopic->topicDescription);

    lpPtr+=wsprintf(lpszText, "\r\nWelcome to %s/%s\r\n",
                    lpTopicInfo->szConfName, lpTopic->topicName);

    if (lpTopic->topicFlags&TOPIC_FILES)
        lpPtr+=wsprintf(lpPtr, "This topic has a file list\r\n");

    if (lpTopic->topicFlags&TOPIC_READONLY)
        wsprintf(lpPtr, "This topic is read-only\r\n");

    lpTopic->hoobSeedMessage=PutObject(NULL, &sayObject, lpszText);

    if (bEdit) {
        MessageEditData messageEditData;
        FARPROC lpEditProc=MakeProcInstance((FARPROC)messageEditProc, lpGlobals->hInst);

        messageEditData.lpcConfName=lpTopicInfo->szConfName;
        messageEditData.lpcTopicName=lpTopic->topicName;
        messageEditData.hoobMessage=lpTopic->hoobSeedMessage;
        lstrcpy(messageEditData.szTitle, lpTopic->topicDescription);
		messageEditData.lpText=lpszText;
        messageEditData.nTextLength=2048;
        messageEditData.bImportEnabled=FALSE;
		messageEditData.fHasRect=FALSE;

	if (StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_MESSAGE_EDIT", 
						  (HWND)hwnd, (DLGPROC)lpEditProc, (LPARAM)(DWORD)(LPMessageEditData)&messageEditData))
            lpTopic->hoobSeedMessage=messageEditData.hoobMessage;

    }

    gfree(lpszText);	lpszText = NULL;
}



BOOL saveTopicInfo(HWND hwnd, HCONF hConf, LPTopicInfo lpTopicInfo)

//  Create/update topic information file and write script

{
    CONFERENCEINFO confInfo;
    LPCSTR lpcConfName;
    HSTREAM hStreamInfo;
    char szFileInfo[LEN_PATHNAME];
    int topicPtr;

	alertLevel(0);

    if (!lpTopicInfo->changed)
        return(TRUE);

    GetConferenceInfo(hConf, &confInfo);
    lpcConfName=GetConferenceName(hConf);
    wsprintf(szFileInfo, "%s\\%s.CTI", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
    hStreamInfo=openFile(szFileInfo, OF_WRITE);

    if (!hStreamInfo)
        return(FALSE);

    busy(TRUE);

    writeTopicHeader(hStreamInfo, &lpTopicInfo->topicHeader);

    for (topicPtr=0; topicPtr<lpTopicInfo->totalCount; topicPtr++) 
	{
        //  for each topic...
        LPTopic lpTopic=lpTopicInfo->topics[topicPtr];

        if (lpTopic->topicFlags&TOPIC_DELETE)
		{
            if (lpTopic->topicFlags&(TOPIC_NEW|TOPIC_NEWCONF)) 
			{
                if (lpTopic->hoobSeedMessage) 
				{
                    RemoveObject(lpTopic->hoobSeedMessage);
                    lpTopic->hoobSeedMessage=NULL;
				}
            } 
			else 
			{
				HSCRIPT hScript=initScript("Moderate", FALSE);

                addToScript(hScript, "put `mod %s`¬"
				     "if waitfor(`Mod:`, `M:`) == 0¬"
					 "  put `remove topic %s`¬"
					 "  waitfor `Mod:`¬"
					 "  put `quit`¬"
					 "  waitfor `M:`¬"
				     "endif¬",
			    lpcConfName, lpTopic->topicName);

				actionScript(hScript, OT_PREINCLUDE, "delete topic %s/%s",
						lpcConfName, lpTopic->topicName);

				lpTopicInfo->topicHeader.updateFlags|=TOPIC_UPDATELIST;
			}
		}
		else 
		{
	    //  save topic information
			writeLine(hStreamInfo, "%s %ld %d %d %d %ld %ld %ld %ld %d %d %s\n",
				  lpTopic->topicName,
				  (long)lpTopic->lastUpdate,
				  exportSelect(lpTopic->updateSelect),
				  (int)lpTopic->updateDays,
				  (int)(lpTopic->topicFlags&TOPIC_SAVEMASK),
				  lpTopic->topicSize,
				  lpTopic->topicMax,
				  lpTopic->topicWarn,
				  (long)lpTopic->lastFlistUpdate,
				  exportSelect(lpTopic->flistSelect),
				  (int)lpTopic->flistDays,
				  lpTopic->topicDescription);

		    if (!lpTopic->topicHandle && !(lpTopic->topicHandle=OpenTopic(hConf, lpTopic->topicName)) &&
				(lpTopic->topicFlags&(TOPIC_NEWCONF|TOPIC_NEW) || setup.bCreateMissingTopics))
			{
				if (lpTopic->topicHandle=CreateTopic(hConf, lpTopic->topicName))
				{
					SetTopicFlags(lpTopic->topicHandle, TF_RESIGNED,
						!(lpTopic->topicFlags&(TOPIC_NEWCONF|TOPIC_NEW)));
				}
				else
				{
					alert(hwnd, "Topic %s/%s does not exist offline and cannot be created",
					lpcConfName, lpTopic->topicName);
				}
			}
			

			if (lpTopic->topicHandle) 
			{
				SetTopicFlags(lpTopic->topicHandle, TF_HASFILES, lpTopic->topicFlags&TOPIC_FILES);
				SetTopicFlags(lpTopic->topicHandle, TF_READONLY, lpTopic->topicFlags&TOPIC_READONLY);
			}

			if (lpTopic->topicFlags&(TOPIC_NEWCONF|TOPIC_NEW)) 
			{
				if (lpTopic->topicFlags&TOPIC_NEW) 
				{
					HSCRIPT hScript=initScript("Moderate", FALSE);

					addToScript(hScript, "put `mod %s`¬"
									"if waitfor(`Mod:`, `M:`) == 0¬"
									"put `add topic`¬"
									"waitfor `opicname:`¬"
									"put `%s`¬"
									"waitfor `FLIST) (y/n)?`¬"
									"put `%s`¬"
									"waitfor `Description of`¬"
									"put `%s`¬"
									"waitfor `opicname:`¬"
									"put `quit`¬",
									lpcConfName, lpTopic->topicName,
									(LPSTR)(lpTopic->topicFlags&TOPIC_FILES ? "yes" : "no"),
									lpTopic->topicDescription);

					if (lpTopic->topicFlags&TOPIC_READONLY)
						addToScript(hScript, "waitfor `Mod:`¬"
									"put `rdonly %s`¬",
									lpTopic->topicName);

					addToScript(hScript,     "waitfor `Mod:`¬"
									"put `quit`¬"
									"waitfor `M:`¬"
									"endif¬");

					actionScript(hScript, OT_PREINCLUDE, "add topic %s/%s",
						lpcConfName, lpTopic->topicName);

				}

				if (lpTopic->topicFlags&TOPIC_FILES) 
				{
					downloadFdir(lpTopic->topicHandle, TRUE);
					downloadFlist(lpTopic->topicHandle, TRUE);
					downloadFuser(lpTopic->topicHandle, TRUE);
				}
			} 
			else 
			{
				if ((lpTopic->topicFlags^lpTopic->topicOldFlags)&TOPIC_READONLY) 
				{
					HSCRIPT hScript=initScript("Moderate", FALSE);

					if(lpTopic->topicFlags&TOPIC_READONLY)
					{
						addToScript(hScript, "put `mod %s`¬"
								 "if waitfor(`Mod:`, `M:`) == 0¬"
									 "put `rdonly %s`¬"
									 "if waitfor (`Write`, `Mod:`) == 0¬"
									 "  put `rdonly %s`¬"
									 "  waitfor `Mod:`¬"
									 "endif¬"
									 "put `quit`¬"
									 "waitfor `M:`¬"
								 "endif¬",
							lpcConfName, lpTopic->topicName, lpTopic->topicName);
							actionScript(hScript, OT_PREINCLUDE, "Set %s/%s To Read Only",
								 lpcConfName, lpTopic->topicName);
					}
					else
					{
						addToScript(hScript, "put `mod %s`¬"
								 "if waitfor(`Mod:`, `M:`) == 0¬"
									 "put `rdonly %s`¬"
									 "if waitfor (`Only`, `Mod:`) == 0¬"
									 "  put `rdonly %s`¬"
									 "  waitfor `Mod:`¬"
									 "endif¬"
									 "put `quit`¬"
									 "waitfor `M:`¬"
								 "endif¬",
							lpcConfName, lpTopic->topicName, lpTopic->topicName);
							actionScript(hScript, OT_PREINCLUDE, "Set %s/%s To Read Write",
								 lpcConfName, lpTopic->topicName);
					}
				}

				if (lpTopic->topicFlags&TOPIC_PRUNE) 
				{
					HSCRIPT hScript=initScript("Moderate", FALSE);

					addToScript(hScript, "put `mod %s`¬"
							"if waitfor(`Mod:`, `M:`) == 0¬"
							"  put `prune %s`¬"
							"  waitfor `Mod:`¬"
							"  put `quit`¬"
							"  waitfor `M:`¬"
							"endif¬",
							lpcConfName, lpTopic->topicName);
					
					if(lpTopic->topicFlags&TOPIC_READONLY)
					{
						addToScript(hScript, "put `mod %s`¬"
								 "if waitfor(`Mod:`, `M:`) == 0¬"
									 "put `rdonly %s`¬"
									 "if waitfor (`Write`, `Mod:`) == 0¬"
									 "  put `rdonly %s`¬"
									 "  waitfor `Mod:`¬"
									 "endif¬"
									 "put `quit`¬"
									 "waitfor `M:`¬"
								 "endif¬",
							lpcConfName, lpTopic->topicName, lpTopic->topicName);
					}

					actionScript(hScript, OT_PREINCLUDE, "prune topic %s/%s",
							lpcConfName, lpTopic->topicName);
				}
			}
		}
    }

    closeFile(hStreamInfo);
    busy(FALSE);
    return(TRUE);
}

