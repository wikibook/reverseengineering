#pragma once

#ifndef __TRACEBC_H__
#define __TRACEBC_H__

#if !defined(__RELEASE)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define TRACEB                   CMyTrace::TraceFormat

class CMyTrace
{
public:
   CMyTrace() 
	{
	}
   ~CMyTrace() 
	{
	}

   static void TraceFormat(LPCTSTR szFormat, ...)
   {
      TCHAR szBuf[512];
      va_list fmtList;

      va_start(fmtList, szFormat);
      _vsntprintf(szBuf, 511, szFormat, fmtList);
      va_end(fmtList);

      szBuf[511] = 0;

      OutputDebugString(szBuf);
   }

private:
   CMyTrace(const CMyTrace &rhs);
   CMyTrace &operator=(const CMyTrace &rhs);
};


#else // !defined(__RELEASE)


#define TRACEB


#endif // !defined(__RELEASE)


#endif // __TRACEBC_H__
