// anti3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <io.h>
#include <stdio.h>
#include <conio.h>

HANDLE hProcess = NULL;

PROCESS_INFORMATION pi;
STARTUPINFO si;


bool StackSegment()
{
	DWORD dwValue = 0;
    __asm
    {
        push ss
        pop ss
        pushf
		popf		
        pop eax
		and eax, 0x100
		or eax, eax
		mov dwValue, eax
    }

	if (dwValue)
		return true;
	else
		return false;
}

bool Int2DCheck()
{
    __try
    {
        __asm
        {
            int 0x2d
            xor eax, eax
            add eax, 2
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
    
    return true;
}

bool ObtainSeDebugPrivilege()
{
	BOOL Result;
	TOKEN_PRIVILEGES TokenPrivileges;
	TOKEN_PRIVILEGES PreviousTokenPrivileges;
	LUID luid;
	HANDLE hToken;
	DWORD dwPreviousTokenPrivilegesSize = sizeof(TOKEN_PRIVILEGES);
	
	Result = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	
	if (Result == FALSE)
		return false;
	
	Result = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);
	
	if (Result == FALSE)
		return false;
	
	TokenPrivileges.PrivilegeCount            = 1;
	TokenPrivileges.Privileges[0].Luid        = luid;
	TokenPrivileges.Privileges[0].Attributes  = 0;
	
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES),
		&PreviousTokenPrivileges, &dwPreviousTokenPrivilegesSize);
	
	if (GetLastError() != ERROR_SUCCESS)
		return false;
	
	PreviousTokenPrivileges.PrivilegeCount             = 1;
	PreviousTokenPrivileges.Privileges[0].Luid         = luid;
	PreviousTokenPrivileges.Privileges[0].Attributes  |= SE_PRIVILEGE_ENABLED;
	
	AdjustTokenPrivileges(hToken, FALSE, &PreviousTokenPrivileges,
		dwPreviousTokenPrivilegesSize, NULL, NULL);
	
	if (GetLastError() != ERROR_SUCCESS)
		return false;
	
	CloseHandle(hToken);
	
	return true;
}

void fnTRAP()
{
	//RtlAdjustPrivilege 권한을 얻어야 모듈과 프로세스명이 제대로 보인다.
   BYTE WasEn;
   typedef LONG (WINAPI *TRAP)(DWORD, BOOL, DWORD, BYTE*);
   TRAP RAP;
   // "RtlAdjustPrivilege"
	RAP = (TRAP)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlAdjustPrivilege");
   if (RAP)
      if (RAP(20, FALSE, 0, &WasEn) >= 0)
         RAP(20, TRUE, 0, &WasEn);
}

void DebugSelf()
{
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));

	si.cb = sizeof(si);
    GetStartupInfo(&si);

    // DEBUG_PROCESS 로 연다!
    CreateProcess(NULL, "C:\\FantasticTools\\DllAdmin\\AdvancedDll.exe", NULL, NULL, FALSE,
            DEBUG_PROCESS, NULL, NULL, &si, &pi); 

	DEBUG_EVENT de;
    WaitForDebugEvent(&de, INFINITE);

	ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE); 

	/*




    // Wait for an event

	*/

	//WaitForSingleObject(pi.hProcess, INFINITE);

	//계속 실행되지 않는가!!!!!!!!!

	//if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
}



DWORD CALLBACK DebugThread(LPVOID lpParam)
{
	while(1)
	{
		DEBUG_EVENT de;
		WaitForDebugEvent(&de, INFINITE);

		// Continue execution
		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE); 
			// Wait for an event		
		//printf("de.dwDebugEventCode: %d %d\n", de.dwDebugEventCode, GetLastError());

	}
	return 0;
}

void ErasePE()
{
    DWORD OldProtect = 0;
    LPBYTE lpBaseAddr = (LPBYTE)GetModuleHandle(NULL);

    VirtualProtect(lpBaseAddr, 4096, PAGE_READWRITE, &OldProtect);
	memset(lpBaseAddr, 0, 4096);
}

void ChangeSizeOfImage()
{
	__asm 
	{
		// 이미지 베이스 삭제
		mov eax, dword ptr fs:[0x30]
		mov dword ptr [eax+0x8], 0

		// LDR_DATA 의 첫번째 모듈 정보 조작
		mov eax, dword ptr fs:[0x30]
		mov eax, dword ptr [eax+0x0c]
		mov eax, dword ptr [eax+0x0c]

		// 모듈의 베이스 주소 삭제
		mov dword ptr [eax+0x18], 0

		// 모듈의 엔트리 포인트 삭제 
		mov dword ptr [eax+0x1c], 0
		
		// 모듈의 사이즈 변조
		add dword ptr [eax+0x20], 0x2000
	}
}

bool HardwareBP_Detection()
{
	bool bDetect = false;
	CONTEXT ctx;

	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	if (GetThreadContext((HANDLE)0xFFFFFFFE, &ctx))
	{
		PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS pNtHdrs = (PIMAGE_NT_HEADERS)((DWORD)pDosHdr + pDosHdr->e_lfanew);
		PIMAGE_SECTION_HEADER pSecHdr = (PIMAGE_SECTION_HEADER)((DWORD)pNtHdrs + sizeof(IMAGE_NT_HEADERS));

		DWORD dwStartAddr = (DWORD)pDosHdr + pNtHdrs->OptionalHeader.BaseOfCode;
		DWORD dwEndAddr   = dwStartAddr + pNtHdrs->OptionalHeader.SizeOfImage;

		if (ctx.Dr0>=dwStartAddr && ctx.Dr0<dwEndAddr)
			bDetect = true;

		if (ctx.Dr1>=dwStartAddr && ctx.Dr1<dwEndAddr)
			bDetect = true;	

		if (ctx.Dr2>=dwStartAddr && ctx.Dr2<dwEndAddr)
			bDetect = true;

		if (ctx.Dr3>=dwStartAddr && ctx.Dr3<dwEndAddr)
			bDetect = true;
	}

	return bDetect;
}

int main(int argc, char* argv[])
{
	//ObtainSeDebugPrivilege();
	fnTRAP();

	if (StackSegment())
		printf("debugger detected 1, StackSegment()\n");

	// 뻑날 수 있으므로 주석을 해 놓고 테스트 해야 그 아래 함수들도 실행됨
	//if (Int2DCheck())
		//printf("debugger detected 2, Int2DCheck()\n");

	DebugSelf();

	// 이것도 주석해야 아래 HardwareBP_Detection 가 제대로 실행된다
	//ErasePE();

	// 이것도 주석해야 아래 HardwareBP_Detection 가 제대로 실행된다
	//ChangeSizeOfImage();

	if (HardwareBP_Detection())
		printf("debugger detected 3, HardwareBP_Detection()\n");
	
	printf("Hello World! %d\n", GetCurrentProcessId());

	DWORD th = 0;
	CreateThread(NULL, 0, DebugThread, NULL, 0, &th);

	int i=0;
	while(1)
	{
		//OutputDebugString("a");
		//Check();
		//printf("%d.", i++);
		Sleep(500);
	}

	_getch();
	return 0;
}

