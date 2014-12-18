//#include <windows.h>
//#include <windowsx.h>
//#include <stdlib.h>
//#include <memory.h>
//#include "compat32.h"
//#include "moderate.h"
//#include "flist.h"

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <sys/stat.h>
#include <commdlg.h>
#include "ameolapi.h"
#include "winhorus.h"
#include "tabcntrl.h"
#include "hctools.h"
#include "setup.h"
#include "help.h"
#include "moderate.h"
#include "verify.h"
#include "status.h"
#include "globals.h"
#include "flist.h"
#include "export.h"
#include "strftime.h"

#define ID_TIMER 100
#define LINE_WIDTH 1


// style flags for the DrawIndicator() function
#define DI_TOPERASED        0x0001  // erasing a line drawn on the top of the list
#define DI_BOTTOMERASED     0x0002  // erasing a line drawn on the bottom of the list
#define DI_ERASEICON        0x0004  // erasing the icon

UINT idTimer;            // the id for the timer used in scrolling the list
HFONT hFont;             // a new font for the list box
HCURSOR hCurDrag;        // a cursor to indicate dragging
HINSTANCE hInst;         // the instance handle       
int nHtItem;             // the height of an individual item in the list box
BOOL bNoIntegralHeight;  // does the list box have the LBS_NOINTEGRALHEIGHT style flag 

HWND ghDlg;              // handle to the main window
HWND ghList;             // handle to the list box     
HBRUSH ghBrush;          // handle to the brush with the color of the windows background

long FAR PASCAL EXPORT NewListProc(HWND hwndList, UINT message, WPARAM wParam , LPARAM lParam); 
void DrawIndicator(HDC hDC, int nYpos, int nWidth, WORD wFlags);
void EXPORT FAR PASCAL MyYield(void);

FARPROC lpfnOldListProc, LstProc;      

WORD FAR PASCAL GetIndexFromCursorPos(HWND hWnd, LPARAM lParam)
{
POINT pt;
int   iTopIndex;
RECT  r;
WORD    gLBItemHeight;

    gLBItemHeight=(WORD)SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0L);               
    iTopIndex=(int)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0L);
    GetWindowRect(hWnd, &r);
    ScreenToClient(hWnd, (LPPOINT)&r);
    ScreenToClient(hWnd, (LPPOINT)&r.right);
#ifdef WIN32
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
#else
	pt = MAKEPOINT(lParam);
#endif
    return((WORD)iTopIndex+(WORD)((pt.y-r.top)/gLBItemHeight));
}

