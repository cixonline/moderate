#include <windows.h>                    //  Windows API definitions
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <io.h>
#include "amctrls.h"
#include "ameolapi.h"     //  Ameol API definitions
#include "winhorus.h"                   //  Windows utility routines
#include "tabcntrl.h"
#include "hctools.h"                    //  Ameol script routines
#include "moderate.h"                   //  Resource definitions
#include "help.h"             		//  Help resource definitions
#include "globals.h"                    //  Global variable and routine definitions
#include "setup.h"
#include "confs.h"
#include "strftime.h"


typedef struct {
    LPTabData lpTabData;
    WNDPROC lpfnUpdateDaysOldProc;	//  original update days edit control message handler
    FARPROC lpfnUpdateDaysProc;      	//  subclasses the update days edit control
    LPEditTopicData lpEditTopicData;
    TopicInfo topicInfo;
    BOOL bTopicsizeWarning;
} UpdateTopicData, FAR *LPUpdateTopicData;



static LRESULT CALLBACK tabTopic_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK tabFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static TabDescription tabUpdateTopic[] = {
    {"Topic Info", "DID_UPDATE_TOPIC_SIZES", tabTopic_DlgProc},
    {"File Lists", "DID_UPDATE_TOPIC_FLISTS", tabFlist_DlgProc}
};



static void updateTopic_OnSelChange(HWND hwnd);
LONG _EXPORT CALLBACK updateDaysProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



