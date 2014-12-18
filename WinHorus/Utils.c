//  utils.c - Windows API utilities
//            © 1993-1996 Pete Jordan, Horus Communication
//            Please see the accompanying file "copyleft.txt" for details
//            of your licence to use and distribute this program.



#include <stdarg.h>
#include "winhorus.h"
#include "debugdef.h"
#include <ctype.h>



static char etoa[]={
    //  EBCDIC to ASCII translation table
    '\x00', '\x01', '\x02', '\x03', '\x9c', '\x09', '\x86', '\x7f',
    '\x97', '\x8d', '\x8e', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
    '\x10', '\x11', '\x12', '\x13', '\x9d', '\x85', '\x08', '\x87',
    '\x18', '\x19', '\x92', '\x8f', '\x1c', '\x1d', '\x1e', '\x1f',
    '\x80', '\x81', '\x82', '\x83', '\x84', '\x0A', '\x17', '\x1b',
    '\x88', '\x89', '\x8a', '\x8b', '\x8c', '\x05', '\x06', '\x07',
    '\x90', '\x91', '\x16', '\x93', '\x94', '\x95', '\x96', '\x04',
    '\x98', '\x99', '\x9a', '\x9b', '\x14', '\x15', '\x9e', '\x1a',

    '\x20', '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6',
    '\xa7', '\xa8', '\x5b', '\x2e', '\x3c', '\x28', '\x2b', '\x21',
    '\x26', '\xa9', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf',
    '\xb0', '\xb1', '\x5d', '\x24', '\x2a', '\x29', '\x3b', '\x5e',
    '\x2d', '\x2f', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7',
    '\xb8', '\xb9', '\x7c', '\x2c', '\x25', '\x5f', '\x3e', '\x3f',
    '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf', '\xc0', '\xc1',
    '\xc2', '\x60', '\x3a', '\x23', '\x40', '\x27', '\x3d', '\x22',

    '\xc3', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67',
    '\x68', '\x69', '\xc4', '\xc5', '\xc6', '\xc7', '\xc8', '\xc9',
    '\xca', '\x6a', '\x6b', '\x6c', '\x6d', '\x6e', '\x6f', '\x70',
    '\x71', '\x72', '\xcb', '\xcc', '\xcd', '\xce', '\xcf', '\xd0',
    '\xd1', '\x7e', '\x73', '\x74', '\x75', '\x76', '\x77', '\x78',
    '\x79', '\x7a', '\xd2', '\xd3', '\xd4', '\xd5', '\xd6', '\xd7',
    '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd', '\xde', '\xdf',
    '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7',

    '\x7b', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47',
    '\x48', '\x49', '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed',
    '\x7d', '\x4a', '\x4b', '\x4c', '\x4d', '\x4e', '\x4f', '\x50',
    '\x51', '\x52', '\xee', '\xef', '\xf0', '\xf1', '\xf2', '\xf3',
    '\x5c', '\x9f', '\x53', '\x54', '\x55', '\x56', '\x57', '\x58',
    '\x59', '\x5a', '\xf4', '\xf5', '\xf6', '\xf7', '\xf8', '\xf9',
    '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37',
    '\x38', '\x39', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff'
};