long FAR PASCAL EXPORT NewListProc(HWND hwndList, 
                                     UINT message, 
                                     WPARAM wParam, 
                                     LPARAM lParam)
{
    
static BOOL bTracking = FALSE;
static BOOL bDrag = FALSE;  
static HCURSOR hCursorOld = NULL;
static WORD wSelected, wIndex;

	LPSubclassData lpSubclassData= (LPSubclassData)GetWindowLong(GetParent(hwndList), MOD_DATA);
	if(!lpSubclassData)
		return FALSE;
	
	ghList = hwndList;

	switch (message)
    {    
		case WM_CANCELMODE:
			{
       // WM_CANCELMODE is sent to the window that has captured the mouse before
       // a message box or modal dialog is displayed. If we were dragging the item
       // cancel the drag.
				bTracking = FALSE;
				ReleaseCapture();
				if (bDrag)
					SetCursor(hCursorOld);    
				break; 
			}

		case WM_LBUTTONDOWN:
		{
        
        // Was the list box item dragged into the destination?        
			BOOL bDragSuccess = FALSE;  
			MSG msg;
			POINT pt;      
			POINT point;
        
			RECT rectIsDrag;            // Rectangle to determine if dragging has started.  
			int nOldPos;
        
			int nOldY = -1;                            // the last place that we drew on
			HDC hdc;   // dc to draw on  
			div_t divt;                            // get remainder a quotient with "div"   
			int nCount;
			div_t divVis;          
// space for scroll bar -  starts off at 1 so we don't overwrite the border
			int dxScroll = 1;      
			RECT rect;   
			int nVisible;                   // the number of items visible
			int idTimer;                    // id for the timer
			int nNewPos;                    // the new position
			int nTopIndex;                  // the top index        
        
        
			BringWindowToTop(GetParent(hwndList));
        
         
			GetWindowRect(hwndList, &rect);        
           
         // Pass the WM_LBUTTONDOWN to the list box window procedure. Then
         // fake a WM_LBUTTONUP so that we can track the drag.
//        CallWindowProc(lpfnOldListProc, hwndList, message, wParam, lParam);

//		xPos = LOWORD(lParam);  // horizontal position of cursor 
//		yPos = HIWORD(lParam);

			wIndex=GetIndexFromCursorPos(hwndList, lParam);//MAKELPARAM((x), (y)));
			wSelected=ListBox_GetSel(hwndList, wIndex);

			if (!wSelected)
				return CallWindowProc (lpSubclassData->lpfnOldProc, hwndList, message, wParam, lParam);
//			WFORWARD_MESSAGE(hwndList, message, wParam, lParam, lpSubclassData->lpfnOldProc);
         
// the number of items in the list box
			nCount = (int)SendMessage(hwndList, LB_GETCOUNT,0,0L);         
			if (nCount == 0 ) // don't do anything to and empty list box
				return (long)NULL;         
// fake the WM_LBUTTONUP            
   			WFORWARD_MESSAGE(hwndList, WM_LBUTTONUP, wParam, lParam, lpSubclassData->lpfnOldProc);
//		CallWindowProc(lpfnOldListProc, hwndList, WM_LBUTTONUP, wParam, lParam);        
// get a dc to draw on
			UpdateWindow(GetParent(hwndList));

			hdc = GetDC(hwndList);                               
         
// the height of each item   
			nHtItem = (int)SendMessage(hwndList, LB_GETITEMHEIGHT,0,0L);          
// the current item
			nOldPos = (int)SendMessage(hwndList, LB_GETCURSEL,0,0L);    
         
			divVis = div((rect.bottom - rect.top), nHtItem);
// the number of visible items                  
			nVisible = divVis.quot;
// some items are invisible - there must be scroll bars - we don't want
// to draw on them         
			if (nVisible < nCount)                                         
				dxScroll = GetSystemMetrics(SM_CXVSCROLL) + 1; 
            
			idTimer = 0;
			idTimer = SetTimer(hwndList, ID_TIMER,100,NULL);  
        
              
     // Create a tiny rectangle to determine if item was dragged or merely clicked on.
     // If the user moves outside this rectangle we assume that the dragging has
     // started.
#ifdef WIN32
			point.x = (short)LOWORD(lParam);
			point.y = (short)HIWORD(lParam);
#else
			point = MAKEPOINT(lParam);
#endif
			SetRect(&rectIsDrag, point.x, point.y - nHtItem / 2,
                               point.x, point.y + nHtItem / 2); 
                               
                                          
			bTracking = TRUE;         
			SetCapture(hwndList);
         
         
			// Drag loop                      
			while (bTracking)
			{  
			// Retrieve mouse, keyboard, and timer messages. We retrieve keyboard
			// messages so that the system queue is not filled by keyboard messages
			// during the drag (This can happen if the user madly types while dragging!)
			// If none of these messages are available we wait. Both PeekMessage() 
			// and Waitmessage() will yield to other apps.   
                                      
				while (!PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)
						&& !PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)
						&& !PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE)) 
					WaitMessage();

				switch(msg.message)
				{
					case WM_MOUSEMOVE:
					{
#ifdef WIN32
						pt.x = (short)LOWORD(msg.lParam);
						pt.y = (short)HIWORD(msg.lParam);
#else
						pt = MAKEPOINT(msg.lParam);
#endif
						if (!bDrag)
						{
						// Check if the user has moved out of the Drag rect. 
						// in the vertical direction.  This indicates that 
						// the drag has started.
							if ( (pt.y > rectIsDrag.bottom) || 
								(pt.y < rectIsDrag.top)) // !PtInRect(&rectIsDrag,pt))
							{
								hCursorOld = SetCursor(lpGlobals->dragCursor);      
								bDrag = TRUE;     // Drag has started                           
                        
							}
						}
                          
                          
						if (bDrag)
						{  
  
							ClientToScreen(hwndList, &pt);
							if(WindowFromPoint(pt) == hwndList)
								SetCursor(lpGlobals->dragCursor);  
							else
								SetCursor(lpGlobals->noDragCursor);  
							// if we are above or below the list box, then we are scrolling it, and
							// we shouldn't be drawing here              
							if ((pt.y >= rect.top) && (pt.y <= rect.bottom))
							{
							// convert the point back to client coordinates
								ScreenToClient(hwndList, &pt);
								divt = div(pt.y,nHtItem);                        
                                
								// if we are half way to the item
								// AND it is a new item
								// AND we are not past the end of the list..                        
								if ( divt.rem < nHtItem / 2 && 
									(nOldY != nHtItem * divt.quot) && 
									(divt.quot < nCount + 1)) 
								{
                              
									if (nOldY != -1)
									{
                                // erase the old one                                
										DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, DI_ERASEICON);
									}  
                                    
									nOldY = nHtItem * divt.quot;                            
									DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, 0);
                                    
								}
							} // end if in the list box window                        
                            
						} // end if bDrag
                            
						break;                   
					}
					case WM_TIMER:
					{
						POINT pt;                  
            
						GetCursorPos(&pt); 
						nTopIndex = (int)SendMessage(hwndList, LB_GETTOPINDEX,0,0L);;                                      
						if (pt.y < rect.top) // scroll up
						{
                           
							if (nTopIndex > 0)
							{
                                
								nTopIndex--;
								SendMessage(hwndList, LB_SETTOPINDEX, nTopIndex,0L);
                         // when you scroll up, the line always stays on the top index                            
                         // erase the one we've moved down
								DrawIndicator(hdc, nHtItem,(rect.right - rect.left) - dxScroll, DI_TOPERASED|DI_ERASEICON);
                         // draw the new one          
								DrawIndicator(hdc, 0,(rect.right - rect.left) - dxScroll, 0);                                                             
                         // the new one was drawn at y = 0 
								nOldY = 0;                           
                           
							}                  
                      
						}
						else if (pt.y > rect.bottom) // scroll down
						{                       
                       // if the number of visible items (ie seen in the list box)
                       // plus the number above the list is less than the total number
                       // of items, then we need to scroll down
							if (nVisible + nTopIndex < nCount)
							{                                
                            
								if (nOldY - nTopIndex != nVisible)
								{
                        // if them move below the list REALLY REALLY FAST, then
                        // the last line will not be on the bottom - so we want to reset the last
                        // line to be the bottom                            
                                
                                // erase the old line
									DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, DI_ERASEICON);                                       
                                // reset the index
									divt.quot = nVisible;
									nOldY = divt.quot * nHtItem;                            
                                // draw the new line
									DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, 0);                                       
                                
                                
								}
                        // scroll up
								nTopIndex++;
								SendMessage(hwndList, LB_SETTOPINDEX, nTopIndex,0L);
                        
                       // erase the line that has moved up.. 
								DrawIndicator(hdc, nOldY - nHtItem,(rect.right - rect.left) - dxScroll, DI_BOTTOMERASED|DI_ERASEICON);
                        // draw the new one
								DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, 0);
							}
						}               
						break;
					}
					case WM_LBUTTONUP: 
					{
                  // End of Drag                             
                        
						nTopIndex = (int)SendMessage(hwndList, LB_GETTOPINDEX, 0, 0L);                  
						if (bDrag) 
						{                        
						// get rid of any line we've drawn - the position of the line 
						// divided by the height of the itme is where our new index
						// is going to be                    
							DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, DI_ERASEICON);
                    
							nNewPos = (nOldY / nHtItem) + nTopIndex;                     
						// the old position can't equal the new one                                        
							if (nNewPos != nOldPos)
								bDragSuccess = TRUE;
						}
						else
						{
							ReleaseCapture();
							bTracking = FALSE;                  
				   			WFORWARD_MESSAGE(hwndList, WM_LBUTTONDOWN, wParam, lParam, lpSubclassData->lpfnOldProc);
				   			return WFORWARD_MESSAGE(hwndList, WM_LBUTTONUP, wParam, lParam, lpSubclassData->lpfnOldProc);
						}
						bTracking = FALSE;                  
						break;                     
					}
					default:
					{
                  // Process the keyboard messages
						TranslateMessage(&msg);
						DispatchMessage(&msg);
						break;      
					}
				}          
			}// end while bTracking
        
			ReleaseCapture();
			if (bDrag)
			{
                SetCursor(hCursorOld);
                if (bDragSuccess) 
                {
					LPFlistEntry lpFlistEntry;
                    nTopIndex = (int)SendMessage(hwndList, LB_GETTOPINDEX,0,0L);                    
                    SendMessage(hwndList, WM_SETREDRAW, FALSE,0L);
					lpFlistEntry=(LPFlistEntry)ListBox_GetItemData(hwndList, nOldPos);
                    
/*------------------------------------------------------------------------
 | strategy:  given ABCD and moving to BCAD do the following:
 |
 |           1. delete A -- giving BCD
 |           2. insert A -- giving BCAD
 |           3. hilite A
 |           4. set the top index so A is visible
 -------------------------------------------------------------------------*/                                    
					if(nOldPos < nNewPos)
					{
						while(nOldPos < nNewPos-1)
						{
							FORWARD_WM_COMMAND(GetParent(hwndList), CID_MOVE_DOWN, 0, 0, SendMessage);
							nOldPos++;
						}
					}
					else
					{
						while(nNewPos < nOldPos)
						{
							FORWARD_WM_COMMAND(GetParent(hwndList), CID_MOVE_UP, 0, 0, SendMessage);
							nNewPos++;
						}
					}
                    SendMessage(hwndList, WM_SETREDRAW, TRUE,0L);                 
                            
                } // end if bDragSuccess                  
			} // end if bDrag
			bDrag = FALSE;    
			ReleaseDC(hwndList, hdc);
			KillTimer(hwndList, idTimer);    
			break;
		} 
		default:
			return WFORWARD_MESSAGE(hwndList, message, wParam, lParam, lpSubclassData->lpfnOldProc);
