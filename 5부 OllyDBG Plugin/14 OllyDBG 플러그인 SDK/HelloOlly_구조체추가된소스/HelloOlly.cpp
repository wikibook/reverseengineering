// HelloOlly.cpp : Defines the entry point for the DLL application.
//





#include "stdafx.h"
#include "plugin.h"
#include "HelloOlly.h"
#include "TraceBC.h"
#include <tlhelp32.h>
#include "Log.h"

#include "resource.h"


#pragma comment(lib, "OLLYDBG.LIB")


HWND             hwmain;               // Handle of main OllyDbg window
HINSTANCE        hinst;                // DLL instance

CLog log;


#define MYLOG			log.WriteLog //이걸ㅇ 선언해주3





BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call==DLL_PROCESS_ATTACH)
	{
		hinst=(HINSTANCE)hModule;       // Mark plugin instance
		log.Start(1, 2);

	}

	return TRUE;
}

extc int _export cdecl ODBG_Plugindata(char shortname[32]) 
{
	strcpy(shortname, "HelloOlly");       // Name of plugin
	return PLUGIN_VERSION;
};

extc int _export cdecl ODBG_Plugininit(
  int ollydbgversion,HWND hw,ulong *features) 
{
	// OllyDBG와 Plugin SDK 버전을 비교하여
	// Plugin이 더 높다면 -1 리턴하여 실행시키지 않음
	if (ollydbgversion<PLUGIN_VERSION)
		return -1;

	// OllyDBG의 메인 윈도우 핸들을 받아놓는다. 
	// 이 핸들은 메시지박스등을 출력하기 위해 반드시 필요하다	
	hwmain=hw;

	// Log Window에 문자열을 출력한다.
	Addtolist(0, 0, "Hello Olly  plugin v0.01 (test plugin)");
	Addtolist(0, -1, "made by window31");	

	return 0;
}

extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item) 
{
	switch (origin) 
	{
	case PM_MAIN: // Plugin menu in main window
		strcpy(data,
			"0 &Hello Olly Dialog,"
			"1 Test|"
			"2 About"
			);
		return 1;
	
	case PM_DISASM:
		if (Getstatus() == STAT_NONE) 
		{
			return 0;
		}
		strcpy(data,"0 &Hello Olly Dialog");
		return 1;
	
	case PM_THREADS:
		if (Getstatus() == STAT_NONE) 
		{
			return 0;
		}
		strcpy(data,"0 &Hello Olly Dialog");
		return 1;
	
	default:
		break; // Any other window
	};
	return 0; // Window not supported by plugin
};

// attach to last created process.
void attach_to_last (void)
{
	int next;
	HANDLE h_proc;
	PROCESSENTRY32 last_pe, pe;
	
	h_proc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	next = Process32First(h_proc, &pe);
	
	while (next)
	{
		// get the next process entry.
		next = Process32Next(h_proc, &pe);		
		
		// ensure we don't attach to ourself. if the last process was ollydbg, then we want the second to last.
		if (stricmp(pe.szExeFile, "ollydbg.exe") != 0)
			memcpy(&last_pe, &pe, sizeof(PROCESSENTRY32));			
		
		// if we've found the last process, attach to the process.
		if (!next && (GetLastError() == ERROR_NO_MORE_FILES))
		{
			Addtolist(0, 0, "Attaching to last created process %s (%d).", last_pe.szExeFile, last_pe.th32ProcessID);
			TRACEB("Attaching to last created process %s (%d).", last_pe.szExeFile, last_pe.th32ProcessID);
			Attachtoactiveprocess(last_pe.th32ProcessID);
		}
	}
}

void BypassIDP()
{
	// 프로세스가 Attach 되지 않았으면 에러 출력
	if (Getstatus() == STAT_NONE) 
	{
		MessageBox(hwmain, "No process!!", "ERR", MB_OK);
		//return;
	}	

	attach_to_last();

	return;
	
	
	
	TRACEB("IDP: %X", IsDebuggerPresent);

	// 33C0            xor eax, eax
	// C3              retn
	BYTE data[3] = {
		0x33, 0xC0, 0xC3
	};

	

	DWORD dwOldProtect = 0;
	VirtualProtect(IsDebuggerPresent, sizeof(data), PAGE_EXECUTE_READWRITE, &dwOldProtect);

	Writememory(IsDebuggerPresent, 0x90, 3, 0);

	for (int i=0; i<sizeof(data); ++i)
	{
		*((LPBYTE)IsDebuggerPresent + i) = data[i];
		TRACEB("음? %X", *((LPBYTE)IsDebuggerPresent + i));
		
	}
}

