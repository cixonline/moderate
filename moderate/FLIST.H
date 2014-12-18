#ifndef _HORUS_MODERATE_FLIST_H
#define _HORUS_MODERATE_FLIST_H



#include <time.h>
#include "tabcntrl.h"



typedef struct tagFdirEntry FAR *LPFdirEntry;
typedef LPFdirEntry FAR *LPPFdirEntry;

typedef struct tagFlistEntry FAR *LPFlistEntry;
typedef LPFlistEntry FAR *LPPFlistEntry;



typedef struct {
    WORD first;
    WORD last;
} EditRange, FAR *LPEditRange;



typedef struct {
    enum {FLIST_DELETE_CONFIRM, FLIST_DELETE_ALL} flistConfirm;
    enum {FDIR_DELETE_CONFIRM, FDIR_DELETE_LEAVE, FDIR_DELETE_ALL} flistFdirConfirm;
} FlistConfirm, FAR *LPFlistConfirm;



typedef enum {CR_CANCEL, CR_OK, CR_DUPLICATE, CR_REPLACE} CheckReplace;
typedef enum {EXIT_CANCEL, EXIT_OK, EXIT_SWITCHTOPIC} FlistExit;
typedef enum {FPC_MISSING, FPC_EMPTY, FPC_NOFILES, FPC_FILES} FlistPresenceCategory;
typedef enum {FSC_NONE, FSC_STATIC, FSC_FILEPOOL, FSC_NEW, FSC_LOCAL} FlistSelectCategory;



typedef enum {
    FLIST_UNSET,                //  internal: only used during parsing
    FLIST_MEMO,                 //  hidden comment
    FLIST_COMMENT,              //  comment
    FLIST_TABBED,               //  indented comment
    FLIST_FROM_FILEPOOL,        //  'f' file line
    FLIST_FROM_FDIR,            //  'c' file line
    FLIST_FROM_MAIL_DIR,        //  internal: 'c' inserted from mail dir
    FLIST_FROM_UPLOAD,          //  internal: 'c' from uploaded file
    FLIST_FROM_NOTIFICATION     //  internal: 'c' from mail notification
} FlistSelect;



typedef struct {
    WORD position;
    FlistSelect source;
    BOOL bEdit;
} InsertControl, FAR *LPInsertControl;



enum {MAX_HOOBS=256, MAX_REFERENCES=4, MAX_FLIST_SELECTIONS=10000};



enum {
    FDIR_FLAG_FILEPOOL=0x0001,
    FDIR_FLAG_EXPORT=0x0002,
    FDIR_FLAG_UPLOAD_RENAME=0x0004,
    FDIR_FLAG_SELECTED=0x0008,

    FDIR_DEFAULT_FLAGS=0
};



typedef struct tagFdirEntry {
    LPPFdirEntry fdirHead;
    LPFdirEntry fdirNext;
    char fdirName[LEN_CIXFILENAME];
    LPSTR fdirLocalPath;
    long fdirSize;
    time_t fdirTimestamp;
    int fdirReferences;
    WORD fdirFlags;
    LPFlistEntry fdirFlistEntry[MAX_REFERENCES];
} FdirEntry;



typedef struct {
    time_t dateUpdate;
    BOOL bInclude;
} FstatEntry, FAR *LPFstatEntry;



typedef struct {
    time_t dateOrigin;
    LPFstatEntry lpFstatUpdates;
    WORD nFstatCount;
    BOOL bUpdate;
    HWND hwnd;
} Fstats, FAR *LPFstats;



enum {
    FLIST_FLAG_FILEPOOL=0x0001,
    FLIST_FLAG_EXPORT=0x0002,
    FLIST_FLAG_HOLD=0x0004,
    FLIST_FLAG_SELECTED=0x0008,

    FLIST_DEFAULT_FLAGS=0
};



typedef struct tagFlistEntry {
    LPFlistEntry flistNext;
    LPFlistEntry flistPrevious;
    FlistSelect flistSelect;
    WORD flistFlags;
    char flistName[LEN_CIXFILENAME];
    char flistDescription[128];
    LPLONG flistStats;
    LPFdirEntry flistSource;
} FlistEntry;



