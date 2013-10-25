#include <windows.h>
#include "stdio.h"
#include <tchar.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include "TestDll.h"
#include "TraceBC.h"

#pragma comment(linker, "/base:0x33300000 /fixed" )

//typedef void (__cdecl *TPseudoSend)(int, char *, int);
//TPsudeSend lpPseudoFunc;

__declspec(naked) HookSend()
{
	__asm
	{
		mov eax, [ebp+0xC]
		push eax
		call ds:OutputDebugString
		mov eax, 0x719E4C43
		jmp eax
	}
}

void InitHooking1()
{
	typedef void (WINAPI *Tsend)(SOCKET, const char *, int, int);
	Tsend fnsend;

	TCHAR szWs2_32Dll[MAX_PATH];
	GetSystemDirectory(szWs2_32Dll, MAX_PATH);
	_tcscat(szWs2_32Dll, _T("\\ws2_32.dll"));
	HMODULE hMod = LoadLibrary(szWs2_32Dll);
	fnsend = (Tsend)GetProcAddress(hMod, "send");

	//719E4C3D                          0F84 256A0000              je ws2_32.719EB668
	//719E4C3D 를 후킹할 것임
	LPVOID lpTargetAdr = (LPVOID)((DWORD)fnsend + 0x16);

	TRACEB(_T("lpTargetAdr: %x"), lpTargetAdr);
	DWORD dwOldProtect = 0;
	VirtualProtect(lpTargetAdr, 6, PAGE_READWRITE, &dwOldProtect);
	
	// 기초 설명을 위한 하드코딩
	//719E4C3D                        - E9 BEC3619E              jmp btmmhook.10001000
	//719E4C42                          90                       nop
	//위 코드를 아래와 같이 변경함.
	//719E4C3D         - E9 BEC391C1     jmp 33301000
	//719E4C42           90              nop
	*((LPBYTE)lpTargetAdr + 0) = 0xE9;
	*((LPBYTE)lpTargetAdr + 1) = 0xBE;
	*((LPBYTE)lpTargetAdr + 2) = 0xC3;
	*((LPBYTE)lpTargetAdr + 3) = 0x91;
	*((LPBYTE)lpTargetAdr + 4) = 0xC1;
	*((LPBYTE)lpTargetAdr + 5) = 0x90;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)hinstDLL);
		InitHooking1();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
	}

	return TRUE;
}