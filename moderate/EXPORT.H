#ifndef _HORUS_MODERATE_EXPORT_H
#define _HORUS_MODERATE_EXPORT_H



typedef struct {
    LPFdirEntry lpFdirEntry;
    BOOL bTransformOK;
    BOOL bExportOK;
    WORD wHelpTag;
} ExportFdirData, FAR *LPExportFdirData;



void exportFdirEntry(LPContext lpContext, LPFdirEntry lpFdirEntry);
BOOL _EXPORT CALLBACK exportFdirProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void exportFlistEntry(LPContext lpContext, LPFlistEntry lpFlistEntry);
BOOL _EXPORT CALLBACK exportFlistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



#endif

