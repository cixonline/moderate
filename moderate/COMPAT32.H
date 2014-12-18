#ifndef _HORUS_COMPAT32_H
#define _HORUS_COMPAT32_H



#include <string.h>



#if !defined(MVS)
    #if defined(STRICT)
        #define __WNDPROC WNDPROC
    #else
        #define __WNDPROC FARPROC
    #endif
#endif



#if defined(_WIN32) || defined(WIN32) || defined(MVS)
    #if !defined(MVS)
        #if !defined(_WIN32)
            #define _WIN32
        #endif

        #if !defined(WIN32)
            #define WIN32
        #endif
    #endif

    #if !defined(_EXPORT)
        #define _EXPORT
    #endif

    #if !defined(EXPORT)
        #define EXPORT
    #endif

    #if !defined(HUGE)
        #define HUGE
    #endif

    #if !defined(DECLARE_HANDLE32)
        #define DECLARE_HANDLE32 DECLARE_HANDLE
    #endif

    #if !defined(MVS)
        #define Post_ComboBox_SetCurSel(hwndCtl, index) \
            ((int)(DWORD)PostMessage((hwndCtl), CB_SETCURSEL, (WPARAM)(int)(index), 0L))

        #define WFORWARD_MESSAGE(hwnd, uMsg, wParam, lParam, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), (uMsg), (wParam), (lParam))

        #define WFORWARD_WM_DESTROY(hwnd, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_DESTROY, 0, 0L)

        #define WFORWARD_WM_SETFOCUS(hwnd, hwndOldFocus, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_SETFOCUS, (WPARAM)(HWND)(hwndOldFocus), 0L)

        #define WFORWARD_WM_KILLFOCUS(hwnd, hwndNewFocus, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_KILLFOCUS, (WPARAM)(HWND)(hwndNewFocus), 0L)

        #define WFORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_KEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

        #define WFORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_KEYUP, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

        #define WFORWARD_WM_CHAR(hwnd, ch, cRepeat, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_CHAR, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(cRepeat),0))

        #define WFORWARD_WM_LBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), (fDoubleClick) ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN, (WPARAM)(UINT)(keyFlags)

        #define WFORWARD_WM_NOTIFY(hwnd, idFrom, pnmhdr, fn) \
            CallWindowProc((__WNDPROC)(fn), (hwnd), WM_NOTIFY, (WPARAM)(int)(id), (LPARAM)(NMHDR FAR*)(pnmhdr))

        #define InstanceFromWindow(hwnd) ((HMODULE)GetWindowLong((hwnd), GWL_HINSTANCE))
    #endif

    #define lmemcpy(dest, src, count) memcpy(dest, src, count)
    #define lmemmove(dest, src, count) memmove(dest, src, count)
    #define lmemcmp(addr1, addr2, count) memcmp(addr1, addr2, count)
    #define lmemset(dest, val, count) memset(dest, val, count)
#else
    #if !defined(_EXPORT)
        #define _EXPORT _export
    #endif

    #if !defined(EXPORT)
        #define EXPORT _export
    #endif

    #if !defined(HUGE)
        #define HUGE _huge
    #endif

    #define Post_ComboBox_SetCurSel(hwndCtl, index) \
        ((int)(DWORD)PostMessage((hwndCtl), CB_SETCURSEL, (WPARAM)(int)(index), 0L))

    #define WFORWARD_MESSAGE(hwnd, uMsg, wParam, lParam, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), (uMsg), (wParam), (lParam))

    #define WFORWARD_WM_DESTROY(hwnd, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_DESTROY, 0, 0L)

    #define WFORWARD_WM_SETFOCUS(hwnd, hwndOldFocus, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_SETFOCUS, (WPARAM)(HWND)(hwndOldFocus), 0L)

    #define WFORWARD_WM_KILLFOCUS(hwnd, hwndNewFocus, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_KILLFOCUS, (WPARAM)(HWND)(hwndNewFocus), 0L)

    #define WFORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_KEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

    #define WFORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_KEYUP, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

    #define WFORWARD_WM_CHAR(hwnd, ch, cRepeat, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_CHAR, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(cRepeat),0))

    #define WFORWARD_WM_LBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), (fDoubleClick) ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MA

    #define WFORWARD_WM_NOTIFY(hwnd, idFrom, pnmhdr, fn) \
        CallWindowProc((__WNDPROC)(fn), (hwnd), WM_NOTIFY, (WPARAM)(int)(id), (LPARAM)(NMHDR FAR*)(pnmhdr))

    #define InstanceFromWindow(hwnd) ((HINSTANCE)GetWindowWord(hwnd, GWW_HINSTANCE))

    #define lmemcpy(dest, src, count) _fmemcpy(dest, src, count)
    #define lmemmove(dest, src, count) _fmemmove(dest, src, count)
    #define lmemcmp(addr1, addr2, count) _fmemcmp(addr1, addr2, count)
    #define lmemset(dest, val, count) _fmemset(dest, val, count)
#endif



#endif
