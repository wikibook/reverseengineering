#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <dir.h>

#include <odbg\plugin.h>
#include "resource.h"

#define CUP_DELETEALL   0
#define CUP_EXISTONLY   1
#define CUP_DELETEBAK   2
#define CUP_DELETELAST  3
#define CUP_LISTING     10
#define CUP_OPTIONS     30
#define CUP_ABOUT       63

#define DEL_UDD_SUCCESS   1
#define DEL_UDD_CANCELED  0
#define DEL_UDD_FAILED   -1

#define MAXBUFLEN   260

#define PNAME   "CleanupEx"
#define PVERS   "v1.12.108"
#define ANAME   "Gigapede"

HINSTANCE        hinst;                   // DLL instance
HWND             hwmain;                  // Handle of main OllyDbg window
char             sessionlistwinclass[32]; // Name of bookmark window class

int sorted[2]={1,1};

typedef struct t_sessionlist {
  ulong          index;
  ulong          size;                 // Size of index, always 1 in our case
  ulong          type;                 // Type of entry, always 0
  ulong          addr;                 // Address of bookmark
  char           FName[MAX_PATH];
  char           DbgePath[MAX_PATH];
  char           Cmnt[TEXTLEN];
} t_sessionlist;

t_table          sessionlist;          // Session File List table

void CleanupEx(const char *mask, const int existflag);
BOOL DebuggeeExist(const char *);
BOOL IsDbgSessionFile(const char *, char *);
int uddDelete(const char *, const char *);
int GetCurrentSessionFile(char *CurSesFile);
int Sessionlistsortfunc(t_sessionlist *b1,t_sessionlist *b2,int sort);
int SessionListGetText(char *s,char *mask,int *select, t_sortheader *ph,int column);
void CreateSessionListwindow(void);
void CreateSessionList(t_sessionlist *psl);
LRESULT CALLBACK Sessionlistwinproc(HWND hw,UINT msg,WPARAM wp,LPARAM lp);
LRESULT CALLBACK OptDlgProc(HWND, UINT, WPARAM, LPARAM);
void CenterDlgWnd(HWND);

char *dbgename;
char szODPath[MAX_PATH],szODIni[MAX_PATH],szPluginPath[MAX_PATH],szPluginIni[MAX_PATH],szUDDPath[MAX_PATH];
int confirmcheck   = 1; // 1 = TRUE, 0 = FALSE

BOOL WINAPI DllEntryPoint(HINSTANCE hi,DWORD reason,LPVOID reserved) {
  if (reason==DLL_PROCESS_ATTACH)
    hinst=hi;                          // Mark plugin instance
  return 1;                            // Report success
};

extc int _export cdecl ODBG_Plugindata(char shortname[32]) {
  strcpy(shortname,PNAME);       // Name of plugin
  return PLUGIN_VERSION;
};

extc int _export cdecl ODBG_Plugininit(int ollydbgversion,HWND hw,ulong *features) {
  char* pdest;
  char confirmstate[10];
  //char *exename;

  if (ollydbgversion<PLUGIN_VERSION) {
    return -1;
  }
  hwmain=hw;

  //exename = (char*)Plugingetvalue(VAL_EXEFILENAME);
  //MessageBox(hwmain,exename,"DEBUG",MB_OK);

  if(Createsorteddata(&(sessionlist.data),"Session List", sizeof(t_sessionlist),10,(SORTFUNC *)Sessionlistsortfunc,NULL)!=0) {
    return -1;
  }
  if(Registerpluginclass(sessionlistwinclass,NULL,hinst,Sessionlistwinproc)<0) {
    Destroysorteddata(&(sessionlist.data));
    return -1;
  }

  Addtolist(0, 0,PNAME" "PVERS"  by "ANAME);

  GetModuleFileName(NULL, szODPath, MAX_PATH);
  pdest = strrchr(szODPath, '\\');
  pdest[1] = '\0';
  sprintf(szODIni,"%sollydbg.ini",szODPath);
  GetPrivateProfileString("History","Plugin path",szODPath,szPluginPath,sizeof(szPluginPath),szODIni);
  sprintf(szPluginIni,"%s\\%s.ini",szPluginPath,PNAME);
  GetPrivateProfileString("History","Udd path",szODPath,szUDDPath,sizeof(szUDDPath),szODIni);

  GetPrivateProfileString("flag", "confirm", "TRUE", confirmstate, 10, szPluginIni);
  if(strcmp(confirmstate,"FALSE") ==0) {
    confirmcheck = 0;
  }
  return 0;
}

