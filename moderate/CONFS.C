//  confs.c - Ameol moderator addon CIX conference and topic creation
//            © 1994 CIX Ltd



#include <windows.h>                    //  Windows API definitions
#include <windowsx.h>                   //  Windows API definitions
#include <stdio.h>                      //  For sscanf()
#include <dos.h>                        //  For _unlink()
#include <io.h>
#include <ctype.h>                      //  Character classification
#include "ameolapi.h"                   //  Ameol API definitions
#include "winhorus.h"                   //  Windows utility routines
#include "hctools.h"                    //  Ameol script routines
#include "setup.h"                      //  Setup (ameol.ini) routines and variables
#include "moderate.h"                   //  Resource definitions
#include "help.h"             		//  Help resource definitions
#include "globals.h"                    //  Global variable and routine definitions
#include "confs.h"



typedef struct {
    TopicSubclassData topicSubclass;
    BOOL bConfOK;                        //  conference/topic name is valid
    BOOL bInValidate;                    //  currently validating conf or topic
    BOOL bConfFocus;                     //  conf name has input focus
    BOOL bConfChanged;                   //  user has edited conf name
    TopicInfo topicInfo;                 //  conference topics
    Topic newTopic;                      //  new topic during creation
    LPTopic lpTopic;                	 //  current topic, if any
    LPTopic lpPrevTopic;
    BOOL bNewTopic;
    BOOL bFlistSet;
    BOOL bReadonlySet;
    BOOL bTopicFocus;                    //  topic name has input focus
    BOOL bTopicChanged;                  //  user has edited current topic name
    BOOL bTopicSet;
    BOOL bCheckFlag;
    int nHelpDisplayed;
} CreateConfData, FAR *LPCreateConfData;



typedef struct {
    HCONF hConference;
    LPTopic lpTopic;
} ConfirmDeleteData, FAR *LPConfirmDeleteData;



LONG _EXPORT CALLBACK topicNameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK pruneTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK deleteTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



void deleteConfFiles(HCONF hConf)

//  Delete .CTI file for conference (if it exists)