void getPendingTopicInfo(HWND hwnd)
{
    HCONF hConf;
    TopicInfo topicInfo;
    time_t now=time(NULL);

    initTopicInfo(&topicInfo);

    for (hConf=GetModConference(NULL); hConf; hConf=GetModConference(hConf)) 
	{
        LPCSTR lpcConfName=GetConferenceName(hConf);
        CONFERENCEINFO confInfo;

		GetConferenceInfo(hConf, &confInfo);

        if (loadTopicInfo(hwnd, hConf, &topicInfo)) 
		{
            int nTopic;
            HSCRIPT hScriptSize=NULL;

            BOOL bGetAll=topicInfo.topicHeader.updateFlags&TOPIC_UPDATESIZE &&
                         !(topicInfo.topicHeader.updateFlags&TOPIC_REQUESTSIZE);

            if (setup.bDebug && bGetAll)
                inform(hwnd, "Updating all topic sizes for %s (%s)",
		       lpcConfName, (LPSTR)confInfo.szFileName);

            if (topicInfo.topicHeader.updateFlags&TOPIC_PUTNOTE) 
			{
                char szConfNote[LEN_PATHNAME];
				HSTREAM hStreamIn;

				wsprintf(szConfNote, "%s\\%s.CNO", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
                hStreamIn=openFile(szConfNote, OF_READ);

                if (hStreamIn) 
				{
					HSTREAM hStreamOut1, hStreamOut2;

                    wsprintf(szConfNote, "%s\\%s.CNB", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
                    hStreamOut1=openFile(szConfNote, OF_WRITE);
                    wsprintf(szConfNote, "%s\\%s.CNU", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
                    hStreamOut2=openFile(szConfNote, OF_WRITE);

                    if (hStreamOut1 && hStreamOut2) 
					{
						char szBuff[256];
                        HSCRIPT hScriptList;

                        readLine(hStreamIn, szBuff, 255);
						writeLine(hStreamOut1, "%s\n", (LPSTR)szBuff);

                        while (readLine(hStreamIn, szBuff, 255)>=0) 
						{
                            writeLine(hStreamOut1, "%s\n", (LPSTR)szBuff);
                            writeLine(hStreamOut2, "%s\n", (LPSTR)szBuff);
                        }

						closeFile(hStreamOut2);
						closeFile(hStreamOut1);
						hScriptList=initScript("Moderate", FALSE);

						addToScript(hScriptList, "put `join %s`¬"
									 "if waitfor(`opic?`, `R:`, `M:`) == 0¬"
										 "put ``¬"
										 "waitfor `R:`¬"
									 "endif¬"
									 "put `time`¬"
									 "if waitfor(`R:`, `M:`) == 0¬"
										 "put `upload`¬"
										 "upload `%s`¬"
										 "put `scput note`¬"
										 "waitfor `R:`¬"
										 "delete `%s`¬"
										 "put `quit`¬"
										 "waitfor `M:`¬"
									 "endif¬",
								lpcConfName, (LPSTR)szConfNote, (LPSTR)szConfNote);

						actionScript(hScriptList, OT_PREINCLUDE,
								 "uploading forum note for %s", lpcConfName);

						topicInfo.topicHeader.updateFlags&= ~TOPIC_PUTNOTE;
						topicInfo.changed=TRUE;
					} 
					else if (hStreamOut1)
					{
						closeFile(hStreamOut1);
					}
					else if (hStreamOut2)
					{
						closeFile(hStreamOut2);
					}
					closeFile(hStreamIn);
				} 
				else 
				{
					topicInfo.topicHeader.updateFlags&= ~TOPIC_PUTNOTE;
					topicInfo.changed=TRUE;
				}
			}

            if (topicInfo.topicHeader.updateFlags&TOPIC_GETNOTE) 
			{
				HSCRIPT hScriptList=initScript("Moderate", FALSE);

		//  Remember to sanitise the following when scgadd is fixed in CoSy!!!!!

				addToScript(hScriptList, "put `join %s`¬"
							 "if waitfor(`opic?`, `R:`) == 0¬"
								 "put ``¬"
								 "waitfor `R:`¬"
							 "endif¬"
							 "put `time`¬"
							 "if waitfor(`R:`, `M:`) == 0¬"
								 "put `scget note`¬"
								 "if waitfor(`cannot access`, `R:`) == 0¬"
								 "put `quit`¬"
								 "waitfor `M:`¬"
								 "put `killscratch`¬"
								 "waitfor `M:`¬"
								 "put `file time`¬"
								 "waitfor `M:`¬"
								 "put `download`¬"
								 "download `%s\\%s.CNO`¬"
								 "put `killscratch`¬"
								 "waitfor `M:`¬"
								 "end¬"
								 "endif¬"
								 "put `quit`¬"
								 "waitfor `M:`¬"
								 "put `killscratch`¬"
								 "waitfor `M:`¬"
								 "put `file time`¬"
								 "put `join %s`¬"
								 "if waitfor(`opic?`, `R:`) == 0¬"
								 "put ``¬"
								 "waitfor `R:`¬"
								 "endif¬"
								 "put `scgadd note`¬"
								 "waitfor `R:`¬"
								 "put `quit`¬"
								 "waitfor `M:`¬"
								 "put `download`¬"
								 "download `%s\\%s.CNO`¬"
								 "put `killscratch`¬"
								 "waitfor `M:`¬"
							 "endif¬",
						lpcConfName, (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName,
									lpcConfName, (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);

				actionScript(hScriptList, OT_INCLUDE, "downloading forum note for %s", lpcConfName);
				topicInfo.topicHeader.updateFlags&= ~TOPIC_GETNOTE;
				topicInfo.changed=TRUE;
			}

			if (topicInfo.topicHeader.updateFlags&TOPIC_UPDATELIST &&
				!(topicInfo.topicHeader.updateFlags&TOPIC_REQUESTLIST)) 
			{
				HSCRIPT hScriptList=initScript("Moderate", FALSE);

				addToScript(hScriptList, "put `file time show %s`¬"
					 "waitfor `M:`¬"
					 "put `download`¬"
					 "download `%s\\%s.CTL`¬"
					 "put `killscratch`¬"
					 "waitfor `M:`¬",
			    lpcConfName, (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);

				actionScript(hScriptList, OT_INCLUDE, "refreshing topic information for %s", lpcConfName);
				topicInfo.topicHeader.updateFlags|=TOPIC_REQUESTLIST;
				topicInfo.topicHeader.updateFlags&= ~TOPIC_UPDATELIST;
				topicInfo.changed=TRUE;
			}

			for (nTopic=0; nTopic<topicInfo.totalCount; nTopic++) 
			{
				LPTopic lpTopic=topicInfo.topics[nTopic];
				BOOL updateNow = FALSE;

//				if ((bGetAll || lpTopic->topicFlags&TOPIC_UPDATESIZE ||
//					lpTopic->updateSelect==CID_UPDATE_ALWAYS ||
//					(lpTopic->updateSelect==CID_UPDATE_PERIODIC &&
//					datediff(now, lpTopic->lastUpdate) >= (time_t) lpTopic->updateDays)) &&
//					!(lpTopic->topicFlags&TOPIC_REQUESTSIZE)) 

				if (bGetAll)
					updateNow = TRUE;

				if(lpTopic->topicFlags&TOPIC_UPDATESIZE)
					updateNow = TRUE;
				
				if(lpTopic->updateSelect==CID_UPDATE_ALWAYS)
					updateNow = TRUE;

				if(lpTopic->updateSelect==CID_UPDATE_PERIODIC &&
					datediff(now, lpTopic->lastUpdate) >= (time_t) lpTopic->updateDays &&
					!(lpTopic->topicFlags&TOPIC_REQUESTSIZE)) 
					updateNow = TRUE;
				
				if(updateNow)
				{
					if (setup.bDebug) 
					{
						char szBuff1[32], szBuff2[32];

						strftime(szBuff1, 31, "%d%b%Y %I:%M %p", localtime(&now));
						if( lpTopic->lastUpdate > 0)
							strftime(szBuff2, 31, "%d%b%Y %I:%M %p", localtime(&lpTopic->lastUpdate));
						else
							wsprintf(szBuff2, "Never");

						inform(hwnd, "Updating topic size for %s/%s (%s)\r\n"
								 "get all=%d\r\n"
								 "update=%d\r\n"
								 "always=%d\r\n"
								 "periodic=%d (diff=%ld, days=%d)\r\n"
								 "now=%ld (%s)\r\n"
								 "last update=%ld (%s)",
							   lpcConfName, lpTopic->topicName, (LPSTR)confInfo.szFileName, bGetAll,
							   (lpTopic->topicFlags&TOPIC_UPDATESIZE)!=0,
							   lpTopic->updateSelect==CID_UPDATE_ALWAYS,
							   lpTopic->updateSelect==CID_UPDATE_PERIODIC,
							   datediff(now, lpTopic->lastUpdate), lpTopic->updateDays,
							   now, (LPSTR)szBuff1, lpTopic->lastUpdate, (LPSTR)szBuff2);

					}

				    if (!hScriptSize) 
					{
						hScriptSize=initScript("Moderate", FALSE);

						addToScript(hScriptSize, "holdlog¬"
								 "openlog `%s\\%s.CTS`¬"
								 "put `time`¬"
								 "waitfor `M:`¬"
								 "put `mod %s`¬"
								 "if waitfor(`Mod:`, `M:`) == 0¬",
							(LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName, lpcConfName);

				    }

					addToScript(hScriptSize, "put `topicsize %s`¬"
								 "if waitfor(`Mod:`, `Topic?`) == 1¬"
								"put ``¬"
								"waitfor `Mod:`¬"
								 "endif¬",
						lpTopic->topicName);

					lpTopic->topicFlags|=TOPIC_REQUESTSIZE;
					lpTopic->topicFlags&= ~TOPIC_UPDATESIZE;
				}
			}

			if (hScriptSize) 
			{
				addToScript(hScriptSize, "put `quit`¬"
							 "waitfor `M:`¬"
							 "endif¬"
							 "closelog¬"
							 "resumelog¬");

				actionScript(hScriptSize, OT_INCLUDE, "refreshing topic sizes in forum %s", lpcConfName);
				topicInfo.topicHeader.updateFlags|=TOPIC_REQUESTSIZE;
				topicInfo.topicHeader.updateFlags&= ~TOPIC_UPDATESIZE;
				topicInfo.changed=TRUE;
		    }

			saveTopicInfo(hwnd, hConf, &topicInfo);
		}
    }

    freeTopicInfo(&topicInfo);
}



void updateTopicInfo(HWND hwnd)
{
	HCONF hConf;
	TopicInfo topicInfo;

	initTopicInfo(&topicInfo);
	for( hConf = GetModConference( NULL ); hConf; hConf = GetModConference( hConf ) )
	{
		LPCSTR lpcConfName = GetConferenceName( hConf );
		CONFERENCEINFO confInfo;
		char szFilename[ LEN_PATHNAME ];
		HSTREAM hStream;
		char szBuff[ 256 ];
		BOOL bTopicInfoLoaded = FALSE;

			/* Process conference list files (*.CTL)
			 */
		GetConferenceInfo(hConf, &confInfo);
		wsprintf( szFilename, "%s\\%s.CTL", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName );
		hStream = openFile( szFilename, OF_READ );
		if( hStream )
		{
			int nTopic;

			loadTopicInfo( hwnd, hConf, &topicInfo );
			bTopicInfoLoaded = TRUE;
			topicInfo.changed = TRUE;
			if( readLine( hStream, szBuff, 256 ) > 0 )
			{
				topicInfo.topicHeader.lastUpdate=parseTime(szBuff);
				if( !topicInfo.topicHeader.lastUpdate )
				{
					inform( hwnd, "Topic list for %s (%s) updated\r\nError parsing time from \"%s\"",
									lpcConfName, (LPSTR)confInfo.szFileName, (LPSTR)szBuff );
					topicInfo.topicHeader.lastUpdate = time( NULL );
				}
				else if( setup.bDebug )
				{
					char szTime[64];
		
					strftime( szTime, 63, "on %d%b%Y at %I:%M %p", localtime( &topicInfo.topicHeader.lastUpdate ) );
					inform( hwnd, "Topic list for %s (%s) updated on %s", lpcConfName,
								  (LPSTR)confInfo.szFileName, (LPSTR)szTime );
				}
			}
			
			while( readLine( hStream, szBuff, 256 ) >= 0 && *szBuff != '-' );
			while( readLine( hStream, szBuff, 256 ) > 0 && *szBuff != ' ' )
			{
				char szTopicName[ 16 ];
				char szDescription[ DESC_LENGTH ];
				LPSTR lpPtrIn = szBuff;
				LPSTR lpPtrOut = szTopicName;
				int nCount = 0;
				LPTopic lpTopic;

				while( *lpPtrIn && isgraph(*lpPtrIn) && ++nCount < 15 )
				{
					*lpPtrOut++ = *lpPtrIn++;
				}
				*lpPtrOut='\0';
				while( *lpPtrIn && !isgraph(*lpPtrIn) )
				{
						lpPtrIn++;
				}
				lpPtrOut = szDescription;
				nCount = 0;
				while( *lpPtrIn && ++nCount < DESC_LENGTH )
				{
					*lpPtrOut++ = *lpPtrIn++;
				}
				*lpPtrOut = '\0';
				for( nTopic = 0; nTopic < topicInfo.totalCount &&
					lstrcmp( topicInfo.topics[ nTopic ]->topicName, szTopicName );
					nTopic++ );
				if( nTopic < topicInfo.totalCount )
				{
					lpTopic=topicInfo.topics[nTopic];
				}
				else 
				{
					lpTopic=allocateTopic(&topicInfo, NULL);
					lstrcpy(lpTopic->topicName, szTopicName);
				}
				lstrcpy( lpTopic->topicDescription, szDescription );
				lpTopic->topicFlags |= TOPIC_NEW;
			}
			topicInfo.topicHeader.updateFlags &= ~(TOPIC_UPDATELIST|TOPIC_REQUESTLIST);
			closeFile( hStream );
			if( setup.bDebug ) 
			{
				char szBackup[ LEN_PATHNAME ];

				wsprintf( szBackup, "%s\\%s.BTL", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName );
				_unlink( szBackup );
				rename( szFilename, szBackup );
			}
			else
			{
				_unlink( szFilename );
			}
			for( nTopic = topicInfo.totalCount - 1; nTopic >= 0; nTopic-- )
				{
				LPTopic lpTopic = topicInfo.topics[ nTopic ];

				if( lpTopic->topicFlags & TOPIC_NEW )
					lpTopic->topicFlags &= ~TOPIC_NEW;
				else if( lpTopic->topicHandle ) 
					{
					SetTopicFlags( lpTopic->topicHandle, TF_LOCAL, TRUE );
					removeTopic( &topicInfo, lpTopic );
					}
				}
/*			for( nTopic = topicInfo.totalCount - 1; nTopic >= 0; nTopic-- )
			{
				LPTopic lpTopic = topicInfo.topics[ nTopic ];

				if( lpTopic->topicFlags & TOPIC_NEW )
				{
					lpTopic->topicFlags &= ~TOPIC_NEW;
					if( lpTopic->topicHandle ) 
					{
						SetTopicFlags( lpTopic->topicHandle, TF_LOCAL, FALSE );
					}
				}
				else if( lpTopic->topicHandle ) 
				{
					SetTopicFlags( lpTopic->topicHandle, TF_LOCAL, TRUE );
					removeTopic( &topicInfo, lpTopic );
				}
			}*/
		}

		wsprintf(szFilename, "%s\\%s.CTS", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
		hStream=openFile(szFilename, OF_READ);

		if (hStream) 
		{
			time_t updateTime;
			int nTopic;
			int nStatus;
		
			if (!bTopicInfoLoaded) 
			{
				loadTopicInfo(hwnd, hConf, &topicInfo);
				bTopicInfoLoaded=TRUE;
			}

			topicInfo.changed=TRUE;
			while ((nStatus=readLine(hStream, szBuff, 256))>=0 && !strstr(szBuff, "time"));

			if (nStatus>=0) 
			{
				nStatus=readLine(hStream, szBuff, 256);
				updateTime=parseTime(szBuff);

				if (!updateTime) 
				{
					inform(hwnd, "Topic sizes for %s (%s) updated\r\n"
								 "Error parsing time from \"%s\"",
						   lpcConfName, (LPSTR)confInfo.szFileName, (LPSTR)szBuff);

					updateTime=time(NULL);
				}

				do 
				{
					LPSTR lpToken;

					while ((nStatus=readLine(hStream, szBuff, 256))>=0 && !strchr(szBuff, '/'));

					if (nStatus>=0 &&
						lstrcmp(_fstrtok(szBuff, "/"), lpcConfName)==0 &&
						(lpToken=_fstrtok(NULL, ": "))) 
					{
						for (nTopic=0;
							 nTopic<topicInfo.totalCount &&
							 lstrcmp(topicInfo.topics[nTopic]->topicName, lpToken);
							 nTopic++);

						if (nTopic<topicInfo.totalCount &&
								(lpToken=_fstrtok(NULL, ": "))) 
						{
							LPTopic lpTopic=topicInfo.topics[nTopic];

							lpTopic->topicSize=atol(lpToken);
							lpToken=_fstrtok(NULL, "\r\n");

							if (lpToken && (lpToken=_fstrchr(lpToken, '=')))
								lpTopic->topicMax=atol(lpToken+1);

							lpTopic->lastUpdate=updateTime;
							lpTopic->topicFlags&= ~(TOPIC_UPDATESIZE|TOPIC_REQUESTSIZE);

							if (setup.bDebug) 
							{
								char szTime[64];

								strftime(szTime, 63, "on %d%b%Y at %I:%M %p", localtime(&updateTime));

								inform(hwnd, "Topic size for %s/%s (%s) updated on %s",
										   lpcConfName, lpTopic->topicName, (LPSTR)confInfo.szFileName, (LPSTR)szTime);

							}

							if (setup.bTopicsizeWarning &&
								lpTopic->topicFlags&TOPIC_SIZEWARNING &&
								lpTopic->topicMax-lpTopic->topicSize<=lpTopic->topicWarn) 
							{
								char szSize[64];

								formatLong(szSize, "", lpTopic->topicMax-lpTopic->topicSize, " bytes!");

								alert(hwnd, "Free space in %s/%s is %s",
										  lpcConfName, lpTopic->topicName, (LPSTR)szSize);

							}
						}
					}
				} while (nStatus>=0);
			}

			topicInfo.topicHeader.updateFlags&= ~(TOPIC_UPDATESIZE|TOPIC_REQUESTSIZE);
			closeFile(hStream);

			if (setup.bDebug) 
			{
				char szBackup[LEN_PATHNAME];

				wsprintf(szBackup, "%s\\%s.BTS", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
				_unlink(szBackup);
				rename(szFilename, szBackup);
			}
			else
			{
				_unlink(szFilename);
			}

		}

		if (bTopicInfoLoaded)
		{
			saveTopicInfo(hwnd, hConf, &topicInfo);
		}

    }

    freeTopicInfo(&topicInfo);
}



static void setTopicControls(HWND hwnd, LPUpdateTopicData lpUpdateTopicData)
{
    LPTopicInfo lpTopicInfo= &lpUpdateTopicData->topicInfo;
    LPTopic lpTopic;
    int nUpdateFlag;
    int nUpdateSelect;
    int nUpdateDays;
    int nWarningFlag;
    UINT nWarningThreshold;
    BOOL bResetAllowed=lpTopicInfo->topicHeader.updateFlags&(TOPIC_REQUESTLIST|TOPIC_REQUESTSIZE);

    if (lpTopicInfo->totalCount) {
	int nTopic;

	for (nTopic=0;
	     nTopic<lpTopicInfo->totalCount && lpTopicInfo->topics[nTopic]->topicFlags&TOPIC_DELETE;
	     nTopic++);

	lpTopic=lpTopicInfo->topics[nTopic];
	nUpdateFlag=lpTopic->topicFlags&TOPIC_UPDATESIZE ? 1 : 0;
	nUpdateSelect=lpTopic->updateSelect;
	nUpdateDays=lpTopic->updateDays;
	nWarningFlag=lpTopic->topicFlags&TOPIC_SIZEWARNING ? 1 : 0;
	nWarningThreshold=(UINT)(((lpTopic->topicMax-lpTopic->topicWarn)*100L)/lpTopic->topicMax);

	if (lpTopic->topicFlags&TOPIC_REQUESTSIZE)
	    bResetAllowed=TRUE;

	while (++nTopic<lpTopicInfo->totalCount) {
	    lpTopic=lpTopicInfo->topics[nTopic];

	    if (lpTopic->topicFlags&TOPIC_REQUESTSIZE)
		bResetAllowed=TRUE;

	    if (!(lpTopic->topicFlags&TOPIC_DELETE)) {
		if (nUpdateFlag!=(lpTopic->topicFlags&TOPIC_UPDATESIZE ? 1 : 0))
		    nUpdateFlag=2;

		if ((WORD) nUpdateSelect != lpTopic->updateSelect)
		    nUpdateSelect=0;

		if ((WORD) nUpdateDays != lpTopic->updateDays)
		    nUpdateDays=0;

		if (nWarningFlag!=(lpTopic->topicFlags&TOPIC_SIZEWARNING ? 1 : 0))
		    nWarningFlag=2;

		if (nWarningThreshold!=(UINT)(((lpTopic->topicMax-lpTopic->topicWarn)*100L)/lpTopic->topicMax))
		    nWarningThreshold=0;

	    }
	}

	if (nUpdateFlag==1)
	    lpTopicInfo->topicHeader.updateFlags|=TOPIC_UPDATESIZE;
	else
	    lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_UPDATESIZE;

	lpTopicInfo->topicHeader.updateSelect=nUpdateSelect;

	if (nUpdateDays)
	    lpTopicInfo->topicHeader.updateDays=nUpdateDays;

	if (nWarningFlag==1)
	    lpTopicInfo->topicHeader.updateFlags|=TOPIC_SIZEWARNING;
	else
	    lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_SIZEWARNING;

	if (nWarningThreshold)
	    lpTopicInfo->topicHeader.topicWarn=(2047000L*(100-nWarningThreshold))/100L;

    } else {
	nUpdateFlag=lpTopicInfo->topicHeader.updateFlags&TOPIC_UPDATESIZE ? 1 : 0;
	nUpdateSelect=lpTopicInfo->topicHeader.updateSelect;
	nUpdateDays=lpTopicInfo->topicHeader.updateDays;
	nWarningFlag=lpTopicInfo->topicHeader.updateFlags&TOPIC_SIZEWARNING ? 1 : 0;
	nWarningThreshold=(UINT)(((2047000L-lpTopicInfo->topicHeader.topicWarn)*100L)/2047000L);
    }

    lpTopic=lpTopicInfo->currentTopic;

    if (lpTopic) {
	BOOL bEnable= !(lpTopic->topicFlags&TOPIC_DELETE);

	Button_SetStyle(GetDlgItem(hwnd, CID_UPDATE_NEXT), BS_AUTOCHECKBOX, TRUE);
	CheckDlgButton(hwnd, CID_UPDATE_NEXT, lpTopic->topicFlags&TOPIC_UPDATESIZE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_NEXT), bEnable);

	Button_SetStyle(GetDlgItem(hwnd, CID_TOPICSIZE_ENABLE), BS_AUTOCHECKBOX, TRUE);
	CheckDlgButton(hwnd, CID_TOPICSIZE_ENABLE, lpTopic->topicFlags&TOPIC_SIZEWARNING);
	SetDlgItemInt(hwnd, CID_TOPICSIZE_THRESHOLD, (UINT)(((lpTopic->topicMax-lpTopic->topicWarn)*100L)/lpTopic->topicMax), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_ENABLE), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_THRESHOLD), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_TAG), bEnable);

	RadioButton(hwnd, CID_UPDATE_MANUAL, lpTopic->updateSelect);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_MANUAL), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_PERIODIC), bEnable);
	SetDlgItemInt(hwnd, CID_UPDATE_DAYS, lpTopic->updateDays, FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_TAG_DAYS), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_ALWAYS), bEnable);
    } else {
	Button_SetStyle(GetDlgItem(hwnd, CID_UPDATE_NEXT), BS_AUTO3STATE, TRUE);
	CheckDlgButton(hwnd, CID_UPDATE_NEXT, nUpdateFlag);

	Button_SetStyle(GetDlgItem(hwnd, CID_TOPICSIZE_ENABLE), BS_AUTO3STATE, TRUE);
	CheckDlgButton(hwnd, CID_TOPICSIZE_ENABLE, nWarningFlag);

	if (nWarningThreshold)
	    SetDlgItemInt(hwnd, CID_TOPICSIZE_THRESHOLD, nWarningThreshold, FALSE);
	else
	    SetDlgItemText(hwnd, CID_TOPICSIZE_THRESHOLD, "");

	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_THRESHOLD), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_TAG), TRUE);

	RadioButton(hwnd, CID_UPDATE_MANUAL, nUpdateSelect ? nUpdateSelect : -1);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_MANUAL), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_PERIODIC), TRUE);

	if (nUpdateDays)
	    SetDlgItemInt(hwnd, CID_UPDATE_DAYS, nUpdateDays, FALSE);
	else
	    SetDlgItemText(hwnd, CID_UPDATE_DAYS, "");

	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_TAG_DAYS), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_ALWAYS), TRUE);
    }

    CheckDlgButton(hwnd, CID_UPDATE_TOPICS, lpTopicInfo->topicHeader.updateFlags&TOPIC_UPDATELIST);
    EnableWindow(GetDlgItem(hwnd, CID_RESET), bResetAllowed);
    EnableWindow(GetDlgItem(hwnd, CID_RESET_TAG), bResetAllowed);
    CheckDlgButton(hwnd, CID_TOPICSIZE_WARNING, lpUpdateTopicData->bTopicsizeWarning);
}



