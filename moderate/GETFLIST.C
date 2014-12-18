#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "moderate.h"
#include "help.h"
#include "globals.h"
#include "status.h"
#include "flist.h"



typedef struct tagBuffList {
    char buff[128];
    struct tagBuffList FAR *prev;
    struct tagBuffList FAR *next;
} BuffList, FAR *LPBuffList;



typedef struct {
    HSTREAM streamFLM;
    BOOL flistChanged;
    char fileName[15];
    LPBuffList buff;
    int lineCount;
    int nonblankLines;
    int blankLines;
    int nWrapped;
    LPFlistEntry flist;
    LPFlistEntry lastFlist;
    LPINT flistEntries;
    LPINT flistFiles;
} FlistContext, FAR *LPFlistContext;



BOOL _EXPORT CALLBACK flistErrorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



time_t checkFile(HTOPIC hTopic, LPCSTR extension, LPSTR fullname)
{
    TOPICINFO topicInfo;

    GetTopicInfo(hTopic, &topicInfo);
    wsprintf(fullname, "%s\\flist\\%s.%s", (LPSTR)setup.szDataDir, (LPSTR)topicInfo.szFileName, extension);

    if (_access(fullname, 0)!=0)
        return((time_t)0);
    else {
        struct stat statBuff;

        stat(fullname, &statBuff);
        return((time_t)statBuff.st_mtime);
    }
}



void freeFlist(LPPFlistEntry lpFlist)
{
    LPFlistEntry nextFlist= *lpFlist;

    while (nextFlist) 
	{
        LPFlistEntry thisFlist=nextFlist;

        nextFlist=thisFlist->flistNext;
        gfree(thisFlist);
		thisFlist = NULL;
    }

    *lpFlist=NULL;
}



void freeFdir(LPPFdirEntry lpFdir)
{
    LPFdirEntry nextFdir= *lpFdir;

    while (nextFdir) 
	{
        LPFdirEntry thisFdir=nextFdir;

        nextFdir=thisFdir->fdirNext;

        if (thisFdir->fdirLocalPath)
		{
            gfree(thisFdir->fdirLocalPath);
			thisFdir->fdirLocalPath = NULL;
		}

        gfree(thisFdir);
		thisFdir = NULL;
    }

    *lpFdir=NULL;
}



BOOL flistError(HWND hwnd, LPFlistContext lpFlistContext)
{
    FARPROC lpProc;
    BOOL ok;

    lpProc=MakeProcInstance(flistErrorProc, lpGlobals->hInst);
    ok=StdDialogBoxParam((HINSTANCE)lpGlobals->hInst, (LPCSTR)"DID_FLIST_ERROR", 
						 (HWND)hwnd, (DLGPROC)lpProc, (LPARAM)(DWORD)lpFlistContext);
    FreeProcInstance(lpProc);
    return(ok);
}



int fdirError(HSTREAM hStream, LPPFdirEntry lpFdir, int err)
{
    if (hStream)
        closeFile(hStream);

    freeFdir(lpFdir);
    return(err);
}



int debugReadLine(HSTREAM hStream, LPSTR lpLineBuff, UINT wMaxLength)
{
    int status=readLine(hStream, lpLineBuff, wMaxLength);

    if (status<0)
        alert(NULL, "readLine() status is %d", status);

    return(status);
}
// #define readLine(hStream, lpLineBuff, wMaxLength) debugReadLine(hStream, lpLineBuff, wMaxLength)