{
    CONFERENCEINFO confInfo;
    char szFileInfo[LEN_PATHNAME];

    GetConferenceInfo(hConf, &confInfo);
    wsprintf(szFileInfo, "%s\\%s.CTI", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
    _unlink(szFileInfo);
}



static BOOL matchName(LPCSTR lpcName, LPCSTR lpcCommand)
{
    while (*lpcName && *lpcName== *lpcCommand) {
        lpcName++;
        lpcCommand++;
    }

    return(*lpcName=='\0');
}



static BOOL reservedName(LPCSTR lpcName)
{
    switch (*lpcName) {

    case 'a':
        return(matchName(lpcName, "absence") ||
               matchName(lpcName, "accept") ||
               matchName(lpcName, "add") ||
               matchName(lpcName, "again") ||
               matchName(lpcName, "all") ||
               matchName(lpcName, "anon") ||
               matchName(lpcName, "ansi") ||
               matchName(lpcName, "april") ||
               matchName(lpcName, "arcscratch") ||
               matchName(lpcName, "august") ||
               matchName(lpcName, "autorecent"));

    case 'b':
        return(matchName(lpcName, "backward") ||
               matchName(lpcName, "binmail") ||
               matchName(lpcName, "bit8") ||
               matchName(lpcName, "buddy") ||
               matchName(lpcName, "bye"));

    case 'c':
        return(matchName(lpcName, "cc") ||
               matchName(lpcName, "change") ||
               matchName(lpcName, "chat") ||
               matchName(lpcName, "checkpoint") ||
               matchName(lpcName, "clear") ||
               matchName(lpcName, "comment") ||
               matchName(lpcName, "comod") ||
               matchName(lpcName, "continue") ||
               matchName(lpcName, "current"));

    case 'd':
        return(matchName(lpcName, "date") ||
               matchName(lpcName, "december") ||
               matchName(lpcName, "delete") ||
               matchName(lpcName, "dir") ||
               matchName(lpcName, "download"));

    case 'e':
        return(matchName(lpcName, "echo") ||
               matchName(lpcName, "edit") ||
               matchName(lpcName, "erase") ||
               matchName(lpcName, "export"));

    case 'f':
        return(matchName(lpcName, "fal") ||
               matchName(lpcName, "fdir") ||
               matchName(lpcName, "fdl") ||
               matchName(lpcName, "february") ||
               matchName(lpcName, "fed") ||
               matchName(lpcName, "fexport") ||
               matchName(lpcName, "ffind") ||
               matchName(lpcName, "file") ||
               matchName(lpcName, "filter") ||
               matchName(lpcName, "find") ||
               matchName(lpcName, "first") ||
               matchName(lpcName, "flist") ||
               matchName(lpcName, "forward") ||
               matchName(lpcName, "from") ||
               matchName(lpcName, "ful"));

    case 'g':
        return(matchName(lpcName, "go"));

    case 'h':
        return(matchName(lpcName, "header") ||
               matchName(lpcName, "help"));

    case 'i':
        return(matchName(lpcName, "inbasket") ||
               matchName(lpcName, "ixany"));

    case 'j':
        return(matchName(lpcName, "january") ||
               matchName(lpcName, "join") ||
               matchName(lpcName, "july") ||
               matchName(lpcName, "june"));

    case 'k':
        return(matchName(lpcName, "killarc") ||
               matchName(lpcName, "killscratch"));

    case 'l':
        return(matchName(lpcName, "last") ||
               matchName(lpcName, "leave") ||
               matchName(lpcName, "level") ||
               matchName(lpcName, "list"));

    case 'm':
        return(matchName(lpcName, "mail") ||
               matchName(lpcName, "main") ||
               matchName(lpcName, "march") ||
               matchName(lpcName, "markread") ||
               matchName(lpcName, "markunread") ||
               matchName(lpcName, "may") ||
               matchName(lpcName, "member") ||
               matchName(lpcName, "messagesize") ||
               matchName(lpcName, "misc") ||
               matchName(lpcName, "missing") ||
               matchName(lpcName, "moderate"));

    case 'n':
        return(matchName(lpcName, "new") ||
               matchName(lpcName, "news") ||
               matchName(lpcName, "next") ||
               matchName(lpcName, "note") ||
               matchName(lpcName, "november"));

    case 'o':
        return(matchName(lpcName, "october") ||
               matchName(lpcName, "olrname") ||
               matchName(lpcName, "olruse") ||
               matchName(lpcName, "option") ||
               matchName(lpcName, "original") ||
               matchName(lpcName, "orphans") ||
               matchName(lpcName, "outbasket"));

    case 'p':
        return(matchName(lpcName, "participants") ||
               matchName(lpcName, "password") ||
               matchName(lpcName, "prices") ||
               matchName(lpcName, "profile"));

    case 'q':
        return(matchName(lpcName, "quit"));

    case 'r':
        return(matchName(lpcName, "raw") ||
               matchName(lpcName, "rdonly") ||
               matchName(lpcName, "read") ||
               matchName(lpcName, "recent") ||
               matchName(lpcName, "reference") ||
               matchName(lpcName, "remove") ||
               matchName(lpcName, "rename") ||
               matchName(lpcName, "reply") ||
               matchName(lpcName, "resign") ||
               matchName(lpcName, "restore") ||
               matchName(lpcName, "resume") ||
               matchName(lpcName, "root") ||
               matchName(lpcName, "run"));

    case 's':
        return(matchName(lpcName, "say") ||
               matchName(lpcName, "scgadd") ||
               matchName(lpcName, "scget") ||
               matchName(lpcName, "script") ||
               matchName(lpcName, "scpadd") ||
               matchName(lpcName, "scput") ||
               matchName(lpcName, "scratchname") ||
               matchName(lpcName, "scratchpad") ||
               matchName(lpcName, "search") ||
               matchName(lpcName, "semi") ||
               matchName(lpcName, "send") ||
               matchName(lpcName, "september") ||
               matchName(lpcName, "show") ||
               matchName(lpcName, "skip") ||
               matchName(lpcName, "squick") ||
               matchName(lpcName, "status") ||
               matchName(lpcName, "subject") ||
               matchName(lpcName, "substitute") ||
               matchName(lpcName, "switchtopic") ||
               matchName(lpcName, "synonym"));

    case 't':
        return(matchName(lpcName, "terminal") ||
               matchName(lpcName, "terminate") ||
               matchName(lpcName, "terse") ||
               matchName(lpcName, "thread") ||
               matchName(lpcName, "time") ||
               matchName(lpcName, "timeout") ||
               matchName(lpcName, "tnext") ||
               matchName(lpcName, "to") ||
               matchName(lpcName, "topic") ||
               matchName(lpcName, "tprevious") ||
               matchName(lpcName, "type"));

    case 'u':
        return(matchName(lpcName, "upload"));

    case 'v':
        return(matchName(lpcName, "verbose") ||
               matchName(lpcName, "version") ||
               matchName(lpcName, "view") ||
               matchName(lpcName, "vote"));

    case 'w':
        return(matchName(lpcName, "who") ||
               matchName(lpcName, "withdraw") ||
               matchName(lpcName, "workfile"));

    case 'x':
        return(matchName(lpcName, "xtra"));

    case 'z':
        return(matchName(lpcName, "zwindow"));

    default:
        return(FALSE);

    }
}



static BOOL conferenceExists(HWND hwnd, LPSTR lpConfName)

//  Check if conference exists locally or in the full CIX conf list

{
    HCONF hConf;
    HAMLIST hAmConfList;
    ConfListEntry confListEntry;
    int nStatus;

    //  Scan local conference list...

    for (hConf=GetConference(NULL);
	 hConf && lstrcmp(GetConferenceName(hConf), lpConfName);
	 hConf=GetConference(hConf));

    if (hConf)
	return(TRUE);

    //  Open CIX conference list...

    hAmConfList=OpenConferenceList();

    if (!hAmConfList) {
	// Can't find it...
	alert(hwnd, "WARNING:\r\n\r\n"
		    "The full CIX forums list is not available offline; "
		    "your new forum name has only been checked against "
		    "your offline messagebase\r\n\r\n"
		    "Use the Ameol CIX Forums/Show All Forums... command to download a "
		    "copy of the full CIX forums list");

	return(FALSE);
    }

    busy(TRUE);

    while ((nStatus=ReadConferenceList(hAmConfList, &confListEntry))==0)
	if (lstrcmp(confListEntry.szConfName, lpConfName)==0) {
	    //  We've found it...
	    busy(FALSE);
	    CloseConferenceList(hAmConfList);
	    return(TRUE);
	}

    busy(FALSE);

    if (nStatus!=STREAM_EOF)
	alert(hwnd, "Error status from ReadConferenceList(): %d", nStatus);

    //  No match found...
    CloseConferenceList(hAmConfList);
    return(FALSE);
}



static BOOL validateConfName(HWND hwnd, LPSTR lpConfName)

//  Validate conf name - check for invalid characters, illegal names and
//  if the beast already exists.

{
    LPSTR ptr=lpConfName;

    //  Strip off leading spaces...
    while (*ptr==' ')
        ptr++;

    if (*ptr) {
        //  ...and trailing spaces...
        LPSTR nameEnd=ptr+lstrlen(ptr)-1;

        while (*nameEnd==' ')
            --nameEnd;

        *(nameEnd+1)='\0';

        if (ptr!=lpConfName) {
            //  ...and copy the name back where it came from
            LPSTR copyPtr=lpConfName;

            do
                *copyPtr++ = *ptr++;
            while (*ptr);
        }
    } else
        //  empty string - invalid
        return(FALSE);

    if (*lpConfName=='.') {
	//  can't start with a '.'
	alert(hwnd, "Forum name \"%s\" starts with a '.'", lpConfName);
	return(FALSE);
    }

    for (ptr=lpConfName;
	 *ptr &&
	 //  valid characters in conf name:
	 !(*ptr&0x80) &&
	 (isalnum(*ptr) || *ptr=='.' || *ptr=='-' || *ptr=='_' || *ptr=='+' || *ptr=='!');
	 ptr++);

    if (*ptr) {
	//  We've found a gash character
	alert(hwnd, "Illegal character '%c' in forum name \"%s\"", *ptr, lpConfName);
	return(FALSE);
    }

    //  Check for reserved names...

    if (reservedName(lpConfName) ||
        lstrcmp(lpConfName, "usenet")==0) {
        alert(hwnd, "Forum name \"%s\" matches a CIX reserved word", lpConfName);
        return(FALSE);
    }

    //  Check to see if the conf already exists...

    if (conferenceExists(hwnd, lpConfName)) {
	alert(hwnd, "Forum \"%s\" already exists", lpConfName);
        return(FALSE);
    }

    return(TRUE);
}



static BOOL validateTopicName(HWND hwnd, LPSTR lpTopicName)

//  Validate topic name - check for invalid characters and illegal names

{
    LPSTR ptr=lpTopicName;

    //  Strip off leading spaces...
    while (*ptr==' ')
        ptr++;

    if (*ptr) {
        //  ...and trailing spaces...
        LPSTR nameEnd=ptr+lstrlen(ptr)-1;

        while (*nameEnd==' ')
            --nameEnd;

        *(nameEnd+1)='\0';

        if (ptr!=lpTopicName) {
            //  ...and copy the name back where it came from
            LPSTR copyPtr=lpTopicName;

            do
                *copyPtr++ = *ptr++;
	    while (*ptr);
	}
    } else
	//  empty string - OK
	return(TRUE);

    if (*lpTopicName=='.') {
	//  can't start with a '.'
	alert(hwnd, "Topic name \"%s\" starts with a '.'", lpTopicName);
	return(FALSE);
    }

    for (ptr=lpTopicName;
	 *ptr &&
	 //  valid characters in topic name:
	 !(*ptr&0x80) &&
	 (isalnum(*ptr) || *ptr=='.' || *ptr=='-' || *ptr=='_' || *ptr=='+' || *ptr=='!');
	 ptr++);

    if (*ptr) {
	//  We've found a gash character
	alert(hwnd, "Illegal character '%c' in topic name \"%s\"", *ptr, lpTopicName);
	return(FALSE);
    }

    //  Check for reserved names...

    if (reservedName(lpTopicName)) {
        alert(hwnd, "Topic name \"%s\" matches a CIX reserved word", lpTopicName);
        return(FALSE);
    }

    return(TRUE);
}



static void drawConfName(const DRAWITEMSTRUCT FAR * lpDis)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rc;
    LPCSTR lpcConfName=GetConferenceName((HCONF)lpDis->itemData);

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
    DrawText(lpDis->hDC, lpcConfName, -1, &rc, DT_LEFT|DT_SINGLELINE);
    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



void drawTopicName(const DRAWITEMSTRUCT FAR * lpDis)
{
    HBRUSH hBrush;
    COLORREF newBg, oldBg, newFg, oldFg;
    RECT rc;
    LPTopic lpTopic=(LPTopic)lpDis->itemData;
    LPSTR lpText=lpTopic ? lpTopic->topicName : (LPSTR)"<all topics>";

    CopyRect(&rc, &lpDis->rcItem);
    InflateRect(&rc, -2, 0);

    if (lpDis->itemState&ODS_FOCUS || lpDis->itemState&ODS_SELECTED)
        newBg=GetSysColor(COLOR_HIGHLIGHT);
    else
        newBg=GetSysColor(COLOR_WINDOW);

    if (lpTopic && lpTopic->topicFlags&TOPIC_DELETE)
        newFg=GetSysColor(COLOR_GRAYTEXT);
    else if (lpDis->itemState&ODS_FOCUS || lpDis->itemState&ODS_SELECTED)
        newFg=GetSysColor(COLOR_HIGHLIGHTTEXT);
    else
        newFg=GetSysColor(COLOR_WINDOWTEXT);

    oldBg=(COLORREF)SetBkColor(lpDis->hDC, newBg);
    oldFg=(COLORREF)SetTextColor(lpDis->hDC, newFg);
    hBrush=CreateSolidBrush(newBg);
    FillRect(lpDis->hDC, &lpDis->rcItem, hBrush);
    DeleteObject(hBrush);
    DrawText(lpDis->hDC, lpText, -1, &rc, DT_LEFT|DT_SINGLELINE);
    SetBkColor(lpDis->hDC, oldBg);
    SetTextColor(lpDis->hDC, oldFg);
}



static LPTopic findNextTopic(HWND hwnd, LPTopicInfo lpTopicInfo, BOOL bSkip)
{
    LPTopic lpTopic=NULL;

    if (lpTopicInfo->totalCount) {
        HWND hwndTopic=GetDlgItem(hwnd, CID_TOPIC);
        int nCurrentIndex=ListBox_GetCurSel(hwndTopic);
        int nIndex=nCurrentIndex<0 ? 0 : bSkip ? (nCurrentIndex+1)%lpTopicInfo->totalCount : nCurrentIndex;
        char szTopic[16];

        GetDlgItemText(hwnd, CID_TOPIC_NAME, szTopic, 15);
	lpTopic=findTopic(lpTopicInfo, szTopic, nIndex);

        if (!lpTopic)
            ListBox_SetCurSel(hwndTopic, -1);
        else if (nCurrentIndex!=lpTopic->topicIndex)
            ListBox_SetCurSel(hwndTopic, lpTopic->topicIndex);

    }

    if (lpTopic)
        selectTopic(hwnd, lpTopicInfo, lpTopic->topicIndex, FALSE);
    else {
	selectTopic(hwnd, lpTopicInfo, -1, FALSE);
	MessageBeep(0);
    }

    return(lpTopic);
}



static void createNewConference(HWND hwnd, HCONF hConf, LPCreateConfData lpCreateConfData)
{
    char szNickName[40];
    int nTopicPtr;
    BOOL bConfidential=IsDlgButtonChecked(hwnd, CID_CONF_CONFIDENTIAL);
    BOOL bClosed=bConfidential || IsDlgButtonChecked(hwnd, CID_CONF_CLOSED);
    HSCRIPT hScript=initScript("Moderate", FALSE);
    GETPARLISTOBJECT getparlistObject;
    char szConfCreated[ELOG_DESCRIPTION+1];
    char szConfNote[LEN_PATHNAME];

    busy(TRUE);

    //  Mark user as moderator
	 lstrcpy( szNickName, "$CIX$" );
    GetRegistry(szNickName);
    GetSetModeratorList(hConf, szNickName, TRUE);

    //  write creation script
    addToScript(hScript, "put `mod new %s`¬"
			 "if waitfor(`Closed (y/n)?`, `M:`) == 0¬"
			 "put `%s`¬",
		lpCreateConfData->topicInfo.szConfName, (LPSTR)(bClosed ? "yes" : "no"));

    if (bClosed)
	addToScript(hScript, "waitfor `Confidential (y/n)?`¬"
			     "put `%s`¬",
		    (LPSTR)(bConfidential ? "yes" : "no"));

    if (!bConfidential) 
	{  //  no description for CC confs...
		char szConfDescription[DESC_LENGTH];

		GetDlgItemText(hwnd, CID_CONF_DESCRIPTION, szConfDescription, DESC_LENGTH);

		addToScript(hScript, "waitfor `Description:`¬"
			    "put `%s`¬",
				(LPSTR)szConfDescription);

    }

    for (nTopicPtr=0; nTopicPtr<lpCreateConfData->topicInfo.totalCount; nTopicPtr++) 
	{
	//  create topics...
		LPTopic lpTopic=lpCreateConfData->topicInfo.topics[nTopicPtr];

		if (!(lpTopic->topicFlags&TOPIC_DELETE))
		{
			addToScript(hScript, "waitfor `opicname:`¬"
				 "put `%s`¬"
				 "if waitfor (`FLIST) (y/n)?`, `Sorry`) == 0¬"
				 "  put `%s`¬"
				 "  waitfor `Description of`¬"
				 "  put `%s`¬"
				 "endif¬",
			lpTopic->topicName,
			(LPSTR)(lpTopic->topicFlags&TOPIC_FILES ? "yes" : "no"),
			lpTopic->topicDescription);
		}

    }

    addToScript(hScript, "waitfor `opicname:`¬"
			             "put `quit`¬");

    for (nTopicPtr=0; nTopicPtr<lpCreateConfData->topicInfo.totalCount; nTopicPtr++) 
	{
		//  mark read only topics as such...
		LPTopic lpTopic=lpCreateConfData->topicInfo.topics[nTopicPtr];

		if (!(lpTopic->topicFlags&TOPIC_DELETE))
		{
			if (lpTopic->topicFlags&TOPIC_READONLY)
			{
				addToScript(hScript, "waitfor `Mod:`¬"
							         "put `rdonly %s`¬",
						             lpTopic->topicName);
			}
		}
    }

    addToScript(hScript, "waitfor `Mod:`¬"
			 "put `quit`¬"
			 "waitfor `M:`¬",
			 "endif¬");


    actionScript(hScript, OT_PREINCLUDE, "create conference %s",
		lpCreateConfData->topicInfo.szConfName);

    //  Get initial participant and moderator lists
    InitObject(&getparlistObject, OT_GETPARLIST, GETPARLISTOBJECT);
    lstrcpy(getparlistObject.szConfName, lpCreateConfData->topicInfo.szConfName);

    if (!FindObject(&getparlistObject))
		PutObject(NULL, &getparlistObject, NULL);

    wsprintf(szConfNote, "%s\\newconf.CNO", (LPSTR)setup.szDataDir);

    if (_access(szConfNote, 0)==0) 
	{
		CONFERENCEINFO confInfo;
		char szRealNote[LEN_PATHNAME];

		GetConferenceInfo(hConf, &confInfo);
		wsprintf(szRealNote, "%s\\%s.CNO", (LPSTR)setup.szDataDir, (LPSTR)confInfo.szFileName);
		_unlink(szRealNote);
		rename(szConfNote, szRealNote);
    }

    busy(FALSE);

    //  Create topic information file
    if (!saveTopicInfo(hwnd, hConf, &lpCreateConfData->topicInfo))
	{
	//  can't create .CTI file...
		alert(hwnd, "Cannot create topic information file for forum %s\r\n\r\n",
			lpCreateConfData->topicInfo.szConfName);
	}
	if (setup.bCreateConfHelp) 
	{
		setup.bCreateConfHelp=FALSE;
		AmWritePrivateProfileString("Moderate", "createconfhelp", "no", setup.szIniFile);
	}

	wsprintf(szConfCreated, "%s forum %s created",
			(LPSTR)(bConfidential ? "Closed Confidential" :
			bClosed       ? "Closed"              :
			"Open"),
			lpCreateConfData->topicInfo.szConfName);

	AddEventLogItem(ETYP_MESSAGE|ETYP_DATABASE, szConfCreated);
}



static void newConfControls(HWND hwnd, LPCreateConfData lpCreateConfData)
{
    BOOL bNewTopic=lpCreateConfData->bNewTopic;
    BOOL bTopicSet=lpCreateConfData->bTopicSet;
    BOOL bConfOK=lpCreateConfData->bConfOK;
    BOOL bDeleted=lpCreateConfData->lpTopic && lpCreateConfData->lpTopic->topicFlags&TOPIC_DELETE;
    BOOL bHasTopics=lpCreateConfData->topicInfo.totalCount>0;
    BOOL bHasRealTopics=lpCreateConfData->topicInfo.topicCount>0;
    BOOL bConfTypeSet = RadioButton(hwnd, CID_CONF_OPEN, 0) != 0;

    SetDlgItemText(hwnd, CID_ADD, bNewTopic ? "&Add" : "Ne&w topic");
    SetDlgItemText(hwnd, CID_DELETE, bDeleted ? "Unde&lete" : "De&lete");
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), (bNewTopic || bHasTopics) && bConfOK);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC), bHasTopics && bConfOK);
    EnableWindow(GetDlgItem(hwnd, CID_ADD), (bTopicSet || !bNewTopic) && bConfOK);
    EnableWindow(GetDlgItem(hwnd, CID_DELETE), bNewTopic || bHasTopics);
    EnableWindow(GetDlgItem(hwnd, CID_CONF_NOTE), bConfOK);
    EnableWindow(GetDlgItem(hwnd, CID_EDIT_SEED), bTopicSet && !(bNewTopic || bDeleted));
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_FLIST), bTopicSet && !bDeleted);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_READONLY), bTopicSet && !bDeleted);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_DESC_TAG), bTopicSet && !bDeleted);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_DESCRIPTION), bTopicSet && !bDeleted);
    EnableWindow(GetDlgItem(hwnd, CID_OK), !lpGlobals->bEvaluation && bConfOK && bHasRealTopics && bConfTypeSet);
}