static BOOL getTopicControls(HWND hwnd, LPUpdateTopicData lpUpdateTopicData)
{
    LPTopicInfo lpTopicInfo= &lpUpdateTopicData->topicInfo;
    LPTopic lpTopic=lpTopicInfo->currentTopic;
    int nUpdateDays=GetDlgItemInt(hwnd, CID_UPDATE_DAYS, NULL, FALSE);
    UINT nWarningThreshold=GetDlgItemInt(hwnd, CID_TOPICSIZE_THRESHOLD, NULL, FALSE);

    if (nUpdateDays<0 || 100<nUpdateDays || nUpdateDays==0 /*|| !IsDlgButtonChecked(hwnd, CID_UPDATE_MANUAL)*/ && lpTopic) {
	alert(hwnd, "The number of days specified for updates is out of range");
	NextDlgCtl(hwnd, CID_UPDATE_DAYS);
	return(FALSE);
    }

    if ((nWarningThreshold<50 && !(nWarningThreshold==0 && !lpTopic)) || 100<nWarningThreshold) {
	alert(hwnd, "The threshold specified for topic size warnings is out of range");
	NextDlgCtl(hwnd, CID_TOPICSIZE_THRESHOLD);
	return(FALSE);
    }

    lpUpdateTopicData->bTopicsizeWarning=IsDlgButtonChecked(hwnd, CID_TOPICSIZE_WARNING);

    if (lpTopic) {
	if (IsDlgButtonChecked(hwnd, CID_UPDATE_NEXT))
	    lpTopic->topicFlags|=TOPIC_UPDATESIZE;
	else
	    lpTopic->topicFlags&= ~TOPIC_UPDATESIZE;

	if (IsDlgButtonChecked(hwnd, CID_TOPICSIZE_ENABLE))
	    lpTopic->topicFlags|=TOPIC_SIZEWARNING;
	else
	    lpTopic->topicFlags&= ~TOPIC_SIZEWARNING;

	lpTopic->topicWarn=(lpTopic->topicMax*(100-nWarningThreshold))/100L;
	lpTopic->updateSelect=RadioButton(hwnd, CID_UPDATE_MANUAL, 0);
	lpTopic->updateDays=nUpdateDays;
    } else {
	int nTopic;

	switch (IsDlgButtonChecked(hwnd, CID_UPDATE_NEXT)) {

	case 0:
	    lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_UPDATESIZE;

	    for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++)
		lpTopicInfo->topics[nTopic]->topicFlags&= ~TOPIC_UPDATESIZE;

	    break;

	case 1:
	    lpTopicInfo->topicHeader.updateFlags|=TOPIC_UPDATESIZE;

	    for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++)
		lpTopicInfo->topics[nTopic]->topicFlags|=TOPIC_UPDATESIZE;

	    break;

	}

	switch (IsDlgButtonChecked(hwnd, CID_TOPICSIZE_ENABLE)) {

	case 0:
	    lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_SIZEWARNING;

	    for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++)
		lpTopicInfo->topics[nTopic]->topicFlags&= ~TOPIC_SIZEWARNING;

	    break;

	case 1:
	    lpTopicInfo->topicHeader.updateFlags|=TOPIC_SIZEWARNING;

	    for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++)
		lpTopicInfo->topics[nTopic]->topicFlags|=TOPIC_SIZEWARNING;

	    break;

	}

	lpTopicInfo->topicHeader.updateSelect=RadioButton(hwnd, CID_UPDATE_MANUAL, 0);

	if (nUpdateDays)
	    lpTopicInfo->topicHeader.updateDays=nUpdateDays;

	if (lpTopicInfo->topicHeader.updateSelect)
	    for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++) {
		lpTopicInfo->topics[nTopic]->updateSelect=lpTopicInfo->topicHeader.updateSelect;

		if (lpTopicInfo->topicHeader.updateSelect==CID_UPDATE_PERIODIC && nUpdateDays)
		    lpTopicInfo->topics[nTopic]->updateDays=nUpdateDays;

	    }

	if (nWarningThreshold) {
	    lpTopicInfo->topicHeader.topicWarn=(2047000L*(100-nWarningThreshold))/100L;

	    for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++) {
		LPTopic lpTopic=lpTopicInfo->topics[nTopic];

		lpTopic->topicWarn=(lpTopic->topicMax*(100-nWarningThreshold))/100L;
	    }
	}
    }

    if (IsDlgButtonChecked(hwnd, CID_UPDATE_TOPICS))
	lpTopicInfo->topicHeader.updateFlags|=TOPIC_UPDATELIST;
    else
	lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_UPDATELIST;

    return(TRUE);
}