int Sessionlistsortfunc(t_sessionlist *s1,t_sessionlist *s2,int sort)
{
  int i=0;

  switch(sort) {
  case 0:
    i = lstrcmpi(s1->FName,s2->FName);
    break;
  case 1:
    i = lstrcmpi(s1->DbgePath,s2->DbgePath);
    break;
  case 2:
    i = lstrcmpi(s1->Cmnt,s2->Cmnt) * -1;
    break;
  default:
    break;
  }
  return i;
}

extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item) {
  switch (origin) {
    case PM_MAIN: // Plugin menu in main window
      strcpy(data,
             "62 == Delete Debug Session Files ==,"
             "0  &All(*.udd *.bak),"
             "1  &No Debuggee(*.udd *.bak),"
             "2  All &Backup(*.bak),"
             "3  Last Used,"
             "10 &List...|"
             "30 &Option...|"
             "63 About..."
            );
      return 1;
    default:
      break; // Any other window
  };
  return 0; // Window not supported by plugin
};

extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item) {

  char t_or_f[10];
  int id;
  t_sessionlist sl;
  char buf[MAX_PATH],debuggee[MAX_PATH],*p,*pexe;

  switch(origin) {
  case PM_MAIN:
    switch (action) {
    case CUP_DELETEALL:
      CleanupEx("*.udd",CUP_DELETEALL);
      CleanupEx("*.bak",CUP_DELETEALL);
      MessageBox(hwmain, "Finish!!","Done",MB_OK|MB_ICONINFORMATION);
      break;
    case CUP_EXISTONLY:
      CleanupEx("*.udd",CUP_EXISTONLY);
      CleanupEx("*.bak",CUP_EXISTONLY);
      MessageBox(hwmain, "Finish!!","Done",MB_OK|MB_ICONINFORMATION);
      break;
    case CUP_DELETEBAK:
      CleanupEx("*.bak",CUP_DELETEALL);
      MessageBox(hwmain, "Finish!!","Done",MB_OK|MB_ICONINFORMATION);
      break;
    case CUP_DELETELAST:
      pexe = (char*)Plugingetvalue(VAL_EXEFILENAME);
      p = strrchr(pexe,'\\');
      p++;
      pexe = p;
      p = strchr(pexe,'.');
      p++;
      strcpy(p,"udd");
      wsprintf(buf,"%s\\%s",szUDDPath,pexe);
      if(IsDbgSessionFile(buf,debuggee)) {
        uddDelete(buf,debuggee);
      }
      //MessageBox(hwmain,buf,"DEBUG",MB_OK);
      break;
    case CUP_LISTING:
      CreateSessionList(&sl);
      CreateSessionListwindow();
      break;
    case CUP_OPTIONS:
      id = DialogBox(hinst,MAKEINTRESOURCE(IDD_DELSESOPT),hwmain,(DLGPROC)OptDlgProc);
      if(id == IDOK) {
        if(confirmcheck == 1) {
          strcpy(t_or_f,"TRUE");
        }
        else {
          strcpy(t_or_f,"FALSE");
        }
        WritePrivateProfileString("flag","confirm",t_or_f,szPluginIni);
      }
      break;
    case CUP_ABOUT:
      // Menu item "About", displays plugin info.
      MessageBox(hwmain,
                 PNAME" "PVERS
                 "\n    "
                 "by "ANAME"  ",
                 "About "PNAME,
                 MB_OK|MB_ICONINFORMATION);
      break;
    default:
      break;
    }
  }
}

