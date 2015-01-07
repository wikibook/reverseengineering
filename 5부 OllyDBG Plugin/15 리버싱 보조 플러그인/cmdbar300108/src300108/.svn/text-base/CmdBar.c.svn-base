////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                      COMMAND LINE PLUGIN FOR OLLYDBG                       //
//                                                                            //
//                  (Plugin window and interface to OllyDbg)                  //
//                                                                            //
//                                                                            //
//                     Originally written by Oleh Yuschuk                     //
//                                                                            //
//                               Contributors:                                //
//                                    TBD                                     //
//                                                                            //
//                                                                            //
// This code is distributed "as is", without warranty of any kind, expressed  //
// or implied, including, but not limited to warranty of fitness for any      //
// particular purpose. In no event will Oleh Yuschuk be liable to you for any //
// special, incidental, indirect, consequential or any other damages caused   //
// by the use, misuse, or the inability to use of this code, including any    //
// lost profits or lost savings, even if Oleh Yuschuk has been advised of the //
// possibility of such damages. Or, translated into English: use at your own  //
// risk!                                                                      //
//                                                                            //
// This code is free. You can modify this code, include parts of it into your //
// own programs and redistribute modified code.                               //
//                                                                            //
// Any contributions are welcome. Please send them to Ollydbg@t-online.de     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// VERY IMPORTANT NOTICE: COMPILE THIS DLL WITH BYTE ALIGNMENT OF STRUCTURES
// AND UNSIGNED CHAR! THIS IS OLLYDBG'S DEFAULT!

// ToDo list:
//   - tips displayed each time the window is created;
//   - any ideas, please?

#define STRICT                         // Avoids some type mismatches
#include <windows.h>
#include <string.h>
#include <stdio.h>
//#include <dir.h>
#include <malloc.h>
#include "Plugin.h"
#include "cmdexec.h"
#include "cmdlist.h"

#define PNAME   "CommandBar"
#define PVERS   "3.00.108"
#define ANAME   "Gigapede"
#define CNAME   "TBD\n  Wayne\n  psyCK0\n  mfn"
#define CNAM2   "TBD Wayne psyCK0 mfn"

#define TOOLBAR_HIGHT   23
#define STATUSBAR_HIGHT 20
#define CMDBAR_HIGHT    25

#define CMDBAR_SHOW      1
#define CMDBAR_HIDE      0

#define TAG_CMDBAR     0x6C6D430AL     // Command line record type in .udd file

#define ID_HWBOX       1001            // Identifier of hwbox
#define ID_HWERR       1002            // Identifier of hwerr
#define ID_HWSTC       1003            // Identifier of hwerr

#define NHIST          32              // Number of commands in the history

HINSTANCE hinst;  // DLL instance
HWND      hwmain; // Handle of main OllyDbg window
HWND      hwcmd;  // Command line window
HWND      hwbox;  // Combobox with edit and history list
HWND      hwedt;  // Edit control inside the combobox
HWND      hwstc;  // Static control
HWND      hwerr;  // Error message
HWND      hwMDI;
HFONT     hFont;

int NumberOfCommand = 0;
BOOL bMacExec = FALSE;
COMMAND_QUE cmdquehead;
char *MacroNames = NULL;

t_command *pcmdlist;

static WNDPROC   oldhweditproc;  // Original window procedure of hwedit
static WNDPROC   oldOllyWndProc; // Original window procedure of hwedit

static CHOOSEFONT cf;
static LOGFONT    lf;

static char      CmdbarWinClass[32];   // Name of command line window class

static char      hist[NHIST][TEXTLEN]; // Previously entered commands
static int       nhist;                // Number of commands in history
//static int       poponstop;            // Foreground on pause
static int       iShowCmdbar;

char szODPath[MAX_PATH],szODIni[MAX_PATH],szIni[MAX_PATH],szPluginPath[MAX_PATH],szMacDefFile[MAX_PATH];

