#include <windows.h>
#include <windowsx.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "msgedit.h"
#include "flist.h"
#include "malloc.h"

size_t __cdecl strftime (
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr
        );


void AddMailNotifyItem(LPFdirEntry lpFdirEntry, int type)
{
	char lpszText[10240];
	char szContributor[LEN_CIXNAME];
    char szFileName[LEN_CIXFILENAME];
	char szFileDate[64];
	char szFileSize[64];
	MAILOBJECT mailObject;
	LPFlistEntry lpFlistEntry;// = (LPFlistEntry)lpFdirEntry->fdirFlistEntry[0];
//	Context context = (Context)lpGlobals->context;

	if (lpFdirEntry->fdirFlistEntry[0] != NULL )
	{
		lpFlistEntry = (LPFlistEntry)lpFdirEntry->fdirFlistEntry[0];
	}
	lstrcpy( szContributor, "$CIX$" );
	GetRegistry((LPSTR)&szContributor);

	if(lstrcmp(lpGlobals->context.lpcConfName, "filepool") == 0)
		return;

	formatLong(szFileSize, "", lpFdirEntry->fdirSize, " bytes");
	strftime(szFileDate, 63, "%A %d %B %Y", localtime(&lpFdirEntry->fdirTimestamp));
    setCase(szFileName, lpFdirEntry->fdirName, setup.wNotifyCase);

	if (lpFdirEntry->fdirFlistEntry[0] != NULL )
	{
		wsprintf(lpszText, "\r\n"
					 " %s\r\n"
					 "\r\n"
					 "   Filename: %s\r\n"
					 "    Hotlink: cixfile:\r\n"
					 "  File size: %s\r\n"
					 "Contributor: %s\r\n"
					 "       Date: %s\r\n"
					 "%s"
					 "\r\n"
					 "Description: \r\n\r\n"
					 "%s\r\n",
					 type?"The following local file has been exported to the filepool:":"The following file has been exported to the filepool:",
				   (LPSTR)szFileName, 
				   (LPSTR)szFileSize,
				   (LPSTR)szContributor, 
				   (LPSTR)szFileDate,
				   setup.os_and_status?"     Status: \r\n        O/S: \r\n":"",
				   (LPSTR)lpFlistEntry->flistDescription);
	}
	else
	{
		wsprintf(lpszText, "\r\n"
					 " %s\r\n"
					 "\r\n"
					 "   Filename: %s\r\n"
					 "    Hotlink: cixfile:\r\n"
					 "  File size: %s\r\n"
					 "Contributor: %s\r\n"
					 "       Date: %s\r\n"
					 "%s"
					 "\r\n"
					 "Description: <Unknown>\r\n\r\n"
					 "\r\n",
					 type?"The following local file has been exported to the filepool:":"The following file has been exported to the filepool:",
				   (LPSTR)szFileName, 
				   (LPSTR)szFileSize,
				   (LPSTR)szContributor, 
				   (LPSTR)szFileDate,
				   setup.os_and_status?"     Status: \r\n        O/S: \r\n":"");
	}
	InitObject(&mailObject, OT_MAIL, MAILOBJECT);
	lstrcpy(mailObject.szTo, "filepool@cix.co.uk");
	lstrcpy(mailObject.szSubject, "Export To Filepool - ");
	lstrcat(mailObject.szSubject, szFileName);
	mailObject.szCC[0] = '\x0';
	mailObject.szReply[0] = '\x0';
	PutObject(NULL, &mailObject, lpszText);
}