void CreateSessionListwindow(void) {
  // Describe table columns. Note that column names are pointers, so strings
  // must exist as long as table itself.
  if (sessionlist.bar.nbar==0) {
    // Bar still uninitialized.
    sessionlist.bar.name[0]="Session File";   // Name of bookmark
    sessionlist.bar.defdx[0]=30;
    sessionlist.bar.mode[0]=0;
    sessionlist.bar.name[1]="Debuggee Path";    // Bookmark address
    sessionlist.bar.defdx[1]=80;
    sessionlist.bar.mode[1]=0;
    sessionlist.bar.name[2]="Comment";// Disassembled command
    sessionlist.bar.defdx[2]=32;
    sessionlist.bar.mode[2]=0;
    sessionlist.bar.nbar=3;
    sessionlist.mode=TABLE_COPYMENU|TABLE_SORTMENU|TABLE_APPMENU|TABLE_SAVEPOS;
    sessionlist.drawfunc=SessionListGetText;
  }
  // If window already exists, Quicktablewindow() does not create new window,
  // but restores and brings to top existing. This is the simplest way,
  // Newtablewindow() is more flexible but more complicated. I do not recommend
  // custom (plugin-drawn) windows without very important reasons to do this.
  Quicktablewindow(&sessionlist,15,3,sessionlistwinclass,"Session File List");
}

int SessionListGetText(char *s,char *mask,int *select, t_sortheader *ph,int column)
{
  int n;
  t_sessionlist *psl=(t_sessionlist *)ph;
  if (column==0) {
    n = sprintf(s,"%s",psl->FName);
  }
  else if (column==1) {
    n = sprintf(s,"%s",psl->DbgePath);
  }
  else if (column==2) {
    n = sprintf(s,"%s",psl->Cmnt);
  }
  else {
    n = 0;
  }
  return n;
}