static BOOL tabTopic_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    lpUpdateTopicData->lpfnUpdateDaysOldProc=SubclassWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS),
							    lpUpdateTopicData->lpfnUpdateDaysProc);

    setTopicControls(hwnd, lpUpdateTopicData);
    return(TRUE);
}



static void tabTopic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    switch (id) {

    case CID_UPDATE_NEXT:
	if (codeNotify==BN_CLICKED && IsDlgButtonChecked(hwnd, CID_UPDATE_NEXT)==2)
	    RadioButton(hwnd, CID_UPDATE_MANUAL, -1);

	break;

    case CID_UPDATE_DAYS:
	if (codeNotify==EN_SETFOCUS)
	    RadioButton(hwnd, CID_UPDATE_MANUAL, CID_UPDATE_PERIODIC);

	break;

    case CID_UPDATE_PERIODIC:
	if (codeNotify==BN_CLICKED)
	    NextDlgCtl(hwnd, CID_UPDATE_DAYS);

	break;

    case CID_RESET: {
	LPTopicInfo lpTopicInfo= &lpUpdateTopicData->topicInfo;
	int nTopic;

	lpTopicInfo->topicHeader.updateFlags&= ~(TOPIC_REQUESTLIST|TOPIC_REQUESTSIZE);

	for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++)
	    lpTopicInfo->topics[nTopic]->topicFlags&= ~TOPIC_REQUESTSIZE;

	EnableWindow(GetDlgItem(hwnd, CID_RESET), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_RESET_TAG), FALSE);
	NextDlgCtl(hwnd, CID_TOPIC);
	break;
    }

    }
}



