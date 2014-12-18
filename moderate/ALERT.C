#include <windows.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "globals.h"
#include "setup.h"



void busy(BOOL waitFlag)
{
    static HCURSOR waitCursor=NULL;
    static HCURSOR oldCursor=NULL;

    if (!waitCursor)
        waitCursor=LoadCursor(NULL, IDC_WAIT);

    if (waitFlag)
        oldCursor=SetCursor(waitCursor);
    else
        SetCursor(oldCursor);

}



void inform(HWND hwnd, LPCSTR format,...)
{
    va_list args;
    char buff[1024];

    va_start(args, format);
    wvsprintf(buff, format, args);
    va_end(args);
    MessageBox(hwnd ? hwnd : GetFocus(), buff, "Moderate", MB_OK|MB_ICONEXCLAMATION);
}



void alert(HWND hwnd, LPCSTR format,...)
{
    va_list args;
    char buff[1024];

    va_start(args, format);
    wvsprintf(buff, format, args);
    va_end(args);
    MessageBox(hwnd ? hwnd : GetFocus(), buff, "Moderate", MB_OK|MB_ICONSTOP);
}



int query(HWND hwnd, WORD wOptions, LPCSTR format,...)
{
    va_list args;
    char buff[1024];

    va_start(args, format);
    wvsprintf(buff, format, args);
    va_end(args);

    if (!(wOptions&MB_ICONMASK))
        wOptions|=MB_ICONQUESTION;

    return(MessageBox(hwnd ? hwnd : GetFocus(), buff, "Moderate", wOptions));
}



static LPSTR extractNumber(LPSTR lpPtr, LONG lValue)
{
    if (lValue>=1000L) {
        lpPtr=extractNumber(lpPtr, lValue/1000L);
        lstrcpy(lpPtr, setup.szThousandsSep);
        lpPtr+=wsprintf(lpPtr, "%s%03ld", (LPSTR)setup.szThousandsSep, lValue%1000L);
    } else
        lpPtr+=wsprintf(lpPtr, "%ld", lValue);

    return(lpPtr);
}



LPCSTR formatLong(LPSTR lpBuff, LPCSTR lpcBefore, LONG lValue, LPCSTR lpcAfter)
{
    LPSTR lpPtr=lpBuff;

    while (*lpcBefore)
        *lpPtr++ = *lpcBefore++;

    if (lValue<0)
        *lpPtr++ ='-';

    lpPtr=extractNumber(lpPtr, lValue);
    lstrcpy(lpPtr, lpcAfter);
    return(lpBuff);
}



LPCSTR setCase(LPSTR lpBuff, LPCSTR lpcText, WORD wSelect)
{
    char cPrev='\0';

    while (*lpcText) {
        *lpBuff++ =wSelect==0 ||
                   wSelect==2 && isalpha(cPrev) ? tolower(*lpcText)
                                                : toupper(*lpcText);

        cPrev= *lpcText++;            
    }

    *lpBuff='\0';
    return(lpBuff);
}



time_t datediff(time_t time1, time_t time2)
{
    static const time_t secondsInDay=60L*60L*24L;

    return((time1-time2)/secondsInDay);
}



time_t parseTime(LPSTR lpBuff)
{
    static const char monthNames[12][4]={
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    struct tm buildTime;
    char szBuff[128];
    LPSTR lpToken;
    int nIndex;

    lstrcpy(szBuff, lpBuff);
    lpToken=strtok(szBuff, " :");

    if (!lpToken)
        return(0);

    for (nIndex=0; nIndex<12 && lstrcmpi(lpToken, monthNames[nIndex]); nIndex++);

    if (nIndex>=12)
        return(0);

    buildTime.tm_mon=nIndex;
    lpToken=strtok(NULL, " :");

    if (!lpToken)
        return(0);

    buildTime.tm_mday=atoi(lpToken);
    lpToken=strtok(NULL, " :");

    if (!lpToken)
        return(0);

    buildTime.tm_hour=atoi(lpToken);
    lpToken=strtok(NULL, " :");

    if (!lpToken)
        return(0);

    buildTime.tm_min=atoi(lpToken);
    lpToken=strtok(NULL, " :");

    if (!lpToken)
        return(0);

    buildTime.tm_year=atoi(lpToken);
	if(buildTime.tm_year < 70)
		buildTime.tm_year += 100;
    buildTime.tm_sec=0;
    buildTime.tm_wday=0;
    buildTime.tm_yday=0;
    buildTime.tm_isdst=0;
    return(mktime(&buildTime));
}



int fontHeight(HWND hdlg)
{
    int nTextHeight;
    HDC hDC=GetDC(hdlg);
    TEXTMETRIC tm;

    GetTextMetrics(hDC, &tm);
    nTextHeight=tm.tmHeight;
    ReleaseDC(hdlg, hDC);
    return(nTextHeight);
}



BOOL setWindowData(HWND hwnd, LPCSTR lpcID, LPVOID lpData)
{
#ifdef WIN32// (sizeof(HANDLE) == 4)
  	return(SetProp(hwnd, lpcID, (HANDLE)lpData));
#else
	 char szID1[32];
	 BOOL bOK;

	 wsprintf(szID1, "%s.1", lpcID);
	 bOK=SetProp(hwnd, szID1, (HANDLE)(SELECTOROF(lpData)));

	 if (bOK) {
	     char szID2[32];

	     wsprintf(szID2, "%s.2", lpcID);
	     bOK=SetProp(hwnd, szID2, (HANDLE)(OFFSETOF(lpData)));

	     if (!bOK)
		 RemoveProp(hwnd, szID1);

	 }

	 return(bOK);
#endif
}



LPVOID getWindowData(HWND hwnd, LPCSTR lpcID)
{
#ifdef WIN32 // (sizeof(HANDLE) == 4)

   return((LPVOID)GetProp(hwnd, lpcID));
#else
char szID1[32], szID2[32];

 wsprintf(szID1, "%s.1", lpcID);
 wsprintf(szID2, "%s.2", lpcID);
 return(MAKELP(GetProp(hwnd, szID1), GetProp(hwnd, szID2)));
#endif
}



void freeWindowData(HWND hwnd, LPCSTR lpcID)
{
#ifdef WIN32 // (sizeof(HANDLE)==4)
    RemoveProp(hwnd, lpcID);
#else
   char szID1[32], szID2[32];

    wsprintf(szID1, "%s.1", lpcID);
    RemoveProp(hwnd, szID1);
    wsprintf(szID2, "%s.2", lpcID);
    RemoveProp(hwnd, szID2);
#endif
}