int readFdir(LPCSTR fnFLD, LPPFdirEntry lpFdir, LPINT fdirFiles)
{
    HSTREAM streamFLD;
    LPFdirEntry lastFdir=NULL;
    char buff[128];
    int lineLength;

    *fdirFiles=0;

    streamFLD=openFile(fnFLD, OF_READ);

    if (!streamFLD)
        return(ERR_OPEN);

    while ((lineLength=readLine(streamFLD, buff, 127))>=0 &&
            lstrcmp(buff, "    ===========         ====  ====================="));

    while ((lineLength=readLine(streamFLD, buff, 127))>=0)
        if (lineLength==45) {
            LPFdirEntry newFdir=(LPFdirEntry)gmalloc(sizeof(FdirEntry));
            struct tm timeBuff;
            int inPtr=0;
            int outPtr=0;
            int thisReference;

            if (lastFdir)
                lastFdir->fdirNext=newFdir;
            else
                *lpFdir=newFdir;

            newFdir->fdirHead=lpFdir;
            newFdir->fdirNext=NULL;
            lastFdir=newFdir;
            (*fdirFiles)++;

            while (inPtr<16 && buff[inPtr]==' ')
                inPtr++;

            if (inPtr>15)
                return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));

            while (inPtr<16 && buff[inPtr]!=' ')
                newFdir->fdirName[outPtr++]=buff[inPtr++];

            newFdir->fdirName[outPtr]='\0';
            newFdir->fdirLocalPath=NULL;
            newFdir->fdirSize=strtoul(buff+15, NULL, 10);

            switch (buff[30]) {

            case 'J':
                switch (buff[31]) {

                case 'a':      timeBuff.tm_mon=0;        break;

                case 'u':
                    switch (buff[32]) {

                    case 'n':  timeBuff.tm_mon=5;        break;
                    case 'l':  timeBuff.tm_mon=6;        break;

                    default:
                        return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));

                    }

                    break;

                default:
                    return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));

                }

                break;

            case 'F':          timeBuff.tm_mon=1;        break;

            case 'M':
                switch (buff[32]) {

                case 'r':      timeBuff.tm_mon=2;        break;
                case 'y':      timeBuff.tm_mon=4;        break;

                default:
                    return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));

                }

                break;

            case 'A':
                switch (buff[31]) {

                case 'p':      timeBuff.tm_mon=3;        break;
                case 'u':      timeBuff.tm_mon=7;        break;

                default:
                    return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));

                }

                break;

            case 'S':          timeBuff.tm_mon=8;        break;
            case 'O':          timeBuff.tm_mon=9;        break;
            case 'N':          timeBuff.tm_mon=10;       break;
            case 'D':          timeBuff.tm_mon=11;       break;

            default:
                return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));

            }

            timeBuff.tm_mday=(int)strtoul(buff+34, NULL, 10);
            timeBuff.tm_hour=(int)strtoul(buff+37, NULL, 10);
            timeBuff.tm_min=(int)strtoul(buff+40, NULL, 10);
            timeBuff.tm_year=(int)strtoul(buff+43, NULL, 10);
			if(timeBuff.tm_year < 70)
				timeBuff.tm_year += 100;
            timeBuff.tm_sec=0;
			timeBuff.tm_wday=0;
			timeBuff.tm_yday=0;
            timeBuff.tm_isdst= -1;
            newFdir->fdirTimestamp=mktime(&timeBuff);
            newFdir->fdirReferences=0;
            newFdir->fdirFlags=FDIR_DEFAULT_FLAGS;

            for (thisReference=0; thisReference<MAX_REFERENCES; thisReference++)
                newFdir->fdirFlistEntry[thisReference]=NULL;

        } else if (lineLength>0)
            if (_fstrncmp(buff, "****_access on ", 14)==0)
                alert(NULL, "File %s is not accessible in the flist for %s - "
                            "please contact support@cix.co.uk for assistance", (LPSTR)buff+14, (LPSTR)fnFLD);

            else {
                alert(NULL, "Unexpected line length in %s", (LPSTR)fnFLD);
                return(fdirError(streamFLD, lpFdir, ERR_BAD_FLD));
            }

    closeFile(streamFLD);
    return(OK);
}