// Parses and executes command in s. Returns 0 on success (valid command) and
// -1 on error.
int Execute(char *s,char *answer) {
  int i,j,n;
  char type;
  t_result result;

  if(s==NULL || s[0]=='\0' || answer==NULL) {
    return -1;                         // Error or nothing to execute
  }
  // Extract command and convert it to uppercase.
  while(*s==' ' || *s=='\t') {
    s++;     // Skip spaces
  }
  answer[0]='\0';
  i=0;
  // Valid command is either '?','*',':' or consists only of characters.
  while(i<32) {
    if(isalpha(*s)) {
      cmd[i++]=(char)toupper(*s++);
    }
    else if(*s=='?' || *s=='*' || *s==':') {
      cmd[i++]=*s++;
      break;
    }
    else {
      break;
    };
  }
  cmd[i]='\0';
  // Recognize command.
  for(i=0; i<sizeof(cmdlist)/sizeof(cmdlist[0]); i++)
    if(strcmp(cmd,cmdlist[i].cmd)==0) break;
  if(i>=sizeof(cmdlist)/sizeof(cmdlist[0])) {
    wsprintf(answer,"Unrecognized command: %s",cmd);
    return -1; };
  // Command found. It has index i in cmdlist. Now get operands.
  j=0;
  while(1) {
    while(*s==' ' || *s=='\t') s++;   // Skip spaces from the command
    type=cmdlist[i].args[j++];         // Get type of next expected operand
    if(type=='\0')
      break;                           // End of list of arguments reached
    switch(type) {
      case 'a':                        // Address expression or empty
        if(*s==',' || *s=='=' || *s=='\0' || *s==';') {
          address=0;
          if(*s!='\0') s++;
          break;                       // Note: conditional break
        };
      case 'A':                        // Address expression
        n=Expression(&result,s,0,0,NULL,0,0,Getcputhreadid());
        if(n<0 || result.type==DEC_UNKNOWN) {
          strcpy(answer,result.value); // Error in expression
          return -1; };
        if(result.dtype!=DEC_DWORD) { // Address must be integer
          strcpy(answer,"Integer expression expected"); return -1; };
        if(Findmemory(result.u)==NULL) {
          strcpy(answer,"Address points to nowhere");
          return -1; };
        address=result.u;
        s+=n; break;
      case 'S':                        // String, rest of command
        strcpy(string,s);
        s+=strlen(s); break;
      case 'v':                        // Value of expression or empty
        if(*s==',' || *s=='=' || *s=='\0' || *s==';') {
          result.type=DEC_UNKNOWN;
          result.u=0;
          if(*s!='\0') s++;
          break;                       // Note: conditional break
        };
      case 'V':                        // Value of expression
        n=Expression(&value,s,0,0,NULL,0,0,Getcputhreadid());
        if(n<0 || value.type==DEC_UNKNOWN) {
          strcpy(answer,value.value);  // Error in expression
          return -1;
        };
        s+=n; break;
      default:
        wsprintf(answer,"Internal error: undefined operand type %c",type);
        return -1;
      ;
    };
    if(cmdlist[i].args[j]!='\0') {
      // More operands will follow, skip delimiter if any.
      while(*s==' ' || *s=='\t') s++;
      if(*s==',' || *s=='=') s++;
    };
  };
  if(*s!='\0' && *s!=';') {           // Unprocessed characters
    wsprintf(answer,"Extra characters on line: %.64s",s);
    return -1; };
  // Command parsed correctly, now call subroutine that executes it.
  return cmdlist[i].func(answer,cmdlist[i].parm);
};