// Each window class needs its own window procedure. Both standard and custom
// OllyDbg windows must pass some system and OllyDbg-defined messages to
// Tablefunction(). See description of Tablefunction() for more details.
LRESULT CALLBACK Sessionlistwinproc(HWND hw,UINT msg,WPARAM wp,LPARAM lp)
{
  int i,shiftkey,controlkey;
  HMENU menu;
  t_sessionlist *psl;
  STARTUPINFO si;
  char buf[MAX_PATH];
  //PROCESS_INFORMATION pi;

  si.cb = sizeof(STARTUPINFO);
  switch (msg) {
    // Standard messages. You can process them, but - unless absolutely sure -
    // always pass them to Tablefunction().
    case WM_DESTROY:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_TIMER:
    case WM_SYSKEYDOWN:
      Tablefunction(&sessionlist,hw,msg,wp,lp);
      break;                           // Pass message to DefMDIChildProc()
    // Custom messages responsible for scrolling and selection. User-drawn
    // windows must process them, standard OllyDbg windows without extra
    // functionality pass them to Tablefunction().
    case WM_USER_SCR:
    case WM_USER_VABS:
    case WM_USER_VREL:
    case WM_USER_VBYTE:
    case WM_USER_STS:
    case WM_USER_CNTS:
    case WM_USER_CHGS:
      return Tablefunction(&sessionlist,hw,msg,wp,lp);
    case WM_USER_MENU:
      menu=CreatePopupMenu();
      // Find selected bookmark. Any operations with bookmarks make sense only
      // if at least one bookmark exists and is selected. Note that sorted data
      // has special sort index table which is updated only when necessary.
      // Getsortedbyselection() does this; some other sorted data functions
      // don't and you must call Sortsorteddata(). Read documentation!
      psl=(t_sessionlist *)Getsortedbyselection(&(sessionlist.data),sessionlist.data.selected);
      if (menu!=NULL && psl!=NULL) {
        AppendMenu(menu,MF_STRING,1,"&Delete\tDel");
        AppendMenu(menu,MF_STRING,2,"&Launch Debuggee\tEnter");
      }
      // Even when menu is NULL, call to Tablefunction is still meaningful.
      i=Tablefunction(&sessionlist,hw,WM_USER_MENU,0,(LPARAM)menu);
      if (menu!=NULL) DestroyMenu(menu);
      if (i==1) {
        psl=(t_sessionlist *)Getsortedbyselection(&(sessionlist.data),sessionlist.data.selected);
        sprintf(buf,"%s\\%s",szUDDPath,psl->FName);
        if(uddDelete(buf,psl->DbgePath)==DEL_UDD_SUCCESS) {
          Deletesorteddata(&(sessionlist.data),psl->index);
        }
        // There is no automatical window update, do it yourself.
        InvalidateRect(hw,NULL,FALSE);
      }
      else if(i==2) {
        psl=(t_sessionlist *)Getsortedbyselection(&(sessionlist.data),sessionlist.data.selected);
        if(psl != NULL) {
          /*
          MessageBox(NULL,psl->DbgePath,"DEBUG",MB_OK);
          if(CreateProcess(NULL,psl->DbgePath,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
          }
          */
          WinExec(psl->DbgePath,SW_SHOWNORMAL);
        }
      }
      return 0;
    case WM_KEYDOWN:
      // Processing of WM_KEYDOWN messages is - surprise, surprise - very
      // similar to that of corresponding menu entries.
      shiftkey=GetKeyState(VK_SHIFT) & 0x8000;
      controlkey=GetKeyState(VK_CONTROL) & 0x8000;
      if (wp==VK_RETURN && shiftkey==0 && controlkey==0) {
        // Return key follows bookmark in Disassembler.
        psl=(t_sessionlist *)Getsortedbyselection(&(sessionlist.data),sessionlist.data.selected);
        if (psl!=NULL) {
        /*
          if(CreateProcess(NULL,psl->DbgePath,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
          }
        */
          WinExec(psl->DbgePath,SW_SHOWNORMAL);
        }
      }
      else if (wp==VK_DELETE && shiftkey==0 && controlkey==0) {
        // DEL key deletes bookmark.
        psl=(t_sessionlist *)Getsortedbyselection(&(sessionlist.data),sessionlist.data.selected);
        if (psl!=NULL) {
          sprintf(buf,"%s\\%s",szUDDPath,psl->FName);
          if(uddDelete(buf,psl->DbgePath)==DEL_UDD_SUCCESS) {
            Deletesorteddata(&(sessionlist.data),psl->index);
          }
          InvalidateRect(hw,NULL,FALSE);
        };
      }
      else
        // Add all this arrow, home and pageup functionality.
        Tablefunction(&sessionlist,hw,msg,wp,lp);
      break;
    case WM_USER_DBLCLK:
      // Doubleclicking row follows bookmark in Disassembler.
      psl=(t_sessionlist *)Getsortedbyselection(&(sessionlist.data),sessionlist.data.selected);
      if (psl!=NULL) {
        /*
        MessageBox(NULL,psl->DbgePath,"DEBUG",MB_OK);
        if(CreateProcess(NULL,psl->DbgePath,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) {
          CloseHandle(pi.hThread);
          CloseHandle(pi.hProcess);
        }
        */
        WinExec(psl->DbgePath,SW_SHOWNORMAL);
      }
      return 1;                        // Doubleclick processed
    case WM_USER_CHALL:
    case WM_USER_CHMEM:
      // Something is changed, redraw window.
      InvalidateRect(hw,NULL,FALSE);
      return 0;
    case WM_PAINT:
      // Painting of all OllyDbg windows is done by Painttable(). Make custom
      // drawing only if you have important reasons to do this.
      Painttable(hw,&sessionlist,SessionListGetText);
      return 0;
    default: break;
  };
  return DefMDIChildProc(hw,msg,wp,lp);
};