int readFlist(HWND hwnd, HTOPIC hTopic,
              LPPFlistEntry lpFlist, time_t FAR * flistTime, LPINT flistEntries, LPINT flistFiles,
              LPPFdirEntry lpFdir, time_t FAR *fdirTime, LPINT fdirFiles, LPINT fdirOrphans,
              time_t FAR *fuserTime, LPPFdirEntry lpFilepool)
{
    char fnFLM[LEN_PATHNAME+LEN_FILENAME];
    char fnFLD[LEN_PATHNAME+LEN_FILENAME];
    char fnFLS[LEN_PATHNAME+LEN_FILENAME];
    LPFlistEntry thisFlist;
    BuffList buffList[4];
    FlistContext flistContext;
    int lineLength;
    int status;
    int ptr;

    for (ptr=0; ptr<3; ptr++) 
	{
        buffList[ptr].next=buffList+ptr+1;
        buffList[ptr+1].prev=buffList+ptr;
    }

    buffList[0].prev=buffList+3;
    buffList[3].next=buffList;

    flistContext.flistChanged=FALSE;
    *flistContext.fileName='\0';
    flistContext.buff=buffList;
    flistContext.lineCount=0;
    flistContext.nonblankLines=0;
    flistContext.blankLines=0;
    flistContext.nWrapped=0;
    flistContext.flist=NULL;
    flistContext.lastFlist=NULL;
    flistContext.flistEntries=flistEntries;
    flistContext.flistFiles=flistFiles;

    *lpFlist=NULL;
    *flistTime=checkFile(hTopic, "FLM", fnFLM);
    *flistEntries=0;
    *flistFiles=0;

    *lpFdir=NULL;
    *fdirTime=checkFile(hTopic, "FLD", fnFLD);
    *fdirFiles=0;
    *fdirOrphans=0;

    *fuserTime=checkFile(hTopic, "FLS", fnFLS);

    *lpFilepool=NULL;

    if (! *flistTime || ! *fdirTime)
        return(ERR_FILE_MISSING);

    flistContext.streamFLM=openFile(fnFLM, OF_READ);

    if (!flistContext.streamFLM)
        return(ERR_OPEN);

    for (ptr=lstrlen(fnFLM)-1; ptr>=0 && fnFLM[ptr]!='\\'; ptr--);
    lstrcpy(flistContext.fileName, fnFLM+ptr+1);
    lineLength=readLine(flistContext.streamFLM, flistContext.buff->buff, 127);

    while (lineLength>=0 || lineLength==STREAM_OVERFLOW) {
        BOOL bOverflow=lineLength==STREAM_OVERFLOW;
        LPSTR lpPtr=NULL;
        LPSTR lpBuff;

        if (bOverflow) {
            lpPtr=flistContext.buff->buff+126;
            lineLength=127;

            while (lpPtr>=flistContext.buff->buff+63 && *lpPtr!=' ') {
                lpPtr--;
                lineLength--;
            }

            if (*lpPtr==' ') {
                *lpPtr='\0';
                lpPtr++;
                lineLength--;
            } else {
                lpPtr=NULL;
                lineLength=127;
            }
        }

        if (!flistContext.nWrapped)
            flistContext.lineCount++;

        if (lineLength==0)
            flistContext.blankLines++;
        else 
		{
            LPFlistEntry newFlist=(LPFlistEntry)gmalloc(sizeof(FlistEntry));

            lpBuff=flistContext.buff->buff;

            if (flistContext.lastFlist) 
			{
                flistContext.lastFlist->flistNext=newFlist;
                newFlist->flistPrevious=flistContext.lastFlist;
            } 
			else 
			{
                flistContext.flist=newFlist;
                newFlist->flistPrevious=NULL;
            }

            newFlist->flistNext=NULL;
            newFlist->flistFlags=FLIST_DEFAULT_FLAGS;
            newFlist->flistSelect=FLIST_UNSET;
            newFlist->flistSource=NULL;
            flistContext.lastFlist=newFlist;
            (*flistEntries)++;
            flistContext.nonblankLines++;

            if (flistContext.nWrapped) 
			{
                if (newFlist->flistPrevious)
                    switch (newFlist->flistPrevious->flistSelect) 
					{

                    case FLIST_MEMO:
                    case FLIST_COMMENT:
                    case FLIST_TABBED:
                        newFlist->flistSelect=newFlist->flistPrevious->flistSelect;
                        break;

                    case FLIST_FROM_FILEPOOL:
                    case FLIST_FROM_FDIR:
                        newFlist->flistSelect=FLIST_TABBED;
                        break;

                    }

                if (flistError(hwnd, &flistContext))
                    lstrcpy(newFlist->flistDescription, lpBuff);
                else
                    return(ERR_BAD_FLM);

            } 
			else 
			{
            Parse:
                switch (*lpBuff) 
				{

                case '#':
                    newFlist->flistSelect=FLIST_MEMO;

                    if (newFlist->flistFlags&FLIST_FLAG_HOLD ||
                        lineLength<3 || lpBuff[1]!='|' || lpBuff[2]!='|') 
					{
                        if (flistError(hwnd, &flistContext)) 
						{
                            newFlist->flistFlags&= ~FLIST_FLAG_HOLD;
                            lstrcpy(newFlist->flistDescription, lpBuff+1);
                        } 
						else
                            return(ERR_BAD_FLM);

                    } 
					else 
					{
                        if (lineLength>3 && lpBuff[3]=='~') 
						{
                            // I don't believe I'm doing this...
                            lpBuff+=4;
                            lineLength-=4;
                            newFlist->flistFlags|=FLIST_FLAG_HOLD;
                            goto Parse;
                        }

                        lstrcpy(newFlist->flistDescription, lpBuff+3);
                    }

                    break;

                case '|':
                    newFlist->flistSelect=FLIST_COMMENT;

                    if (lineLength>=2 && lpBuff[1]=='|')
                        lstrcpy(newFlist->flistDescription, lpBuff+2);
                    else if (flistError(hwnd, &flistContext))
                        lstrcpy(newFlist->flistDescription, lpBuff+1);
                    else
                        return(ERR_BAD_FLM);

                    break;

                case 'f':
                case 'c': 
				{
                    FlistSelect flistSelect= *lpBuff=='f' ? FLIST_FROM_FILEPOOL : FLIST_FROM_FDIR;
                    int ptrIn=1;

                    while (lpBuff[ptrIn] && lpBuff[ptrIn]!='|')
                        ptrIn++;

                    if (lpBuff[ptrIn]=='|') 
					{
                        BOOL err=ptrIn>1;
                        int ptrOut=0;

                        ptrIn++;

                        while (ptrOut<14 && lpBuff[ptrIn] && lpBuff[ptrIn]!='|')
                            newFlist->flistName[ptrOut++]=lpBuff[ptrIn++];

                        newFlist->flistName[ptrOut]='\0';

                        if (ptrOut>0 && (!lpBuff[ptrIn] || lpBuff[ptrIn]=='|')) 
						{
                            newFlist->flistSelect=flistSelect;
                            (*flistFiles)++;

                            if (!err && lpBuff[ptrIn]=='|')
                                lstrcpy(newFlist->flistDescription, lpBuff+ptrIn+1);
                            else if (flistError(hwnd, &flistContext)) 
							{
                                if (lpBuff[ptrIn])
                                    lstrcpy(newFlist->flistDescription, lpBuff+ptrIn+1);
                                else
                                    *newFlist->flistDescription='\0';

                            } 
							else
                                return(ERR_BAD_FLM);

                        } 
						else if (flistError(hwnd, &flistContext))
                            lstrcpy(newFlist->flistDescription, lpBuff);
                        else
                            return(ERR_BAD_FLM);

                    } 
					else if (flistError(hwnd, &flistContext))
                        lstrcpy(newFlist->flistDescription, lpBuff);
                    else
                        return(ERR_BAD_FLM);

                    break;
                }

                case '.':
                    newFlist->flistSelect=FLIST_TABBED;

                    if (lineLength>=3 && lpBuff[1]=='|' && lpBuff[2]=='|')
                        lstrcpy(newFlist->flistDescription, lpBuff+3);
                    else if (flistError(hwnd, &flistContext))
                        lstrcpy(newFlist->flistDescription, lpBuff+1);
                    else
                        return(ERR_BAD_FLM);

                    break;

                default:
                    if (flistError(hwnd, &flistContext))
                        lstrcpy(newFlist->flistDescription, lpBuff);
                    else
                        return(ERR_BAD_FLM);

                }
            }

            flistContext.blankLines=0;
            flistContext.buff=flistContext.buff->next;
        }

        lpBuff=flistContext.buff->buff;
        lineLength=127;

        if (bOverflow) {
            flistContext.nWrapped++;

            if (lpPtr && *lpPtr) {
                lstrcpy(lpBuff, lpPtr);
                lpBuff+=lstrlen(lpPtr);
                lineLength-=lstrlen(lpPtr);
            }
        } else
            flistContext.nWrapped=0;

        lineLength=readLine(flistContext.streamFLM, lpBuff, lineLength);
    }

    closeFile(flistContext.streamFLM);
    *lpFlist=flistContext.flist;

    if ((status=readFdir(fnFLD, lpFdir, fdirFiles))!=OK) {
        freeFlist(lpFlist);
        return(status);
    }

    *fdirOrphans= *fdirFiles;

    for (thisFlist= *lpFlist; thisFlist; thisFlist=thisFlist->flistNext)
        if (thisFlist->flistSelect==FLIST_FROM_FDIR) 
		{
            LPFdirEntry thisFdir;

            for (thisFdir= *lpFdir;
                 thisFdir && lstrcmp(thisFlist->flistName, thisFdir->fdirName);
                 thisFdir=thisFdir->fdirNext);

            if (thisFdir) 
			{
                thisFlist->flistSource=thisFdir;

                if (thisFdir->fdirReferences<MAX_REFERENCES) 
				{
                    if (!thisFdir->fdirReferences)
                        (*fdirOrphans)--;

                    thisFdir->fdirFlistEntry[thisFdir->fdirReferences++]=thisFlist;
                }
            }
        } 
		else if (thisFlist->flistSelect==FLIST_FROM_FILEPOOL) 
		{
            LPFdirEntry thisFdir;

            for (thisFdir= *lpFilepool;
                 thisFdir && lstrcmp(thisFlist->flistName, thisFdir->fdirName);
                 thisFdir=thisFdir->fdirNext);

            if (thisFdir) 
			{
                thisFlist->flistSource=thisFdir;

                if (thisFdir->fdirReferences<MAX_REFERENCES)
                    thisFdir->fdirFlistEntry[thisFdir->fdirReferences++]=thisFlist;

            } 
			else 
			{
                thisFdir=(LPFdirEntry)gmalloc(sizeof(FdirEntry));

                thisFdir->fdirHead=lpFilepool;
                thisFdir->fdirNext= *lpFilepool;
                lstrcpy(thisFdir->fdirName, thisFlist->flistName);
                thisFdir->fdirLocalPath=NULL;
                thisFdir->fdirSize= -1;
                thisFdir->fdirTimestamp=0;
                thisFdir->fdirReferences=1;
                thisFdir->fdirFlags=FDIR_DEFAULT_FLAGS;
                thisFdir->fdirFlistEntry[0]=thisFlist;
                *lpFilepool=thisFdir;
            }
        }

    return(flistContext.flistChanged ? ERR_FLIST_CHANGED : OK);
}