static BOOL createConf_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPCreateConfData lpCreateConfData=InitDlgData(hwnd, CreateConfData);
    char szConfNote[LEN_PATHNAME];

    //  Initialise flags and variables
    lpCreateConfData->topicSubclass.lpfnTopicNameProc=NULL;
    lpCreateConfData->topicSubclass.hwndTopicList=NULL;
    lpCreateConfData->topicSubclass.lpfnTopicListProc=NULL;
    lpCreateConfData->topicSubclass.lpfnTopicNameOldProc=NULL;
    lpCreateConfData->bConfOK=FALSE;
    lpCreateConfData->bInValidate=FALSE;
    lpCreateConfData->bConfFocus=FALSE;
    lpCreateConfData->bConfChanged=FALSE;
    lpCreateConfData->bNewTopic=FALSE;
    lpCreateConfData->bTopicFocus=FALSE;
    lpCreateConfData->bTopicChanged=FALSE;
    lpCreateConfData->bTopicSet=FALSE;
    lpCreateConfData->bFlistSet=FALSE;
    lpCreateConfData->bReadonlySet=FALSE;
    lpCreateConfData->bCheckFlag=FALSE;
    lpCreateConfData->nHelpDisplayed=0;
    initTopicInfo(&lpCreateConfData->topicInfo);
    initTopic(&lpCreateConfData->newTopic, &lpCreateConfData->topicInfo.topicHeader);
    lpCreateConfData->newTopic.topicFlags|=TOPIC_NEWCONF;
    lpCreateConfData->lpTopic=NULL;
    lpCreateConfData->lpPrevTopic=NULL;
    lpCreateConfData->topicInfo.changed=TRUE;

    //  Subclass topic name edit control - we need the window handle and message
    //  handler for the topic list list box as they are referred to in topicNameProc()
    lpCreateConfData->topicSubclass.lpfnTopicNameProc=MakeProcInstance((FARPROC)topicNameProc, lpGlobals->hInst);
    lpCreateConfData->topicSubclass.hwndTopicList=GetDlgItem(hwnd, CID_TOPIC);
    lpCreateConfData->topicSubclass.lpfnTopicListProc=GetWindowProc(lpCreateConfData->topicSubclass.hwndTopicList);
    lpCreateConfData->topicSubclass.lpfnTopicNameOldProc=SubclassWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), lpCreateConfData->topicSubclass.lpfnTopicNameProc);

    //  Initialise buttons
    RadioButton(hwnd, CID_CONF_OPEN, -1);
    CheckDlgButton(hwnd, CID_TOPIC_FLIST, FALSE);
    CheckDlgButton(hwnd, CID_TOPIC_READONLY, FALSE);

    wsprintf(szConfNote, "%s\\newconf.CNO", (LPSTR)setup.szDataDir);
    _unlink(szConfNote);

    //  Limit text length for conf/topic names and descriptions
    Edit_LimitText(GetDlgItem(hwnd, CID_CONF_NAME), 14);
    Edit_LimitText(GetDlgItem(hwnd, CID_CONF_DESCRIPTION), DESC_LENGTH);
    Edit_LimitText(GetDlgItem(hwnd, CID_TOPIC_NAME), 14);
    Edit_LimitText(GetDlgItem(hwnd, CID_TOPIC_DESCRIPTION), DESC_LENGTH);

    newConfControls(hwnd, lpCreateConfData);
    return(TRUE);
}



static void createConf_OnPaint(HWND hwnd)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    if ((lpGlobals->bEvaluation || setup.bCreateConfHelp) &&
        lpCreateConfData->nHelpDisplayed<1) 
	{
		lpCreateConfData->nHelpDisplayed++;
		Post_Moderate_CreateConfHelp(hwnd);
    }

    DefDlgProcEx(hwnd, WM_PAINT, 0, 0L, &lpGlobals->bRecursionFlag);
}



static void createConf_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void createConf_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR * lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
		drawTopicName(lpDrawItem);
}



static void createConf_OnCreateConfHelp(HWND hwnd)
{
    HtmlHelp(hwnd, setup.szHelpFile, HH_HELP_CONTEXT, HID_CREATING_A_CONFERENCE);
}



