//=============================================================================
//    File : OllyMemScan.cpp
//     Use : OllyDBG plugin. Advanced Scan, Memory Compare
//  Editor : ByungTak Kang (window31@empal.com)
//=============================================================================

#include "windows.h"
#include "plugin.h"
#include "OllyMemScan.h"
#include "Log.h"

#include "resource.h"

#include <tlhelp32.h>
#include <Commctrl.h>

#pragma comment(lib, "OLLYDBG.LIB")



HWND             hwmain;               // Handle of main OllyDbg window
HINSTANCE        hinst;                // DLL instance


CLog log;
HWND g_hProgress = NULL;





#define		__REVISION				1






#define MYLOG			log.WriteLog

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call==DLL_PROCESS_ATTACH)
		hinst=(HINSTANCE)hModule;       // Mark plugin instance

	return TRUE;
}

extc int _export cdecl ODBG_Plugindata(char shortname[32]) 
{
	_tcscpy(shortname, _T("OllyMemScan"));       // Name of plugin
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
	Addtolist(0, 0, _T("OllyMemScan plugin v0.01"));
	Addtolist(0, -1, _T("made by window31"));	

	log.Start(__REVISION, _T("OllyMemScan"));


	return 0;
}

extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item) 
{
	switch (origin) 
	{
	case PM_MAIN: // Plugin menu in main window
		_tcscpy(data,
			_T("0 &OllyMemScan|")
			_T("1 About")
			);
		return 1;
	
	case PM_DISASM:
		if (Getstatus() == STAT_NONE) 
		{
			return 0;
		}
		_tcscpy(data, _T("0 &OllyMemScan"));
		return 1;
	
	case PM_THREADS:
		if (Getstatus() == STAT_NONE) 
		{
			return 0;
		}
		_tcscpy(data, _T("0 &OllyMemScan"));
		return 1;
	
	default:
		break; // Any other window
	};
	return 0; // Window not supported by plugin
};

#define MENU_OLLYMEMSCAN_DIALOG		0 // "0 &OllyMemScan,"
#define MENU_TEST					1 // "1 Test|"
#define MENU_ABOUT					2 // "2 About"

extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item) 
{
	switch(origin) 
	{
	case PM_MAIN:
	case PM_DISASM:
		switch (action) 
		{
		case MENU_OLLYMEMSCAN_DIALOG:
			// 프로세스가 Attach 되지 않았으면 에러 출력
			if (Getstatus() == STAT_NONE) 
			{
				MessageBox(hwmain, _T("No process!!"), _T("ERR"), MB_OK);
				return;
			}
			
			DialogBox(hinst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), hwmain, (DLGPROC)MainDlgProc);	
			break;
			
			break;
		case MENU_ABOUT:			
			MessageBox(hwmain, "OllyMemScan ver 0.01", "TEST", MB_OK);

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

	static HWND hDllName = NULL;
	static HWND hSelectAllDll = NULL;
	static HWND hSearchXOR = NULL;
	static HWND hSelectDll = NULL;
	static HWND hRunScan = NULL;
	static HWND hShowText = NULL;

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

			hDllName = GetDlgItem(hDlgWnd, IDC_EDIT_DLL);
			hSelectAllDll = GetDlgItem(hDlgWnd, IDC_CHECK_ALLMODULE);
			hSearchXOR = GetDlgItem(hDlgWnd, IDC_BUTTON_SEARCHXOR);
			hSelectDll = GetDlgItem(hDlgWnd, IDC_EDIT_DLL);
			hRunScan = GetDlgItem(hDlgWnd, IDC_BUTTON_CMP);
			g_hProgress = GetDlgItem(hDlgWnd, IDC_PROGRESS_SCAN);	
			hShowText = GetDlgItem(hDlgWnd, IDC_STATIC_TEXT);
			break;

		case WM_COMMAND:
			switch (LOWORD(wp)) 
			{
			case IDC_BUTTON_CMP:
				{
					EnableWindow(hRunScan, FALSE);
					EnableWindow(hSearchXOR, FALSE);

					// OllyAPI
					DWORD dwDebugeePid = Plugingetvalue(VAL_PROCESSID);

					// scan all dll
					if (SendMessage(hSelectAllDll, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						DWORD dwModuleCount = GetModuleCount(dwDebugeePid);
						PostMessage(g_hProgress, PBM_SETRANGE, 0, MAKELONG(0, dwModuleCount));

						DWORD dwThreadId = 0;
						PTHREAD_PARAM tr;
						tr = (PTHREAD_PARAM)malloc(sizeof(THREAD_PARAM));
						tr->dwPid = dwDebugeePid;
						tr->dwScanOption = DC_MEMCMP;
						tr->hDlg = hDlgWnd;
						tr->hShowText = hShowText;
						tr->dwModuleCount = dwModuleCount;
						HANDLE hThread = CreateThread(NULL, 0, ScanAllDllThread, (LPVOID)tr, 0, &dwThreadId);
					}
					else
					{
						PostMessage(g_hProgress, PBM_SETRANGE, 0, MAKELONG(0, 100));

						TCHAR szDllName[MAX_PATH] = {0,};
						GetWindowText(hDllName, szDllName, MAX_PATH);

						// for progressbar :)
						_tcslwr(szDllName);
						DWORD dwBaseAdr = GetDllAddressFromPid(dwDebugeePid, szDllName);
						MYLOG(_T("[%s] pid: %d, Base: %X"), szDllName, dwDebugeePid, dwBaseAdr);

						if (dwBaseAdr == 0)
							MessageBox(hDlgWnd, "module not found", "err", MB_OK);
						else
						{
							// OllyAPI
							t_memory *pmem;
							pmem = Findmemory(dwBaseAdr);

							DoScan(dwDebugeePid, (LPBYTE)dwBaseAdr, (LPBYTE)pmem->size, DC_MEMCMP);

							// 하나의 모듈은 금방 검색완료하므로 이런 꽁수 사용 :p
							PostMessage(g_hProgress, PBM_SETPOS, 100, 0);

							MessageBox(hDlgWnd, _T("Done. Check OllyMemScan.log !"), _T("scanning..."), MB_OK);
						}
					}

					EnableWindow(hRunScan, TRUE);	
					EnableWindow(hSearchXOR, TRUE);
					break;
				}

			case IDC_BUTTON_SEARCHXOR:
				{
					EnableWindow(hRunScan, FALSE);
					EnableWindow(hSearchXOR, FALSE);

					// OllyAPI
					DWORD dwDebugeePid = Plugingetvalue(VAL_PROCESSID);

					// scan all dll
					if (SendMessage(hSelectAllDll, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						DWORD dwModuleCount = GetModuleCount(dwDebugeePid);
						
						DWORD dwThreadId = 0;
						PTHREAD_PARAM tr;
						tr = (PTHREAD_PARAM)malloc(sizeof(THREAD_PARAM));
						tr->dwPid = dwDebugeePid;
						tr->dwScanOption = DC_SEARCHXOR;
						tr->hDlg = hDlgWnd;
						tr->hShowText = hShowText;
						tr->dwModuleCount = dwModuleCount;
						HANDLE hThread = CreateThread(NULL, 0, ScanAllDllThread, (LPVOID)tr, 0, &dwThreadId);
					}
					else
					{
						TCHAR szDllName[MAX_PATH] = {0,};
						GetWindowText(hDllName, szDllName, MAX_PATH);

						// for progressbar :)	
						_tcslwr(szDllName);
						DWORD dwBaseAdr = GetDllAddressFromPid(dwDebugeePid, szDllName);
						MYLOG(_T("[%s] pid: %d, Base: %X"), szDllName, dwDebugeePid, dwBaseAdr);

						if (dwBaseAdr == 0)
							MessageBox(hDlgWnd, "module not found", "err", MB_OK);
						else
						{
							// OllyAPI
							t_memory *pmem;
							pmem = Findmemory(dwBaseAdr);

							DoScan(dwDebugeePid, (LPBYTE)dwBaseAdr, (LPBYTE)pmem->size, DC_SEARCHXOR);

							MessageBox(hDlgWnd, _T("Done. Check OllyMemScan.log !"), _T("scanning..."), MB_OK);
						}
					}

					EnableWindow(hRunScan, TRUE);	
					EnableWindow(hSearchXOR, TRUE);
					break;
				}

			case IDC_CHECK_ALLMODULE:
				if (SendMessage(hSelectAllDll, BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					EnableWindow(hSelectDll, FALSE);
				}
				else
				{
					EnableWindow(hSelectDll, TRUE);
				}
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
	// OllyAPI
	Addtolist(0, -1, _T("== Close OllyMemScan Plugin =="));
	log.End();
	return 0;
}

DWORD GetModuleCount(DWORD dwPid)
{
	DWORD count = 0;

	MODULEENTRY32 me32       = {0,};
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
	
	if (hModuleSnap == INVALID_HANDLE_VALUE) 
		return 0;
	
	me32.dwSize = sizeof(MODULEENTRY32);
	
	if (Module32First(hModuleSnap, &me32)) 
	{ 
		do 
		{
			count++;
		} 
		while (Module32Next(hModuleSnap, &me32)); 
	}
	CloseHandle (hModuleSnap);


	return count;
}

DWORD GetDllAddressFromPid(DWORD dwPid, LPCTSTR szDllName)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return false; 
	
	pe32.dwSize = sizeof(PROCESSENTRY32); 
	DWORD dwDllAddress = 0;

	bool bDetect = false;

	if (Process32First(hProcessSnap, &pe32)) 
	{
		do 
		{
			if (bDetect)
				break;

			MODULEENTRY32 me32       = {0,};
			HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
			
			if (hModuleSnap == INVALID_HANDLE_VALUE) 
				continue;
			
			me32.dwSize = sizeof(MODULEENTRY32);
			
			if (Module32First(hModuleSnap, &me32)) 
			{ 
				do 
				{
					_tcslwr(me32.szExePath);
					if (_tcsstr(me32.szExePath, szDllName))
					{
						dwDllAddress = (DWORD)me32.modBaseAddr;
						bDetect = true;
						break;
					}
				} 
				while (Module32Next(hModuleSnap, &me32)); 
			}
			CloseHandle (hModuleSnap);
		} 
		while (Process32Next(hProcessSnap, &pe32)); 
	}
	CloseHandle(hProcessSnap);

	return dwDllAddress;
}

bool DoScan(DWORD dwPid, LPBYTE lpLoadedAddress, LPBYTE lpBaseAdr, DWORD dwScanOption)
{
   LPVOID lpAddr = 0;
   bool bRet = false;
	
   __try
   {      
		bRet = DoScanInside(dwPid, lpLoadedAddress, lpBaseAdr, dwScanOption);
   }
   __except (EXCEPTION_EXECUTE_HANDLER, lpAddr = ((LPEXCEPTION_POINTERS)GetExceptionInformation())->ExceptionRecord->ExceptionAddress)
   {
		MYLOG(_T("-.- DoCompareInside Exception, code: %08x, addr: %08x"), GetExceptionCode(), lpAddr);
   }
	
   return bRet;	
}

// Names of the data directory elements that are defined
char *ImageDirectoryNames[] = {
    "EXPORT", "IMPORT", "RESOURCE", "EXCEPTION", "SECURITY", "BASERELOC",
    "DEBUG", "COPYRIGHT", "GLOBALPTR", "TLS", "LOAD_CONFIG",
    "BOUND_IMPORT", "IAT",  // These two entries added for NT 3.51
	"DELAY_IMPORT" };		// This entry added in NT 5

#define NUMBER_IMAGE_DIRECTORY_ENTRYS \
    (sizeof(ImageDirectoryNames)/sizeof(char *))

bool DoScanInside(DWORD dwPid, LPBYTE lpLoadedAddress, LPBYTE lpBaseAdr, DWORD dwScanOption)
{
	if (dwPid == 0)
		return false;
	
	bool bRet = false;
	DWORD dwTotal = 0;
	DWORD dwDiffCount = 0;
	
	// OllyAPI
	t_module *tmod = Findmodule((DWORD)lpLoadedAddress);
	TCHAR szFullPath[MAX_PATH];
	_tcscpy(szFullPath, tmod->path);
	_tcslwr(szFullPath);

	DWORD count = 0;

	// native Process, Strange path @@@@@ lol
	// \SystemRoot\System32\smss.exe
	// \??\C:\WINDOWS\system32\csrss.exe
	// \??\C:\WINDOWS\system32\winlogon.exe
	_tcslwr(szFullPath);
	if (_tcsstr(szFullPath, _T("system32\\smss.exe")))
	{
		TCHAR szTemp[MAX_PATH];
		GetSystemDirectory(szTemp, MAX_PATH);
		_tcscat(szTemp, _T("\\smss.exe"));
		memset(szFullPath, 0, MAX_PATH);
		_tcscpy(szFullPath, szTemp);
	}
	else if (_tcsstr(szFullPath, _T("system32\\csrss.exe")))
	{
		TCHAR szTemp[MAX_PATH];
		GetSystemDirectory(szTemp, MAX_PATH);
		_tcscat(szTemp, _T("\\csrss.exe"));
		memset(szFullPath, 0, MAX_PATH);
		_tcscpy(szFullPath, szTemp);
	}
	else if (_tcsstr(szFullPath, _T("system32\\winlogon.exe")))
	{
		TCHAR szTemp[MAX_PATH];
		GetSystemDirectory(szTemp, MAX_PATH);
		_tcscat(szTemp, _T("\\winlogon.exe"));
		memset(szFullPath, 0, MAX_PATH);
		_tcscpy(szFullPath, szTemp);
	}

	// Header 정보도 Olly API 로 얻을 수 있지만, Binary Compare 를 위해 전체 버퍼가 필요하기 때문에
	// CreateFile() - ReadFile() Win32 API 를 사용했다.
	HANDLE hTargetFile = CreateFile(szFullPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	DWORD dwTempSize = GetFileSize(hTargetFile, NULL); 
	PBYTE lpFileBuffer = NULL;
	lpFileBuffer = (PBYTE)malloc(dwTempSize);
	memset(lpFileBuffer, 0, dwTempSize);
	DWORD read = 0;
	ReadFile(hTargetFile, lpFileBuffer, dwTempSize, &read, NULL);
	SetFilePointer(hTargetFile, NULL, NULL, FILE_BEGIN);

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpFileBuffer;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS) ((long)pDosHeader + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER section;
	section = (PIMAGE_SECTION_HEADER)(pNTHeader+1);
	
#ifdef __USE_DETAIL_LOG
	MYLOG(_T("section: %s, VirtualAdr: 0x%X, VirtualSize: 0x%X"), (char *)section->Name, section->VirtualAddress, section->Misc.VirtualSize);
#endif
	DWORD dwVirtualAdr = section->VirtualAddress;
	DWORD dwCodeSize = section->Misc.VirtualSize;	
	DWORD dwSizeOfHeaders = pNTHeader->OptionalHeader.SizeOfHeaders;

	DWORD dwIATRVA = 0;
	DWORD dwIATSize = 0;

	for ( DWORD j=0; j < pNTHeader->OptionalHeader.NumberOfRvaAndSizes; j++)
	{
		if (strstr(ImageDirectoryNames[j], "IAT"))
		{
			dwIATRVA = pNTHeader->OptionalHeader.DataDirectory[j].VirtualAddress;
			dwIATSize = pNTHeader->OptionalHeader.DataDirectory[j].Size;
			break;
		}
	}
	
	DWORD dwFileStartAddr = 0;
	// system dll 다시 !!!!, IAT가 BaseOfCode 랑 같으면 .text에  IAT 가 겹쳐져 있는 system dll 임
	// 따라서 IAT 만큼의 사이즈만큼 빼고, 거기서부터 비교 계산임.
	if (dwVirtualAdr == dwIATRVA)
	{
		MYLOG(_T("[pid: %d] [0x%X] %s, (system dll !)"), dwPid, (DWORD)lpLoadedAddress, szFullPath);
		dwFileStartAddr = dwSizeOfHeaders + dwIATSize;
		lpLoadedAddress = (LPBYTE)((DWORD)lpLoadedAddress + dwVirtualAdr + dwIATSize);
		dwCodeSize -= dwIATSize;		
	}
	// 아닌 경우는 그냥 계산
	else
	{
		MYLOG(_T("[pid: %d] [0x%X] %s"), dwPid, (DWORD)lpLoadedAddress, szFullPath);
		dwFileStartAddr = dwSizeOfHeaders;
		lpLoadedAddress = (LPBYTE)((DWORD)lpLoadedAddress + dwVirtualAdr);			
	}

#ifdef __USE_DETAIL_LOG
	MYLOG(_T("dwIATRVA: 0x%X, dwIATSize: 0x%X"), dwIATRVA, dwIATSize);
	MYLOG(_T("dwCodeSize: 0x%X, dwSizeOfHeaders: 0x%X"), dwCodeSize, dwSizeOfHeaders);
#endif

	// IAT 가 0인 경우는 EXE 패킹으로 간주하고 스킵 
	// 그러나 ntdll.dll 은 논 패킹엠에도 불구하고 
	// IAT 가 존재하지 않으므로 화이트 리스트 처리
	if (IS_SET(DC_MEMCMP, dwScanOption) && _tcsstr(szFullPath, _T("ntdll.dll")) == 0)
	{
		if (dwIATRVA == 0 || dwIATSize == 0)
		{
			MYLOG(_T("Packing, skip"));
			free(lpFileBuffer);
			return false;
		}
	}

#ifdef __USE_DETAIL_LOG
	MYLOG(_T("compare size: 0x%X"), dwCodeSize);
	MYLOG(_T("Start Memory: %X"), lpLoadedAddress);
	MYLOG(_T("Start File : %X"), dwFileStartAddr);
#endif

	PBYTE lpMemBuffer = NULL;		
	lpMemBuffer = (PBYTE)malloc(dwCodeSize);
	memset(lpMemBuffer, 0, dwCodeSize);

	// OllyAPI
	Readmemory(lpMemBuffer, (DWORD)lpLoadedAddress, dwCodeSize, MM_RESTORE | MM_SILENT);

	// Search XOR
	if (IS_SET(DC_SEARCHXOR, dwScanOption))
	{		
		PostMessage(g_hProgress, PBM_SETRANGE, 0, MAKELONG(0, dwCodeSize));

		for (DWORD t=0; t<dwCodeSize; t++)
		{
			PostMessage(g_hProgress, PBM_SETPOS, t, 0);

			BYTE cmd[MAXCMDSIZE];
			// OllyAPI
			Readmemory(cmd, (DWORD)lpLoadedAddress + t, MAXCMDSIZE, MM_RESTORE | MM_SILENT);
			
			// OllyAPI
			t_disasm da;
			Disasm(cmd, MAXCMDSIZE, (DWORD)lpLoadedAddress + t, NULL, &da, DISASM_CODE, 0);


			// "30", "31", "32", "34", "35"
			if (da.dump[0] == 0x33 && da.dump[1] == 0x30 ||
				da.dump[0] == 0x33 && da.dump[1] == 0x31 ||
				da.dump[0] == 0x33 && da.dump[1] == 0x32 ||
				da.dump[0] == 0x33 && da.dump[1] == 0x34 ||
				da.dump[0] == 0x33 && da.dump[1] == 0x35
				)
			{
				// White List
				if (!IsWhiteListXOR(*((LPBYTE)lpMemBuffer + t), *((LPBYTE)lpMemBuffer + t + 1)))
				{
					MYLOG(_T("Xor - [0x%X] %s   %s"), 
						lpLoadedAddress + t,         // 1
						da.result, 
						da.comment);  // 4
				}
			}

			DWORD dwLen = _tcslen(da.dump);
			DWORD dwMinusLen = 0;
			for (DWORD a=0; a<dwLen; a++)
			{
				// : 와 공백 만큼의 Len 제거 꽁수임. ex) "68 4CE8342A" , "49:E4 5874EA" etc
				if (da.dump[a] == 0x20 || da.dump[a] == 0x3A)
					dwMinusLen++;				
			}

			dwLen -= dwMinusLen;

			DWORD dwSkipByte = (dwLen / 2) - 1;
			t += dwSkipByte;

		}
	}

	// Hex Compare
	if (IS_SET(DC_MEMCMP, dwScanOption))
	{		
		for (DWORD i=0; i<dwCodeSize; i++)
		{		
			if (*((LPBYTE)lpFileBuffer + dwFileStartAddr + i) != *((LPBYTE)lpMemBuffer + i)) 
			{
				// 00 00 인데도 Diff 로 나오는 경우가 있어서 한번 더 체크
				if (*((LPBYTE)lpFileBuffer + dwFileStartAddr + i) != 0 && *((LPBYTE)lpMemBuffer + i) != 0) 
				{
					// 너무 많이 출력되면 적당히 하고 끝낸다.
					if (dwTotal > 50)
					{
						MYLOG(_T("too many diff !! compare stop."));
						break;
					}

					BYTE cmd[MAXCMDSIZE];
					// OllyAPI
					Readmemory(cmd, (DWORD)lpLoadedAddress + i, MAXCMDSIZE, MM_RESTORE | MM_SILENT);
					
					// OllyAPI
					t_disasm da;
					Disasm(cmd, MAXCMDSIZE, (DWORD)lpLoadedAddress + i, NULL, &da, DISASM_CODE, 0);

					MYLOG(_T("Diff - [0x%X] Mem: %02X : [0x%X] File: %02X - %s   %s"), 
						lpLoadedAddress + i,         // 1
						*((LPBYTE)lpMemBuffer + i),  // 2
						dwFileStartAddr + i,         // 3
						*((LPBYTE)lpFileBuffer + dwFileStartAddr + i),
						da.result, 
						da.comment);  // 4
					
					DWORD dwSkipByte = _tcslen(da.dump) / 2 - 1;
					i += dwSkipByte;
					dwTotal += dwSkipByte;
					dwTotal++;
	#ifdef __USE_DETAIL_LOG
					MYLOG(_T("dump: %s, [%d]"), da.dump, _tcslen(da.dump) / 2);
	#endif
					
				}
			}

			count ++;
		}

		if (dwCodeSize == count)
			bRet = true;

		MYLOG(_T("Complete: %d byte diff"), dwTotal);
	}
	
	free(lpFileBuffer);
	free(lpMemBuffer);

	return bRet;
}

DWORD WINAPI ScanAllDllThread(LPVOID pParam)
{
	bool bRet = true;
	PTHREAD_PARAM tr = (PTHREAD_PARAM)pParam;	
	DWORD dwProgressBarCnt = 1;

	MODULEENTRY32 me32       = {0,};
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, tr->dwPid);
	
	if (hModuleSnap == INVALID_HANDLE_VALUE) 
		return 0;
	
	me32.dwSize = sizeof(MODULEENTRY32);
	
	if (Module32First(hModuleSnap, &me32)) 
	{ 
		do 
		{
			// OllyAPI
			t_memory *pmem;
			pmem = Findmemory((DWORD)me32.modBaseAddr);
			
			TCHAR szShowText[MAX_PATH] = {0,};
			wsprintf(szShowText, _T("[%d/%d] %s"), dwProgressBarCnt, tr->dwModuleCount, me32.szExePath);
			SetWindowText(tr->hShowText, szShowText);

			dwProgressBarCnt++;
			if (IS_SET(DC_MEMCMP, tr->dwScanOption))
			{
				PostMessage(g_hProgress, PBM_SETPOS, dwProgressBarCnt, 0);				
			}
			
			DoScan(tr->dwPid, (LPBYTE)me32.modBaseAddr, (LPBYTE)pmem->size, tr->dwScanOption);
		} 
		while (Module32Next(hModuleSnap, &me32)); 
	}	
	CloseHandle (hModuleSnap);	

	MessageBox(tr->hDlg, _T("Done. Check OllyMemScan.log !"), _T("scanning..."), MB_OK);
	free(tr);

	return 0;
}

// 0x33 xor XXX XXX <- same operand
// 32C0 - xor al, al
// 30E4 - xor ah, ah
// 32DB - xor bl, bl
// 30FF - xor bh, bh 
// 32C9 - xor cl, cl
// 30ED - xor ch, ch
// 32D2 - xor dl, dl
// 32F6 - xor dh, dh 
//
// 31C0 - xor eax, eax
// 31DB - xor ebx, ebx
// 33C9 - xor ecx, ecx
// 31D2 - xor edx, edx

bool IsWhiteListXOR(DWORD dwOpr1, DWORD dwOpr2)
{
	bool bWhiteXOR = false;

	if (dwOpr1 == 0x32 && dwOpr2 == 0xC0)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x30 && dwOpr2 == 0xE4)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x32 && dwOpr2 == 0xDB)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x30 && dwOpr2 == 0xFF)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x32 && dwOpr2 == 0xC9)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x30 && dwOpr2 == 0xED)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x32 && dwOpr2 == 0xD2)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x32 && dwOpr2 == 0xF6)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x31 && dwOpr2 == 0xC0)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x31 && dwOpr2 == 0xDB)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x33 && dwOpr2 == 0xC9)
		bWhiteXOR = true;
	else if (dwOpr1 == 0x31 && dwOpr2 == 0xD2)
		bWhiteXOR = true;

	return bWhiteXOR;
}