static void tabTopic_OnDestroy(HWND hwnd)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    SubclassWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS), lpUpdateTopicData->lpfnUpdateDaysOldProc);
    lpUpdateTopicData->lpfnUpdateDaysOldProc=NULL;
}



static LRESULT CALLBACK tabTopic_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, tabTopic_OnInitDialog);
    HANDLE_MSG(hwnd, WM_COMMAND, tabTopic_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, tabTopic_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



static void setFlistControls(HWND hwnd, LPUpdateTopicData lpUpdateTopicData)
{ 
/*    LPTopicInfo lpTopicInfo= &lpUpdateTopicData->topicInfo;
    LPTopic lpTopic;
    int nUpdateUserFlist;
    int nUpdateModFlist;
    int nUpdateFdir;
    int nUpdateSelect;
    int nUpdateDays;

    if (lpTopicInfo->totalCount) {
	int nTopic;

	for (nTopic=0;
	     nTopic<lpTopicInfo->totalCount && lpTopicInfo->topics[nTopic]->topicFlags&TOPIC_DELETE;
	     nTopic++);

	lpTopic=lpTopicInfo->topics[nTopic];
	nUpdateUserFlist=downloadFuser(lpTopic->hTopic, FALSE) ? 1 : 0;
	nUpdateSelect=lpTopic->updateSelect;
	nUpdateDays=lpTopic->updateDays;
	nWarningFlag=lpTopic->topicFlags&TOPIC_SIZEWARNING ? 1 : 0;
	nWarningThreshold=(UINT)(((lpTopic->topicMax-lpTopic->topicWarn)*100L)/lpTopic->topicMax);

	if (lpTopic->topicFlags&TOPIC_REQUESTSIZE)
	    bResetAllowed=TRUE;

	while (++nTopic<lpTopicInfo->totalCount) {
	    lpTopic=lpTopicInfo->topics[nTopic];

	    if (lpTopic->topicFlags&TOPIC_REQUESTSIZE)
		bResetAllowed=TRUE;

	    if (!(lpTopic->topicFlags&TOPIC_DELETE)) {
		if (nUpdateFlag!=(lpTopic->topicFlags&TOPIC_UPDATESIZE ? 1 : 0))
		    nUpdateFlag=2;

		if (nUpdateSelect!=lpTopic->updateSelect)
		    nUpdateSelect=0;

		if (nUpdateDays!=lpTopic->updateDays)
		    nUpdateDays=0;

		if (nWarningFlag!=(lpTopic->topicFlags&TOPIC_SIZEWARNING ? 1 : 0))
		    nWarningFlag=2;

		if (nWarningThreshold!=(UINT)(((lpTopic->topicMax-lpTopic->topicWarn)*100L)/lpTopic->topicMax))
		    nWarningThreshold=0;

	    }
	}

	if (nUpdateFlag==1)
	    lpTopicInfo->topicHeader.updateFlags|=TOPIC_UPDATESIZE;
	else
	    lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_UPDATESIZE;

	lpTopicInfo->topicHeader.updateSelect=nUpdateSelect;

	if (nUpdateDays)
	    lpTopicInfo->topicHeader.updateDays=nUpdateDays;

	if (nWarningFlag==1)
	    lpTopicInfo->topicHeader.updateFlags|=TOPIC_SIZEWARNING;
	else
	    lpTopicInfo->topicHeader.updateFlags&= ~TOPIC_SIZEWARNING;

	if (nWarningThreshold)
	    lpTopicInfo->topicHeader.topicWarn=(2047000L*(100-nWarningThreshold))/100L;

    } else {
	nUpdateFlag=lpTopicInfo->topicHeader.updateFlags&TOPIC_UPDATESIZE ? 1 : 0;
	nUpdateSelect=lpTopicInfo->topicHeader.updateSelect;
	nUpdateDays=lpTopicInfo->topicHeader.updateDays;
	nWarningFlag=lpTopicInfo->topicHeader.updateFlags&TOPIC_SIZEWARNING ? 1 : 0;
	nWarningThreshold=(UINT)(((2047000L-lpTopicInfo->topicHeader.topicWarn)*100L)/2047000L);
    }

    lpTopic=lpTopicInfo->currentTopic;

    if (lpTopic) {
	BOOL bEnable= !(lpTopic->topicFlags&TOPIC_DELETE);

	ShowWindow(GetDlgItem(hwnd, CID_UPDATE_ALL), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, CID_UPDATE_NEXT), SW_SHOW);
	CheckDlgButton(hwnd, CID_UPDATE_NEXT, lpTopic->topicFlags&TOPIC_UPDATESIZE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_NEXT), bEnable);

	ShowWindow(GetDlgItem(hwnd, CID_TOPICSIZE_ALL), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, CID_TOPICSIZE_ENABLE), SW_SHOW);
	CheckDlgButton(hwnd, CID_TOPICSIZE_ENABLE, lpTopic->topicFlags&TOPIC_SIZEWARNING);
	SetDlgItemInt(hwnd, CID_TOPICSIZE_THRESHOLD, (UINT)(((lpTopic->topicMax-lpTopic->topicWarn)*100L)/lpTopic->topicMax), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_ENABLE), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_THRESHOLD), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_TAG), bEnable);

	RadioButton(hwnd, CID_UPDATE_MANUAL, lpTopic->updateSelect);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_MANUAL), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_PERIODIC), bEnable);
	SetDlgItemInt(hwnd, CID_UPDATE_DAYS, lpTopic->updateDays, FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_TAG_DAYS), bEnable);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_ALWAYS), bEnable);
    } else {
	ShowWindow(GetDlgItem(hwnd, CID_UPDATE_ALL), SW_SHOW);
	ShowWindow(GetDlgItem(hwnd, CID_UPDATE_NEXT), SW_HIDE);
	CheckDlgButton(hwnd, CID_UPDATE_ALL, nUpdateFlag);

	ShowWindow(GetDlgItem(hwnd, CID_TOPICSIZE_ALL), SW_SHOW);
	ShowWindow(GetDlgItem(hwnd, CID_TOPICSIZE_ENABLE), SW_HIDE);
	CheckDlgButton(hwnd, CID_TOPICSIZE_ALL, nWarningFlag);

	if (nWarningThreshold)
	    SetDlgItemInt(hwnd, CID_TOPICSIZE_THRESHOLD, nWarningThreshold, FALSE);
	else
	    SetDlgItemText(hwnd, CID_TOPICSIZE_THRESHOLD, "");

	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_THRESHOLD), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_TOPICSIZE_TAG), TRUE);

	RadioButton(hwnd, CID_UPDATE_MANUAL, nUpdateSelect ? nUpdateSelect : -1);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_MANUAL), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_PERIODIC), TRUE);

	if (nUpdateDays)
	    SetDlgItemInt(hwnd, CID_UPDATE_DAYS, nUpdateDays, FALSE);
	else
	    SetDlgItemText(hwnd, CID_UPDATE_DAYS, "");

	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_TAG_DAYS), TRUE);
	EnableWindow(GetDlgItem(hwnd, CID_UPDATE_ALWAYS), TRUE);
    }

    CheckDlgButton(hwnd, CID_UPDATE_TOPICS, lpTopicInfo->topicHeader.updateFlags&TOPIC_UPDATELIST);
    EnableWindow(GetDlgItem(hwnd, CID_RESET), bResetAllowed);
    EnableWindow(GetDlgItem(hwnd, CID_RESET_TAG), bResetAllowed);
    CheckDlgButton(hwnd, CID_TOPICSIZE_WARNING, lpUpdateTopicData->bTopicsizeWarning);
*/
    if (!lpUpdateTopicData->topicInfo.currentTopic) {
	EnableWindow(GetDlgItem(hwnd, CID_FLM_UPDATE), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_FLM_DATE), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_FLD_UPDATE), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_FLD_DATE), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_FLS_UPDATE), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_FLS_DATE), FALSE);
    }
}