static void createConf_OnCheckConf(HWND hwnd)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    lpCreateConfData->bConfChanged=FALSE;

    //  get and validate conf name
    GetDlgItemText(hwnd, CID_CONF_NAME, lpCreateConfData->topicInfo.szConfName, 15);
    lpCreateConfData->bConfOK=validateConfName(hwnd, lpCreateConfData->topicInfo.szConfName);

    //  focus sticks on the conf name if it's bad...
    if (!lpCreateConfData->bConfOK)
        NextDlgCtl(hwnd, CID_CONF_NAME);

    newConfControls(hwnd, lpCreateConfData);
    lpCreateConfData->bInValidate=FALSE;
}



static void createConf_OnCheckTopic(HWND hwnd)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    lpCreateConfData->bTopicChanged=FALSE;

    if (lpCreateConfData->bNewTopic) {
        lpCreateConfData->lpTopic=NULL;

	//  get and validate topic name
	GetDlgItemText(hwnd, CID_TOPIC_NAME, lpCreateConfData->newTopic.topicName, 15);
	lpCreateConfData->bTopicSet= *lpCreateConfData->newTopic.topicName!='\0';

	if (!validateTopicName(hwnd, lpCreateConfData->newTopic.topicName))
	    //  focus sticks on the topic name if it's bad...
	    NextDlgCtl(hwnd, CID_TOPIC_NAME);
	else {
	    if (lpCreateConfData->topicInfo.totalCount && *lpCreateConfData->newTopic.topicName)
		//  see if the user has entered a name already added...
		lpCreateConfData->lpTopic=findTopic(&lpCreateConfData->topicInfo,
						    lpCreateConfData->newTopic.topicName, -1);

	    if (lpCreateConfData->lpTopic) {
		selectTopic(hwnd, &lpCreateConfData->topicInfo, lpCreateConfData->lpTopic->topicIndex, TRUE);
		lpCreateConfData->bNewTopic=FALSE;
		lpCreateConfData->bTopicSet=TRUE;
		lpCreateConfData->lpPrevTopic=NULL;
		MessageBeep(0);
	    } else {
		if (!lpCreateConfData->bFlistSet)
		    if (lstrcmp(lpCreateConfData->newTopic.topicName, "files")==0)
			lpCreateConfData->newTopic.topicFlags|=TOPIC_FILES;
		    else
			lpCreateConfData->newTopic.topicFlags&= ~TOPIC_FILES;

		if (!lpCreateConfData->bReadonlySet)
		    if (lstrcmp(lpCreateConfData->newTopic.topicName, "files")==0 ||
			lstrcmp(lpCreateConfData->newTopic.topicName, "digest")==0 ||
			lstrcmp(lpCreateConfData->newTopic.topicName, "announce")==0 ||
			lstrcmp(lpCreateConfData->newTopic.topicName, "announcements")==0)
			lpCreateConfData->newTopic.topicFlags|=TOPIC_READONLY;
		    else
			lpCreateConfData->newTopic.topicFlags&= ~TOPIC_READONLY;

		displayTopic(hwnd, &lpCreateConfData->newTopic, FALSE);
	    }
	}
    } else if (!(lpCreateConfData->lpTopic=findNextTopic(hwnd, &lpCreateConfData->topicInfo, FALSE)))
	NextDlgCtl(hwnd, CID_TOPIC_NAME);

    newConfControls(hwnd, lpCreateConfData);
    lpCreateConfData->bInValidate=FALSE;
}



static void createConf_OnKillFocus(HWND hwnd, HWND hwndNext)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    if (lpCreateConfData->bTopicChanged) {
	int nNext=GetDlgCtrlID(hwndNext);

        lpCreateConfData->bCheckFlag=nNext==CID_TOPIC_FLIST && !lpCreateConfData->bFlistSet ||
				     nNext==CID_TOPIC_READONLY && !lpCreateConfData->bReadonlySet;

    }
}



static void createConf_OnNextTopic(HWND hwnd)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    //  user has pressed spacebar
    if (lpCreateConfData->bNewTopic)
	MessageBeep(0);
    else {
        lpCreateConfData->lpTopic=findNextTopic(hwnd, &lpCreateConfData->topicInfo, TRUE);
        newConfControls(hwnd, lpCreateConfData);
    }
}



static void createConf_OnResetControls(HWND hwnd)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    newConfControls(hwnd, lpCreateConfData);
}



static LPCSTR createConf_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_CREATE_CONF, id));
}



static void createConf_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_CREATE_CONF);
}



