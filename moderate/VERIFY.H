#ifndef _HORUS_MODERATE_VERIFY_H
#define _HORUS_MODERATE_VERIFY_H

typedef enum {CHECK_DOS, CHECK_CIX} CheckDomain;

BOOL checkName( LPCSTR, CheckDomain );
BOOL verifyName( HWND, LPSTR, LPCSTR );
BOOL extractName( LPCSTR, LPSTR, LPSTR, BOOL );

#endif