//			return  CallWindowProc(lpfnOldListProc, hwndList, message, wParam, lParam);
	}
	return (long)NULL;
}     


/****************************************************************************

    FUNCTION: AboutDlgProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - Input received

****************************************************************************/
 
BOOL FAR PASCAL EXPORT AboutDlgProc (HWND hDlg, UINT message,  
                                       WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
            return (TRUE);
        case WM_COMMAND:
            if (wParam == IDOK)
            {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);

}
/****************************************************************************

    FUNCTION: DrawIndicator(HDC hDC, int nYpos, int nWidth, WORD wFlags)

    PURPOSE:  gives the user a visual indication where the item will be 
              dropped.  Two methods are shown.  They are:
 
 
                     ______
                    |      |
               Icon | Line |
                --> ||----||
                    |      |
                    |______|
                    
               where the "-->"  is an arrow icon indicating the position, 
               and the "|---|" is a line drawn betwenn the items in the 
               list indicating the new position.  To correctly draw the 
               line in the border situations, the DC is clipped to the 
               client area of the list                    
              

    PARAMETERS: 
    
                hDC         a device context to draw on
                nYpos       the vertical postion
                wFlags      style bits used to indicate if something is 
                            being erased, drawn on a border, or any other
                            style that may be applicable.

    

****************************************************************************/
void DrawIndicator(HDC hDC, int nYpos, int nWidth, WORD wFlags)
{      

// draw a horizontal line     
    int nTop, nHeight;   
//    HICON hIcon;   
    HRGN hClipRgn;                 // the clipping region    
    RECT rect;    

// we don't want the clip anything when we are drawing
// the icon outside the list box
	SelectClipRgn(hDC, NULL);    
/*   if (wFlags & DI_ERASEICON)
   {


      
      rect.left = -33;
      rect.right = -1;
      rect.top = nYpos -16;
      rect.bottom = nYpos + 16;   
      // ghBrush is created in WM_INITDIALOG   
      FillRect(hDC, &rect, ghBrush);
        
   }
   else
   {
        
       hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ARROW)); 
       if (hIcon)
       {
           DrawIcon(hDC,-33,nYpos - 16,hIcon);
           DestroyIcon(hIcon);
       }
   
   
   }*/
   
    
// create a clipping region for drawing the lines in the list box
    GetWindowRect(ghList, &rect);         
    hClipRgn = CreateRectRgn(0,0, rect.right - rect.left, rect.bottom - rect.top);
 
	SelectClipRgn(hDC, hClipRgn);
     // we can delete it emmdiately because SelectClipRgn makes a COPY of the region
    DeleteObject(hClipRgn); 
    
    GetClientRect(ghList, &rect);         
	rect.top+=nYpos;
	rect.bottom = rect.top+ListBox_GetItemHeight(ghList, 0);

//	DrawFocusRect(hDC, &rect);
//	return;
    
/****************************************************

  erasing something drawn on top
  the top is drawn like 
  
   ______              |_____|
  |      |  instead of |     |
  
  so we want to NOT draw the two vertical lines
  above the horzontal

*****************************************************/    
  // if (nYpos = 0) wFlags |= DI_TOPERASED;
    if (wFlags & DI_TOPERASED) 
    {
        nTop = nYpos;
        nHeight = nHtItem / 4;
    }     
/****************************************************

  erasing something originally drawn on the bottom
  
  if the list box is NOT LBS_NOINTEGRALHEIGHT, then
  the botton line will be on the border of the list
  box, so we don't want to draw the horizontal line at  
  all, ie we draw
  
  |    |           |_____|
        instead of |     |  
   

*****************************************************/     
    else if (wFlags & DI_BOTTOMERASED && !bNoIntegralHeight)
    {
    
        nTop = nYpos - nHtItem / 4;                              
        nHeight = nHtItem / 4;    
    
    } 
    
    else
    {
        nTop = nYpos - nHtItem / 4;                          
        nHeight =  nHtItem / 2;    
    
    }

    
   if (!(wFlags & DI_BOTTOMERASED && !bNoIntegralHeight)) // see above comment  
   
   {
        
        PatBlt(hDC,
               LINE_WIDTH,
               nYpos,
               nWidth - 2 * LINE_WIDTH,
               LINE_WIDTH,
               PATINVERT);   
    }           
    PatBlt(hDC,
           0,
           nTop,
           LINE_WIDTH,
           nHeight , 
           PATINVERT);                  
            
    PatBlt(hDC,
           nWidth - LINE_WIDTH,
           nTop, 
           LINE_WIDTH,
           nHeight,
           PATINVERT);
 
}