static void createConf_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    switch (id) {

    case CID_CONF_NAME:
	//  Conference name edit control...
		switch (codeNotify) 
		{

			case EN_SETFOCUS:
				lpCreateConfData->bConfFocus=TRUE;
				break;

			case EN_CHANGE:
				if (lpCreateConfData->bConfFocus) 
				{
				//  user has edited conf name
					lpCreateConfData->bConfChanged=TRUE;
					GetWindowText(hwndCtl, lpCreateConfData->topicInfo.szConfName, 15);

					if (xor(*lpCreateConfData->topicInfo.szConfName, lpCreateConfData->bConfOK)) 
					{
					//  "OK" button is enabled if the conf name is not empty and at
					//  least 1 topic has been defined
						lpCreateConfData->bConfOK= !lpCreateConfData->bConfOK;

						EnableWindow(GetDlgItem(hwnd, CID_OK),
							!lpGlobals->bEvaluation &&
							lpCreateConfData->bConfOK && lpCreateConfData->topicInfo.topicCount);
			
					}
				}

				break;

			case EN_KILLFOCUS:
			{
				lpCreateConfData->bConfFocus=FALSE;

				if (!lpCreateConfData->bInValidate && lpCreateConfData->bConfChanged) 
				{
					lpCreateConfData->bInValidate=TRUE;
					Post_Moderate_CheckConf(hwnd);
				}

				break;
			}

		}

		break;

    case CID_CONF_OPEN:
    case CID_CONF_CLOSED:
    case CID_CONF_CONFIDENTIAL:
        if (codeNotify==BN_CLICKED)
	    Post_Moderate_ResetControls(hwnd);

	break;

    case CID_TOPIC_NAME:
        //  Topic name edit control...
	switch (codeNotify) {

	case EN_SETFOCUS:
	    lpCreateConfData->bTopicFocus=TRUE;
	    break;

	case EN_CHANGE:
	    if (lpCreateConfData->bTopicFocus) {
		//  user has edited topic name
		lpCreateConfData->bTopicChanged=TRUE;

		if (lpCreateConfData->bNewTopic) {
		    GetWindowText(hwndCtl, lpCreateConfData->newTopic.topicName, 15);
		    lpCreateConfData->bTopicSet= *lpCreateConfData->newTopic.topicName!='\0';
		    newConfControls(hwnd, lpCreateConfData);
		} else if (!lpCreateConfData->bInValidate) {
		    lpCreateConfData->bInValidate=TRUE;
		    lpCreateConfData->lpTopic=findNextTopic(hwnd, &lpCreateConfData->topicInfo, FALSE);
		    newConfControls(hwnd, lpCreateConfData);
		    lpCreateConfData->bInValidate=FALSE;
		}
	    }

	    break;

	case EN_KILLFOCUS:
	    lpCreateConfData->bTopicFocus=FALSE;

	    if (!lpCreateConfData->bInValidate && lpCreateConfData->bTopicChanged) {
		lpCreateConfData->bInValidate=TRUE;
		Post_Moderate_CheckTopic(hwnd);
	    }

	    break;

	}

	break;

    case CID_TOPIC:
	//  Topic list list box...
        switch (codeNotify) {

	case LBN_SELCHANGE: {
            //  selection has changed...
            int topicIndex=ListBox_GetCurSel(hwndCtl);

            lpCreateConfData->lpTopic=selectTopic(hwnd, &lpCreateConfData->topicInfo, topicIndex, TRUE);
            lpCreateConfData->bTopicSet=TRUE;
            lpCreateConfData->bNewTopic=FALSE;
            newConfControls(hwnd, lpCreateConfData);
            break;
	}

	}

        break;

    case CID_ADD:
        //  Add topic button...
        if (lpCreateConfData->bConfOK)
            if (!lpCreateConfData->bNewTopic) {
                saveTopic(hwnd, lpCreateConfData->lpTopic);
		lpCreateConfData->lpPrevTopic=lpCreateConfData->lpTopic;
                initTopic(&lpCreateConfData->newTopic, &lpCreateConfData->topicInfo.topicHeader);
                lpCreateConfData->newTopic.topicFlags|=TOPIC_NEWCONF;
		lpCreateConfData->bTopicChanged=FALSE;
                lpCreateConfData->bFlistSet=FALSE;
                lpCreateConfData->bReadonlySet=FALSE;
		lpCreateConfData->bNewTopic=TRUE;
                lpCreateConfData->bTopicSet=FALSE;
		lpCreateConfData->lpTopic=NULL;
		selectTopic(hwnd, &lpCreateConfData->topicInfo, -1, TRUE);
                NextDlgCtl(hwnd, CID_TOPIC_NAME);
            } else if (!lpCreateConfData->lpTopic && lpCreateConfData->bTopicSet) {
                lpCreateConfData->bNewTopic=FALSE;
                lpCreateConfData->lpTopic=allocateTopic(&lpCreateConfData->topicInfo, &lpCreateConfData->newTopic);
		saveTopic(hwnd, lpCreateConfData->lpTopic);
                lpCreateConfData->lpPrevTopic=NULL;

                //  Add topic name to list box
		ListBox_AddItemData(GetDlgItem(hwnd, CID_TOPIC), lpCreateConfData->lpTopic);
                selectTopic(hwnd, &lpCreateConfData->topicInfo, lpCreateConfData->lpTopic->topicIndex, FALSE);
		writeSeedMessage(hwnd, &lpCreateConfData->topicInfo, setup.bEditSeedMessages);
            }

        newConfControls(hwnd, lpCreateConfData);
        break;

    case CID_DELETE:
	//  Delete topic button...
        if (lpCreateConfData->bNewTopic) {
	    lpCreateConfData->lpTopic=lpCreateConfData->lpPrevTopic;
            lpCreateConfData->bTopicSet=lpCreateConfData->lpTopic!=NULL;
            lpCreateConfData->bNewTopic=FALSE;
            lpCreateConfData->lpPrevTopic=NULL;
        } else if (lpCreateConfData->lpTopic) {
            HWND hwndTopic=GetDlgItem(hwnd, CID_TOPIC);
	    RECT rect;

	    saveTopic(hwnd, lpCreateConfData->lpTopic);
	    lpCreateConfData->lpTopic->topicFlags^=TOPIC_DELETE;

            if (lpCreateConfData->lpTopic->topicFlags&TOPIC_DELETE)
                lpCreateConfData->topicInfo.topicCount--;
            else
                lpCreateConfData->topicInfo.topicCount++;

            if (ListBox_GetItemRect(hwndTopic, lpCreateConfData->lpTopic->topicIndex, &rect)!=LB_ERR)
                InvalidateRect(hwndTopic, &rect, FALSE);

        }

        displayTopic(hwnd, lpCreateConfData->lpTopic, TRUE);
        newConfControls(hwnd, lpCreateConfData);

        if (lpCreateConfData->topicInfo.totalCount==0)
            NextDlgCtl(hwnd, CID_ADD);

        break;

    case CID_CONF_NOTE:
        // Conf Note button...
	editConfNote(hwnd, &lpCreateConfData->topicInfo, NULL);
        break;

    case CID_EDIT_SEED:
	if (lpCreateConfData->lpTopic && lpCreateConfData->lpTopic->hoobSeedMessage) {
            saveTopic(hwnd, lpCreateConfData->lpTopic);
	    editSeedMessage(hwnd, &lpCreateConfData->topicInfo);
	}

        break;

    case CID_TOPIC_FLIST: {
        LPTopic lpTopic=lpCreateConfData->bNewTopic ? &lpCreateConfData->newTopic : lpCreateConfData->lpTopic;

		if (lpTopic)
		{
			if (!(lpCreateConfData->bCheckFlag && lpTopic->topicFlags&TOPIC_FILES))
				lpTopic->topicFlags^=TOPIC_FILES;

			CheckDlgButton(hwnd, CID_TOPIC_FLIST, lpTopic->topicFlags&TOPIC_FILES);

			if (codeNotify==BN_CLICKED)
				lpCreateConfData->bFlistSet=TRUE;

			lpCreateConfData->bCheckFlag=FALSE;
		}
		else
		{
			alert(hwnd, "Error Invalid Topic");
		}
		break;
    }

    case CID_TOPIC_READONLY: {
	LPTopic lpTopic;
		
	if(lpCreateConfData->bNewTopic)
		lpTopic = &lpCreateConfData->newTopic;
	else if ( lpCreateConfData->lpTopic != NULL)
		lpTopic = lpCreateConfData->lpTopic;
	else
		lpTopic = &lpCreateConfData->newTopic;

	if (lpTopic)
	{
		if (!(lpCreateConfData->bCheckFlag && lpTopic->topicFlags&TOPIC_READONLY))
			lpTopic->topicFlags^=TOPIC_READONLY;

		CheckDlgButton(hwnd, CID_TOPIC_READONLY, lpTopic->topicFlags&TOPIC_READONLY);

		if (codeNotify==BN_CLICKED)
			lpCreateConfData->bReadonlySet=TRUE;

		lpCreateConfData->bCheckFlag=FALSE;
	}
	else
	{
		alert(hwnd, "Error Invalid Topic");
	}
	break;
    }

    case CID_OK: {
	//  OK button - create new conference if all is well
	BOOL bOK= !lpGlobals->bEvaluation && lpCreateConfData->bConfOK;

	if (bOK && !lpCreateConfData->lpTopic && lpCreateConfData->bTopicSet)
	    //  current topic not added...
	    switch (query(hwnd, MB_YESNOCANCEL,
			  "You haven't added topic %s to forum %s\r\n\r\n"
			  "Do you want to add it?",
			  lpCreateConfData->newTopic.topicName,
			  lpCreateConfData->topicInfo.szConfName)) {

	    case IDYES:
		lpCreateConfData->bNewTopic=FALSE;
		lpCreateConfData->lpTopic=allocateTopic(&lpCreateConfData->topicInfo, &lpCreateConfData->newTopic);
		saveTopic(hwnd, lpCreateConfData->lpTopic);
		lpCreateConfData->lpPrevTopic=NULL;
		ListBox_AddString(GetDlgItem(hwnd, CID_TOPIC), lpCreateConfData->lpTopic);
		selectTopic(hwnd, &lpCreateConfData->topicInfo, lpCreateConfData->lpTopic->topicIndex, FALSE);
		writeSeedMessage(hwnd, &lpCreateConfData->topicInfo, setup.bEditSeedMessages);
		break;

	    case IDCANCEL:
		bOK=FALSE;
		break;

	    }

	if (bOK) {
	    //  create conference locally
	    HCONF hConf;

	    busy(TRUE);
	    saveTopic(hwnd, lpCreateConfData->lpTopic);
	    hConf=CreateConference(lpCreateConfData->topicInfo.szConfName);
	    busy(FALSE);

	    if (hConf ||
		//  couldn't create local conference...
		query(hwnd, MB_YESNO,
		      "Error creating offline forum %s\r\n\r\n"
		      "Continue with creation on CIX?",
		      lpCreateConfData->topicInfo.szConfName))
		createNewConference(hwnd, hConf, lpCreateConfData);
	    else {
		char szConfNote[LEN_PATHNAME];

		wsprintf(szConfNote, "%s\\newconf.CNO", (LPSTR)setup.szDataDir);
		_unlink(szConfNote);
		bOK=FALSE;
	    }

	    deleteSeedMessages(&lpCreateConfData->topicInfo, !bOK);
	    EndDialog(hwnd, 0);
	}

	break;
    }

    case CID_CANCEL: {
	//  Cancel button
	char szConfNote[LEN_PATHNAME];

	wsprintf(szConfNote, "%s\\newconf.CNO", (LPSTR)setup.szDataDir);
	_unlink(szConfNote);
	deleteSeedMessages(&lpCreateConfData->topicInfo, TRUE);
	EndDialog(hwnd, 0);
        break;
    }

    }
}



static void createConf_OnDestroy(HWND hwnd)
{
    LPCreateConfData lpCreateConfData=GetDlgData(hwnd, CreateConfData);

    if (lpCreateConfData->topicSubclass.lpfnTopicNameOldProc) {
        SubclassWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), lpCreateConfData->topicSubclass.lpfnTopicNameOldProc);
        FreeProcInstance(lpCreateConfData->topicSubclass.lpfnTopicNameProc);
    }

    freeTopicInfo(&lpCreateConfData->topicInfo);
    FreeDlgData(hwnd);
}



static LRESULT createConf_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

//  Create CIX Conference dialog message handler

{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, createConf_OnInitDialog);
    HANDLE_MSG(hwnd, WM_PAINT, createConf_OnPaint);
    HANDLE_MSG(hwnd, WM_MEASUREITEM, createConf_OnMeasureItem);
    HANDLE_MSG(hwnd, WM_DRAWITEM, createConf_OnDrawItem);
    HANDLE_MSG(hwnd, UM_CREATECONFHELP, createConf_OnCreateConfHelp);
    HANDLE_MSG(hwnd, UM_CHECKCONF, createConf_OnCheckConf);
    HANDLE_MSG(hwnd, UM_CHECKTOPIC, createConf_OnCheckTopic);
    HANDLE_MSG(hwnd, UM_KILLFOCUS, createConf_OnKillFocus);
    HANDLE_MSG(hwnd, UM_NEXTTOPIC, createConf_OnNextTopic);
    HANDLE_MSG(hwnd, UM_RESETCONTROLS, createConf_OnResetControls);
    HANDLE_MSG(hwnd, WM_POPUPHELP, createConf_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, createConf_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, createConf_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, createConf_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK createConfProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=createConf_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static void newTopicControls(HWND hwnd, LPEditTopicData lpEditTopicData)
{
    BOOL bNewTopic=lpEditTopicData->bNewTopic;
    BOOL bTopicSet=lpEditTopicData->bTopicSet;
    BOOL bDeleted=lpEditTopicData->lpTopic && lpEditTopicData->lpTopic->topicFlags&TOPIC_DELETE;
    BOOL bNewAdded=lpEditTopicData->lpTopic && lpEditTopicData->lpTopic->topicFlags&TOPIC_NEW && !bDeleted;
    BOOL bNewEdit=bNewTopic && bTopicSet || bNewAdded;
    BOOL bHasTopics=lpEditTopicData->topicInfo.totalCount>0;

    SetDlgItemText(hwnd, CID_ADD, bNewTopic ? "&Add" : "Ne&w topic");
    SetDlgItemText(hwnd, CID_DELETE, bDeleted ? "Unde&lete" : "De&lete");
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), bNewTopic || bHasTopics);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC), bHasTopics);
    EnableWindow(GetDlgItem(hwnd, CID_ADD), bTopicSet || !bNewTopic);
    EnableWindow(GetDlgItem(hwnd, CID_DELETE), bNewTopic || bHasTopics);
    EnableWindow(GetDlgItem(hwnd, CID_PRUNE), !(bNewTopic || bNewAdded || bDeleted));
    EnableWindow(GetDlgItem(hwnd, CID_EDIT_SEED), bNewAdded);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_FLIST), bNewEdit);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_READONLY), bTopicSet && !bDeleted);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_DESC_TAG), bNewEdit);
    EnableWindow(GetDlgItem(hwnd, CID_TOPIC_DESCRIPTION), bNewEdit);
}