static BOOL getFlistControls(HWND hwnd, LPUpdateTopicData lpUpdateTopicData)
{
    return(TRUE);
}



static BOOL tabFlist_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    lpUpdateTopicData->lpfnUpdateDaysOldProc=SubclassWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS),
							    lpUpdateTopicData->lpfnUpdateDaysProc);

    setFlistControls(hwnd, lpUpdateTopicData);
    return(TRUE);
}



static void tabFlist_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    switch (id) {

    case CID_UPDATE_NEXT:
	if (codeNotify==BN_CLICKED && IsDlgButtonChecked(hwnd, CID_UPDATE_NEXT)==2)
	    RadioButton(hwnd, CID_UPDATE_MANUAL, -1);

	break;

    case CID_UPDATE_DAYS:
	if (codeNotify==EN_SETFOCUS)
	    RadioButton(hwnd, CID_UPDATE_MANUAL, CID_UPDATE_PERIODIC);

	break;

    case CID_UPDATE_PERIODIC:
	if (codeNotify==BN_CLICKED)
	    NextDlgCtl(hwnd, CID_UPDATE_DAYS);

	break;

    case CID_RESET: {
	LPTopicInfo lpTopicInfo= &lpUpdateTopicData->topicInfo;
	int nTopic;

	lpTopicInfo->topicHeader.updateFlags&= ~(TOPIC_REQUESTLIST|TOPIC_REQUESTSIZE);

	for (nTopic=0; nTopic<lpTopicInfo->totalCount; nTopic++)
	    lpTopicInfo->topics[nTopic]->topicFlags&= ~TOPIC_REQUESTSIZE;

	EnableWindow(GetDlgItem(hwnd, CID_RESET), FALSE);
	EnableWindow(GetDlgItem(hwnd, CID_RESET_TAG), FALSE);
	NextDlgCtl(hwnd, CID_TOPIC);
	break;
    }

    }
}