#define isFile(lpEntry) \
    ((lpEntry)->flistSelect==FLIST_FROM_FILEPOOL || \
     (lpEntry)->flistSelect==FLIST_FROM_FDIR || \
     (lpEntry)->flistSelect==FLIST_FROM_MAIL_DIR || \
     (lpEntry)->flistSelect==FLIST_FROM_UPLOAD || \
     (lpEntry)->flistSelect==FLIST_FROM_NOTIFICATION)



#define isComment(lpEntry) \
    ((lpEntry)->flistSelect==FLIST_MEMO || \
     (lpEntry)->flistSelect==FLIST_COMMENT || \
     (lpEntry)->flistSelect==FLIST_TABBED)



#define isContinuation(lpEntry) \
    ((lpEntry)->flistSelect==FLIST_TABBED)



typedef struct {
    WNDPROC lpfnNewProc;
    WNDPROC lpfnOldProc;
    HWND hwndDialog;
} SubclassData, FAR *LPSubclassData;



typedef struct {
    SubclassData subclassData;			//  This *must* be the first item in the structure
    LPTabData lpTabData;                        //  Data defining the tab controls in the flist dialog
    Context context;				//  Current conference and topic
    UINT wHelpContext;				//  Used to resolve file selector Help messages
    HOOB outbasketActions[MAX_HOOBS];		//  Ameol outbasket handles generated in this session
    int actionCount;				//  Count of Ameol outbasket handles generated
    InsertControl insertControl;		//  Entry insertion description data for the file list
    LPINT lpPrevSelected;

    //  Moderator's file list data
    HMENU hMenuFlistPopup;
    HMENU hMenuFlistInsert;
    HMENU hMenuFlistAppend;
    HMENU hMenuFlistUpdate;
    HMENU hMenuFlistAction;
    HWND hwndFlist;
    LPFlistEntry flist;
    LPINT lpFlistSelected;
    BOOL bFlistSelCheck;
    FlistSelectCategory flistSelectCategory;
    BOOL bFlistMultipleFiles;
    BOOL bCreating;
    SortData flistSort;
    time_t flistTime;
    int flistEntries;
    int flistFiles;
    BOOL flistChanged;
    BOOL flistMustUpload;
    BOOL flistUploaded;
    BOOL flistUpdate;
    int flistInsertPoint;
    FlistConfirm flistConfirm;

    //  File directory data
    HMENU hMenuFdirPopup;
    HMENU hMenuFdirSort;
    HMENU hMenuFdirUpdate;
    HMENU hMenuFdirAction;
    HWND hwndFdir;
    LPFdirEntry fdir;
    LPINT lpFdirSelected;
    BOOL bFdirSelCheck;
    BOOL bFdirFlistChanged;
    int fdirSelect;
    SortData fdirSort;
    time_t fdirTime;
    int fdirFiles;
    int fdirOrphans;
    BOOL fdirChanged;
    BOOL fdirUpdate;

    //  File statistics data
    Fstats fstats;
    time_t fstatTime;

    //  Users' file list data
    time_t fuserTime;
    BOOL fuserUpdate;

    //  Filepool file data
    LPFdirEntry filepooldir;

    //  Binmail directory data
    LPFdirEntry maildir;
    time_t maildirTime;
    int maildirFiles;
    int maildirExported;
    BOOL maildirUpdate;

    //  Local upload file data
    LPFdirEntry localdir;
    int localdirUploaded;

	//  MDI Dialog Sizes
	RECT rcOrg;		//Original Size of Dialog
	RECT rcLast;	//Last Size of Dialog

} FlistData, FAR *LPFlistData;



#define flistResetSelection(lpFlistData, bSelCheck) \
    (*(lpFlistData)->lpFlistSelected=0, \
     (lpFlistData)->bFlistSelCheck=(bSelCheck), \
     (lpFlistData)->flistSelectCategory=FSC_NONE, \
     (lpFlistData)->bFlistMultipleFiles=FALSE)