static BOOL checkAdd(HWND hwnd, HCONF hConference, LPEditTopicData lpEditTopicData)
{
    BOOL bOK=TRUE;

    if (lpEditTopicData->bNewTopic && lpEditTopicData->bTopicSet)
	//  current topic not added...
	switch (query(hwnd, MB_YESNOCANCEL,
		      "You haven't added topic %s to forum %s\r\n\r\n"
		      "Do you want to add it?",
		      lpEditTopicData->newTopic.topicName,
		      GetConferenceName(hConference))) {

	case IDYES:
	    lpEditTopicData->lpTopic=allocateTopic(&lpEditTopicData->topicInfo, &lpEditTopicData->newTopic);
	    saveTopic(hwnd, lpEditTopicData->lpTopic);
	    lpEditTopicData->lpPrevTopic=NULL;
	    ListBox_AddString(GetDlgItem(hwnd, CID_TOPIC), lpEditTopicData->lpTopic);
	    selectTopic(hwnd, &lpEditTopicData->topicInfo, lpEditTopicData->lpTopic->topicIndex, FALSE);
	    writeSeedMessage(hwnd, &lpEditTopicData->topicInfo, setup.bEditSeedMessages);
	    lpEditTopicData->bNewTopic=FALSE;
	    break;

	case IDCANCEL:
	    bOK=FALSE;
	    break;

	}

    if (lpEditTopicData->lpTopic)
	saveTopic(hwnd, lpEditTopicData->lpTopic);

    return(bOK);
}



static BOOL confirmDelete(HWND hwnd, HCONF hConference, LPTopic lpTopic)
{
    BOOL ok=hConference && lpTopic;

    if (ok) {
        FARPROC lpProc=MakeProcInstance(deleteTopicProc, lpGlobals->hInst);
        ConfirmDeleteData confirmDeleteData;

        confirmDeleteData.hConference=hConference;
	confirmDeleteData.lpTopic=lpTopic;

        ok=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_DELETE_TOPIC", (HWND)hwnd, (DLGPROC)lpProc,
                             (LPARAM)(DWORD)(LPConfirmDeleteData)&confirmDeleteData);

        FreeProcInstance(lpProc);
    }

    return(ok);
}



static BOOL editTopic_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPEditTopicData lpEditTopicData=InitDlgData(hwnd, EditTopicData);
    HCONF hConf=NULL;
    HWND hConfList=GetDlgItem(hwnd, CID_CONFERENCE);
    int nConfCount=0;
    int nConfSelect= -1;

    lpEditTopicData->hConference=(HCONF)lParam;
    lpEditTopicData->topicSubclass.lpfnTopicNameProc=NULL;
    lpEditTopicData->topicSubclass.hwndTopicList=NULL;
    lpEditTopicData->topicSubclass.lpfnTopicListProc=NULL;
    lpEditTopicData->topicSubclass.lpfnTopicNameOldProc=NULL;
    initTopicInfo(&lpEditTopicData->topicInfo);
    initTopic(&lpEditTopicData->newTopic, &lpEditTopicData->topicInfo.topicHeader);
    lpEditTopicData->lpTopic=NULL;
    lpEditTopicData->lpPrevTopic=NULL;
    lpEditTopicData->bNewTopic=FALSE;
    lpEditTopicData->bTopicFocus=FALSE;
    lpEditTopicData->bTopicChanged=FALSE;
    lpEditTopicData->bTopicSet=FALSE;
    lpEditTopicData->bInValidate=FALSE;
    lpEditTopicData->bFlistSet=FALSE;
    lpEditTopicData->bReadonlySet=FALSE;
    lpEditTopicData->bCheckFlag=FALSE;

    //  Subclass topic name edit control - we need the window handle and message
    //  handler for the topic list list box as they are referred to in topicNameProc()
    lpEditTopicData->topicSubclass.lpfnTopicNameProc=MakeProcInstance((FARPROC)topicNameProc, lpGlobals->hInst);
    lpEditTopicData->topicSubclass.hwndTopicList=GetDlgItem(hwnd, CID_TOPIC);
    lpEditTopicData->topicSubclass.lpfnTopicListProc=GetWindowProc(lpEditTopicData->topicSubclass.hwndTopicList);
    lpEditTopicData->topicSubclass.lpfnTopicNameOldProc=SubclassWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), lpEditTopicData->topicSubclass.lpfnTopicNameProc);

    if (!lpEditTopicData->hConference) {
        CURMSG currentMessage;

        if (GetCurrentMsg(&currentMessage))
            lpEditTopicData->hConference=currentMessage.pcl;

    }

    for (hConf=GetConference(NULL); hConf; hConf=GetConference(hConf)) {
        LPCSTR lpcConfName=GetConferenceName(hConf);

        if (lstrcmp(lpcConfName, "mail")!=0 &&
            lstrcmp(lpcConfName, "Usenet")!=0 &&
	    lstrcmp(lpcConfName, "UseNet")!=0 &&
            IsModerator(hConf, NULL)) {
            ComboBox_AddItemData(hConfList, hConf);

            if (hConf==lpEditTopicData->hConference)
                nConfSelect=nConfCount;

            nConfCount++;
        }
    }

    if (nConfCount==0) {
        alert(hwnd, "You are not marked in Ameol as moderating any conferences!");
        FORWARD_WM_CLOSE(hwnd, PostMessage);
    } else {
        if (nConfSelect>=0)
            ComboBox_SetCurSel(hConfList, nConfSelect);
        else {
            lpEditTopicData->hConference=(HCONF)ComboBox_GetItemData(hConfList, 0);
	    ComboBox_SetCurSel(hConfList, 0);
        }

        getTopicInfo(hwnd, lpEditTopicData->hConference, &lpEditTopicData->topicInfo);
        lpEditTopicData->topicInfo.changed=TRUE;
        lpEditTopicData->lpTopic=selectTopic(hwnd, &lpEditTopicData->topicInfo, 0, TRUE);
	lpEditTopicData->bTopicSet=lpEditTopicData->lpTopic!=NULL;
	Edit_LimitText(GetDlgItem(hwnd, CID_TOPIC_NAME), 14);
        Edit_LimitText(GetDlgItem(hwnd, CID_TOPIC_DESCRIPTION), DESC_LENGTH);
        SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_SIZE), lpGlobals->hfNormal, FALSE);
        SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_MAX), lpGlobals->hfNormal, FALSE);
	SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_FREE), lpGlobals->hfNormal, FALSE);
        SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_PERCENT_FULL), lpGlobals->hfNormal, FALSE);
	newTopicControls(hwnd, lpEditTopicData);
    }

    return(TRUE);
}



static void editTopic_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    lpMeasureItem->itemHeight=fontHeight(hwnd);
}



static void editTopic_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR * lpDrawItem)
{
    if (lpDrawItem->itemID!=(UINT)-1)
	if (lpDrawItem->CtlID==CID_CONFERENCE)
	    drawConfName(lpDrawItem);
	else
	    drawTopicName(lpDrawItem);

}



static void editTopic_OnCheckTopic(HWND hwnd)
{
    LPEditTopicData lpEditTopicData=GetDlgData(hwnd, EditTopicData);

    lpEditTopicData->bTopicChanged=FALSE;

    if (lpEditTopicData->bNewTopic) {
	lpEditTopicData->lpTopic=NULL;

	//  get and validate topic name
	GetDlgItemText(hwnd, CID_TOPIC_NAME, lpEditTopicData->newTopic.topicName, 15);
	lpEditTopicData->bTopicSet= *lpEditTopicData->newTopic.topicName!='\0';

	if (!validateTopicName(hwnd, lpEditTopicData->newTopic.topicName))
	    //  focus sticks on the topic name if it's bad...
	    NextDlgCtl(hwnd, CID_TOPIC_NAME);
	else {
	    if (lpEditTopicData->topicInfo.totalCount && *lpEditTopicData->newTopic.topicName)
		//  see if the user has entered a name already added...
		lpEditTopicData->lpTopic=findTopic(&lpEditTopicData->topicInfo,
						   lpEditTopicData->newTopic.topicName, -1);

	    if (lpEditTopicData->lpTopic) {
		selectTopic(hwnd, &lpEditTopicData->topicInfo, lpEditTopicData->lpTopic->topicIndex, TRUE);
		lpEditTopicData->bNewTopic=FALSE;
		lpEditTopicData->bTopicSet=TRUE;
		lpEditTopicData->lpPrevTopic=NULL;
		MessageBeep(0);
	    } else {
		if (!lpEditTopicData->bFlistSet)
		    if (lstrcmp(lpEditTopicData->newTopic.topicName, "files")==0)
			lpEditTopicData->newTopic.topicFlags|=TOPIC_FILES;
		    else
			lpEditTopicData->newTopic.topicFlags&= ~TOPIC_FILES;

		if (!lpEditTopicData->bReadonlySet)
		    if (lstrcmp(lpEditTopicData->newTopic.topicName, "files")==0 ||
			lstrcmp(lpEditTopicData->newTopic.topicName, "digest")==0 ||
			lstrcmp(lpEditTopicData->newTopic.topicName, "announce")==0 ||
			lstrcmp(lpEditTopicData->newTopic.topicName, "announcements")==0)
			lpEditTopicData->newTopic.topicFlags|=TOPIC_READONLY;
		    else
			lpEditTopicData->newTopic.topicFlags&= ~TOPIC_READONLY;

		displayTopic(hwnd, &lpEditTopicData->newTopic, FALSE);
	    }
	}
    } else if (!(lpEditTopicData->lpTopic=findNextTopic(hwnd, &lpEditTopicData->topicInfo, FALSE)))
	NextDlgCtl(hwnd, CID_TOPIC_NAME);

    newTopicControls(hwnd, lpEditTopicData);
    lpEditTopicData->bInValidate=FALSE;
}



