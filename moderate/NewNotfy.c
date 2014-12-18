#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "globals.h"
#include "flist.h"
#include "msgedit.h"
#include "strftime.h"

HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE hInstance,
									  LPCSTR lpTemplateName,
									  int x, int y, int cx, int cy);

void WINAPI OpenNotifyWindow(LPSTR confName, LPSTR topicName, LPSTR subject, LPSTR msg)
{
HWND hwndMDIFlist;
HWND confWindow;
HWND topicWindow;
HWND subjectWindow;
HWND messageWindow;
//HWND  amWindows[7];
int i;
//HINSTANCE hInst;

//	GetAmeolWindows((HWND *)&amWindows);
//	BringWindowToTop(amWindows[1]);

//	hwndMDIFlist = (HWND)SendMessage(lpGlobals->hwndAmeol, 2026, 0, 0);
//	hInst = (HINSTANCE)GetWindowLong(GetParent(amWindows[4]), GWL_HINSTANCE);
//	if(hInst)
//	{
//		hwndMDIFlist = CreateMDIDialogParam( hInst, "amctl_saywndcls", 10, 10, 630, 470);
		hwndMDIFlist = StdMDIDialogBoxFrame( lpGlobals->hInst, "amctl_saywndcls", 10, 10, 630, 470);

		if(IsWindow(hwndMDIFlist))
		{
			confWindow    = GetDlgItem(hwndMDIFlist, 6061); // Conference;
			topicWindow	  = GetDlgItem(hwndMDIFlist, 6006); // Topic
			subjectWindow = GetDlgItem(hwndMDIFlist, 5009); // Subject;
			messageWindow = GetDlgItem(hwndMDIFlist, 6031); // Message Text;

			i = ComboBox_FindStringExact(confWindow,  0, confName);
				ComboBox_SetCurSel(confWindow, i);
			i = ComboBox_FindStringExact(topicWindow, 0, topicName);
				ComboBox_SetCurSel(topicWindow, i);

			Edit_SetText(messageWindow, msg);
			Edit_SetText(subjectWindow, subject);
		}
//	}
}