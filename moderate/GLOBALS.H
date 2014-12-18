#ifndef _HORUS_MODERATOR_GLOBALS_H
#define _HORUS_MODERATOR_GLOBALS_H

#include <time.h>
#include "compat32.h"
#include "htmlhelp.h"

#ifndef WIN32
#define MOD_DATA	DWL_USER
#else
#define MOD_DATA	GWL_USERDATA
#endif

#define WM_MYSETFOCUS	WM_USER+665

typedef enum {DISABLE, ENABLE, QUERY} Enable;   //  Used for menu item enable routine

enum {
    UM_CHECKTOPIC=WM_USER+666,
    UM_KILLFOCUS,
    UM_NEXTTOPIC,
    UM_CHECKCONF,
    UM_CREATECONFHELP,
    UM_POPUPMENU,
    UM_RESETCONTROLS
};

/* void Cls_OnCheckTopic(HWND hwnd); */
#define HANDLE_UM_CHECKTOPIC(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define Post_Moderate_CheckTopic(hwndCtl) \
    ((void)PostMessage((hwndCtl), UM_CHECKTOPIC, 0, 0L))

/* void Cls_OnKillFocus(HWND hwnd, HWND hwndNext); */
#define HANDLE_UM_KILLFOCUS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define Post_Moderate_KillFocus(hwndCtl, hwndNext) \
    ((void)PostMessage((hwndCtl), UM_KILLFOCUS, (WPARAM)(hwndNext), 0L))

/* void Cls_OnNextTopic(HWND hwnd); */
#define HANDLE_UM_NEXTTOPIC(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define Post_Moderate_NextTopic(hwndCtl) \
    ((void)PostMessage((hwndCtl), UM_NEXTTOPIC, 0, 0L))

/* void Cls_OnCheckConf(HWND hwnd); */
#define HANDLE_UM_CHECKCONF(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define Post_Moderate_CheckConf(hwndCtl) \
    ((void)PostMessage((hwndCtl), UM_CHECKCONF, 0, 0L))

/* void Cls_OnCreateConfHelp(HWND hwnd); */
#define HANDLE_UM_CREATECONFHELP(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define Post_Moderate_CreateConfHelp(hwndCtl) \
    ((void)PostMessage((hwndCtl), UM_CREATECONFHELP, 0, 0L))

/* void Cls_OnPopupMenu(HWND hwnd, HWND hwndCtl, int x, int y); */
#define HANDLE_UM_POPUPMENU(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (int)LOWORD(lParam), (int)HIWORD(lParam)), 0L)
#define Post_Moderate_PopupMenu(hwndCtl, hwnd, x, y) \
    ((void)PostMessage((hwndCtl), UM_POPUPMENU, (WPARAM)(hwnd), MAKELPARAM((x), (y))))

/* void Cls_OnResetControls(HWND hwnd); */
#define HANDLE_UM_RESETCONTROLS(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define Post_Moderate_ResetControls(hwndCtl) \
    ((void)PostMessage((hwndCtl), UM_RESETCONTROLS, 0, 0L))

enum {
    //  Ameol main menu bar indices
    MAIN_FILE_MENU=AMM_MAINMENU,
    MAIN_EDIT_MENU,
    MAIN_MAIL_MENU,
    MAIN_GLOBAL_MENU,
    MAIN_SETTINGS_MENU,
    MAIN_TOPIC_MENU,
    MAIN_MESSAGE_MENU,
    MAIN_UTILITIES_MENU,
    MAIN_WINDOW_MENU,
    MAIN_HELP_MENU,

    //  Ameol conference list menu bar indices
    CONF_FILE_MENU=AMM_CONFMENU,
    CONF_MAIL_MENU,
    CONF_GLOBAL_MENU,
    CONF_SETTINGS_MENU,
    CONF_VIEW_MENU,
    CONF_TOPIC_MENU,
    CONF_UTILITIES_MENU,
    CONF_WINDOW_MENU,
    CONF_HELP_MENU,

    //  Ameol terminal menu bar indices
    TERM_FILE_MENU=AMM_TERMMENU,
    TERM_EDIT_MENU,
    TERM_MAIL_MENU,
    TERM_GLOBAL_MENU,
    TERM_SETTINGS_MENU,
    TERM_TERMINAL_MENU,
    TERM_UTILITIES_MENU,
    TERM_WINDOW_MENU,
    TERM_HELP_MENU,