int writeFlist(HTOPIC hTopic, LPFlistEntry lpFlist)
{
    char fnFLM[LEN_PATHNAME+LEN_FILENAME];
    HSTREAM streamFLM;

    if (checkFile(hTopic, "FLM", fnFLM)) {
        char fnFLB[LEN_PATHNAME+LEN_FILENAME];

        if (checkFile(hTopic, "FLB", fnFLB))
            _unlink(fnFLB);

        rename(fnFLM, fnFLB);
    }

    streamFLM=openFile(fnFLM, OF_WRITE);

    if (!streamFLM)
        return(ERR_OPEN);

    while (lpFlist) {
        int status;
        LPCSTR holdFlag=lpFlist->flistFlags&FLIST_FLAG_HOLD ? "#||~" : "";

        switch (lpFlist->flistSelect) {

        case FLIST_MEMO:
            status=writeLine(streamFLM, "#||%s\n", lpFlist->flistDescription);
            break;

        case FLIST_COMMENT:
	    status=writeLine(streamFLM, "%s||%s\n", holdFlag, lpFlist->flistDescription);
            break;

        case FLIST_TABBED:
            status=writeLine(streamFLM, "%s.||%s\n", holdFlag, lpFlist->flistDescription);
            break;

        case FLIST_FROM_FILEPOOL:
            status=writeLine(streamFLM, "%sf|%s|%s\n", holdFlag, lpFlist->flistName,
                             lpFlist->flistDescription);

            break;

        case FLIST_FROM_FDIR:
        case FLIST_FROM_MAIL_DIR:
        case FLIST_FROM_UPLOAD:
        case FLIST_FROM_NOTIFICATION:
            status=writeLine(streamFLM, "%s%c|%s|%s\n", holdFlag,
                             lpFlist->flistFlags&(FLIST_FLAG_EXPORT|FLIST_FLAG_FILEPOOL) ? 'f' : 'c',
                             lpFlist->flistName, lpFlist->flistDescription);

            break;

        }

        if (status) {
            closeFile(streamFLM);
            return(status);
        }

        lpFlist=lpFlist->flistNext;
    }

    closeFile(streamFLM);
    return(OK);
}