// Inserts line into the first position in history and update combobox. If
// exactly the same line is already in the list, old copy will be deleted. If
// resulting number of lines exceeds NHIST, last line will be deleted. If
// text is NULL, function clears history, if "" - redraws combobox.
static void Addline(char *text) {
  int i;
  if(text==NULL) {
    nhist=0;                           // Request to clear history
    SetWindowText(hwerr,""); }         // Clear error message
  else if(text[0]!='\0') {
    // Remove identical strings from the history.
    for(i=nhist-1; i>=0; i--) {
      if(strcmp(hist[i],text)==0) {
        nhist--;
        if(i<nhist) memmove(hist[i],hist[i+1],(nhist-i)*TEXTLEN);
      };
    };
    // Insert new string.
    if(nhist>=NHIST) nhist=NHIST-1;
    memmove(hist[1],hist[0],nhist*TEXTLEN);
    strcpy(hist[0],text);
    nhist++;
  };
  // Copy history to combobox.
  if(hwcmd!=NULL) {
    SendMessage(hwbox,CB_RESETCONTENT,0,0);
    for(i=0; i<nhist; i++)
      SendMessage(hwbox,CB_ADDSTRING,0,(LPARAM)hist[i]);
    if(text!=NULL && nhist>0) {
      SetWindowText(hwbox,text);
      SendMessage(hwbox,CB_SETEDITSEL,0,TEXTLEN);
    }
  }
}

void ExecMacro(void)
{
  PCOMMAND_QUE p,q;
  char s[TEXTLEN],answer[TEXTLEN];

  p = cmdquehead.next;
  wsprintf(s,"%s",p->cmd);
  Addtolist(0,1,"  %3d: %s",p->n,s);
  q = p;
  p = p->next;
  free(q);
  cmdquehead.next = p;
  if(Execute(s,answer)==0) {
    Addline(s);                  // Add line only if command valid
  }
  if(answer) {
    SetWindowText(hwerr,answer); // Display result or error message
  }
  if(!cmdquehead.next) {
    Addtolist(0,1," Macro finished!!");
    cmdquehead.next = NULL;
    bMacExec = FALSE;
  }
}

void FreeMacro(void)
{
  PCOMMAND_QUE p,q;

  if(cmdquehead.next == NULL) {
    return;
  }
  p = cmdquehead.next;
  while(p != NULL) {
    q = p;
    p = p->next;
    free(q);
  }
  cmdquehead.next = NULL;
}

BOOL IsToolbarShown(void)
{
  RECT rcMain,rcMDI;
  POINT pt;
  int y1;

  GetClientRect(hwmain,&rcMain);
  GetWindowRect(hwMDI,&rcMDI);
  pt.x = rcMDI.left;
  pt.y = rcMDI.top;
  ScreenToClient(hwmain,&pt);
  y1 = pt.y;
  if(rcMain.top == y1) {
    return FALSE;
  }
  return TRUE;
}

void MoveMDIClientWindow(void)
{
  RECT rc;
  int ToolbarShown,x,y,w,h;

  ToolbarShown = IsToolbarShown();
  GetClientRect(hwmain,&rc);
  x = 0;
  y = (ToolbarShown) ? TOOLBAR_HIGHT : 0;
  w = rc.right - rc.left;
  h = rc.bottom-rc.top-STATUSBAR_HIGHT-((iShowCmdbar) ? CMDBAR_HIGHT : 0)-((ToolbarShown==TRUE) ? TOOLBAR_HIGHT : 0);
  MoveWindow(hwMDI,x,y,w,h,TRUE);
}

void MoveCmdbarWindow(void)
{
  RECT rc;
  int x,y,w,h;

  GetClientRect(hwmain,&rc);
  x = 0;
  y = rc.bottom - STATUSBAR_HIGHT - CMDBAR_HIGHT;
  w = rc.right - rc.left;
  h = CMDBAR_HIGHT;
  MoveWindow(hwcmd,x,y,w,h,TRUE);

  GetClientRect(hwcmd,&rc);
  x = 265;
  y = 3;
  w = rc.right-267;
  h = CMDBAR_HIGHT-6,
  MoveWindow(hwerr,x,y,w,h,TRUE);

  if(iShowCmdbar) {
    ShowWindow(hwcmd,SW_SHOW);
  }
  else {
    ShowWindow(hwcmd,SW_HIDE);
  }
}

