//=============================================================================
//    File : Log.h
//     Use : Log Class
//  Editor : ByungTak Kang (window31@nexon.co.kr)
//    Date : 2007/10/09
//  Modify : 2008/09/25 : delete date, add Start() param
//=============================================================================

#ifndef __LOG_H__
#define __LOG_H__

#include "malloc.h"
#include "TraceBC.h"


class CCriticalSector
{
public:
   CCriticalSector()    { InitializeCriticalSection(&m_cs); }
   ~CCriticalSector()   { DeleteCriticalSection(&m_cs); }
	
   void Enter() { EnterCriticalSection(&m_cs); }
   void Leave() { LeaveCriticalSection(&m_cs); }
	
private:
   CRITICAL_SECTION m_cs;
};





// #define MYLOG			log.WriteLog 이걸ㅇ 선언해주3


#define TRACE_BUFFER_SIZE     2048


class CLog
{
public:
   CLog() 	{	m_hFile = NULL; }
   ~CLog()	{}
	Start(DWORD dwVersion, DWORD dwFileName);
	WriteLog(LPCTSTR lpszFormat, ...);
	End();	
	
	HANDLE m_hFile;
};

CLog::Start(DWORD dwVersion, DWORD dwFileName)
{
	// CreatePacketBuffer 용 로그 파일 따로 준비
	SYSTEMTIME st;
	GetLocalTime(&st);

	TCHAR szLogFile[MAX_PATH];
	TCHAR szTemp[64];	
	GetCurrentDirectory(MAX_PATH, szLogFile);
	sprintf(szTemp, "\\plugin\\HelloOlly_%d.log", dwFileName);
	_tcscat(szLogFile, szTemp);
	
	m_hFile = CreateFile(szLogFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (!m_hFile)
	{
		TRACEB("CreateFile Failed: %d", GetLastError());
		return 0;
	}
	
	SetFilePointer(m_hFile, 0, 0, FILE_END);
	
	char szBuf[MAX_PATH * 3] = {0,};
	memset(szBuf, 0, MAX_PATH * 3);

	/*
	char test[64];
	sprintf(test, "aaaaaaaaaaaaa %d", dwVersion);
	OutputDebugString(test);
	*/
	
	wsprintf(szBuf, "===========================================================================\r\n============       Log Start: %d, %02d, %02d - %02d:%02d:%02d:%03d     =============\r\n============                                                  =============\r\n============    Revision [%03d]. Build: %s, %s  =============\r\n===========================================================================\r\n\r\n", 
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
	
	char szBuf[MAX_PATH * 2] = {0,};
	memset(szBuf, 0, MAX_PATH * 2);
	
	wsprintf(szBuf, "===========================================================================\r\n============       Log End: %d, %02d, %02d - %02d:%02d:%02d:%03d     =============\r\n===========================================================================\r\n", 
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
	{
		OutputDebugString("왜 핸들이 없노");
		return 0;
	}
	
	//TCHAR szBuffer[TRACE_BUFFER_SIZE];	
	//TCHAR szBuf[TRACE_BUFFER_SIZE];
	char *szBuf = (char *)malloc(TRACE_BUFFER_SIZE);
	char *szBuffer = (char *)malloc(TRACE_BUFFER_SIZE);
	memset(szBuf, 0, TRACE_BUFFER_SIZE);
	memset(szBuffer, 0, TRACE_BUFFER_SIZE);

	va_list fmtList;
	
	va_start(fmtList, lpszFormat);
	_vsntprintf(szBuffer, TRACE_BUFFER_SIZE - 1, lpszFormat, fmtList);
	va_end(fmtList);
	
	szBuffer[TRACE_BUFFER_SIZE - 1] = 0;
	
	char szTime[TRACE_BUFFER_SIZE] = {0,};
	wsprintf(szBuf, "%s\r\n", szBuffer);
	
	DWORD dwLen = _tcslen(szBuf);

	DWORD dwWritten = 0;
	WriteFile(m_hFile, szBuf, dwLen, &dwWritten, NULL);

	free(szBuf);
	free(szBuffer);

	return 0;
}



#endif