void CreateSessionList(t_sessionlist *psl)
{

  WIN32_FIND_DATA wfd;
  HANDLE hFind;
  char debuggee[MAX_PATH];
  int idx=0,i;
  const char *mask[] = {"*.udd","*.bak"};
  char *udd_state[] = {"","No Debuggee","Not Debug Session"};
  char FindMask[MAX_PATH],uddbuf[MAX_PATH];

  psl->type = 0;
  psl->size = 1;

  for(i=0; i<2; i++) {
  	sprintf(FindMask,"%s\\%s",szUDDPath,mask[i]);
    hFind = FindFirstFile(FindMask, &wfd);
    if(hFind == INVALID_HANDLE_VALUE) {
      FindClose(hFind);
      return;
    }

    psl->index = idx;
    psl->addr = idx;
    strcpy(psl->FName,wfd.cFileName);
    sprintf(uddbuf,"%s\\%s",szUDDPath,wfd.cFileName);
    if(IsDbgSessionFile(uddbuf,debuggee)) {
      strcpy(psl->DbgePath,debuggee);
      if(!DebuggeeExist(debuggee)) {
        strcpy(psl->Cmnt,udd_state[1]);
      }
      else {
        strcpy(psl->Cmnt,udd_state[0]);
      }
    }
    else {
      strcpy(psl->DbgePath,udd_state[0]);
      strcpy(psl->Cmnt,udd_state[2]);
    }
    Addsorteddata(&(sessionlist.data),psl);
    idx++;

    while(FindNextFile(hFind, &wfd)) {
      psl->index = idx;
      psl->addr = idx;
      strcpy(psl->FName,wfd.cFileName);
      //if(IsDbgSessionFile(wfd.cFileName,debuggee)) {
      sprintf(uddbuf,"%s\\%s",szUDDPath,wfd.cFileName);
      if(IsDbgSessionFile(uddbuf,debuggee)) {
        strcpy(psl->DbgePath,debuggee);
        if(!DebuggeeExist(debuggee)) {
          strcpy(psl->Cmnt,udd_state[1]);
        }
        else {
          strcpy(psl->Cmnt,udd_state[0]);
        }
      }
      else {
        strcpy(psl->DbgePath,udd_state[0]);
        strcpy(psl->Cmnt,udd_state[2]);
      }
      Addsorteddata(&(sessionlist.data),psl);
      idx++;
    }
    FindClose(hFind);
  }
  return;
}

void CleanupEx(const char *mask, const int delflag)
{
  WIN32_FIND_DATA wfd;
  HANDLE hFind;
  char debuggee[MAX_PATH],uddbuf[MAX_PATH],UddMask[MAX_PATH];
  int ret;

  sprintf(UddMask,"%s\\%s",szUDDPath,mask);
  hFind = FindFirstFile(UddMask, &wfd);
  if(hFind == INVALID_HANDLE_VALUE) {
    FindClose(hFind);
    return;
  }
  Infoline("Now processing \"%s\"",wfd.cFileName);
  sprintf(uddbuf,"%s\\%s",szUDDPath,wfd.cFileName);
  if(!IsDbgSessionFile(uddbuf,debuggee)) {
    Message(0,"CleanupEx:Deletion skiped because \"%s\" isn't debug session file",wfd.cFileName);
  }
  else {
    if(delflag == CUP_EXISTONLY) {
      if(!DebuggeeExist(debuggee)) {
        ret = uddDelete(uddbuf,debuggee);
      }
    }
    else {
      ret = uddDelete(uddbuf,debuggee);
    }
    if(ret == DEL_UDD_SUCCESS) {
      Message(0,"CleanupEx:\"%s\" was deleted.",wfd.cFileName);
    }
    else if(ret == DEL_UDD_FAILED) {
      Message(0,"CleanupEx:\"%s\" deletion failed.",wfd.cFileName);
    }
    else if(ret == DEL_UDD_CANCELED) {
      Message(0,"CleanupEx:\"%s\" deletion canceled.",wfd.cFileName);
    }
  }

  while(FindNextFile(hFind, &wfd)) {
    Infoline("Now processing \"%s\"",wfd.cFileName);
    sprintf(uddbuf,"%s\\%s",szUDDPath,wfd.cFileName);
    if(!IsDbgSessionFile(uddbuf,debuggee)) {
      Message(0,"CleanupEx:Deletion skiped because \"%s\" isn't debug session file",wfd.cFileName);
    }
    else {
      if(delflag == CUP_EXISTONLY) {
        if(!DebuggeeExist(debuggee)) {
          ret = uddDelete(uddbuf,debuggee);
        }
      }
      else {
        ret = uddDelete(uddbuf,debuggee);
      }
      if(ret == DEL_UDD_SUCCESS) {
        Message(0,"CleanupEx:\"%s\" was deleted.",wfd.cFileName);
      }
      else if(ret == DEL_UDD_FAILED) {
        Message(0,"CleanupEx:\"%s\" deletion failed.",wfd.cFileName);
      }
      else if(ret == DEL_UDD_CANCELED) {
        Message(0,"CleanupEx:\"%s\" deletion canceled.",wfd.cFileName);
      }
    }
  }

  FindClose(hFind);
  Infoline("Cleanup Finish!!");
  return;
}

