//  amscript.c - Ameol script creation utility routines
//               © 1993-1997 Pete Jordan, Horus Communications
//               Please see the accompanying file "copyleft.txt" for details
//               of your licence to use and distribute this program.



#include <windows.h>
#include <io.h>
#include <stdarg.h>
#include "ameolapi.h"
#include "..\winhorus\winhorus.h"
#include "hctools.h"
#include "_script.h"



static char scriptPath[LEN_PATHNAME];
static BOOL pathSet=FALSE;
static int scriptErrcode=0;
static LPSTR lpTranslate=NULL;
static LPSTR lpFormatBuff=NULL;



int FAR PASCAL _EXPORT lastScriptError(HSCRIPT hScript)
{
    LPScript lpScript=(LPScript)hScript;

    return(lpScript ? lpScript->scriptErrcode : scriptErrcode);
}



HSCRIPT FAR PASCAL _EXPORT initScript(LPCSTR prefix, BOOL bSuspend)
{
    LPScript lpScript=(LPScript)gmalloc(sizeof(Script));
    char fnScript[LEN_PATHNAME+LEN_FILENAME];
    int status=0;
    int nPtr;

    if (!pathSet) {
	char setupIniFile[256];

	GetInitialisationFile(setupIniFile);

	AmGetPrivateProfileString("Directories", "script", "\\ameol\\script",
				  scriptPath, LEN_PATHNAME-1, setupIniFile);

	pathSet=TRUE;
    }

    if (!lpTranslate) 
	{
		lpTranslate=(LPSTR)gmalloc(257);
	}
	
	if (lpTranslate) {

	    for (nPtr=0; nPtr<256; nPtr++)
		lpTranslate[nPtr]=(char)nPtr;

	    lpTranslate['`']='"';
	    lpTranslate[(BYTE)'¬']='\n';
	}

    if (!lpScript || !lpTranslate) 
	{
		if (lpScript)
		{
			gfree(lpScript);	lpScript = NULL;
		}

		scriptErrcode=ERR_SCRIPT_MEMORY;
		return(NULL);
    }

    lstrcpy(lpScript->scriptPrefix, prefix);
    lpScript->scriptIndex=0;

    do {
        wsprintf(fnScript, "%s\\%s.%03d", (LPSTR)scriptPath, lpScript->scriptPrefix, lpScript->scriptIndex);
        lpScript->scriptStream=openFile(fnScript, OF_READ);

        if (lpScript->scriptStream)
            closeFile(lpScript->scriptStream);

    } while (lpScript->scriptStream && ++lpScript->scriptIndex<1000);

    if (lpScript->scriptStream)
        status=ERR_SCRIPT_NAME_OVERFLOW;
    else if (!(lpScript->scriptStream=openFile(fnScript, OF_WRITE|OF_CRLFCONVERT)))
        status=ERR_SCRIPT_CREATE;

    if (status) 
	{
        scriptErrcode=status;
        gfree(lpScript);	lpScript = NULL;
		return(NULL);
    } 
	else 
	{
		lpScript->scriptEmpty=TRUE;
		lpScript->scriptSuspend=bSuspend;

		if (bSuspend)
			suspendFile(lpScript->scriptStream);

		return((HSCRIPT)lpScript);
    }
}



LPCSTR FAR PASCAL _EXPORT scriptFile(HSCRIPT hScript, WORD wSelect)
{
    LPScript lpScript=(LPScript)hScript;

    if (!lpScript) {
        scriptErrcode=ERR_SCRIPT_NOT_OPEN;
        return(NULL);
    } else
        switch (wSelect) {

        case 0:
            return(openFileName(lpScript->scriptStream));

        case 1: {
            static char name[13];

            wsprintf(name, "%s.%03d", lpScript->scriptPrefix, lpScript->scriptIndex);
            return(name);
        }

        case 2: {
            static char ext[4];

            wsprintf(ext, "%03d", lpScript->scriptIndex);
            return(ext);
        }

        default:
            return(NULL);

        }

}



