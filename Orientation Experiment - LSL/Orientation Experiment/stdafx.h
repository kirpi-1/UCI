
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
// If app hasn't choosen, set to work with Windows 2000, Windows XP and beyond
#ifndef WINVER
#define WINVER			0x0520
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS	0x0520 
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0520 
#endif


#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds
#pragma warning( disable : 4324 ) // disable padded structures warnings for /W4 builds


#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

//#ifndef VC_EXTRALEAN
//#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
//#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes




#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#include <afxsock.h>






#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//#include <windows.h>
#include <new.h>	  // for placement new
#include <tchar.h>
#include <stdio.h>
#include <cstdlib>

#include <vector>
#include <atlbase.h>
#include <atlcom.h>
#include <atlsafe.h>
#include <atlstr.h>