static void tabFlist_OnDestroy(HWND hwnd)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    SubclassWindow(GetDlgItem(hwnd, CID_UPDATE_DAYS), lpUpdateTopicData->lpfnUpdateDaysOldProc);
    lpUpdateTopicData->lpfnUpdateDaysOldProc=NULL;
}



static LRESULT CALLBACK tabFlist_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, tabFlist_OnInitDialog);
    HANDLE_MSG(hwnd, WM_COMMAND, tabFlist_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, tabFlist_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



static void setUpdateControls(HWND hwnd, LPUpdateTopicData lpUpdateTopicData)
{
    switch (lpUpdateTopicData->lpTabData->nIndex) {

    case 0:
	setTopicControls(hwnd, lpUpdateTopicData);
	break;

    case 1:
	setFlistControls(hwnd, lpUpdateTopicData);
	break;

    }
}



static BOOL getUpdateControls(HWND hwnd, LPUpdateTopicData lpUpdateTopicData)
{
    switch (lpUpdateTopicData->lpTabData->nIndex) {

    case 0:
	return(getTopicControls(hwnd, lpUpdateTopicData));

    case 1:
	return(getFlistControls(hwnd, lpUpdateTopicData));

    default:
	return(TRUE);

    }
}



static BOOL updateTopic_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPUpdateTopicData lpUpdateTopicData=InitDlgData(hwnd, UpdateTopicData);
    HWND hwndTopic=GetDlgItem(hwnd, CID_TOPIC);
    int nTopic;

    lpUpdateTopicData->lpEditTopicData=(LPEditTopicData)lParam;
    lpUpdateTopicData->topicInfo=lpUpdateTopicData->lpEditTopicData->topicInfo;
    lpUpdateTopicData->bTopicsizeWarning=setup.bTopicsizeWarning;
    lpUpdateTopicData->lpfnUpdateDaysProc=MakeProcInstance((FARPROC)updateDaysProc, lpGlobals->hInst);

    SetDlgItemText(hwnd, CID_CONF_NAME,
		   GetConferenceName(lpUpdateTopicData->lpEditTopicData->hConference));

    ComboBox_AddString(hwndTopic, NULL);

    for (nTopic=0; nTopic<lpUpdateTopicData->topicInfo.totalCount; nTopic++)
	ComboBox_AddString(hwndTopic, lpUpdateTopicData->topicInfo.topics[nTopic]);

    if (lpUpdateTopicData->topicInfo.currentTopic)
	ComboBox_SetCurSel(hwndTopic, lpUpdateTopicData->topicInfo.currentTopic->topicIndex+1);
    else
	ComboBox_SetCurSel(hwndTopic, 0);

    lpUpdateTopicData->lpTabData=initTabs(hwnd, CID_UPDATE_TOPIC_FRAME, 1, tabUpdateTopic, 0);
    return(TRUE);
}



