#include <windows.h>
#include "stdio.h"
#include <tchar.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include "TestDll.h"
#include "TraceBC.h"

//#pragma comment(linker, "/base:0x33300000 /fixed" )

typedef void (__cdecl *TPseudoSend)(int, char *, int);
TPseudoSend lpPseudoFunc;

char *g_buf = NULL;
int g_len = 0;

__declspec(naked) HookSend()
{
	__asm
	{
		push ebx
		push eax
		push edx
		mov ebx, [ebp+0x10]
		mov eax, [ebp+0xC]
		mov edx, [ebp+0x4]
		push ebx
		push eax
		push edx
		call lpPseudoFunc
		add esp, 0xc
		pop edx
		pop eax
		pop ebx
		mov eax, 0x719E4C43
		jmp eax
	}
}

void MakeJMP(BYTE *pAddress, DWORD dwJumpTo, DWORD dwLen)
{
    DWORD dwOldProtect, dwBkup, dwRelAddr;
    VirtualProtect(pAddress, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    dwRelAddr = (DWORD) (dwJumpTo - (DWORD) pAddress) - 5;
    *pAddress = 0xE9;
    *((DWORD *)(pAddress + 0x1)) = dwRelAddr;
    for(DWORD x = 0x5; x < dwLen; x++) *(pAddress + x) = 0x90;
    VirtualProtect(pAddress, dwLen, dwOldProtect, &dwBkup);
}

void PseudoFunc(int retaddr, char *buf, int len)
{
	if (len != 1 && len > 0)
	{
		//TRACEB("[%x] send : [%d]", retaddr, len);

		int len_temp = 0;
		int len_temp2 = 0;
		char szMsg1[MAX_PATH] = {0,};
		char szMsg2[MAX_PATH] = {0,};
		for (int i=0; i<len; ++i)
		{
			if (MAX_PATH < i)
				break;

			if (buf[len_temp] == '\0')
				len_temp += sprintf(szMsg1 + len_temp, "_");
			else
				len_temp += sprintf(szMsg1 + len_temp, "%c", buf[i]);
			
			// 200 바이트 이상은 찍지 않는다
			if (len_temp2 < 200)
				len_temp2 += sprintf(szMsg2 + len_temp2, "%02X ", *(buf + i));
		}
		
		TRACEB("[%x] send : [%d] %s", retaddr, len, szMsg1);
		TRACEB("     dump : %s", szMsg2);
	}
}

void InitHooking3()
{
	lpPseudoFunc = PseudoFunc;

	typedef void (WINAPI *Tsend)(SOCKET, const char *, int, int);
	Tsend fnsend;

	TCHAR szWs2_32Dll[MAX_PATH];
	GetSystemDirectory(szWs2_32Dll, MAX_PATH);
	_tcscat(szWs2_32Dll, _T("\\ws2_32.dll"));
	HMODULE hMod = LoadLibrary(szWs2_32Dll);
	fnsend = (Tsend)GetProcAddress(hMod, "send");
	LPVOID lpTargetAdr = (LPVOID)((DWORD)fnsend + 0x16);

	MakeJMP((LPBYTE)lpTargetAdr, (DWORD)HookSend, 6);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)hinstDLL);
		InitHooking3();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
	}

	return TRUE;
}