static void editTopic_OnKillFocus(HWND hwnd, HWND hwndNext)
{
    LPEditTopicData lpEditTopicData=GetDlgData(hwnd, EditTopicData);

    if (lpEditTopicData->bTopicChanged) {
        int nNext=GetDlgCtrlID(hwndNext);

        lpEditTopicData->bCheckFlag=nNext==CID_TOPIC_FLIST && !lpEditTopicData->bFlistSet ||
                                    nNext==CID_TOPIC_READONLY && !lpEditTopicData->bReadonlySet;

    }
}



static void editTopic_OnNextTopic(HWND hwnd)
{
    LPEditTopicData lpEditTopicData=GetDlgData(hwnd, EditTopicData);

    //  user has pressed spacebar
    if (lpEditTopicData->bNewTopic)
        MessageBeep(0);
    else {
        lpEditTopicData->lpTopic=findNextTopic(hwnd, &lpEditTopicData->topicInfo, TRUE);
        newTopicControls(hwnd, lpEditTopicData);
    }
}



static LPCSTR editTopic_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_TOPIC_MAINTENANCE, id));
}



static void editTopic_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_EDIT_TOPICS);
}



static void editTopic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPEditTopicData lpEditTopicData=GetDlgData(hwnd, EditTopicData);

    switch (id) {

    case CID_CONFERENCE:
	if (codeNotify==CBN_SELCHANGE) {
	    WORD iConf=ComboBox_GetCurSel(hwndCtl);
	    HCONF hNewConference=(HCONF)ComboBox_GetItemData(hwndCtl, iConf);

	    if (lpEditTopicData->hConference!=hNewConference &&
		checkAdd(hwnd, lpEditTopicData->hConference, lpEditTopicData)) {
		saveTopicInfo(hwnd, lpEditTopicData->hConference, &lpEditTopicData->topicInfo);
		lpEditTopicData->hConference=hNewConference;
                getTopicInfo(hwnd, lpEditTopicData->hConference, &lpEditTopicData->topicInfo);
                lpEditTopicData->topicInfo.changed=TRUE;
                lpEditTopicData->lpTopic=selectTopic(hwnd, &lpEditTopicData->topicInfo, 0, TRUE);
		lpEditTopicData->bNewTopic=FALSE;
		lpEditTopicData->bTopicSet=lpEditTopicData->topicInfo.currentTopic!=NULL;
		newTopicControls(hwnd, lpEditTopicData);
	    }
	}

        break;

    case CID_TOPIC_NAME:
	//  Topic name edit control...
	switch (codeNotify) {

	case EN_SETFOCUS:
	    lpEditTopicData->bTopicFocus=TRUE;
	    break;

	case EN_CHANGE:
	    if (lpEditTopicData->bTopicFocus) {
		//  user has edited topic name
		lpEditTopicData->bTopicChanged=TRUE;

		if (lpEditTopicData->bNewTopic) {
		    GetWindowText(hwndCtl, lpEditTopicData->newTopic.topicName, 15);
		    lpEditTopicData->bTopicSet= *lpEditTopicData->newTopic.topicName!='\0';
		    newTopicControls(hwnd, lpEditTopicData);
		} else if (!lpEditTopicData->bInValidate) {
		    lpEditTopicData->bInValidate=TRUE;
		    lpEditTopicData->lpTopic=findNextTopic(hwnd, &lpEditTopicData->topicInfo, FALSE);
		    newTopicControls(hwnd, lpEditTopicData);
		    lpEditTopicData->bInValidate=FALSE;
		}
	    }

	    break;

	case EN_KILLFOCUS:
	    lpEditTopicData->bTopicFocus=FALSE;

	    if (!lpEditTopicData->bInValidate && lpEditTopicData->bTopicChanged) {
		lpEditTopicData->bInValidate=TRUE;
		Post_Moderate_CheckTopic(hwnd);
	    }

	    break;

	}

	break;

    case CID_TOPIC:
	//  Topic list list box...
	switch (codeNotify) {

	case LBN_SELCHANGE: {
	    //  selection has changed...
	    int nTopicIndex=ListBox_GetCurSel(hwndCtl);

            lpEditTopicData->lpTopic=selectTopic(hwnd, &lpEditTopicData->topicInfo, nTopicIndex, TRUE);
            lpEditTopicData->bTopicSet=TRUE;
            lpEditTopicData->bNewTopic=FALSE;
            newTopicControls(hwnd, lpEditTopicData);
            break;
        }

        }

        break;

    case CID_ADD:
        if (!lpEditTopicData->bNewTopic) {
            saveTopic(hwnd, lpEditTopicData->lpTopic);
            lpEditTopicData->lpPrevTopic=lpEditTopicData->lpTopic;
            initTopic(&lpEditTopicData->newTopic, &lpEditTopicData->topicInfo.topicHeader);
            lpEditTopicData->newTopic.topicFlags|=TOPIC_NEW;
            lpEditTopicData->bTopicChanged=FALSE;
            lpEditTopicData->bFlistSet=FALSE;
            lpEditTopicData->bReadonlySet=FALSE;
            lpEditTopicData->bNewTopic=TRUE;
            lpEditTopicData->bTopicSet=FALSE;
            lpEditTopicData->lpTopic=NULL;
            selectTopic(hwnd, &lpEditTopicData->topicInfo, -1, TRUE);
            NextDlgCtl(hwnd, CID_TOPIC_NAME);
        } else if (!lpEditTopicData->lpTopic && lpEditTopicData->bTopicSet) {
            lpEditTopicData->bNewTopic=FALSE;
            lpEditTopicData->lpTopic=allocateTopic(&lpEditTopicData->topicInfo, &lpEditTopicData->newTopic);
            saveTopic(hwnd, lpEditTopicData->lpTopic);
            lpEditTopicData->lpPrevTopic=NULL;

            //  Add topic name to list box
            ListBox_AddItemData(GetDlgItem(hwnd, CID_TOPIC), lpEditTopicData->lpTopic);
            selectTopic(hwnd, &lpEditTopicData->topicInfo, lpEditTopicData->lpTopic->topicIndex, FALSE);
            writeSeedMessage(hwnd, &lpEditTopicData->topicInfo, setup.bEditSeedMessages);
        }

        newTopicControls(hwnd, lpEditTopicData);
        break;

    case CID_DELETE:
        if (lpEditTopicData->bNewTopic) {
            lpEditTopicData->lpTopic=lpEditTopicData->lpPrevTopic;
            lpEditTopicData->bTopicSet=lpEditTopicData->lpTopic!=NULL;
            lpEditTopicData->bNewTopic=FALSE;
            lpEditTopicData->lpPrevTopic=NULL;
        } else if (lpEditTopicData->lpTopic &&
                   (lpEditTopicData->lpTopic->topicFlags&(TOPIC_NEW|TOPIC_DELETE) ||
                    confirmDelete(hwnd, lpEditTopicData->hConference, lpEditTopicData->lpTopic))) {
            RECT rect;

            saveTopic(hwnd, lpEditTopicData->lpTopic);
            lpEditTopicData->lpTopic->topicFlags^=TOPIC_DELETE;

            if (lpEditTopicData->lpTopic->topicFlags&TOPIC_DELETE)
                lpEditTopicData->topicInfo.topicCount--;
            else
                lpEditTopicData->topicInfo.topicCount++;

            if (ListBox_GetItemRect(GetDlgItem(hwnd, CID_TOPIC),
                                    lpEditTopicData->lpTopic->topicIndex, &rect)!=LB_ERR)
                InvalidateRect(GetDlgItem(hwnd, CID_TOPIC), &rect, FALSE);

        }

        displayTopic(hwnd, lpEditTopicData->lpTopic, TRUE);
        newTopicControls(hwnd, lpEditTopicData);

        if (lpEditTopicData->topicInfo.totalCount==0)
            NextDlgCtl(hwnd, CID_ADD);

        break;

    case CID_PRUNE: {
        FARPROC lpProc=MakeProcInstance(pruneTopicProc, lpGlobals->hInst);

 	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_PRUNE_TOPIC", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpEditTopicData);
	FreeProcInstance(lpProc);
        break;
    }

    case CID_UPDATE: {
        FARPROC lpProc=MakeProcInstance(updateTopicProc, lpGlobals->hInst);

	StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_UPDATE_TOPIC_FRAME", 
					  (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpEditTopicData);
	FreeProcInstance(lpProc);
        break;
    }

    case CID_CONF_NOTE: {
        CONFERENCEINFO confInfo;

        GetConferenceInfo(lpEditTopicData->hConference, &confInfo);
        editConfNote(hwnd, &lpEditTopicData->topicInfo, confInfo.szFileName);
        break;
    }

    case CID_EDIT_SEED:
        if (lpEditTopicData->lpTopic && lpEditTopicData->lpTopic->hoobSeedMessage) {
            saveTopic(hwnd, lpEditTopicData->lpTopic);
            editSeedMessage(hwnd, &lpEditTopicData->topicInfo);
        }

        break;

    case CID_TOPIC_FLIST: {
        LPTopic lpTopic=lpEditTopicData->bNewTopic ? &lpEditTopicData->newTopic : lpEditTopicData->lpTopic;

		if (lpTopic)
		{			 
			if (!(lpEditTopicData->bCheckFlag && lpTopic->topicFlags&TOPIC_FILES))
				lpTopic->topicFlags^=TOPIC_FILES;

			CheckDlgButton(hwnd, CID_TOPIC_FLIST, lpTopic->topicFlags&TOPIC_FILES);

			if (codeNotify==BN_CLICKED)
				lpEditTopicData->bFlistSet=TRUE;

			lpEditTopicData->bCheckFlag=FALSE;
		}
		else
		{
			alert(hwnd, "Error Invalid Topic");
		}
        break;
    }

    case CID_TOPIC_READONLY: {
        LPTopic lpTopic=lpEditTopicData->bNewTopic ? &lpEditTopicData->newTopic : lpEditTopicData->lpTopic;

		if (lpTopic)
		{
			if (!(lpEditTopicData->bCheckFlag && lpTopic->topicFlags&TOPIC_READONLY))
				lpTopic->topicFlags^=TOPIC_READONLY;

			CheckDlgButton(hwnd, CID_TOPIC_READONLY, lpTopic->topicFlags&TOPIC_READONLY);

			if (codeNotify==BN_CLICKED)
				lpEditTopicData->bReadonlySet=TRUE;

			lpEditTopicData->bCheckFlag=FALSE;
		}
		else
		{
			alert(hwnd, "Error Invalid Topic");
		}
        break;
    }

    case CID_OK:
	if (checkAdd(hwnd, lpEditTopicData->hConference, lpEditTopicData)) {
	    saveTopicInfo(hwnd, lpEditTopicData->hConference, &lpEditTopicData->topicInfo);
	    deleteSeedMessages(&lpEditTopicData->topicInfo, FALSE);
	    EndDialog(hwnd, 0);
	}

        break;

    case CID_CANCEL:
        deleteSeedMessages(&lpEditTopicData->topicInfo, TRUE);
	EndDialog(hwnd, 0);
        break;

    }
}



