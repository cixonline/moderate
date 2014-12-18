#ifndef _HORUS_MODERATOR_EDIT_H
#define _HORUS_MODERATOR_EDIT_H



typedef struct {
    UINT wStart;
    UINT wEnd;
} RANGE, FAR *LPRANGE;



enum {
    EDIT_KEEP_TABS=0x0001
};



typedef struct tagEditControl {
    struct tagEditControl FAR *lpEditPrevious;
    WORD wEditControl;
    HWND hwnd;
    HFONT hfont;
    BOOL bFocus;
    POINT pointPosition;
    RANGE rangeSelection;
    BOOL bChanged;
    LPSTR lpBuff;
    WORD wPtr;
    BOOL bTab;
    WORD wFlags;
    FARPROC lpfnEditProc;
    WNDPROC lpfnEditOldProc;
} EditControl, FAR *LPEditControl;



#define editFormat(lpEditControl, bFormat) \
    Edit_FmtLines((lpEditControl)->hwnd, bFormat)

#define editSetText(lpEditControl, lpBuff) \
    Edit_SetText((lpEditControl)->hwnd, lpBuff);



LPEditControl PASCAL editPrelude(HWND hdlg, WORD wEditControl, HFONT hfont, WORD wFlags);
void PASCAL editPostlude(LPEditControl lpEditControl);
WORD PASCAL editPointToOffset(LPEditControl lpEditControl, POINT pointPosition);
POINT PASCAL editOffsetToPoint(LPEditControl lpEditControl, WORD wOffset);
POINT PASCAL editGetPosition(LPEditControl lpEditControl);
void PASCAL editResetPosition(LPEditControl lpEditControl);
void PASCAL editSetPosition(LPEditControl lpEditControl, POINT pointPosition);
RANGE PASCAL editGetSelection(LPEditControl lpEditControl);
void PASCAL editResetSelection(LPEditControl lpEditControl);
void PASCAL editSetSelection(LPEditControl lpEditControl, RANGE rangeSelection);
RANGE PASCAL editClearSelection(LPEditControl lpEditControl);
void PASCAL editInsertText(LPEditControl lpEditControl, LPCSTR lpcText);
BOOL PASCAL editChanged(LPEditControl lpEditControl, WORD wChanged);
BOOL PASCAL editUndo(LPEditControl lpEditControl);
void PASCAL editFocus(LPEditControl lpEditControl);
int PASCAL editExtract(LPEditControl lpEditControl, LPSTR lpBuff, int nSize);



#endif
