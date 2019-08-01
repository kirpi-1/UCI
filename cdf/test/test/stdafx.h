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

//#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <commctrl.h> // for InitCommonControls() 
#include <new.h>	  // for placement new
#include <tchar.h>
#include <stdio.h>
#include <cstdlib>

#include "messageDefinitions.h"

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif

#include <vector>
#include <atlbase.h>
#include <atlcom.h>
#include <atlsafe.h>
#include <atlstr.h>
