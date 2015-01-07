

#ifndef __HELLOOLLY_H__
#define __HELLOOLLY_H__


extc int _export cdecl ODBG_Plugindata(char shortname[32]);
extc int _export cdecl ODBG_Plugininit(int ollydbgversion,HWND hw,ulong *features); 
extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item);
extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item);
extc int _export cdecl ODBG_Pluginclose(void);


LRESULT CALLBACK MainDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);


#endif // __HELLOOLLY_H__