	 AMEOL2_FILE_MENU = AMM_AMEOL2,
	 AMEOL2_EDIT_MENU,
	 AMEOL2_VIEW_MENU,
	 AMEOL2_CIX_MENU,
	 AMEOL2_MAIL_MENU,
	 AMEOL2_NEWS_MENU,
	 //AMEOL2_TERMINAL_MENU,
	 AMEOL2_FOLDER_MENU,
	 AMEOL2_MESSAGE_MENU,
	 AMEOL2_SETTINGS_MENU,
	 AMEOL2_WINDOW_MENU,
	 AMEOL2_HELP_MENU
};

typedef enum {
    ACTION_CREATE_CONF,
    ACTION_EDIT_TOPIC,
    ACTION_CONFIGURE,
    ACTION_EDIT_FLIST,
    ACTION_HELP,
	 ACTION_MAIN,
	 ACTION_ABOUT,
	 TOTAL_ACTIONS
} MenuAction;

typedef struct {
    AMEOLMENU menu;
    MenuAction action;
	 UINT uSmallTool;
	 UINT uBigTool;
	 BOOL fHasButton;
} AmeolMenu, FAR *LPAmeolMenu;



typedef struct {
    HCONF hConference;
    LPCSTR lpcConfName; 
    HTOPIC hTopic;
    LPCSTR lpcTopicName;
    BOOL bInEditor;
} Context, FAR *LPContext;

#define	MAX_MENU_COUNT		18

typedef struct {
	HINSTANCE hInst;									/* Our instance handle */
	HWND hwndAmeol;									/* Main Ameol window handle */
	HWND hwndMDIFlist;
	AmeolMenu ameolMenu[ MAX_MENU_COUNT	];		/* Installed Ameol menu entries */
	LPAmeolMenu lpFlistMenuItem;					/* Menu item for Edit File List on main Ameol menu */
	BOOL bMenusInstalled;							/* Are menu entries installed? */
	UINT cTotalMenus;									/* Total number of menu entries added */
	LPFNEEVPROC lpEventProc;						/* Ameol event handler (for AE_MSGCHANGE) */
	Context context;									/* Current global conf/topic context */
	Context contextFlist;							/* Last file list editor conf/topic context */
	UINT wHelpMessageID;								/* Used for the Help button in file selectors */
	HFONT hfBold;										/* Handle of a bold font */
	HFONT hfNormal;									/* Handle of a normal font */
	HFONT hfMessageEdit;								/* Handle of a message editing font */
	HFONT hfDescEdit;									/* Handle of a description font */
	HFONT hfBoldEdit;									/* Handle of a boldface editing font */
	BOOL bRecursionFlag;								/* Dialog recursion flag */
	BOOL bEvaluation;									/* TRUE if Ameol/Ameol2 is an evaluation version */
	VersionInfo	AmeolVerInfo;						/* Ameol version information */
	WORD wWinVer;										/* Windows version number */
	HCURSOR dragCursor;
	HCURSOR noDragCursor;
} Globals, FAR *LPGlobals;

extern LPGlobals lpGlobals; 

void busy(BOOL waitFlag);
void inform(HWND hwnd, LPCSTR format,...);
void alert(HWND hwnd, LPCSTR format,...);
int query(HWND hwnd, WORD options, LPCSTR format,...);
LPCSTR formatLong(LPSTR lpBuff, LPCSTR lpcBefore, LONG lValue, LPCSTR lpcAfter);
LPCSTR setCase(LPSTR lpBuff, LPCSTR lpcText, WORD wSelect);
time_t datediff(time_t time1, time_t time2);
time_t parseTime(LPSTR lpBuff);
int fontHeight(HWND hdlg);
BOOL setWindowData(HWND hwnd, LPCSTR lpcID, LPVOID lpData);
LPVOID getWindowData(HWND hwnd, LPCSTR lpcID);
void freeWindowData(HWND hwnd, LPCSTR lpcID);

void addAmeolMenus(void);
void removeAmeolMenus(void);
void getResources(void);
void freeResources(void);
BOOL enableFileListMenuItem(Enable enable);
void deleteTopicFiles(HTOPIC hTopic);
void deleteConfFiles(HCONF hConf);
void getPendingTopicInfo(HWND hWnd);
void updateTopicInfo(HWND hWnd);

BOOL _EXPORT CALLBACK topicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK flistProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK createConfProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL _EXPORT CALLBACK editTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



#endif
