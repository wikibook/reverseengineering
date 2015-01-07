//=============================================================================
//    File : Log.h
//     Use : Log Class
//  Editor : ByungTak Kang (window31@empal.com)
//=============================================================================

#ifndef __LOG_H__
#define __LOG_H__

#include <windows.h>
#include <stdio.h>

#include "tchar.h"

class CCriticalSector
{
public:
   CCriticalSector()    { InitializeCriticalSection(&cs); }
   ~CCriticalSector()   { DeleteCriticalSection(&cs); }
	
   void Enter() { EnterCriticalSection(&cs); }
   void Leave() { LeaveCriticalSection(&cs); }
	
private:
   CRITICAL_SECTION cs;
};








#define TRACE_BUFFER_SIZE     2048


class CLog
{
public:
   CLog() 	{	m_hFile = NULL; }
   ~CLog()	{}
	Start(DWORD dwVersion, TCHAR *szFileName);
	WriteLog(LPCTSTR lpszFormat, ...);
	End();	
	
	HANDLE m_hFile;
};

CLog::Start(DWORD dwVersion, TCHAR *szFileName)
{
	// CreatePacketBuffer 용 로그 파일 따로 준비
	SYSTEMTIME st;
	GetLocalTime(&st);

	TCHAR szLogFile[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szLogFile);

	_tcscat(szLogFile, _T("\\"));
	_tcscat(szLogFile, szFileName);
	_tcscat(szLogFile, _T(".log"));
	
	m_hFile = CreateFile(szLogFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// 항상 새로맹금 m_hFile = CreateFile(szLogFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (!m_hFile)
		return 0;
	
	SetFilePointer(m_hFile, 0, 0, FILE_END);
	
	TCHAR szBuf[MAX_PATH * 3] = {0,};
	memset(szBuf, 0, MAX_PATH * 3);

	wsprintf(szBuf, _T("===========================================================================\r\n============       Log Start: %d, %02d, %02d - %02d:%02d:%02d:%03d     =============\r\n============                                                  =============\r\n============    Revision [%03d]. Build: %s, %s  =============\r\n===========================================================================\r\n\r\n"), 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, dwVersion, __DATE__, __TIME__);
	
	DWORD dwLen = _tcslen(szBuf);
	
	DWORD dwWritten = 0;
	WriteFile(m_hFile, szBuf, dwLen, &dwWritten, NULL);

	return 0;
}

CLog::End()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	TCHAR szBuf[MAX_PATH * 2] = {0,};
	memset(szBuf, 0, MAX_PATH * 2);
	
	wsprintf(szBuf, _T("===========================================================================\r\n============        Log End: %d, %02d, %02d - %02d:%02d:%02d:%03d      =============\r\n===========================================================================\r\n"), 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	DWORD dwLen = _tcslen(szBuf);
	
	DWORD dwWritten = 0;
	WriteFile(m_hFile, szBuf, dwLen, &dwWritten, NULL);

	CloseHandle(m_hFile);
}

CLog::WriteLog(LPCTSTR lpszFormat, ...)
{
	CCriticalSector cs;

	if (m_hFile == NULL)
		return 0;
	
	TCHAR *szBuf = (TCHAR *)malloc(TRACE_BUFFER_SIZE);
	TCHAR *szBuffer = (TCHAR *)malloc(TRACE_BUFFER_SIZE);
	memset(szBuf, 0, TRACE_BUFFER_SIZE);
	memset(szBuffer, 0, TRACE_BUFFER_SIZE);

	va_list fmtList;
	
	va_start(fmtList, lpszFormat);
	_vsntprintf(szBuffer, TRACE_BUFFER_SIZE - 1, lpszFormat, fmtList);
	va_end(fmtList);
	
	szBuffer[TRACE_BUFFER_SIZE - 1] = 0;
	
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	TCHAR szTime[TRACE_BUFFER_SIZE] = {0,};
	wsprintf(szBuf, _T("%02d:%02d:%02d:%03d     %s\r\n"), 
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, szBuffer);
	
	DWORD dwLen = _tcslen(szBuf);

	DWORD dwWritten = 0;
	WriteFile(m_hFile, szBuf, dwLen, &dwWritten, NULL);
	
	free(szBuf);
	free(szBuffer);
	
	return 0;
}

#endif