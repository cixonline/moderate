#ifndef _MODERATE_MSGEDIT_H
#define _MODERATE_MSGEDIT_H



#include "edit.h"



typedef struct {
    LPCSTR lpcConfName;
    LPCSTR lpcTopicName;
    HOOB hoobMessage;
    char szTitle[LEN_TITLE];
    LPEditControl lpEditControl;
    LPSTR lpText;
    int nTextLength;
    BOOL bImportEnabled;
    WORD wHelpTag;
	RECT rcOrg;
	BOOL changed;
	BOOL fHasRect;
} MessageEditData, FAR *LPMessageEditData;



BOOL _EXPORT CALLBACK messageEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



#endif