/* This function reads the mail directory file. In Ameol1, this is
 * located in the data directory. In Ameol2, this is located in the
 * users home directory.
 */
int readMaildir(LPPFdirEntry lpMaildir, time_t FAR *maildirTime, LPINT maildirFiles)
{
	char fnMaildir[ LEN_PATHNAME+LEN_FILENAME ];
	struct stat statBuff;

	if( lpGlobals->AmeolVerInfo.nVersion == 2 )
		{
		char szUsername[ 64 ];

		/* Under version 2, look in the user's home directory
		 */
		GetRegistry( szUsername );
		wsprintf( fnMaildir, "%s\\users\\%s\\mail.lst", (LPSTR)setup.szAmeolDir, szUsername );
		if( _access( fnMaildir, 0 ) != 0 )
			{
			*maildirTime = 0;
			return( ERR_FILE_MISSING );
			}
		}
	else
		{
		*maildirFiles = 0;
		wsprintf( fnMaildir, "%s\\mail.lst", (LPSTR)setup.szDataDir );
		if( _access( fnMaildir, 0 ) != 0 )
			{
			wsprintf( fnMaildir, "%s\\mail.lst", (LPSTR)setup.szAmeolDir );
			if( _access( fnMaildir, 0 ) != 0 )
				{
				*maildirTime = 0;
				return( ERR_FILE_MISSING );
				}
			}
		}
	stat( fnMaildir, &statBuff );
	*maildirTime = (time_t)statBuff.st_mtime;
	return( readFdir( fnMaildir, lpMaildir, maildirFiles ) );
}

