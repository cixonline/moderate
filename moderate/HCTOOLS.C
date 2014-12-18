//  hctools.c - Ameol addon utility routine library
//              © 1993-1997 Pete Jordan, Horus Communications
//              Please see the accompanying file "copyleft.txt" for details
//              of your licence to use and distribute this program.



#include <windows.h>
#include <stdarg.h>
#include "ameolapi.h"
#include "hctools.h"



#define HCTOOLS_VERSION 0x00010B64L		// 1.11.0



/*void alert(HWND hwnd, LPCSTR format,...)
{
    va_list args;
    char buff[1024];

    va_start(args, format);
    wvsprintf(buff, format, args);
    va_end(args);
    MessageBox(hwnd ? hwnd : GetFocus(), buff, "HCTools", MB_OK|MB_ICONSTOP);
}
  */

#ifdef WINDLL
#if defined(WIN32)
    #if defined(__BORLANDC__)
        BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
    #else
        BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
    #endif
#else
    int CALLBACK LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
#endif
{
    return(TRUE);
}

#endif

DWORD FAR PASCAL _EXPORT HCToolsVersion(void)
{
    return(HCTOOLS_VERSION);
}



HCONF FAR PASCAL _EXPORT GetModConference(HCONF hConf)

//  Get a conference that the user moderates. If hConf is NULL, get the
//  first such conference in the list, otherwise get the next one. The
//  function returns NULL is there are no more such conferences.

{
    while ((hConf=GetConference(hConf)) && !IsModerator(hConf, NULL));
    return(hConf);
}



BOOL FAR _EXPORT AmWritePrivateProfileItem(LPCSTR lpApplicationName, LPCSTR lpKeyName, LPCSTR lpFileName, LPCSTR lpFormat,...)
{
    char szBuffer[256];
    va_list args;

    va_start(args, lpFormat);
    wvsprintf(szBuffer, lpFormat, args);
    va_end(args);
    return(AmWritePrivateProfileString(lpApplicationName, lpKeyName, szBuffer, lpFileName));
}



BOOL FAR PASCAL _EXPORT AmGetPrivateProfileBool(LPCSTR lpApplicationName, LPCSTR lpKeyName, BOOL bDefault, LPCSTR lpFileName)

//  Read a Boolean flag from the specified initialisation file. Y, N, YES, NO,
//  ON, OFF, T, F, TRUE, FALSE and numeric values are permitted. No parameter is
//  treated as FALSE, an unrecognised string as TRUE.

{
    char szKey[16];

    lstrcpy(szKey, bDefault ? "1" : "0");
    AmGetPrivateProfileString(lpApplicationName, lpKeyName, szKey, szKey, 16, lpFileName);

    return(*szKey && lstrcmp(szKey, "0") && lstrcmpi(szKey, "N") &&
	   lstrcmpi(szKey, "NO") && lstrcmpi(szKey, "OFF") && lstrcmpi(szKey, "F") &&
	   lstrcmpi(szKey, "FALSE"));

}



void FAR PASCAL _EXPORT GetAmeolDir(LPSTR lpAmeolDir)

//  Get the Ameol base directory

{
    HWND hwndList[7];
    LPSTR lpPtr;

    GetAmeolWindows(hwndList);
    GetModuleFileName(InstanceFromWindow(hwndList[0]), lpAmeolDir, LEN_PATHNAME);

    for (lpPtr=lpAmeolDir+lstrlen(lpAmeolDir);
	 lpPtr>lpAmeolDir && *lpPtr!='\\';
	 lpPtr--);

    *lpPtr='\0';
}