BOOL IsDbgSessionFile(const char *uddFileName, char *debuggee)
{
  const char *Header = "Mod";
  char cFile[0x100];
  HANDLE hFile;
  DWORD result;
  char buf[255];

  hFile = CreateFile(uddFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) {
    sprintf(buf,"%s File Not Found!!",uddFileName);
    MessageBox(hwmain, buf,"Error",MB_OK);
    return FALSE;
  }
  else {
    if(!ReadFile(hFile, cFile, 0x100, &result, NULL)) {
      MessageBox(hwmain, "File Read Error","Error",MB_OK);
      CloseHandle(hFile);
      return FALSE;
    }
  }
  CloseHandle(hFile);
  if(!strncmp(cFile,Header,3)) {
    if(debuggee) {
      strcpy(debuggee,cFile+0x3E);
    }
    return TRUE;
  }
  else {
    return FALSE;
  }
}

BOOL DebuggeeExist(const char *Path)
{
  WIN32_FIND_DATA wfd;
  HANDLE hFind;

  hFind = FindFirstFile(Path, &wfd);
  if(hFind == INVALID_HANDLE_VALUE) {
    return FALSE;
  }
  return TRUE;
}

int uddDelete(const char *uddFile, const char *debuggee)
{
  char buf[256];
  int ret;

  if(confirmcheck == 1) {
    sprintf(buf,"Do you wanna delete %s ?\n\n Debuggee:\n  \"%s\"",uddFile,debuggee);
    if(MessageBox(hwmain,buf, "Wanna delete file?",  MB_YESNO|MB_ICONQUESTION) == IDYES) {
      if(DeleteFile(uddFile) == NULL){
        MessageBox(hwmain, uddFile, "Fail to delete file:", MB_OK);
        ret = DEL_UDD_FAILED;
      }
      else {
        ret = DEL_UDD_SUCCESS;
      }
    }
    else {
      ret = DEL_UDD_CANCELED;
    }
  }
  else {
    if(DeleteFile(uddFile)==NULL) {
      MessageBox(hwmain,uddFile,"Fail to delete file:", MB_OK);
      ret = DEL_UDD_FAILED;
    }
    else {
      ret = DEL_UDD_SUCCESS;
    }
  }
  return ret;
}

int GetCurrentSessionFile(char *CurSesFile)
{
  char *exename,*pos;
  char buf[MAX_PATH];

  exename = (char*)Plugingetvalue(VAL_EXEFILENAME);
  if(exename == NULL) {
    return 0;
  }
  strcpy(buf,exename);
  pos = strrchr(buf,'.');
  *(pos+1) = '\0';
  strcat(buf,"udd");
  pos = strrchr(buf,'\\');
  pos++;
  strcpy(CurSesFile,szUDDPath);
  strcat(CurSesFile,pos);
  return 1;
}

LRESULT CALLBACK OptDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

  switch (msg) {
  case WM_INITDIALOG:
    CenterDlgWnd(hDlgWnd);
    SendMessage(GetDlgItem(hDlgWnd, IDC_CONFIRM), BM_SETCHECK, (WPARAM)confirmcheck, 0L);
    return TRUE;

  case WM_COMMAND:
    switch (LOWORD(wp)) {
    case IDOK:
      if(IsDlgButtonChecked(hDlgWnd, IDC_CONFIRM) == BST_CHECKED) {
        confirmcheck = 1;
      }
      else {
        confirmcheck = 0;
      }
      EndDialog(hDlgWnd, IDOK);
      break;

    case IDCANCEL:
      EndDialog(hDlgWnd, IDCANCEL);
      break;

    default:
      return FALSE;
    }
  default:
    return FALSE;
  }
  return TRUE;
}

void CenterDlgWnd(HWND hDlgWnd)
{
  RECT  rect;
  UINT x,y,w,h,bMax,rMax;

  GetWindowRect(hDlgWnd,&rect);
  h = rect.bottom - rect.top;
  w = rect.right  - rect.left;
  rMax = GetSystemMetrics(SM_CXMAXIMIZED);
  bMax = GetSystemMetrics(SM_CYMAXIMIZED);
  y = (bMax - h) / 2;
  x = (rMax - w) / 2;
  MoveWindow(hDlgWnd,x,y,w,h,TRUE);
}