static BOOL flistError_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    LPFlistContext lpFlistContext=(LPFlistContext)lParam;
    char s[32];
    LPBuffList buff;
    long lines;
    int defaultAction;

    SetDlgData(hwnd, lpFlistContext);
    buff=lpFlistContext->buff;
    lines=lpFlistContext->nonblankLines;
    SetWindowFont(GetDlgItem(hwnd, CID_FILENAME), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_LINE_NUMBER), lpGlobals->hfNormal, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_ERR_TEXT_1), lpGlobals->hfDescEdit, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_ERR_TEXT_2), lpGlobals->hfDescEdit, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_ERR_TEXT_3), lpGlobals->hfDescEdit, FALSE);
    SetWindowFont(GetDlgItem(hwnd, CID_ERR_TEXT_4), lpGlobals->hfBoldEdit, FALSE);

    SetDlgItemText(hwnd, CID_DESCRIPTION, lpFlistContext->nWrapped ?
                   "Input buffer overflow during parsing of the Moderator's File List" :
                   "An error has occured during parsing of the Moderator's File List");

    SetDlgItemText(hwnd, CID_FILENAME, lpFlistContext->fileName);

    if (lpFlistContext->nWrapped)
        wsprintf(s, "%d.%d", lpFlistContext->lineCount, lpFlistContext->nWrapped);
    else
        wsprintf(s, "%d", lpFlistContext->lineCount);

    SetDlgItemText(hwnd, CID_LINE_NUMBER, s);
    SetDlgItemText(hwnd, CID_ERR_TEXT_4, buff->buff);

    if (lpFlistContext->blankLines) {
        wsprintf(s, "» %d blank line%s «", lpFlistContext->blankLines,
                 (LPSTR)(lpFlistContext->blankLines>1 ? "s" : ""));

        SetDlgItemText(hwnd, CID_ERR_TEXT_3, s);
    } else if (--lines>0) {
        buff=buff->prev;
        SetDlgItemText(hwnd, CID_ERR_TEXT_3, buff->buff);
    } else
        SetDlgItemText(hwnd, CID_ERR_TEXT_3, "");

    if (--lines>0) {
        buff=buff->prev;
        SetDlgItemText(hwnd, CID_ERR_TEXT_2, buff->buff);
    } else
        SetDlgItemText(hwnd, CID_ERR_TEXT_2, "");

    if (--lines>0) {
        buff=buff->prev;
        SetDlgItemText(hwnd, CID_ERR_TEXT_1, buff->buff);
    } else
	SetDlgItemText(hwnd, CID_ERR_TEXT_1, "");

    switch (lpFlistContext->lastFlist->flistSelect) {

    case FLIST_MEMO:              defaultAction=CID_ERR_MEMO;       break;
    case FLIST_COMMENT:           defaultAction=CID_ERR_COMMENT;    break;
    case FLIST_TABBED:            defaultAction=CID_ERR_INDENTED;   break;
    case FLIST_FROM_FILEPOOL:     defaultAction=CID_ERR_FILE;       break;
    case FLIST_FROM_FDIR:         defaultAction=CID_ERR_FILE;       break;

    default:                      defaultAction=CID_ERR_DISCARD;    break;

    }

    EnableWindow(GetDlgItem(hwnd, CID_ERR_FILE),
                 lpFlistContext->lastFlist->flistSelect==FLIST_FROM_FILEPOOL ||
                 lpFlistContext->lastFlist->flistSelect==FLIST_FROM_FDIR);

    RadioButton(hwnd, CID_ERR_DISCARD, defaultAction);
    return(TRUE);
}



