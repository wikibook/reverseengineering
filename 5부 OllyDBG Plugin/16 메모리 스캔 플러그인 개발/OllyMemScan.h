//=============================================================================
//    File : OllyMemScan.h
//     Use : OllyDBG plugin. Advanced Scan, Memory Compare
//  Editor : ByungTak Kang (window31@empal.com)
//=============================================================================


#ifndef __OLLYMEMSCAN_H__
#define __OLLYMEMSCAN_H__


// export func
extc int _export cdecl ODBG_Plugindata(char shortname[32]);
extc int _export cdecl ODBG_Plugininit(int ollydbgversion,HWND hw,ulong *features); 
extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item);
extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item);
extc int _export cdecl ODBG_Pluginclose(void);

LRESULT CALLBACK MainDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
DWORD GetModuleCount(DWORD dwPid);
DWORD GetDllAddressFromPid(DWORD dwPid, LPCTSTR szDllName);
bool DoScan(DWORD dwPid, LPBYTE lpLoadedAddress, LPBYTE lpBaseAdr, DWORD dwScanOption);
bool DoScanInside(DWORD dwPid, LPBYTE lpLoadedAddress, LPBYTE lpBaseAdr, DWORD dwScanOption);
DWORD WINAPI ScanAllDllThread(LPVOID pParam);
bool IsWhiteListXOR(DWORD dwOpr1, DWORD dwOpr2);

#define DC_MEMCMP				0x0001
#define DC_SEARCHXOR			0x0002
#define DC_SEARCHXOR_FORCED		0x0004

#define IS_SET(flag, bit)        ((flag) & (bit))

typedef struct _THREAD_PARAM
{
	DWORD dwPid;
	DWORD dwScanOption;
	HWND hDlg;
	HWND hShowText;
	DWORD dwModuleCount;
} THREAD_PARAM, *PTHREAD_PARAM;


#endif // __OLLYMEMSCAN_H__