#define flistSuspendControlUpdates(lpFlistData) \
    ((lpFlistData)->bFlistSelCheck ? ((lpFlistData)->bFlistSelCheck=FALSE, TRUE) : FALSE)



#define fdirResetSelection(lpFlistData, bSelCheck) \
    (*(lpFlistData)->lpFdirSelected=0, \
     (lpFlistData)->bFdirSelCheck=(bSelCheck))

#define fdirSuspendControlUpdates(lpFlistData) \
    ((lpFlistData)->bFdirSelCheck ? ((lpFlistData)->bFdirSelCheck=FALSE, TRUE) : FALSE)

#define fdirDisplayCount(lpFlistData) \
    ((lpFlistData)->fdirSelect==CID_ORPHANS  ? (lpFlistData)->fdirOrphans : \
     (lpFlistData)->fdirSelect==CID_IN_FLIST ? (lpFlistData)->fdirFiles-(lpFlistData)->fdirOrphans : \
					       (lpFlistData)->fdirFiles)

#define fdirEntryVisible(lpFlistData, lpFdirEntry) \
    ((lpFlistData)->fdirSelect==CID_ALL_FILES || \
     (lpFlistData)->fdirSelect==CID_ORPHANS && \
	(!(lpFdirEntry)->fdirReferences || (lpFdirEntry)->fdirFlags&(FDIR_FLAG_EXPORT|FDIR_FLAG_FILEPOOL)) || \
     (lpFlistData)->fdirSelect==CID_IN_FLIST && \
	(lpFdirEntry)->fdirReferences && \
	!((lpFdirEntry)->fdirFlags&(FDIR_FLAG_EXPORT|FDIR_FLAG_FILEPOOL)))



int readFdir(LPCSTR fnFLD, LPPFdirEntry lpFdir, LPINT fdirFiles);
int readFlist(HWND hdlg, HTOPIC hTopic,
	      LPPFlistEntry lpFlist, time_t FAR * flistTime, LPINT flistEntries, LPINT flistFiles,
	      LPPFdirEntry lpFdir, time_t FAR *fdirTime, LPINT fdirFiles, LPINT fdirOrphans,
	      time_t FAR *fuserTime, LPPFdirEntry lpFilepool);
int writeFlist(HTOPIC hTopic, LPFlistEntry lpFlist);
void freeFlist(LPPFlistEntry lpFlist);
void freeFdir(LPPFdirEntry lpFdir);
int readMaildir(LPPFdirEntry lpMaildir, time_t FAR *maildirTime, LPINT maildirFiles);
void insertSelection(LPINT lpItemsSelected, int nItem);
void removeSelection(LPINT lpItemsSelected, int nItem);
void flistClearSelection(LPFlistData lpFlistData);
void flistControls(HWND hwnd, LPFlistData lpFlistData, BOOL bUpdateControls);
void fdirClearSelection(LPFlistData lpFlistData);
CheckReplace checkReplace(HWND hwnd, LPFlistData lpFlistData, FlistSelect flistSelect,
			  LPFdirEntry newEntry, LPPFdirEntry oldEntry);
void addFlistItem(HWND hwnd, LPFlistData lpFlistData, LPFdirEntry lpFdirEntry);
LPFlistEntry flistGetEntry(LPFlistData lpFlistData, int nIndex);
int flistFindEntry(LPFlistData lpFlistData, LPFlistEntry lpFlistEntry);
void uploadFlist(HTOPIC hTopic);
BOOL downloadFlist(HTOPIC hTopic, BOOL bAction);
BOOL downloadFdir(HTOPIC hTopic, BOOL bAction);
BOOL downloadFuser(HTOPIC hTopic, BOOL bAction);
BOOL downloadMaildir(BOOL bAction);
void sortFdir(LPPFdirEntry lppFdirHeader, int nFdirCount, LPSortData lpSort);
void notify(HWND hdlg, LPFlistData lpFlistData);
int importDescriptions(HWND hdlg, LPFlistData lpFlistData);

BOOL _EXPORT CALLBACK editProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK insertFdirProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK insertFilepoolProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK duplicateProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK sortFlistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK renameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK confirmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK updateListsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK queryListsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



#endif


