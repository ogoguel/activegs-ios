#include "stdafx.h"

#include <afxinet.h>
#include "ZipArchive.h"
#include <sys/stat.h>
#include "activetools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TCActiveBaseCollection CActiveBase::m_Collection;
HHOOK CActiveBase::m_hHook=NULL;

LRESULT CALLBACK CActiveBase::GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // If this is a keystrokes message, pass it to IsDialogMessage for tab
    // and accelerator processing
    LPMSG lpMsg = (LPMSG) lParam;

    if (
		(nCode >= 0) &&
       // PM_REMOVE == wParam &&
			(
			(lpMsg->message == WM_LBUTTONDOWN) ||
			(lpMsg->message == WM_MOUSEMOVE) ||
			(lpMsg->message == WM_LBUTTONUP) ||
			(lpMsg->message == WM_RBUTTONDOWN) ||
			(lpMsg->message == WM_MBUTTONDOWN) ||
			(lpMsg->message == WM_MBUTTONUP) ||
			(lpMsg->message == WM_RBUTTONUP) 
			)
		)
 

    {
        CActiveBase* pObj; //, hActiveWindow = GetActiveWindow();
        TCActiveBaseCollection::iterator it =m_Collection.begin();

        // check each window we manage to see if the message is meant for them
        while (it != m_Collection.end())
        {
            pObj = *it;

       //     if (::IsWindow(hWnd))
               // ::IsDialogMessage(hWnd, lpMsg)
			   
            {
				pObj->m_ttip.RelayEvent(lpMsg);
                /*
                lpMsg->hwnd = NULL;
                lpMsg->message = WM_NULL;
                lpMsg->lParam = 0L;
                lpMsg->wParam = 0;
				*/

             //   break;
            }

            it++;
        }
    }

    // Passes the hook information to the next hook procedure in
    // the current hook chain.
    return ::CallNextHookEx(m_hHook, nCode, wParam, lParam);
}

HRESULT CActiveBase::InstallHook()
{
    // make sure the hook is installed
    if (m_hHook == NULL)
    {
        m_hHook = ::SetWindowsHookEx(WH_GETMESSAGE,
                                     GetMessageProc,
                                     AfxGetInstanceHandle(),
                                     GetCurrentThreadId());

        // is the hook set?
        if (m_hHook == NULL)
        {
            return E_UNEXPECTED;
        }
    }

    // add the window to our list of managed windows
    if (m_Collection.find(this) == m_Collection.end())
        m_Collection.insert(this);

    return S_OK;
}

HRESULT CActiveBase::UninstallHook()
{
    HRESULT hr = S_OK;

    // was the window found?
    if (m_Collection.erase(this) == 0)
        return E_INVALIDARG;

    // is this the last window? if so, then uninstall the hook
    if (m_Collection.size() == 0 && m_hHook)
    {
        if (!::UnhookWindowsHookEx(m_hHook))
            hr = HRESULT_FROM_WIN32(::GetLastError());

        m_hHook = NULL;
    }

    return hr;
}