static LPCSTR flistError_OnPopupHelp(HWND hwnd, int id)
{
    return(balloonHelp(DID_FLIST_ERROR, id));
}



static void flistError_OnAmHelp(HWND hwnd)
{
    HtmlHelp(lpGlobals->hwndAmeol, setup.szHelpFile, HH_HELP_CONTEXT, HID_FLIST_PARSE_ERROR);
}



static void flistError_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPFlistContext lpFlistContext=GetDlgData(hwnd, FlistContext);

    switch (id) {

    case CID_ABORT:
        closeFile(lpFlistContext->streamFLM);
        freeFlist(&lpFlistContext->flist);
	EndDialog(hwnd, FALSE);
        break;

    case CID_OK:
        if (!IsDlgButtonChecked(hwnd, CID_ERR_FILE)) {
            if (lpFlistContext->lastFlist->flistSelect==FLIST_FROM_FILEPOOL ||
                lpFlistContext->lastFlist->flistSelect==FLIST_FROM_FDIR)
                lpFlistContext->flistFiles--;

            switch (RadioButton(hwnd, CID_ERR_DISCARD, 0)) {

	    case CID_ERR_MEMO:
                lpFlistContext->lastFlist->flistSelect=FLIST_MEMO;
                break;

            case CID_ERR_COMMENT:
                lpFlistContext->lastFlist->flistSelect=FLIST_COMMENT;
                break;

            case CID_ERR_INDENTED:
                lpFlistContext->lastFlist->flistSelect=FLIST_TABBED;
                break;

            case CID_ERR_DISCARD: {
                LPFlistEntry gashFlist=lpFlistContext->lastFlist;

                if (gashFlist==lpFlistContext->flist) {
                    lpFlistContext->flist=NULL;
                    lpFlistContext->lastFlist=NULL;
                } else {
                    lpFlistContext->lastFlist=gashFlist->flistPrevious;
                    lpFlistContext->lastFlist->flistNext=NULL;
                }

                lpFlistContext->flistEntries--;
                gfree(gashFlist);
				gashFlist = NULL;
            }

            }
        }

        lpFlistContext->flistChanged=TRUE;
	EndDialog(hwnd, TRUE);
        break;

    }
}



static LRESULT flistError_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    HANDLE_MSG(hwnd, WM_INITDIALOG, flistError_OnInitDialog);
    HANDLE_MSG(hwnd, WM_POPUPHELP, flistError_OnPopupHelp);
    HANDLE_MSG(hwnd, WM_AMHELP, flistError_OnAmHelp);
    HANDLE_MSG(hwnd, WM_COMMAND, flistError_OnCommand);

    default:
	return(DefDlgProcEx(hwnd, uMsg, wParam, lParam, &lpGlobals->bRecursionFlag));

    }
}



BOOL _EXPORT CALLBACK flistErrorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CheckDefDlgRecursion(&lpGlobals->bRecursionFlag);
    lResult=flistError_DlgProc(hwnd, uMsg, wParam, lParam);
    return(SetDlgMsgResult(hwnd, uMsg, lResult));
}



void uploadFlist(HTOPIC hTopic)
{
    LPCSTR lpcConfName=GetConferenceName(ConferenceFromTopic(hTopic));
    LPCSTR lpcTopicName=GetTopicName(hTopic);
    TOPICINFO topicInfo;
    HSCRIPT hScript=initScript("Moderate", FALSE);

    GetTopicInfo(hTopic, &topicInfo);

    addToScript(hScript, "put `join %s/%s`¬"
			 "if waitfor(`R:`, `M:`) == 0¬"
			     "put `upload`¬"
			     "upload `%s\\flist\\%s.flm`¬"
			     "put `scput flist`¬"
			     "waitfor `R:`¬"
			     "put `quit`¬"
			     "waitfor `M:`¬"
			     "put `killscratch`¬"
			     "waitfor `M:`¬"
			 "endif¬",
		lpcConfName, lpcTopicName,
                (LPSTR)setup.szDataDir, (LPSTR)topicInfo.szFileName);

    actionScript(hScript, OT_PREINCLUDE, "upload flist to %s/%s", lpcConfName, lpcTopicName);
}



