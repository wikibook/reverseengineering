#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <io.h>
#include <stdio.h>
#include "anti.h" // ntdll header
#include <conio.h>

#pragma comment(lib, "version.lib")


DWORD GetPidFromProcessName(LPTSTR szProcess)
{
	DWORD dwPid = 0;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD dwLen = _tcslen(szProcess);
	_tcslwr(szProcess);
	
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return 0; 
	
	pe32.dwSize = sizeof(PROCESSENTRY32); 
	
	if (Process32First(hProcessSnap, &pe32)) 
	{
		do 
		{			
			if (_tcsncmp(szProcess, pe32.szExeFile, dwLen) == 0)
			{				
				dwPid = pe32.th32ProcessID;
				break;
			}
		} 
		while (Process32Next(hProcessSnap, &pe32)); 
	}
	CloseHandle(hProcessSnap);
	
	return dwPid;
}

bool IsDebuggerByVersionInfo_Inside(LPCTSTR lpszFile)
{
	bool bRet = false;
	
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	// 파일없음
	if (_access(lpszFile, 0) == -1)
		return bRet;
	
	DWORD handle;
	DWORD size = GetFileVersionInfoSize(lpszFile, &handle);
	
	if (size)
	{
		char *pBlock = new char[size];
		if (GetFileVersionInfo(lpszFile, handle, size, pBlock))
		{
			UINT uSize;			
			if (VerQueryValue(pBlock, _T("\\VarFileInfo\\Translation"),
				(LPVOID*)&lpTranslate, &uSize))
			{
				TCHAR SubBlock[128], *lpVersionName;
				for (DWORD i = 0; i < (uSize/sizeof(LANGANDCODEPAGE)); i++)
				{
					_stprintf(SubBlock, 
						_T("\\StringFileInfo\\%04x%04x\\InternalName"),
						lpTranslate[i].wLanguage, 
						lpTranslate[i].wCodePage);
					
					if (VerQueryValue(pBlock, SubBlock, 
						(LPVOID*)&lpVersionName, &uSize))
					{
						// debugger detect
						if (_tcsstr(lpVersionName, "Olly"))
							bRet = true;
						else if (_tcsstr(lpVersionName, "windbg"))
							bRet = true;
						else if (_tcsstr(lpVersionName, "Syser"))
							bRet = true;

						if (bRet)
							printf("lpVersionName: %s\n", lpVersionName);
					}
				}
			}
		}
		
		delete [] pBlock;
	}
	
	return bRet;
}

bool IsDebuggerByVersionInfo()
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return false; 
	
	pe32.dwSize = sizeof(PROCESSENTRY32); 
	DWORD dwDllAddress = 0;
	
	if (Process32First(hProcessSnap, &pe32)) 
	{
		do 
		{
			MODULEENTRY32 me32       = {0,};
			HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
			
			if (hModuleSnap == INVALID_HANDLE_VALUE) 
				continue;
			
			me32.dwSize = sizeof(MODULEENTRY32);
			
			if (Module32First(hModuleSnap, &me32)) 
			{ 
				do 
				{
					if (IsDebuggerByVersionInfo_Inside(me32.szExePath))
					{
						CloseHandle (hModuleSnap);
						CloseHandle(hProcessSnap);
						return true;
					}
				} 
				while (Module32Next(hModuleSnap, &me32)); 
			}
			CloseHandle (hModuleSnap);
		} 
		while (Process32Next(hProcessSnap, &pe32)); 
	}
	CloseHandle(hProcessSnap);
	
	return false;
}

typedef LONG	NTSTATUS;
bool CheckParentProcessId()
{
	// explore.exe 의 pid
	DWORD dwExplorerPid = 0;
	GetWindowThreadProcessId(GetShellWindow(), &dwExplorerPid);

	typedef NTSTATUS (WINAPI *TNtQueryInformationProcess)(HANDLE ,UINT ,PVOID ,ULONG , PULONG);
	TNtQueryInformationProcess fnNtQueryInformationProcess;

	NTSTATUS Status = 0;
	PROCESS_BASIC_INFORMATION pbi;
	ZeroMemory(&pbi, sizeof(PROCESS_BASIC_INFORMATION));

	fnNtQueryInformationProcess = (TNtQueryInformationProcess)
								GetProcAddress( 
								GetModuleHandle("ntdll.dll"), 
								"NtQueryInformationProcess" );

	if(fnNtQueryInformationProcess == 0)
		return false;

	Status = fnNtQueryInformationProcess(GetCurrentProcess(), 
		0, (void*)&pbi, sizeof(PROCESS_BASIC_INFORMATION), 0);

	if (Status != 0x00000000)
		return false;

	if (pbi.ParentProcessId != dwExplorerPid)
		return false;

	return true;
}