void CheckStruct(void *item)
{
	t_dump		*pd;

	pd=(t_dump *)item;
	ulong startC = pd->sel0;   // 첫번째 번지
	ulong endC = pd->sel1; // 마지막 번지 -1 해야함

	TRACEB("item: %X", pd);

	
	//TRACEB("dumptype: %X", pd->dumptype);
	TRACEB("threadid: %X", pd->threadid);
	TRACEB("sel0: %X, sel1: %X", pd->sel0, pd->sel1);
	//TRACEB("filename: %s", pd->filename);
	TRACEB("base: %X", pd->base);
	TRACEB("size: %X", pd->size);
	
	
	TRACEB("------------");



	// FindMemory() API 를 사용하면 구조체에 값이 들어온다.
	t_memory		*pmem;
	pmem=Findmemory(pd->sel0);
	TRACEB("pmem: %X", pmem);
	TRACEB("base: %X", pmem->base);
	TRACEB("size: %X", pmem->size);
	TRACEB("type: %X", pmem->type);
	TRACEB("sect: %s", pmem->sect);

	TRACEB("---------------");

	t_disasm da;
	ulong cmdsize = MAXCMDSIZE;
	unsigned char			cmd[MAXCMDSIZE],*pdecode;
	//ulong			decodesize;

	//pdecode=Finddecode(pd->sel0, &decodesize);
	Readmemory(cmd,pd->sel0,cmdsize,MM_RESTORE|MM_SILENT);
	TRACEB("cmd: %x", cmd);
	Disasm(cmd,cmdsize, pd->sel0, NULL, &da, DISASM_CODE, 0);
	
	TRACEB("da: %x", da);
	TRACEB("da: %s", da.result);
	TRACEB("dump: %s", da.dump);
	TRACEB("comment: %s", da.comment);
	TRACEB("cmdtype: %x", da.cmdtype);
	TRACEB("jmpaddr: %x", da.jmpaddr);	
	
	
	//TRACEB("decodesize: %x", decodesize);
		
	
}

#define MENU_HELLOOLLY_DIALOG		0 // "0 &Hello Olly Dialog,"
#define MENU_IDP						1 // "1 Test|"
#define MENU_ABOUT					2 // "2 About"

extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item) {
	int id;	
	
	switch(origin) 
	{
	case PM_MAIN:
	case PM_DISASM:
		switch (action) 
		{
		case MENU_HELLOOLLY_DIALOG:
			// 프로세스가 Attach 되지 않았으면 에러 출력
			if (Getstatus() == STAT_NONE) 
			{
				MessageBox(hwmain, "No process!!", "ERR", MB_OK);
				return;
			}

			CheckStruct(item);
			
			/*
			id = DialogBox(hinst, MAKEINTRESOURCE(IDD_HELLOOLLY), hwmain, (DLGPROC)MainDlgProc);
			if (id == IDOK) 
			{
				//Doit(hwmain);
			}
			*/
			
			break;
		case MENU_IDP:
			//BypassIDP();
			MessageBox(0,0,0,0);
			CheckStruct(item);
			
			

			
			break;
		case MENU_ABOUT:			
			MessageBox(hwmain, "Hello Olly ver 0.01", "TEST", MB_OK);

			break;
		default:
			break;
		}
	}
}

LRESULT CALLBACK MainDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
   HDC hDC;
   PAINTSTRUCT ps;      	

	RECT  rect;
	UINT  x,y,w,h,xMax,yMax;

	switch (msg) 
	{
		case WM_INITDIALOG:
			// 다이얼로그를 한가운데에 출력
			GetWindowRect(hDlgWnd, &rect);
			h = rect.bottom - rect.top;
			w = rect.right  - rect.left;

			xMax = GetSystemMetrics(SM_CXMAXIMIZED);
			yMax = GetSystemMetrics(SM_CYMAXIMIZED);
			
			x = xMax/2 - w/2;
			y = yMax/2 - h;
			
			MoveWindow(hDlgWnd, x, y, w, h, TRUE);
			break;

		case WM_COMMAND:
			switch (LOWORD(wp)) 
			{
			case IDOK:

				EndDialog(hDlgWnd, IDOK);
				break;

			case IDCANCEL:
				EndDialog(hDlgWnd, IDCANCEL);
				break;

			default:
				break;
			}
		case WM_PAINT:			
			hDC = BeginPaint(hDlgWnd, &ps);
			EndPaint(hDlgWnd, &ps);
			break;
			
		default:
			break;
	}
	
	return 0;
}

extc int _export cdecl ODBG_Pluginclose(void) 
{
	//if(lpMyAlloc) 
	//{
	//	FreeMyAlloc();
	//	lpMyAlloc = NULL;
		Addtolist(0, -1, "== Close Hello Plugin ==");
		log.End();
	//}
	return 0;
}
