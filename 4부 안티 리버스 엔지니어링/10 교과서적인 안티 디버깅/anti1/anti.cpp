// anti.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "tchar.h"
#include "anti.h" // ntdll header

void IDP()
{
	typedef BOOL (WINAPI *TIsDebuggerPresent)();
	TIsDebuggerPresent fnIsDebuggerPresent = (TIsDebuggerPresent)GetProcAddress(GetModuleHandle("kernel32.dll"), "IsDebuggerPresent");
	if (fnIsDebuggerPresent())
	{ 
	   printf("1 Debugger Detected !!!\n");
	}

	DWORD dwIsDebugger;
	__asm
	{
		MOV     eax, fs:[18h]
		MOV     eax, [eax+30h]
		MOVZX   eax, byte ptr [eax+2]
		mov     [dwIsDebugger], eax
	}

	if (dwIsDebugger != 0)
	{
		printf("2 IDP !!!\n");
	}	
}

typedef DWORD (WINAPI *TNtQueryInformationProcess)(HANDLE, DWORD, PVOID, DWORD, PVOID);
TNtQueryInformationProcess NtQueryInformationProcess;
typedef BOOL (WINAPI *TCheckRemoteDebuggerPresent)(HANDLE, PBOOL);
TCheckRemoteDebuggerPresent fnCheckRemoteDebuggerPresent;

void CheckDebugPort(HANDLE hProcess)
{
	HANDLE hPort = NULL;
	ULONG result = 0;	
	HMODULE hntdll;
	
	hntdll = GetModuleHandle("ntdll.dll");
	NtQueryInformationProcess = (TNtQueryInformationProcess)GetProcAddress(hntdll, 
		_T("NtQueryInformationProcess"));
	
	NtQueryInformationProcess(hProcess, 7, &hPort, sizeof hPort, &result);

	if (hPort)
	{
		printf("3 NtQueryInformationProcess Debugger !!\n");
		CloseHandle(hPort);
	}


	////////////////////////////////////////////////////////////

	HMODULE hKernel32 = GetModuleHandle("kernel32.dll");
	fnCheckRemoteDebuggerPresent = (TCheckRemoteDebuggerPresent)GetProcAddress(hKernel32, 
		_T("CheckRemoteDebuggerPresent"));

	BOOL bDebugged = FALSE;
	fnCheckRemoteDebuggerPresent(hProcess, &bDebugged);

	if (bDebugged)
		printf("4 CheckRemoteDebuggerPresent\n");

}

typedef LONG	NTSTATUS;
void HideThread(HANDLE hThread)
{
	typedef NTSTATUS (NTAPI *TNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);

	NTSTATUS Status;

	TNtSetInformationThread NtSetInformationThread = (TNtSetInformationThread)
		GetProcAddress(GetModuleHandle( TEXT("ntdll.dll") ),
						"NtSetInformationThread");
	if (NtSetInformationThread == NULL)
		return;

	if (hThread == NULL)
		Status = NtSetInformationThread(GetCurrentThread(), 
						0x11, //ThreadHideFromDebugger
						0, 0);
	else
		Status = NtSetInformationThread(hThread, 0x11, 0, 0);

	if (Status != 0x00000000)
	{
		printf("5 NtSetInformationThread!\n");
		return;
	}
	else
		return;
}

void DebugObjectCheck()
{
	HANDLE hDebugObject = NULL;
	NTSTATUS Status;

	ULONG result = 0;	
	HMODULE hntdll;
	
	hntdll = GetModuleHandle("ntdll.dll");
	NtQueryInformationProcess = (TNtQueryInformationProcess)GetProcAddress(hntdll, 
		_T("NtQueryInformationProcess"));
	
	Status = NtQueryInformationProcess(GetCurrentProcess(), 
					0x1e, // ProcessDebugObjectHandle
					&hDebugObject, 4, NULL);

	if (Status != 0x00000000)
		return;

	if (hDebugObject)
	{
		printf("6 NtQueryInformationProcess!\n");
	}
}