BOOL downloadFlist(HTOPIC hTopic, BOOL bAction)
{
    if (bAction) {
	LPCSTR lpcConfName=GetConferenceName(ConferenceFromTopic(hTopic));
	LPCSTR lpcTopicName=GetTopicName(hTopic);
	TOPICINFO topicInfo;
	HSCRIPT hScript=initScript("Moderate", FALSE);

	GetTopicInfo(hTopic, &topicInfo);

	addToScript(hScript, "put `join %s/%s`¬"
			     "if waitfor(`R:`, `M:`) == 0¬"
				 "put `scget flist`¬"
				 "waitfor `R:`¬"
				 "put `quit`¬"
				 "waitfor `M:`¬"
				 "put `download`¬"
				 "download `%s\\flist\\%s.flm`¬"
				 "put `killscratch`¬"
				 "waitfor `M:`¬"
			     "endif¬",
		    lpcConfName, lpcTopicName,
		    (LPSTR)setup.szDataDir, (LPSTR)topicInfo.szFileName);

	actionScript(hScript, OT_INCLUDE, "download moderator flist from %s/%s", lpcConfName, lpcTopicName);
    }

    return(FALSE);
}



BOOL downloadFdir(HTOPIC hTopic, BOOL bAction)
{
    LPCSTR lpcConfName=GetConferenceName(ConferenceFromTopic(hTopic));
    LPCSTR lpcTopicName=GetTopicName(hTopic);
    FDIROBJECT fdirObject;
    BOOL bAlreadyThere=FALSE;

    if (versionCheck(GetAmeolVersion(), AMEOL_1_22)) {
	InitObject(&fdirObject, OT_FDIR, FDIROBJECT);
	lstrcpy(fdirObject.szConfName, lpcConfName);
	lstrcpy(fdirObject.szTopicName, lpcTopicName);
	bAlreadyThere=FindObject(&fdirObject)!=NULL;

	if (bAction && !bAlreadyThere)
	    PutObject(NULL, &fdirObject, NULL);

    } else if (bAction) {
	TOPICINFO topicInfo;
	HSCRIPT hScript=initScript("Moderate", FALSE);

	GetTopicInfo(hTopic, &topicInfo);

	addToScript(hScript, "put `join %s/%s`¬"
			     "if waitfor(`R:`, `M:`) == 0¬"
				 "put `file fdir`¬"
				 "waitfor `R:`¬"
				 "put `quit`¬"
				 "waitfor `M:`¬"
				 "put `download`¬"
				 "download `%s\\flist\\%s.fld`¬"
				 "put `killscratch`¬"
				 "waitfor `M:`¬"
			     "endif¬",
		    lpcConfName, lpcTopicName,
		    (LPSTR)setup.szDataDir, (LPSTR)topicInfo.szFileName);

	actionScript(hScript, OT_INCLUDE, "download fdir from %s/%s", lpcConfName, lpcTopicName);
    }

    return(bAlreadyThere);
}



BOOL downloadFuser(HTOPIC hTopic, BOOL bAction)
{
    LPCSTR lpcConfName=GetConferenceName(ConferenceFromTopic(hTopic));
    LPCSTR lpcTopicName=GetTopicName(hTopic);
    TOPICINFO topicInfo;
    FILELISTOBJECT filelistObject;
    BOOL bAlreadyThere;

    GetTopicInfo(hTopic, &topicInfo);
    InitObject(&filelistObject, OT_FILELIST, FILELISTOBJECT);
    lstrcpy(filelistObject.szConfName, lpcConfName);
    lstrcpy(filelistObject.szTopicName, lpcTopicName);
    bAlreadyThere=FindObject(&filelistObject)!=NULL;

    if (bAction && !bAlreadyThere)
	PutObject(NULL, &filelistObject, NULL);

    return(bAlreadyThere);
}



BOOL downloadMaildir(BOOL bAction)
{
    MAILLISTOBJECT maillistObject;
    BOOL bAlreadyThere;

    InitObject(&maillistObject, OT_MAILLIST, MAILLISTOBJECT);

    bAlreadyThere=FindObject(&maillistObject)!=NULL;

    if (bAction && !bAlreadyThere)
	PutObject(NULL, &maillistObject, NULL);

    return(bAlreadyThere);
}



void deleteTopicFiles(HTOPIC hTopic)
{
    char fullName[LEN_PATHNAME+LEN_FILENAME];

    if (checkFile(hTopic, "FLM", fullName))
	_unlink(fullName);

    if (checkFile(hTopic, "FLD", fullName))
	_unlink(fullName);

    if (checkFile(hTopic, "FLB", fullName))
        _unlink(fullName);

}