static void updateTopic_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR *lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void updateTopic_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR * lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
	drawTopicName(lpDrawItem);

}



static LPCSTR updateTopic_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_UPDATE_TOPIC_DATA, id));
}



static void updateTopic_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_TOPICS_UPDATE);
}



static BOOL updateTopic_OnSelChanging(HWND hwnd)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    if (getUpdateControls(hwnd, lpUpdateTopicData)) {
	closeTab(hwnd, lpUpdateTopicData->lpTabData);
	return(FALSE);
    } else
	return(TRUE);

}



static void updateTopic_OnSelChange(HWND hwnd)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    openTab(hwnd, lpUpdateTopicData->lpTabData);
}



static void updateTopic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    switch (id) {

    case CID_TOPIC:
	if (codeNotify==CBN_SELCHANGE)
	    if (getUpdateControls(hwnd, lpUpdateTopicData)) {
		int nTopic=ComboBox_GetCurSel(hwndCtl);

		if (nTopic>0)
		    lpUpdateTopicData->topicInfo.currentTopic=lpUpdateTopicData->topicInfo.topics[nTopic-1];
		else
		    lpUpdateTopicData->topicInfo.currentTopic=NULL;

		setUpdateControls(hwnd, lpUpdateTopicData);
	    } else
		Post_ComboBox_SetCurSel(hwndCtl, lpUpdateTopicData->topicInfo.currentTopic ?
						 lpUpdateTopicData->topicInfo.currentTopic->topicIndex+1 : 0);

	break;

    case CID_OK:
	if (getUpdateControls(hwnd, lpUpdateTopicData)) {
	    LPTopic lpTopic=lpUpdateTopicData->lpEditTopicData->topicInfo.currentTopic;

	    lpUpdateTopicData->lpEditTopicData->topicInfo=lpUpdateTopicData->topicInfo;
	    lpUpdateTopicData->lpEditTopicData->topicInfo.currentTopic=lpTopic;
	    lpUpdateTopicData->lpEditTopicData->topicInfo.changed=TRUE;
	    setup.bTopicsizeWarning=lpUpdateTopicData->bTopicsizeWarning;
	    AmWritePrivateProfileString("Moderate", "topicsizewarning", setup.bTopicsizeWarning ? "yes" : "no", setup.szIniFile);
	    EndDialog(hwnd, 0);
	}

	break;

    case CID_CANCEL:
	EndDialog(hwnd, 0);
	break;

    }
}



static DWORD updateTopic_OnNotify(HWND hwnd, int idFrom, LPNMHDR lpNmhdr)
{
    switch (lpNmhdr->code) {

    case TCN_SELCHANGING:
	return(updateTopic_OnSelChanging(hwnd));

    case TCN_SELCHANGE:
	updateTopic_OnSelChange(hwnd);
	return(FALSE);

    default:
	return(FALSE);

    }
}



static void updateTopic_OnDestroy(HWND hwnd)
{
    LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);

    if (lpUpdateTopicData) {
	freeTabs(hwnd, lpUpdateTopicData->lpTabData);

	if (lpUpdateTopicData->lpfnUpdateDaysProc)
	    FreeProcInstance(lpUpdateTopicData->lpfnUpdateDaysProc);

	FreeDlgData(hwnd);
    }
}



static LRESULT CALLBACK updateTopic_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, updateTopic_OnInitDialog);
    HANDLE_MSG(hwnd, WM_MEASUREITEM, updateTopic_OnMeasureItem);
    HANDLE_MSG(hwnd, WM_DRAWITEM, updateTopic_OnDrawItem);
    HANDLE_MSG(hwnd, WM_POPUPHELP, updateTopic_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, updateTopic_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, updateTopic_OnCommand);
    HANDLE_MSG(hwnd, WM_NOTIFY, updateTopic_OnNotify);
    HANDLE_MSG(hwnd, WM_DESTROY, updateTopic_OnDestroy);

    default: {
	LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwnd, UpdateTopicData);
	LPTabData lpTabData=lpUpdateTopicData ? lpUpdateTopicData->lpTabData : NULL;

	return(DefDlgProcTab(hwnd, uMsg, wParam, lParam, lpTabData, &lpGlobals->bRecursionFlag));
    }

    }
}



BOOL _EXPORT CALLBACK updateTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=updateTopic_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static void updateDays_OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    HWND hwndParent=GetParent(hwnd);

    if (vk==VK_UP || vk==VK_DOWN) {
	int nFocus=vk==VK_UP ? CID_UPDATE_MANUAL : CID_UPDATE_ALWAYS;

	NextDlgCtl(hwndParent, nFocus);
	RadioButton(hwndParent, CID_UPDATE_MANUAL, nFocus);
    } else {
	LPUpdateTopicData lpUpdateTopicData=GetDlgData(hwndParent, UpdateTopicData);

	WFORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, lpUpdateTopicData->lpfnUpdateDaysOldProc);
    }
}



static void updateDays_OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (vk!=VK_UP && vk!=VK_DOWN) {
	LPUpdateTopicData lpUpdateTopicData=GetDlgData(GetParent(hwnd), UpdateTopicData);

	WFORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags, lpUpdateTopicData->lpfnUpdateDaysOldProc);
    }
}



LONG _EXPORT CALLBACK updateDaysProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

//  Message handler for subclassing update days edit control - cursor up and
//  cursor down keystrokes shift control focus.

{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_KEYDOWN, updateDays_OnKeyDown);
    HANDLE_MSG(hwnd, WM_KEYUP, updateDays_OnKeyUp);

    default: {
	LPUpdateTopicData lpUpdateTopicData=GetDlgData(GetParent(hwnd), UpdateTopicData);

	return(WFORWARD_MESSAGE(hwnd, uMsg, wParam, lParam, lpUpdateTopicData->lpfnUpdateDaysOldProc));
    }

    }
}