static char atoe[]={
    //  ASCII to EBCDIC translation table
    '\x00', '\x01', '\x02', '\x03', '\x37', '\x2d', '\x2e', '\x2f',
    '\x16', '\x05', '\x25', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
    '\x10', '\x11', '\x12', '\x13', '\x3c', '\x3d', '\x32', '\x26',
    '\x18', '\x19', '\x3f', '\x27', '\x1c', '\x1d', '\x1e', '\x1f',
    '\x40', '\x4f', '\x7f', '\x7b', '\x5b', '\x6c', '\x50', '\x7d',
    '\x4d', '\x5d', '\x5c', '\x4e', '\x6b', '\x60', '\x4b', '\x61',
    '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5', '\xf6', '\xf7',
    '\xf8', '\xf9', '\x7a', '\x5e', '\x4c', '\x7e', '\x6e', '\x6f',

    '\x7c', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5', '\xc6', '\xc7',
    '\xc8', '\xc9', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5', '\xd6',
    '\xd7', '\xd8', '\xd9', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6',
    '\xe7', '\xe8', '\xe9', '\x4a', '\xe0', '\x5a', '\x5f', '\x6d',
    '\x79', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
    '\x88', '\x89', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96',
    '\x97', '\x98', '\x99', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6',
    '\xa7', '\xa8', '\xa9', '\xc0', '\x6a', '\xd0', '\xa1', '\x07',

    '\x20', '\x21', '\x22', '\x23', '\x24', '\x15', '\x06', '\x17',
    '\x28', '\x29', '\x2a', '\x2b', '\x2c', '\x09', '\x0a', '\x1b',
    '\x30', '\x31', '\x1a', '\x33', '\x34', '\x35', '\x36', '\x08',
    '\x38', '\x39', '\x3a', '\x3b', '\x04', '\x14', '\x3e', '\xe1',
    '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48',
    '\x49', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57',
    '\x58', '\x59', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67',
    '\x68', '\x69', '\x70', '\x71', '\x72', '\x73', '\x74', '\x75',

    '\x76', '\x77', '\x78', '\x80', '\x8a', '\x8b', '\x8c', '\x8d',
    '\x8e', '\x8f', '\x90', '\x9a', '\x9b', '\x9c', '\x9d', '\x9e',
    '\x9f', '\xa0', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf',
    '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7',
    '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf',
    '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf', '\xda', '\xdb',
    '\xdc', '\xdd', '\xde', '\xdf', '\xea', '\xeb', '\xec', '\xed',
    '\xee', '\xef', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff'
};



LPCSTR FAR PASCAL _EXPORT ebcdic2ascii(void)
{
    return(etoa);
}



LPCSTR FAR PASCAL _EXPORT ascii2ebcdic(void)
{
    return(atoe);
}



#if defined(_DEBUG)
    static void FAR PASCAL __alertDisplay(LPCSTR lpcszMessage)
    {
        #if defined(MVS)
            fprintf(stderr, "WinHorus Debug: %s\n", lpcszMessage);
        #else
            OutputDebugString("\n\rWinHorus Debug: ");
            OutputDebugString(lpcszMessage);
        #endif
    }



    static AlertDisplay _alertDisplay=__alertDisplay;
    UINT _alertLevel=1;
#endif



AlertDisplay FAR PASCAL _EXPORT alertDisplay(AlertDisplay newDisplay)
{
    #if defined(_DEBUG)
        if (newDisplay) {
            AlertDisplay oldDisplay=_alertDisplay;

            _alertDisplay=newDisplay;
            return(oldDisplay);
        } else
            return(_alertDisplay);

    #else
        return(NULL);
    #endif
}



UINT FAR PASCAL _EXPORT alertLevel(UINT newLevel)
{
    #if defined(_DEBUG)
        if (newLevel) {
            UINT oldLevel=_alertLevel;

            _alertLevel=newLevel;
            return(oldLevel);
        } else
            return(_alertLevel);

    #else
        return(0);
    #endif
}



int _alert(UINT uLevel, LPCSTR lpcszFormat,...)
{
    #if defined(_DEBUG)
        if (uLevel<=_alertLevel) {
            va_list args;
            static char szBuff[1000];

            va_start(args, lpcszFormat);
            wvsprintf(szBuff, lpcszFormat, args);
            va_end(args);
            _alertDisplay(szBuff);
        }
    #endif

    return(1);
}



void FAR PASCAL _EXPORT versionDecode(DWORD dwVersionCode, LPVersionInfo lpVersionInfo)
{
    lpVersionInfo->nVersion=(int)((dwVersionCode>>16)&0xff);
    lpVersionInfo->nRelease=(int)((dwVersionCode>>8)&0xff);
    lpVersionInfo->nRevision=(int)((dwVersionCode&0xff)%100);
    lpVersionInfo->nBeta=(int)(dwVersionCode>>24);
    lpVersionInfo->bEval=(dwVersionCode&0xff)>=200;
}