static void editTopic_OnDestroy(HWND hwnd)
{
    LPEditTopicData lpEditTopicData=GetDlgData(hwnd, EditTopicData);

    if (lpEditTopicData->topicSubclass.lpfnTopicNameOldProc) {
        SubclassWindow(GetDlgItem(hwnd, CID_TOPIC_NAME), lpEditTopicData->topicSubclass.lpfnTopicNameOldProc);
        FreeProcInstance(lpEditTopicData->topicSubclass.lpfnTopicNameProc);
    }

    freeTopicInfo(&lpEditTopicData->topicInfo);
    FreeDlgData(hwnd);
}



static LRESULT editTopic_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

//  Edit CIX Topics dialog message handler

{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, editTopic_OnInitDialog);
    HANDLE_MSG(hwnd, WM_MEASUREITEM, editTopic_OnMeasureItem);
    HANDLE_MSG(hwnd, WM_DRAWITEM, editTopic_OnDrawItem);
    HANDLE_MSG(hwnd, UM_CHECKTOPIC, editTopic_OnCheckTopic);
    HANDLE_MSG(hwnd, UM_KILLFOCUS, editTopic_OnKillFocus);
    HANDLE_MSG(hwnd, UM_NEXTTOPIC, editTopic_OnNextTopic);
    HANDLE_MSG(hwnd, WM_POPUPHELP, editTopic_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, editTopic_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, editTopic_OnCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, editTopic_OnDestroy);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK editTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=editTopic_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static BOOL pruneTopic_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPEditTopicData lpEditTopicData=(LPEditTopicData)lParam;

    SetDlgData(hwnd, lpEditTopicData);
    SetWindowFont(GetDlgItem(hwnd, CID_CONF_NAME), lpGlobals->hfNormal, FALSE);
    SetDlgItemText(hwnd, CID_CONF_NAME, GetConferenceName(lpEditTopicData->hConference));
    SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_NAME), lpGlobals->hfNormal, FALSE);
    SetDlgItemText(hwnd, CID_TOPIC_NAME, lpEditTopicData->lpTopic->topicName);
    CheckDlgButton(hwnd, CID_PRUNE, lpEditTopicData->lpTopic->topicFlags&TOPIC_PRUNE);
    return(TRUE);
}



static LPCSTR pruneTopic_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_PRUNE_TOPIC, id));
}



static void pruneTopic_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_TOPICS_PRUNE);
}



static void pruneTopic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPEditTopicData lpEditTopicData=GetDlgData(hwnd, EditTopicData);

    switch (id) {

    case CID_PRUNE:
        if (codeNotify==BN_CLICKED)
            lpEditTopicData->topicInfo.changed=TRUE;

        break;

    case CID_OK:
    case CID_CANCEL:
        if (IsDlgButtonChecked(hwnd, CID_PRUNE))
            lpEditTopicData->lpTopic->topicFlags|=TOPIC_PRUNE;
        else
            lpEditTopicData->lpTopic->topicFlags&= ~TOPIC_PRUNE;

        EndDialog(hwnd, 0);
        break;

    }
}



static LRESULT pruneTopic_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, pruneTopic_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, pruneTopic_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, pruneTopic_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, pruneTopic_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK pruneTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=pruneTopic_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static BOOL deleteTopic_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPConfirmDeleteData lpConfirmDeleteData=(LPConfirmDeleteData)lParam;

    SetWindowFont(GetDlgItem(hwnd, CID_CONF_NAME), lpGlobals->hfNormal, FALSE);
    SetDlgItemText(hwnd, CID_CONF_NAME, GetConferenceName(lpConfirmDeleteData->hConference));
    SetWindowFont(GetDlgItem(hwnd, CID_TOPIC_NAME), lpGlobals->hfNormal, FALSE);
    SetDlgItemText(hwnd, CID_TOPIC_NAME, lpConfirmDeleteData->lpTopic->topicName);
    return(TRUE);
}



static LPCSTR deleteTopic_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_DELETE_TOPIC, id));
}



static void deleteTopic_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_TOPICS_DELETE);
}



static void deleteTopic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    case CID_DELETE:
        EndDialog(hwnd, TRUE);
        break;

    case CID_CANCEL:
        EndDialog(hwnd, FALSE);
        break;

    }
}



static LRESULT deleteTopic_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, deleteTopic_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, deleteTopic_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, deleteTopic_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, deleteTopic_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK deleteTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=deleteTopic_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



static void topicName_OnChar(HWND hwnd, UINT ch, int cRepeat)
{
    LPTopicSubclassData lpTopicSubclass=GetDlgData(GetParent(hwnd), TopicSubclassData);

    switch (ch) {

    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
        SetFocus(lpTopicSubclass->hwndTopicList);

        WFORWARD_WM_CHAR(lpTopicSubclass->hwndTopicList, ch, cRepeat,
                         lpTopicSubclass->lpfnTopicListProc);

        SetFocus(hwnd);
	break;

    case ' ':
	break;

    default:
        WFORWARD_WM_CHAR(hwnd, ch, cRepeat,
                         lpTopicSubclass->lpfnTopicNameOldProc);

        break;

    }
}



static void topicName_OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    LPTopicSubclassData lpTopicSubclass=GetDlgData(GetParent(hwnd), TopicSubclassData);

    switch (vk) {

    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
        SetFocus(lpTopicSubclass->hwndTopicList);

        WFORWARD_WM_KEYDOWN(lpTopicSubclass->hwndTopicList, vk, cRepeat, flags,
                            lpTopicSubclass->lpfnTopicListProc);

        SetFocus(hwnd);
        break;

    case ' ':
        Post_Moderate_NextTopic(GetParent(hwnd));
        break;

    default:
        WFORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags,
                            lpTopicSubclass->lpfnTopicNameOldProc);

        break;

    }
}



static void topicName_OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    LPTopicSubclassData lpTopicSubclass=GetDlgData(GetParent(hwnd), TopicSubclassData);

    switch (vk) {

    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
        SetFocus(lpTopicSubclass->hwndTopicList);

        WFORWARD_WM_KEYUP(lpTopicSubclass->hwndTopicList, vk, cRepeat, flags,
                          lpTopicSubclass->lpfnTopicListProc);

        SetFocus(hwnd);
        break;

    default:
        WFORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags,
                          lpTopicSubclass->lpfnTopicNameOldProc);

        break;

    }
}



static void topicName_OnKillFocus(HWND hwnd, HWND hwndNewFocus)
{
    LPTopicSubclassData lpTopicSubclass=GetDlgData(GetParent(hwnd), TopicSubclassData);

    Post_Moderate_KillFocus(GetParent(hwnd), hwndNewFocus);
    WFORWARD_WM_KILLFOCUS(hwnd, hwndNewFocus, lpTopicSubclass->lpfnTopicNameOldProc);
}



LONG _EXPORT CALLBACK topicNameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

//  Message handler for subclassing topic name edit control - pass PgUp, PgDown,
//  cursor up and cursor down keystrokes to the topic list list box and signal
//  loss of focus and pressing of the space bar back to the parent dialog.

{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_CHAR, topicName_OnChar);
    HANDLE_MSG(hwnd, WM_KEYDOWN, topicName_OnKeyDown);
    HANDLE_MSG(hwnd, WM_KEYUP, topicName_OnKeyUp);
    HANDLE_MSG(hwnd, WM_KILLFOCUS, topicName_OnKillFocus);

    default: {
        LPTopicSubclassData lpTopicSubclass=GetDlgData(GetParent(hwnd), TopicSubclassData);

        return(WFORWARD_MESSAGE(hwnd, uMsg, wParam, lParam, lpTopicSubclass->lpfnTopicNameOldProc));
    }

    }
}