void SetCmdInfo(char *cmd)
{
  int i,arglen,iHit;
  char cmdinfo[TEXTLEN],buf[TEXTLEN],*pdest;
  DWORD *cmdaddr;
  t_command *hitcmd = NULL;

  if(hitcmd) free(hitcmd);
  cmdaddr = (DWORD*)malloc(sizeof(DWORD));

  if(strlen(cmd) == 0) {
    SetWindowText(hwerr,"");
    return;
  }
  strcpy(buf,cmd);
  pdest = strchr(buf,' ');
  if((buf[0] >= '0' && buf[0] <= '9')) {
    pdest = buf;
  }
  if(pdest) {
    *pdest = '\0';
    iHit = 0;
    for(i=0; i<sizeof(cmdlist)/sizeof(cmdlist[0]); i++) {
      if(!stricmp(buf,cmdlist[i].cmd)) {
        cmdaddr = (DWORD*)realloc(cmdaddr,sizeof(DWORD)*(iHit+1));
        cmdaddr[iHit]  = (DWORD)&cmdlist[i];
        iHit++;
      }
    }
  }
  else {
    iHit = 0;
    for(i=0; i<sizeof(cmdlist)/sizeof(cmdlist[0]); i++) {
      if(!strnicmp(cmd,cmdlist[i].cmd,strlen(cmd))) {
        cmdaddr = (DWORD*)realloc(cmdaddr,sizeof(DWORD)*(iHit+1));
        cmdaddr[iHit]  = (DWORD)&cmdlist[i];
        iHit++;
      }
    }
  }

  if(iHit > 1) {
    memset(cmdinfo,0,sizeof(cmdinfo));
    for(i=0; i<iHit; i++) {
      hitcmd = (t_command*)cmdaddr[i];
      strcat(cmdinfo," ");
      strcat(cmdinfo,hitcmd->cmd);
    }
    SetWindowText(hwerr,cmdinfo);
  }
  else if(iHit == 1) {
    hitcmd = (t_command*)cmdaddr[0];
    memset(buf,0,sizeof(buf));
    arglen = strlen(hitcmd->args);
    for(i=0; i<arglen; i++) {
      switch(hitcmd->args[i]) {
      case 'A':
        strcat(buf," address");
        break;
      case 'a':
        strcat(buf," [address]");
        break;
      case 'S':
        strcat(buf," string");
        break;
      case 'V':
        strcat(buf," expression");
        break;
      case 'v':
        strcat(buf," [expression]");
        break;
      default:
        break;
      }
      if(buf[0] != '\0' && i < arglen-1) {
        strcat(buf,"\,");
      }
    }
    wsprintf(cmdinfo," %s%s -- %s",hitcmd->cmd,buf,hitcmd->info);
    SetWindowText(hwerr,cmdinfo);
  }
  else {
    if(strlen(cmd) > 0) {
      wsprintf(cmdinfo,"No valid command begin with \" %s \".",cmd);
      SetWindowText(hwerr,cmdinfo);
    }
    else {
      SetWindowText(hwerr,"");
    }
  }
  if(hitcmd) {
    free(cmdaddr);
  }
}