BOOL FAR PASCAL _EXPORT versionCheck(DWORD dwVersionCheck, DWORD dwVersionMinimum)
{
    VersionInfo versionInfoCheck, versionInfoMinimum;

    versionDecode(dwVersionCheck, &versionInfoCheck);
    versionDecode(dwVersionMinimum, &versionInfoMinimum);

    return(versionInfoCheck.nVersion>versionInfoMinimum.nVersion ||
           versionInfoCheck.nVersion==versionInfoMinimum.nVersion &&
           (versionInfoCheck.nRelease>versionInfoMinimum.nRelease ||
            versionInfoCheck.nRelease==versionInfoMinimum.nRelease &&
            (versionInfoCheck.nRevision>versionInfoMinimum.nRevision ||
             versionInfoCheck.nRevision==versionInfoMinimum.nRevision &&
             (versionInfoCheck.nBeta==0 ||
              versionInfoCheck.nBeta>=versionInfoMinimum.nBeta))));

}



int FAR PASCAL _EXPORT lstrncmp(LPCSTR lpcS1, LPCSTR lpcS2, UINT uMax)
{
    while (*lpcS1 && uMax>0 && *lpcS1== *lpcS2) {
        lpcS1++;
        lpcS2++;
        uMax--;
    }

    return(uMax==0 ? 0 : (int)(BYTE)*lpcS1-(int)(BYTE)*lpcS2);
}



int FAR PASCAL _EXPORT lstrncmpi(LPCSTR lpcS1, LPCSTR lpcS2, UINT uMax)
{
    while (*lpcS1 && uMax>0 && tolower(*lpcS1)==tolower(*lpcS2)) {
        lpcS1++;
        lpcS2++;
        uMax--;
    }

    return(uMax==0 ? 0 : (int)(BYTE)tolower(*lpcS1)-(int)(BYTE)tolower(*lpcS2));
}



LPSTR FAR PASCAL _EXPORT lstrcpynz(LPSTR lpDest, LPCSTR lpcSrc, int nMax)

//  Copy up to 'nMax' chars from source to destination; destination is always NUL
//  terminated (hence should be at least nMax+1 chars long)

{
    LPSTR lpRet=lpDest;

    while (*lpcSrc && nMax>0) {
        *lpDest++ = *lpcSrc++;
        nMax--;
    }

    *lpDest='\0';
    return(lpRet);
}



LPSTR FAR PASCAL _EXPORT strupper(LPSTR lpDest, LPCSTR lpcSrc)
{
    LPSTR lpPtr=lpDest;
    char ch;

    while (ch= *lpcSrc++)
        *lpPtr++ =toupper(ch);

    *lpPtr='\0';
    return(lpDest);
}



LPSTR FAR PASCAL _EXPORT strlower(LPSTR lpDest, LPCSTR lpcSrc)
{
    LPSTR lpPtr=lpDest;
    char ch;

    while (ch= *lpcSrc++)
        *lpPtr++ =tolower(ch);

    *lpPtr='\0';
    return(lpDest);
}



LPSTR FAR PASCAL _EXPORT strmixed(LPSTR lpDest, LPCSTR lpcSrc)
{
    LPSTR lpPtr=lpDest;
    char ch;
    BOOL fBreak=TRUE;

    while (ch= *lpcSrc++) {
        if (isalpha(ch)) {
            ch=fBreak ? toupper(ch) : tolower(ch);
            fBreak=FALSE;
        } else
            fBreak=TRUE;

        *lpPtr++ =ch;
    }

    *lpPtr='\0';
    return(lpDest);
}



void FAR PASCAL _EXPORT clearStatus(LPStatus lpStatus)
{
    if (lpStatus) {
        lmemset(lpStatus->statusItem, 0,
                sizeof(StatusItem)*lpStatus->nCount);

        lpStatus->nSet=0;
        lpStatus->nErrcode=0;
    }
}



LPStatus FAR PASCAL _EXPORT initStatus(int nCount)
{
    LPStatus lpStatus=NULL;

    if (1<=nCount && nCount<=8) {
        lpStatus=gmalloc(sizeof(Status)+sizeof(StatusItem)*(nCount-1));

        if (lpStatus) {
            lpStatus->nCount=nCount;
            clearStatus(lpStatus);
        }
    }

    return(lpStatus);
}



