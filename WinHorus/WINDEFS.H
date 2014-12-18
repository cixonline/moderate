#ifndef _HORUS_WINDEFS_H
#define _HORUS_WINDEFS_H



#if defined(MVS)
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>

    #define FAR
    #define PASCAL
    #define CALLBACK

    #if !defined(FALSE)
        #define FALSE 0
    #endif

    #if !defined(TRUE)
        #define TRUE (!FALSE)
    #endif

    typedef void *HANDLE;
    #define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

    DECLARE_HANDLE(HINSTANCE);

    typedef void *PVOID;
    typedef void *LPVOID;

    typedef int *PINT;
    typedef int *LPINT;

    typedef char *PSTR;
    typedef char *LPSTR;
    typedef const char *PCSTR;
    typedef const char *LPCSTR;
    typedef char **PPSTR;
    typedef char **LPPSTR;

    typedef unsigned char BYTE;
    typedef BYTE *PBYTE;
    typedef BYTE *LPBYTE;

    typedef unsigned int UINT;

    typedef unsigned short int WORD;
    typedef WORD *PWORD;
    typedef WORD *LPWORD;

    typedef unsigned long int DWORD;
    typedef DWORD *PDWORD;
    typedef DWORD *LPDWORD;

    typedef long int LONG;

    typedef int BOOL;

    typedef FILE *HFILE;
    #define HFILE_ERROR NULL

    #define LOBYTE(w)           ((BYTE)(w))
    #define HIBYTE(w)           ((BYTE)((UINT)(w)>>8))
    #define LOWORD(l)           ((WORD)(l))
    #define HIWORD(l)           ((WORD)((DWORD)(l)>>16))
    #define MAKELONG(low, high) ((LONG)(((WORD)(low))|(((DWORD)((WORD)(high)))<<16)))

    #define lstrcpy(s1, s2) strcpy(s1, s2)
    #define lstrcmp(s1, s2) strcmp(s1, s2)
    #define lstrcat(s1, s2) strcat(s1, s2)
    #define hmemcpy(dest, src, count) memcpy(dest, src, count)
    #define wvsprintf(buff, fmt, args) vsprintf(buff, fmt, args)
    #define wsprintf sprintf
#else
    #include <windows.h>
    #include <windowsx.h>
#endif



#endif