bool GetCsrssHandle()
{
	typedef DWORD (__stdcall *TCsrGetId)();
	TCsrGetId fnCsrGetId;

	OSVERSIONINFO osinfo;
	ZeroMemory(&osinfo, sizeof(OSVERSIONINFO));
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osinfo);

	if (osinfo.dwMajorVersion >= 5 && osinfo.dwMinorVersion >= 1)
	{
		fnCsrGetId = (TCsrGetId)GetProcAddress(GetModuleHandle("ntdll.dll"), "CsrGetProcessId");
		DWORD dwCsrssPidByNtdll = fnCsrGetId();

		// 아래와 같이 구하는 방법도 있음
		//DWORD dwCsrssPidByThlp = GetPidFromProcessName("csrss.exe");

		if (dwCsrssPidByNtdll)
		{
			HANDLE hCsrss = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwCsrssPidByNtdll);
			if (!hCsrss)
				return false;

			CloseHandle(hCsrss);
		}
	}

	return true;
}

bool WindbgClassNameDetect()
{
	HANDLE hWinDbg = ::FindWindow("WinDbgFrameClass", NULL);

	if(hWinDbg)
		return true;

	return false;
}

void BlockAllControl()
{
	typedef BOOL (__stdcall *TBLOCKINPUT)(BOOL);
	TBLOCKINPUT fnBlockInput = (TBLOCKINPUT)GetProcAddress(GetModuleHandle("user32.dll"), "BlockInput");

	fnBlockInput(TRUE);

	Sleep(5000);

	fnBlockInput(FALSE);
}

bool CheckDbgCloseHandle()
{
	HANDLE hDummy = (HANDLE)0xB0A588;
	__try
	{
		CloseHandle(hDummy);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return true;
	}
	
	return false;
}

bool CheckOutputDebugString1()
{
	DWORD dwRet = 0;
	char *szString = "window31";
	__asm
	{
		push szString
		call ds:[OutputDebugString]
		mov dwRet, eax
	}

	if (dwRet == 1)
		return false;
	else
		return true;
}

void CheckOutputDebugString2()
{
	OutputDebugString("%s%s");
}

DWORD TimerCheck(int a, int b)
{
	DWORD dwStart = GetTickCount();
	int c = a ^ 0x369;
	int d = c + b * 0xdead;
	int e = d / a;

	DWORD dwEnd = GetTickCount();

	if (dwEnd - dwStart > 1000)
		e = 0;
	
	return e;
}

bool IsDbgPresentPrefixCheck()
{
    __try
    {
        __asm __emit 0xF3 // 0xF3 0x64 disassembles as PREFIX REP:
        __asm __emit 0x64
        __asm __emit 0xF1 
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		// debugger not found
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
	if (GetPidFromProcessName("Ollydbg.exe"))
		printf("Debugger Detected 1 (process name)\n");

	// 테스트시 느리면 여기 주석처리할 것
	//if (IsDebuggerByVersionInfo())
		//printf("Debugger Detected 2 (version name)\n");

	if (!CheckParentProcessId())
		printf("Debugger Detected 3 (parent pid)\n");

	if (GetCsrssHandle())
		printf("Debugger Detected 4 (csrss handle)\n");

	if (WindbgClassNameDetect())
		printf("Debugger Detected 5 (windbg class)\n");
	
	// 주석을 해제할 것
	//BlockAllControl();

	if (CheckDbgCloseHandle())
		printf("Debugger Detected 6 (CloseHandle)\n");

	if (CheckOutputDebugString1())
		printf("Debugger Detected 7 (OutputDebugString)\n");

	CheckOutputDebugString2();

	DWORD timer = TimerCheck(123, 456);
	if (timer == 0)
		printf("Debugger Detected 8 (TimerCheck)\n");

	if (IsDbgPresentPrefixCheck())
		printf("Debugger Detected 9 (IsDbgPresentPrefixCheck)\n");


	printf("Hello world!\n");
	
	
	_getch();

	return 0;
}