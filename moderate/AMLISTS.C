#include <windows.h>
#include "ameolapi.h"
#include "..\winhorus\winhorus.h"
#include "hctools.h"



HAMLIST FAR PASCAL _EXPORT OpenAmeolList(LPCSTR lpcName)

//  Open one of the Ameol global lists files

{
    LPSTR lpAmeolDir=(LPSTR)gmalloc(LEN_PATHNAME);
    LPSTR lpConfList=(LPSTR)gmalloc(LEN_PATHNAME);
    HSTREAM hStream;

    GetAmeolDir(lpAmeolDir);
    wsprintf(lpConfList, "%s\\%s", lpAmeolDir, lpcName);
    hStream=openFile(lpConfList, OF_READ);

    if (!hStream) {
	LPSTR lpDataDir=(LPSTR)gmalloc(LEN_PATHNAME);
	LPSTR lpIniFile=lpConfList;

	wsprintf(lpDataDir, "%s\\data", lpAmeolDir);
	GetInitialisationFile(lpIniFile);
	AmGetPrivateProfileString("Directories", "data", lpDataDir, lpDataDir, LEN_PATHNAME, lpIniFile);
	wsprintf(lpConfList, "%s\\%s", lpDataDir, lpcName);
	hStream=openFile(lpConfList, OF_READ);
	gfree(lpDataDir);	lpDataDir = NULL;
    }

    gfree(lpConfList);	lpConfList = NULL;
    gfree(lpAmeolDir);	lpAmeolDir = NULL;

    if (hStream) 
	{
		unsigned char szCheck[2];

//		if (readBlock(hStream, szCheck, 2)!=2 || szCheck[0]!=0x91 || szCheck[1]!=0x48) 
		if (readBlock(hStream, szCheck, 2)!=2 || szCheck[0]!=0x92 || szCheck[1]!=0x48) 
		{
			closeFile(hStream);
			hStream=NULL;
		}
    }

    return((HAMLIST)hStream);
}



int FAR PASCAL _EXPORT ReadConferenceList(HAMLIST hStream, LPConfListEntry lpConfListEntry)

//  Read one entry from the CIX conference list; return 0 for OK, negative status code otherwise

{
    char szBuff[256];
    int nStatus;

    while ((nStatus=readLine((HSTREAM)hStream, szBuff, 255))>=0) 
	{
		LPSTR lpPtr=szBuff;

		if (*lpPtr++ =='@' && *lpPtr) 
		{
			LPSTR lpConfName=lpPtr+1;

			lpConfListEntry->eCategory= *lpPtr=='o' ? CONF_OPEN : *lpPtr=='c' ? CONF_CLOSED : CONF_UNKNOWN;
			while (*(++lpPtr) && *lpPtr!='\x09');

			if (*lpPtr)
			lstrcpynz(lpConfListEntry->szConfDescription, lpPtr+1, LEN_CONFDESCRIPTION-1);
			else
			*lpConfListEntry->szConfDescription='\0';

			*lpPtr='\0';
			lstrcpynz(lpConfListEntry->szConfName, lpConfName, LEN_CONFNAME-1);
			return(0);
		}
    }

    return(nStatus);
}



int FAR PASCAL _EXPORT ReadUsenetList(HAMLIST hStream, LPUsenetListEntry lpUsenetListEntry)

//  Read one entry from the CIX Usenet group list; return 0 for OK, negative status code otherwise

{
    char szBuff[256];
    int nStatus;

    while ((nStatus=readLine((HSTREAM)hStream, szBuff, 255))>=0) 
	{
		LPSTR lpPtr=szBuff;

		if (*lpPtr++ =='@' && *lpPtr++) 
		{
			LPSTR lpGroupName=lpPtr;

			while (*lpPtr && *lpPtr!='\x09')
			lpPtr++;

			if (*lpPtr)
			lstrcpynz(lpUsenetListEntry->szGroupDescription, lpPtr+1, LEN_UNETDESCRIPTION-1);
			else
			*lpUsenetListEntry->szGroupDescription='\0';

			*lpPtr='\0';
			lstrcpynz(lpUsenetListEntry->szGroupName, lpGroupName, LEN_UNETNAME-1);
			return(0);
		}
    }

    return(nStatus);
}



int FAR PASCAL _EXPORT ReadUserList(HAMLIST hStream, LPUserListEntry lpUserListEntry)

//  Read one entry from the CIX User list; return 0 for OK, negative status code otherwise

{
    char szBuff[256];
    int nStatus;

    while ((nStatus=readLine((HSTREAM)hStream, szBuff, 255))>=0) 
	{
		LPSTR lpPtr=szBuff;

		if (*lpPtr++ =='@' && *lpPtr++) 
		{
			LPSTR lpUserName=lpPtr;

			while (*lpPtr && *lpPtr!='\x09')
			lpPtr++;

			if (*lpPtr)
			lstrcpynz(lpUserListEntry->szUserDescription, lpPtr+1, LEN_USERDESCRIPTION-1);
			else
			*lpUserListEntry->szUserDescription='\0';

			*lpPtr='\0';
			lstrcpynz(lpUserListEntry->szUserName, lpUserName, LEN_CIXNAME-1);
			return(0);
		}
    }

    return(nStatus);
}



void FAR PASCAL _EXPORT CloseAmeolList(HAMLIST hStream)
{
    closeFile((HSTREAM)hStream);
}