BOOL FAR PASCAL _EXPORT setStatus(LPStatus lpStatus, unsigned uSource, int nStatus)
{
    if (lpStatus && lpStatus->nSet<lpStatus->nCount &&
        (nStatus!=0 || (uSource&STATUS_MASK)!=0)) {
        LPStatusItem lpStatusItem=lpStatus->statusItem+lpStatus->nSet++;

        lpStatusItem->uSource=uSource;
        lpStatusItem->nStatus=nStatus;

        if (lpStatus->nErrcode==0 && (uSource&STATUS_MASK)==0)
            lpStatus->nErrcode=nStatus;

        return(TRUE);
    } else
        return(nStatus==0 && (uSource&STATUS_MASK)==0);

}



BOOL FAR PASCAL _EXPORT setError(LPStatus lpStatus, int nErrcode)
{
    if (lpStatus) {
        lpStatus->nErrcode=nErrcode;
        return(TRUE);
    } else
        return(FALSE);

}



BOOL FAR PASCAL _EXPORT decodeStatus(LPStatus lpStatus, int nItem,
                                     LPSTR lpszBuffer, int nMax)
{
    if (lpStatus &&
        0<=nItem && nItem<lpStatus->nSet) {
        LPStatusItem lpStatusItem=lpStatus->statusItem+nItem;
        char szBuffer[128];

        if ((lpStatusItem->uSource&STATUS_MASK)==0)
            switch (lpStatusItem->uSource) {

            case OS_STATUS:
                wsprintf(szBuffer, "system(%d): %s",
                         lpStatusItem->nStatus,
                         (LPCSTR)strerror(lpStatusItem->nStatus));

                break;

            case GMALLOC_STATUS:
                wsprintf(szBuffer, "gmalloc(%d): %s",
                         lpStatusItem->nStatus,
                         gstrerror(lpStatusItem->nStatus));

                break;

            case STREAMIO_STATUS:
                wsprintf(szBuffer, "streamio(%d): %s",
                         lpStatusItem->nStatus,
                         streamioErrtext(lpStatusItem->nStatus));

                break;

            #if defined(MVS)
            case PDSLIST_STATUS:
                wsprintf(szBuffer, "pdslist(%d): %s",
                         lpStatusItem->nStatus,
                         pdslistErrtext(lpStatusItem->nStatus));

                break;

            case SVC99_STATUS:
                wsprintf(szBuffer, "svc99: error='%x'x (%d) info='%x'x (%d)",
                         HIWORD(lpStatusItem->nStatus), HIWORD(lpStatusItem->nStatus),
                         LOWORD(lpStatusItem->nStatus), LOWORD(lpStatusItem->nStatus));

                break;

            #endif

            default:
                wsprintf(szBuffer, "subsystem %d: status %d",
                         lpStatusItem->uSource, lpStatusItem->nStatus);

                break;

            }
        else {
            LPCSTR lpcszPrefix;

            switch (lpStatusItem->uSource&SOURCE_MASK) {

            case OS_STATUS:       lpcszPrefix="system";    break;
            case GMALLOC_STATUS:  lpcszPrefix="gmalloc";   break;
            case STREAMIO_STATUS: lpcszPrefix="streamio";  break;
            #if defined(MVS)
            case PDSLIST_STATUS:  lpcszPrefix="pdslist";   break;
            case SVC99_STATUS:    lpcszPrefix="svc99";     break;
            #endif

            default:              lpcszPrefix="?";

            }

            wsprintf(szBuffer, "%s: %d => %d", lpcszPrefix,
                     lpStatusItem->uSource&STATUS_MASK,
                     lpStatusItem->nStatus);

        }

        lstrcpynz(lpszBuffer, szBuffer, nMax);
        return(TRUE);
    } else {
        *lpszBuffer='\0';
        return(FALSE);
    }
}