void CheckProcessDebugFlags()
{
	DWORD NoDebugInherit = 0;
	NTSTATUS Status;

	ULONG result = 0;	
	HMODULE hntdll;
	
	hntdll = GetModuleHandle("ntdll.dll");
	NtQueryInformationProcess = (TNtQueryInformationProcess)GetProcAddress(hntdll, 
		_T("NtQueryInformationProcess"));
	
	Status = NtQueryInformationProcess(GetCurrentProcess(), 
		0x1f, // ProcessDebugFlags
		&NoDebugInherit, 4, NULL);

	if (Status != 0x00000000)
		return;

	if (NoDebugInherit == FALSE)
	{
		printf("7 NtQueryInformationProcess!\n");
	}
}

bool ObjectListCheck()
{
	typedef NTSTATUS(NTAPI *TNtQueryObject)(HANDLE, UINT, PVOID, ULONG, PULONG);

	POBJECT_ALL_INFORMATION pObjectAllInfo = NULL;
	void *pMemory = NULL;
	NTSTATUS Status;
	unsigned long Size = 0;

	TNtQueryObject NtQueryObject = (TNtQueryObject)GetProcAddress(GetModuleHandle( TEXT( "ntdll.dll" ) ), "NtQueryObject" );

	Status = NtQueryObject(NULL, 3, //ObjectAllTypesInformation
						&Size, 4, &Size);

	pMemory = VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if(pMemory == NULL)
		return false;

	Status = NtQueryObject((HANDLE)-1, 3, pMemory, Size, NULL);

	if (Status != 0x00000000)
	{
		VirtualFree(pMemory, 0, MEM_RELEASE);
		return false;
	}

	pObjectAllInfo = (POBJECT_ALL_INFORMATION)pMemory;

	unsigned char *pObjInfoLocation = (unsigned char*)pObjectAllInfo->ObjectTypeInformation;
	ULONG NumObjects = pObjectAllInfo->NumberOfObjects;

	for(UINT i = 0; i < NumObjects; i++)
	{

		POBJECT_TYPE_INFORMATION pObjectTypeInfo = (POBJECT_TYPE_INFORMATION)pObjInfoLocation;

		// The debug object will always be present
		if (wcscmp(L"DebugObject", pObjectTypeInfo->TypeName.Buffer) == 0)
		{
			// Are there any objects?
			if (pObjectTypeInfo->TotalNumberOfObjects > 0)
			{
				printf("8 NtQueryObject!\n");
				VirtualFree(pMemory, 0, MEM_RELEASE);
				return true;
			}
			else
			{
				VirtualFree(pMemory, 0, MEM_RELEASE);
				return false;
			}
		}

		pObjInfoLocation = (unsigned char*)pObjectTypeInfo->TypeName.Buffer;
		pObjInfoLocation += pObjectTypeInfo->TypeName.Length;
		ULONG tmp = ((ULONG)pObjInfoLocation) & -4;
		pObjInfoLocation = ((unsigned char*)tmp) + sizeof(unsigned long);
	}

	VirtualFree(pMemory, 0, MEM_RELEASE);
	return true; 
}

void Int3SingStepDetection()
{
	DWORD dwDebugger = 1;
	__try
	{
		__asm
		{
			// int 3
			__emit 0xcc
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwDebugger = 0;
	}

	if (dwDebugger)
		printf("9 Int3SingStepDetection!\n");
}

LONG WINAPI UnhandledExcepFilter(PEXCEPTION_POINTERS pExcepPointers)
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)pExcepPointers->ContextRecord->Eax);

    // Skip the exception code
    pExcepPointers->ContextRecord->Eip += 2;

    return EXCEPTION_CONTINUE_EXECUTION;
}

void UnHandleException()
{
    SetUnhandledExceptionFilter(UnhandledExcepFilter);
    __asm{xor eax, eax}
    __asm{div eax}
}

int main(int argc, char* argv[])
{
	IDP();

	CheckDebugPort(GetCurrentProcess());

	// 테스트할 떄 힘드므로 아래 HideThread() 는 주석처리하며 디버깅 해볼 것
	//HideThread(NULL);

	DebugObjectCheck();

	CheckProcessDebugFlags();

	ObjectListCheck();

	Int3SingStepDetection();

	UnHandleException();
	
	printf("Complete !\n");
	return 0;
}

