#ifndef _HORUS_MODERATE_SETUP_H
#define _HORUS_MODERATE_SETUP_H



typedef struct {
    int sortDirection;
    int sortKey;
} SortData, FAR *LPSortData;



typedef struct {
    char szHelpFile[LEN_PATHNAME];
    char szIniFile[LEN_PATHNAME];
    char szDateFormat[32];
    char szThousandsSep[4];
    SortData sortFlist;
    int nFdirSelect;
    SortData sortFdir;
    int nConfirmDeletes;
    char szDataDir[LEN_PATHNAME];
    char szAmeolDir[LEN_PATHNAME];
    char szDownloadDir[LEN_PATHNAME];
    char szUploadDir[LEN_PATHNAME];
    BOOL bPrefixMenuCommands;
    BOOL bNotifyAuto;
    WORD bNotifyMethod;
    BOOL bNotifySig;
    BOOL bNotifySize;
    WORD wNotifyCase;
    BOOL bEditSeedMessages;
    BOOL bCreateMissingTopics;
    BOOL bTopicsizeWarning;
    BOOL bCreateConfHelp;
    BOOL bDebug;
	BOOL rememberDown;
	BOOL rememberUp;
	BOOL os_and_status;
    DWORD dwCommdlgBuffsize;
} Setup;



extern Setup setup;



void getSetup(HWND hwnd);
void configureSetup(HWND hdlg);
void putSetup(void);



#endif