// Subclassed window procedure of edit control in combobox. Processes return
// (execute) and escape (clear line).
LRESULT CALLBACK Editsubclass(HWND hw,UINT msg,WPARAM wp,LPARAM lp) {
  char s[TEXTLEN],answer[TEXTLEN],cmd[TEXTLEN];
  static int ctrlkey = 0;
  int cmdlen;

  switch(msg) {
  case WM_KEYDOWN:
    switch(wp) {
    case VK_RETURN:
      cmdlen = GetWindowTextLength(hwbox);
      if(cmdlen > 0) {
        GetWindowText(hwbox,s,TEXTLEN-1);
        if(Execute(s,answer)==0) {
          Addline(s);                  // Add line only if command valid
        }
        if(answer) {
          SetWindowText(hwerr,answer);   // Display result or error message
        }
        SetFocus(hwbox);
      }
      return 0;
    case VK_ESCAPE:
      SetWindowText(hwbox,"");       // Clear edit text and message
      SetWindowText(hwerr,"");
      return 0;
    case VK_CONTROL:
      ctrlkey = 1;
      return 0;
    default:
      break;
    };
    break;
  case WM_CHAR:
    switch(wp) {
    case VK_RETURN:
    case VK_ESCAPE:
      return 0;
    default:
      break;
    }
    break;
  case WM_KEYUP:
    memset(cmd, 0, sizeof(cmd));
    switch(wp) {
    case VK_CONTROL:
      ctrlkey = 0;
      break;
    case VK_RETURN:
    case VK_ESCAPE:
      break;
    default:
      GetWindowText(hwbox,cmd,TEXTLEN-1);
      SetCmdInfo(cmd);
      break;
    }
  default:
    break;
  }
  return CallWindowProc(oldhweditproc,hw,msg,wp,lp);
}

// Each window class needs its own window procedure. Command line window is
// neither a table nor MDI, we don't need to call Tablefunction().
LRESULT CALLBACK CmdbarWinProc(HWND hw,UINT msg,WPARAM wp,LPARAM lp) {
  char s[10];
  RECT rc;
  PAINTSTRUCT ps;
  HBRUSH hbr;
  HDC dc;
  HINSTANCE hi;

  switch(msg) {
  case WM_CREATE:
    hi = (HINSTANCE)Plugingetvalue(VAL_HINST);
    GetClientRect(hw,&rc);
    // Create edit control with history list, assign default OllyDbg font and
    // limit length of entered commands to OllyDbg-wide default.
    hwstc = CreateWindowEx(0,
      "STATIC","Command :",
      WS_CHILD|WS_VISIBLE|SS_SIMPLE|SS_GRAYFRAME,
      5,7,60,CMDBAR_HIGHT-7,
      hw,(HMENU)ID_HWSTC,hi,NULL);
    if(hFont) {
      SendMessage(hwstc,WM_SETFONT,(WPARAM)hFont,1);
    }

    hwbox=CreateWindowEx(0,
      "COMBOBOX","",
      WS_CHILD|WS_TABSTOP|WS_BORDER|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWN|
      CBS_HASSTRINGS|CBS_NOINTEGRALHEIGHT|CBS_AUTOHSCROLL|CBS_DISABLENOSCROLL,
      62,3,200,200,
      hw,(HMENU)ID_HWBOX,hi,NULL);
    if(hFont) {
      SendMessage(hwbox,WM_SETFONT,(WPARAM)hFont,1);
    }
    SendMessage(hwbox,CB_LIMITTEXT,TEXTLEN-1,1);

    // To intercept return key, subclass the edit control inside the combobox.
    hwedt = GetWindow(hwbox,GW_CHILD);
    oldhweditproc=(WNDPROC)SetWindowLong(hwedt,GWL_WNDPROC,(long)Editsubclass);

    // Create text control displaying error messages.
    hwerr=CreateWindowEx(0,
      "STATIC","",
      WS_CHILD|WS_VISIBLE|SS_LEFT|SS_SUNKEN,
      265,3,rc.right-267,CMDBAR_HIGHT-6,
      hw,(HMENU)ID_HWERR,hi,NULL);
    if(hFont) {
      SendMessage(hwerr,WM_SETFONT,(WPARAM)hFont,1);
    }
    Addline("");                       // Refill combobox
    //SetFocus(hwbox);
    break;
  //case WM_DESTROY:
    //hwcmd=NULL;
    //break;
  case WM_SETFOCUS:
    SetFocus(hwbox);
    break;
  case WM_CLOSE:
    wsprintf(s,"%d",iShowCmdbar);
    WritePrivateProfileString("Option","Show Command Bar Window",s,szIni);
    return DefWindowProc(hw,msg,wp,lp);
  case WM_PAINT:
    MoveCmdbarWindow();
    MoveMDIClientWindow();
    // Fill background of command line window with default button colour.
    // Necessary because Registerpluginclass() sets hbrBackground to NULL.
    dc=BeginPaint(hw,&ps);
    GetClientRect(hw,&rc);
    hbr=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(dc,&rc,hbr);
    DeleteObject(hbr);
    EndPaint(hw,&ps);
    //Broadcast(WM_USER_CHALL,0,0);
    InvalidateRect(hwmain,NULL,TRUE);
    break;
  case WM_SIZE:
    MoveCmdbarWindow();
    MoveMDIClientWindow();
    //Broadcast(WM_USER_CHALL,0,0);
    InvalidateRect(hwmain,NULL,TRUE);
    break;
  default: return DefWindowProc(hw,msg,wp,lp);
  };
  return 0;
};