int FAR _EXPORT addToScript(HSCRIPT hScript, LPCSTR lpcFormat,...)
{
    LPScript lpScript=(LPScript)hScript;
    int nStatus=0;

    if (!lpScript) {
        scriptErrcode=ERR_SCRIPT_NOT_OPEN;
        return(ERR_SCRIPT_NOT_OPEN);
    }

    if (reopenFile(lpScript->scriptStream))
        nStatus=ERR_SCRIPT_REOPEN;
    else {
        if (!lpFormatBuff) {
            lpFormatBuff=(LPSTR)gmalloc(1024);

            if (!lpFormatBuff)
                nStatus=ERR_SCRIPT_MEMORY;

        }

        if (nStatus==0) {
            va_list args;
            LPSTR lpPtrOut;

            va_start(args, lpcFormat);

            if (lstrcmp(lpcFormat, "%s")==0) {
                LPSTR lpPtrIn=va_arg(args, LPSTR);

                if (!lpPtrIn)
                    nStatus=ERR_SCRIPT_INVALID_ARGUMENT;
                else {
                    for (lpPtrOut=lpFormatBuff;
                         *lpPtrIn;
                         *lpPtrOut++ =lpTranslate[(BYTE)*lpPtrIn++]);

                    *lpPtrOut='\0';
                }
            } else {
                wvsprintf(lpFormatBuff, lpcFormat, args);

                for (lpPtrOut=lpFormatBuff;
                     *lpPtrOut;
                     *lpPtrOut++ =lpTranslate[(BYTE)*lpPtrOut]);

            }

            va_end(args);

            if (nStatus==0) {
                if (writeLine(lpScript->scriptStream, "%s", lpFormatBuff))
                    nStatus=ERR_SCRIPT_WRITE;
                else if (lpScript->scriptEmpty)
                    lpScript->scriptEmpty=FALSE;

            }
        }

        if (lpScript->scriptSuspend)
            if (suspendFile(lpScript->scriptStream))
                if (!nStatus)
                    nStatus=ERR_SCRIPT_SUSPEND;
    }

    if (nStatus)
        lpScript->scriptErrcode=nStatus;

    return(nStatus);
}



HOOB FAR _EXPORT actionScript(HSCRIPT hScript, BYTE byScriptType, LPCSTR lpcFormat,...)
{
    LPScript lpScript=(LPScript)hScript;
    int status=0;
    HOOB hoob=NULL;

    if (!lpScript)
        status=ERR_SCRIPT_NOT_OPEN;
    else if (byScriptType!=OT_INLINE &&
             byScriptType!=OT_INCLUDE &&
             byScriptType!=OT_PREINCLUDE) 
	{
        status=ERR_SCRIPT_INVALID_ARGUMENT;
        suspendFile(lpScript->scriptStream);
        _unlink(openFileName(lpScript->scriptStream));
        closeFile(lpScript->scriptStream);
        gfree(lpScript);	lpScript = NULL;
    } 
	else if (lpScript->scriptEmpty) 
	{
        suspendFile(lpScript->scriptStream);
        _unlink(openFileName(lpScript->scriptStream));
        closeFile(lpScript->scriptStream);
        gfree(lpScript);	lpScript = NULL;
    } 
	else 
	{
        //  Note that this assumes that INLINEOBJECT and INCLUDEOBJECT are identical...
        va_list args;
        INCLUDEOBJECT io;
        char szDescription[256];
        LPSTR ptr=szDescription;

        InitObject(&io, byScriptType, INCLUDEOBJECT);
        lstrcpy(io.szFileName, openFileName(lpScript->scriptStream));

        ptr+=wsprintf(szDescription, "%s [%03d] - ",
                      lpScript->scriptPrefix, lpScript->scriptIndex);

        va_start(args, lpcFormat);
        wvsprintf(ptr, lpcFormat, args);
        va_end(args);
        szDescription[LEN_DESCRIPTION-1]='\0';
        lstrcpy(io.szDescription, szDescription);
        hoob=PutObject(NULL, &io, NULL);

        if (!hoob)
            status=ERR_SCRIPT_ACTION;

        closeFile(lpScript->scriptStream);
        gfree(lpScript);	lpScript = NULL;
		if(lpTranslate)
		{
			gfree(lpTranslate);	lpTranslate = NULL;
		}
    }

    if (status)
        scriptErrcode=status;

    return(hoob);
}



int FAR PASCAL _EXPORT abandonScript(HSCRIPT hScript)
{
    LPScript lpScript=(LPScript)hScript;

    if (!lpScript) {
        scriptErrcode=ERR_SCRIPT_NOT_OPEN;
        return(ERR_SCRIPT_NOT_OPEN);
    }

    suspendFile(lpScript->scriptStream);
    _unlink(openFileName(lpScript->scriptStream));
    closeFile(lpScript->scriptStream);
    gfree(lpScript);	lpScript = NULL;
	if(lpTranslate)
	{
		gfree(lpTranslate);	lpTranslate = NULL;
	}
    return(0);
}

