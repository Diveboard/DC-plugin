//kk: common and often used headers

//#define _WIN32_WINNT 0x601

#include <windows.h>
//#include <windowsx.h>
#include <stdio.h>
//#include <atlstr.h>


#include <string>
#include <vector>
#include <sstream>

#include <boost/format.hpp>

//for Mac
//#define			 WCHAR char;
//#define			 BOOL char;

#ifdef __MACH__
typedef unsigned int UINT;
typedef char BOOL;
typedef wchar_t WCHAR;
typedef unsigned int DWORD;
typedef const char * LPCTSTR;
typedef void * LPVOID;
typedef char * TCHAR;
typedef int HANDLE;
#endif

//Mozilla-API
#include <npfunctions.h>
#include <npruntime.h>
#include "npruntime.h"

#define TRACE __noop