// Creates command line window or, if window already exists, brings it to top.
// Returns handle of the window or NULL on error. Corresponding class is
// already registered.
HWND CreateCmdbarWindow(void) {
  RECT rc;

  if(hwcmd==NULL) {                   // Window is not yet here, create new.
    // Create main command line window.
    GetClientRect(hwmain,&rc);
    hwcmd=CreateWindowEx(0,
      CmdbarWinClass,"Command Bar",
      WS_CHILD|WS_VISIBLE,
      0,rc.bottom-STATUSBAR_HIGHT-CMDBAR_HIGHT,rc.right,CMDBAR_HIGHT,
      hwmain,NULL,(HANDLE)Plugingetvalue(VAL_HINST),NULL);
    if(hwcmd==NULL) {
      return NULL;      // Oops!..
    }

  };
  // Bring window to top
  //SetForegroundWindow(hwcmd);

  return hwcmd;
};

// Entry point into a plugin DLL. Many system calls require DLL instance
// which is passed to DllEntryPoint() as one of parameters. Remember it.
BOOL WINAPI DllEntryPoint(HINSTANCE hi,DWORD reason,LPVOID reserved) {
  if(reason==DLL_PROCESS_ATTACH)
    hinst=hi;                          // Mark plugin instance
  return 1;                            // Report success
};

// Report plugin name and return version of plugin interface.
extc int _export cdecl ODBG_Plugindata(char shortname[32]) {
  strcpy(shortname,PNAME);    // Name of command line plugin
  return PLUGIN_VERSION;
};

LRESULT CALLBACK OllySubclassProc(HWND hw,UINT msg,WPARAM wp,LPARAM lp)
{
  switch(msg) {
  case WM_SIZE:
    SendMessage(hwcmd,WM_SIZE,wp,lp);
    break;
  default:
    break;
  }
  return CallWindowProc(oldOllyWndProc,hw,msg,wp,lp);
}