#if defined(MVS)

    #pragma linkage(hvdecode, COBOL)
    #pragma linkage(hvcheck,  COBOL)
    #pragma linkage(herrtext, COBOL)



    void hvdecode(int nVersionCode, LPVersionInfo lpVersionInfo)
    {
        versionDecode((DWORD)nVersionCode, lpVersionInfo);
    }



    void hvcheck(int nVersionCheck, int nVersionMinimum, LPINT lpnOK)
    {
        *lpnOK=versionCheck((DWORD)nVersionCheck, (DWORD)nVersionMinimum);
    }



    int getCobolString(PSTR pszDest, PSTR psSource, int nLength)
    {
        int nCount;

        for (nCount=0;
             nCount<nLength && *psSource && *psSource!=' ';
             nCount++)
            *pszDest++ = *psSource++;

        return(nCount);
    }



    int putCobolString(PSTR psDest, PSTR pszSource, int nLength)
    {
        int nCount;

        for (nCount=0;
             nCount<nLength && *pszSource;
             nCount++)
            *psDest++ = *pszSource++;

        if (nCount<nLength) {
            memset(psDest, ' ', nLength-nCount);
            return(0);
        } else
            return(*pszSource ? strlen(pszSource) : 0);

    }



    void herrtext(PStatus pStatus, int nItem, PSTR psErrtext, int nBufflength)
    {
        char szErrtext[128];

        decodeStatus(pStatus, nItem, szErrtext, 127);
        putCobolString(psErrtext, szErrtext, nBufflength);
    }

#else

    int FAR PASCAL _EXPORT RadioButton(HWND hDlg, int nGroupID, int nButtonID)

    //  Get/set radio button state. Note that this could have untoward effects
    //  if the next control after the radio buttons doesn't have WM_GROUP set...
    //
    //  hDlg    identifies the dialog box that contains the buttons
    //  nGroupID    the integer ID of the first (or any) control in the group
    //  nButtonID   the integer ID of the button to be selected, -1 or NULL
    //
    //  Return value: the integer ID of the button previously selected (or
    //  currently selected if nButtonID==NULL)

    {
        HWND hGroup=GetDlgItem(hDlg, nGroupID);
        HWND hOld=NULL;

        if (hGroup) {
            HWND hNew=nButtonID>0 ? GetDlgItem(hDlg, nButtonID) : NULL;
            HWND hCurrent=hGroup;

            do {
                if (SendMessage(hCurrent, BM_GETCHECK, 0, 0L)) {
                    hOld=hCurrent;

                    if (nButtonID && hCurrent!=hNew)
                        SendMessage(hCurrent, BM_SETCHECK, FALSE, 0L);

                } else if (hCurrent==hNew)
                    SendMessage(hCurrent, BM_SETCHECK, TRUE, 0L);

                hCurrent=GetNextDlgGroupItem(hDlg, hCurrent, TRUE);
            } while (hCurrent!=hGroup);
        }

        return(hOld ? GetDlgCtrlID(hOld) : 0);
    }



    BOOL FAR _EXPORT WritePrivateProfileItem(LPCSTR lpApplicationName, LPCSTR lpKeyName, LPCSTR lpFileName, LPCSTR lpFormat,...)
    {
        char szBuffer[256];
        va_list args;

        va_start(args, lpFormat);
        wvsprintf(szBuffer, lpFormat, args);
        va_end(args);
        return(WritePrivateProfileString(lpApplicationName, lpKeyName, szBuffer, lpFileName));
    }



    BOOL FAR PASCAL _EXPORT GetPrivateProfileBool(LPCSTR lpApplicationName, LPCSTR lpKeyName, BOOL bDefault, LPCSTR lpFileName)

    //  Read a Boolean flag from the specified initialisation file. Y, N, YES, NO,
    //  ON, OFF, T, F, TRUE, FALSE and numeric values are permitted. No parameter is
    //  treated as FALSE, an unrecognised string as TRUE.

    {
        char szKey[16];

        lstrcpy(szKey, bDefault ? "1" : "0");
        GetPrivateProfileString(lpApplicationName, lpKeyName, szKey, szKey, 16, lpFileName);

        return(*szKey && lstrcmp(szKey, "0") && lstrcmpi(szKey, "N") &&
               lstrcmpi(szKey, "NO") && lstrcmpi(szKey, "OFF") && lstrcmpi(szKey, "F") &&
               lstrcmpi(szKey, "FALSE"));

    }
#endif