// OllyDbg calls this obligatory function once during startup. I place all
// one-time initializations here. Parameter features is reserved for future
// extentions, do not use it.
extc int _export cdecl ODBG_Plugininit(int ollydbgversion,HWND hw,ulong *features) {
  char *pdest;
  int MacNameBufSiz;

  // This plugin uses all the newest features, check that version of OllyDbg is
  // correct. I will try to keep backward compatibility at least to v1.99.
  if(ollydbgversion<PLUGIN_VERSION)
    return -1;
  // Keep handle of main OllyDbg window. This handle is necessary, for example,
  // to display message box.
  hwmain=hw;
  hwMDI = (HWND)Plugingetvalue(VAL_HWCLIENT);

  NumberOfCommand = sizeof(cmdlist)/sizeof(cmdlist[0]);
  pcmdlist = cmdlist;

  // Register window class for command line window - correct although this
  // window is not a table. Note that formally this class belongs to instance
  // of main OllyDbg program and uses standard plugin icon.
  if(Registerpluginclass(CmdbarWinClass,NULL,hinst,CmdbarWinProc)<0) {
    return -1;
  }
  // Report plugin in the log window.
  Addtolist(0,0,PNAME" v%s",PVERS);
  Addtolist(0,-1,"  Originary Written by Oleh Yuschuk  Modified by %s  Contributors:%s",ANAME,CNAM2);

  oldOllyWndProc = (WNDPROC)SetWindowLong(hwmain,GWL_WNDPROC,(long)OllySubclassProc);
  if(oldOllyWndProc == 0) {
    Unregisterpluginclass(CmdbarWinClass);
    return -1;
  }


  GetModuleFileName(NULL, szODPath, MAX_PATH);
  pdest = strrchr(szODPath, '\\');
  pdest[1] = '\0';
  wsprintf(szODIni,"%sollydbg.ini",szODPath);
  GetPrivateProfileString("History","Plugin path",szODPath,szPluginPath,sizeof(szPluginPath),szODIni);

  ZeroMemory(&lf,sizeof(LOGFONT));
  wsprintf(szIni,"%s\\%s",szPluginPath,"CmdBar.ini");
  lf.lfHeight = GetPrivateProfileInt("Font","Size",-12,szIni);
  GetPrivateProfileString("Font","Face","MS UI Gothic",lf.lfFaceName,sizeof(lf.lfFaceName),szIni);
  lf.lfCharSet = (BYTE)GetPrivateProfileInt("Font","CharSet",SHIFTJIS_CHARSET,szIni);
  hFont = CreateFontIndirect(&lf);

  iShowCmdbar = GetPrivateProfileInt("Option","Show Command Bar Window",CMDBAR_SHOW,szIni);
  if(!CreateCmdbarWindow()) {
    Unregisterpluginclass(CmdbarWinClass);
    SetWindowLong(hwmain,GWL_WNDPROC,(long)oldOllyWndProc);
    DeleteObject(hFont);
    return -1;
  }

  MacNameBufSiz = GetPrivateProfileInt("Option","Macro Name Buffer Size",256,szIni);
  MacroNames = (char*)calloc(MacNameBufSiz,sizeof(char));
  wsprintf(szMacDefFile,"%s\\%s",szPluginPath,"macro.def");
  GetPrivateProfileSectionNames(MacroNames,MacNameBufSiz,szMacDefFile);

  // Confirm success.
  return 0;
};

// This function is called each time OllyDbg passes main Windows loop. When
// debugged application stops, bring command line window in foreground.

extc void _export cdecl ODBG_Pluginmainloop(DEBUG_EVENT *debugevent) {
  t_status status;

  status = Getstatus();
  if(status==STAT_STOPPED && bMacExec) {
    ExecMacro();
  }
}

// Save command history to .udd file. History data is project-related.
extc void _export cdecl ODBG_Pluginsaveudd(t_module *pmod,int ismainmodule) {
  int i;
  if(ismainmodule==0)
    return;                            // Save history to main file only
  // When restoring from the .udd, last saved history line will become first
  // in the history list.
  for(i=nhist-1; i>=0; i--) {
    Pluginsaverecord(TAG_CMDBAR,strlen(hist[i])+1,hist[i]);
  };
};

// OllyDbg restores data from .udd file.
extc int _export cdecl ODBG_Pluginuddrecord(t_module *pmod,int ismainmodule,
  ulong tag,ulong size,void *data) {
  if(tag!=TAG_CMDBAR)
    return 0;                          // Not a command line history record
  if(ismainmodule==0)
    return 0;                          // History is saved in main file only
  Addline(data);
  return 1;
};

// Function adds items to main OllyDbg menu (origin=PM_MAIN).
extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item) {
  if(origin!=PM_MAIN)
    return 0;                          // No pop-up menus in OllyDbg's windows
  strcpy(data,
         "0 &Show/Hide CommandBar\tAlt+F1|"
         "1 Choose Bar font|"
         "62 &Help|"
         "63 "PNAME" v"PVERS
         );
  return 1;
};

// Receives commands from main menu.
extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item) {
  char s[10];

  if(origin!=PM_MAIN)
    return;
  switch(action) {
  case 0:
    iShowCmdbar ^= 1;
    MoveCmdbarWindow();
    MoveMDIClientWindow();
    break;
  case 1:
    ZeroMemory(&cf,sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner   = NULL;
    cf.lpLogFont   = &lf;
    cf.Flags       = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS;
    cf.rgbColors   = RGB(0, 0, 0);
    cf.nFontType   = SCREEN_FONTTYPE;
    if(ChooseFont(&cf)) {
      WritePrivateProfileString("Font","Face",lf.lfFaceName,szIni);
      wsprintf(s,"%d",lf.lfHeight);
      WritePrivateProfileString("Font","Size",s,szIni);
      wsprintf(s,"%d",lf.lfCharSet);
      WritePrivateProfileString("Font","CharSet",s,szIni);
      if(hFont) {
        DeleteObject(hFont);
      }
      hFont = CreateFontIndirect(&lf);
      SendMessage(hwstc,WM_SETFONT,(WPARAM)hFont,1);
      SendMessage(hwbox,WM_SETFONT,(WPARAM)hFont,1);
      SendMessage(hwerr,WM_SETFONT,(WPARAM)hFont,1);
    }
    InvalidateRect(hwmain,NULL,TRUE);
    break;
  case 62:                            // "Help", opens help file
    WinHelp(hwmain,"cmdline.hlp",HELP_CONTENTS,0);
    break;
  case 63:                            // "About", displays plugin info
    break;
  default: break;
  };
};

// Command line window recognizes global shortcut Alt+F1.
extc int _export cdecl ODBG_Pluginshortcut(int origin,int ctrl,int alt,int shift,int key,void *item) {
  if(ctrl==0 && alt==1 && shift==0 && key==VK_F1) {
    iShowCmdbar ^= 1;
    MoveCmdbarWindow();
    MoveMDIClientWindow();
    return 1;
  }
  else if(ctrl==1 && alt==0 && shift==0 && (key=='d' || key=='D')) {
    if(iShowCmdbar) {
      SetFocus(hwbox);
    }
    return 1;
  }
  return 0;                            // Shortcut not recognized
};

// User opens new or restarts current application, clear command line history.
extc void _export cdecl ODBG_Pluginreset(void) {
  //poponstop=1;
  Addline(NULL);
  if(cmdquehead.next) {
    FreeMacro();
    bMacExec = FALSE;
  }
};

// OllyDbg calls this optional function when user wants to terminate OllyDbg.
extc int _export cdecl ODBG_Pluginclose(void) {
  // For automatical restoring, mark in .ini file whether command line window
  // is still open and save coordinates. (WM_CLOSE is not sent in this case).
  char s[10];
  wsprintf(s,"%d",iShowCmdbar);
  WritePrivateProfileString("Option","Show Command Bar Window",s,szIni);
  if(cmdquehead.next) {
    FreeMacro();
    bMacExec = FALSE;
  }
  return 0;
};

// OllyDbg calls this optional function once on exit. At this moment, all
// windows created by plugin are already destroyed (and received WM_DESTROY
// messages). Function must free all internally allocated resources, like
// window classes, files, memory and so on.
extc void _export cdecl ODBG_Plugindestroy(void) {
  free(MacroNames);
  DeleteObject(hFont);
  Unregisterpluginclass(CmdbarWinClass);
};

