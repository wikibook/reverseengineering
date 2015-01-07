////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     COMMAND LINE PLUGIN FOR OLLYDBG                        //
//                                                                            //
//                          (Command processing)                              //
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

#define STRICT                         // Avoids some type mismatches
#include <windows.h>
#include <stdio.h>
//#include <dir.h>
#include "plugin.h"
#include "cmdexec.h"

extern BOOL bMacExec;
extern COMMAND_QUE cmdquehead;

typedef struct _MACROCMD {
  char cmd[TEXTLEN];
} MACROCMD, *PMACROCMD;

int Execute(char *s,char *answer);
extern ulong     address;              // Operands A/a (address expression)
extern t_result  value;                // Operands V/v (expression)
extern char      string[TEXTLEN];      // Operand  S   (any string)
extern HWND      hwerr;                // Error message
//extern char szODPath[MAX_PATH],szODIni[MAX_PATH],szPluginPath[MAX_PATH],szMacDefFile[MAX_PATH];
extern char szMacDefFile[MAX_PATH];
extern char *MacroNames;
extern int NumberOfCommand;
extern t_command *pcmdlist;

int IsHex(const char* str)
{
  char* ec;
  if(str == (char*)0 || *str == '\0') {
    return FALSE;
  }
  (void)strtoul(str, &ec, 16) ;
  return *ec == '\0';
}

int Express(char *answer,ulong parm) {
  int i,j,val;
  char *p;
  char *v = value.value;
  char *tmp;
  char h,l,asc;

  tmp = (char*)malloc(5);
  memset(tmp,0,5);

  if(IsHex(v)) {
    j = 0;
    for(i=0; i<4; i++) {
      h = v[i*2];
      l = v[i*2+1];
      if(h >= '0' && h <= '9') {
        asc = (char)(h - 0x30);
      }
      else if(h >= 'A' && h <= 'F') {
        asc = (char)(h - 0x37);
      }
      else if(h >= 'a' && h <= 'f') {
        asc = (char)(h - 0x57);
      }
      asc <<= 4;
      if(l >= '0' && l <= '9') {
        asc += (char)(l - 0x30);
      }
      else if(l >= 'A' && l <= 'F') {
        asc += (char)(l - 0x37);
      }
      else if(l >= 'a' && l <= 'f') {
        asc += (char)(l - 0x57);
      }
      if(asc > 0) {
        tmp[j] = asc;
        j++;
      }
    }
    val = strtol(v,&p,16);
    wsprintf(answer, "HEX: %X - DEC: %d - ASCII: %s", val, val, tmp);
  }
  else {
    strcpy(answer,v);
  }

  free(tmp);
  return 0;
};

int Addwtch(char *answer,ulong parm) {
  if (string[0]!='\0')
    Insertwatch(1000000,string);
  Createwatchwindow();
  return 0;
};

int Dasmcmd(char *answer,ulong parm) {
  Setcpu(0,address,0,0,CPU_ASMHIST|CPU_ASMCENTER);
  return 0;
};

int Dorigin(char *answer,ulong parm) {
  Sendshortcut(PM_DISASM,0,WM_CHAR,0,0,'*');
  return 0;
};

int Dumpcmd(char *answer,ulong parm) {
  Setcpu(0,0,address,0,CPU_DUMPHIST|CPU_DUMPFIRST|CPU_DUMPFOCUS);
  if (parm!=0) Setdumptype((t_dump *)Plugingetvalue(VAL_CPUDDUMP),parm);
  return 0;
};

int Stakcmd(char *answer,ulong parm) {
  Setcpu(0,0,0,address,CPU_DUMPHIST|CPU_DUMPFIRST|CPU_DUMPFOCUS);
  return 0;
};

int Assembl(char *answer,ulong parm) {
  int i,j,k,n,good;
  char s[TEXTLEN];
  t_asmmodel model,attempt;
  t_memory *pmem;
  t_dump *pasm;
  // Visualize changes.
  Setcpu(0,address,0,0,CPU_ASMHIST|CPU_ASMCENTER);
  if (string[0]=='\0')                 // No immediate command
    Sendshortcut(PM_DISASM,address,WM_CHAR,0,0,' ');
  else {
    // Assemble immediate command. If there are several possible encodings,
    // select the shortest one.
    model.length=0;
    for (j=0; ; j++) {                 // Try all possible encodings
      good=0;
      for (k=0; k<4; k++) {            // Try all possible constant sizes
        n=Assemble(string,address,&attempt,j,k,model.length==0?answer:s);
        if (n>0) {
          good=1;
          // If another decoding is found, check if it is shorter.
          if (model.length==0 || n<model.length)
            model=attempt;             // Shortest encoding so far
          ;
        };
      };
      if (good==0) break;              // No more encodings
    };
    if (model.length==0)
      return -1;                       // Invalid command
    // Check for imprecise parameters.
    k=model.mask[0];
    for (i=1; i<model.length; i++) k&=model.mask[i];
    if (k!=0xFF) {
      strcpy(answer,"Command contains imprecise operands");
      return -1; };
    // If there is no backup copy, create it. Dump window always assumes that
    // backup has the same base and size as the dump, so check it to avoid
    // strange ireproducible errors.
    pmem=Findmemory(address);
    if (pmem==NULL) {
      //strcpy(answer,"Attempt to assemble to non-existing memory");
      wsprintf(answer,"%X",model.code[0]);
      for(i=1; i<model.length; i++) {
        wsprintf(answer,"%s%X",answer,model.code[i]);
      }
      return -1; };
    pasm=(t_dump *)Plugingetvalue(VAL_CPUDASM);
    if (pasm!=NULL && pmem->copy==NULL && pmem->base==pasm->base && pmem->size==pasm->size)
      Dumpbackup(pasm,BKUP_CREATE);
    // Now write assembled code to memory.
    Writememory(model.code,address,model.length,MM_RESTORE|MM_DELANAL);
  };
  return 0;
};

int LineAsm(char *answer,ulong parm) {
  int i,j,k,n,good;
  char s[TEXTLEN],*p;
  t_asmmodel model,attempt;
  char *stop;

  p = strstr(string,";");
  if(p) {
    *p = '\0';
    p++;
    address = strtoul(p,&stop,16);
  }
  // Assemble immediate command. If there are several possible encodings,
  // select the shortest one.
  model.length=0;
  for(j=0; ; j++) {                 // Try all possible encodings
    good=0;
    for (k=0; k<4; k++) {            // Try all possible constant sizes
      n=Assemble(string,address,&attempt,j,k,model.length==0?answer:s);
      if (n>0) {
        good=1;
        // If another decoding is found, check if it is shorter.
        if (model.length==0 || n<model.length)
          model=attempt;             // Shortest encoding so far
        ;
      };
    };
    if (good==0) break;              // No more encodings
  };
  if (model.length==0)
    return -1;                       // Invalid command
  // Check for imprecise parameters.
  k=model.mask[0];
  for(i=1; i<model.length; i++) {
    k&=model.mask[i];
  }
  if(k!=0xFF) {
    strcpy(answer,"Command contains imprecise operands");
    return -1; };
  // If there is no backup copy, create it. Dump window always assumes that
  // backup has the same base and size as the dump, so check it to avoid
  // strange ireproducible errors.
  wsprintf(answer,"%02X",model.code[0]);
  for(i=1; i<model.length; i++) {
    wsprintf(answer,"%s%02X",answer,model.code[i]);
  }
  return -1;
};

int LineDsm(char *answer,ulong parm) {
  int i,j,len,n;
  char h,l,asc,*p,buf[TEXTLEN],cmd[MAXCMDSIZE];
  t_disasm dsm;
  char *stop;

  address = 0;
  p = strstr(string,";");
  if(p) {
    *p = '\0';
    p++;
    address = strtoul(p,&stop,16);
  }
  len = strlen(string);
  memset(buf,0,TEXTLEN);
  for(i=0,j=0; i<len; i++) {
    asc = string[i];
    if((asc >= '0' && asc <= '9') || (asc >= 'A' && asc <= 'F') || (asc >= 'a' && asc <= 'f')) {
      buf[j] = string[i];
      j++;
    }
  }
  len = strlen(buf)/2;
  memset(cmd,0,MAXCMDSIZE);
  for(i=0; i<len; i++) {
    h = buf[i*2];
    l = buf[i*2+1];
    if(h >= '0' && h <= '9') {
      asc = (char)(h - 0x30);
    }
    else if(h >= 'A' && h <= 'F') {
      asc = (char)(h - 0x37);
    }
    else if(h >= 'a' && h <= 'f') {
      asc = (char)(h - 0x57);
    }
    asc <<= 4;
    if(l >= '0' && l <= '9') {
      asc += (char)(l - 0x30);
    }
    else if(l >= 'A' && l <= 'F') {
      asc += (char)(l - 0x37);
    }
    else if(l >= 'a' && l <= 'f') {
      asc += (char)(l - 0x57);
    }
    cmd[i] = asc;
  }
  n = Disasm(cmd,len,address,NULL,&dsm,DISASM_CODE,Plugingetvalue(VAL_MAINTHREADID));
  if(n != len) {
    wsprintf(answer,"%s \(%d/%d bytes disassemble\)",dsm.result,n,len);
  }
  else {
    strcpy(answer,dsm.result);
  }
  return -1;
};

int Commlab(char *answer,ulong parm) {
  Insertname(address,parm,string);
  Broadcast(WM_USER_CHALL,0,0);
  return 0;
};

int Breakpt(char *answer,ulong parm) {
  Setbreakpoint(address,TY_ACTIVE,0);
  Insertname(address,NM_BREAK,string);
  Deletenamerange(address,address+1,NM_BREAKEXPL);
  Deletenamerange(address,address+1,NM_BREAKEXPR);
  Broadcast(WM_USER_CHALL,0,0);
  return 0;
};

int Brkname(char *answer,ulong parm) {
  int i;
  char name[TEXTLEN];
  t_table *reftable;
  t_ref *pref;
  if(strlen(string)==0) {
    strcpy(answer,"Function name missed");
    return -1;
  }
  // Find all intermodule calls.
  Findalldllcalls((t_dump *)Plugingetvalue(VAL_CPUDASM),0,"Intermodular calls");
  // Get sorted data.
  reftable=(t_table *)Plugingetvalue(VAL_REFERENCES);
  if(reftable==NULL || reftable->data.n==0) {
    strcpy(answer,"No references");
    return -1;
  }
  if(reftable->data.itemsize<sizeof(t_ref)) {
    strcpy(answer,"Old version of OllyDbg");
    return -1;
  }
  // Loop through all found calls.
  for(i=0; i<reftable->data.n; i++) {
    // The safest way: size of t_ref may change in the future!
    pref=(t_ref *)((char *)reftable->data.data+reftable->data.itemsize*i);
    if(Findlabel(pref->dest,name)==0) {// Unnamed destination
      continue;
    }
    if(stricmp(name,string)!=0) {      // Different function
      continue;
    }
    if(parm==0) {                     // Set breakpoint
      Setbreakpoint(pref->addr,TY_ACTIVE,0);
      Deletenamerange(pref->addr,pref->addr+1,NM_BREAK);
      Deletenamerange(pref->addr,pref->addr+1,NM_BREAKEXPL);
      Deletenamerange(pref->addr,pref->addr+1,NM_BREAKEXPR);
    }
    else {
      Deletebreakpoints(pref->addr,pref->addr+1,0);
    }
  }
  Broadcast(WM_USER_CHALL,0,0);
  return 0;
};

int Delbkpt(char *answer,ulong parm) {
  Deletebreakpoints(address,address+1,0);
  Broadcast(WM_USER_CHALL,0,0);
  return 0;
};

int Membkpt(char *answer,ulong parm) {
  ulong size;
  if (parm==0)
    Setmembreakpoint(0,0,0);
  else {
    if (value.dtype!=DEC_DWORD) {
      strcpy(answer,"Integer expression expected");
      return -1; }
    else if (value.u==0)
      size=1;
    else if (value.u<=address) {
      strcpy(answer,"Invalid memory range");
      return -1; }
    else
      size=value.u-address;
    Setmembreakpoint(parm,address,size); };
  return 0;
};

int Hwbreak(char *answer,ulong parm) {
  int i;
  if (Plugingetvalue(VAL_HARDBP)==0) {
    strcpy(answer,"OS doesn't support hardware breakpoints");
    return -1; };
  if (address==0)
    Hardbreakpoints(0);
  else if (parm==HB_FREE) {
    i=Deletehardwarebreakbyaddr(address);
    if (i==0) wsprintf(answer,"No HW breakpoint on address %08X",address); }
  else {
    i=Sethardwarebreakpoint(address,1,parm);
    if (i!=0) strcpy(answer,"Unable to set HW breakpoint"); };
  return 0;
};

int Runprog(char *answer,ulong parm) {
  Go(0,address,STEP_RUN,parm,1);
  return 0;
};

int Stepper(char *answer,ulong parm) {
  if (parm==0)                         // Step into
    Sendshortcut(PM_MAIN,0,WM_KEYDOWN,0,0,VK_F7);
  else                                 // Step over
    Sendshortcut(PM_MAIN,0,WM_KEYDOWN,0,0,VK_F8);
  return 0;
};

int Tracing(char *answer,ulong parm) {
  ulong threadid;
  t_thread *pthr;
  // If run trace buffer is not active, create it. It requires actual registers
  // of current thread.
  if (Runtracesize()==0) {
    threadid=Getcputhreadid();
    if (threadid==0) threadid=Plugingetvalue(VAL_MAINTHREADID);
    pthr=Findthread(threadid);
    if (pthr!=NULL) Startruntrace(&(pthr->reg)); };
  // If buffer is open, set optional condition and start run trace.
  if (Runtracesize()>0) {
    if (address==0)
      Settracecondition(string,0,0,0,0,0);
    else
      Settracecondition(string,0,address,address+1,0,0);
    if (parm==0) {
      Animate(ANIMATE_TRIN);
      Go(0,0,STEP_IN,0,1); }
    else {
      Animate(ANIMATE_TROVER);
      Go(0,0,STEP_OVER,0,1);
    };
  };
  return 0;
};

int Keystrk(char *answer,ulong parm) {
  Sendshortcut(PM_MAIN,0,WM_KEYDOWN,0,0,parm);
  return 0;
};

int Ctrlkey(char *answer,ulong parm) {
  Sendshortcut(PM_MAIN,0,WM_KEYDOWN,1,0,parm);
  return 0;
};

int CtlDasm(char *answer,ulong parm) {
  Sendshortcut(PM_DISASM,0,WM_KEYDOWN,1,0,parm);
  return 0;
};

int Altkeyd(char *answer,ulong parm) {
  Sendshortcut(PM_MAIN,0,WM_SYSKEYDOWN,0,0,parm);
  return 0;
};

int Openexe(char *answer,ulong parm) {
  if (string[0]=='\0')
    Sendshortcut(PM_MAIN,0,WM_KEYDOWN,0,0,VK_F3);
  else
    OpenEXEfile(string,0);
  return 0;
};

int Heeeelp(char *answer,ulong parm) {
  int n;
  char *apihelp;
  n=strlen(string);
  if (n==0) {
    WinHelp((HWND)Plugingetvalue(VAL_HWMAIN),"cmdline.hlp",HELP_CONTENTS,0);
    return 0; };
  if (stricmp(string,"OllyDbg")==0) {
    WinHelp((HWND)Plugingetvalue(VAL_HWMAIN),"ollydbg.hlp",HELP_CONTENTS,0);
    return 0; };
  apihelp=(char *)Plugingetvalue(VAL_APIHELP);
  if (apihelp==NULL || apihelp[0]=='\0') {
    strcpy(answer,"No API help file selected in OllyDbg");
    return -1;
  }
  // Check for possible ANSI/UNICODE function suffix and remove it. Topics in
  // API help usually contain generic names.
  if (n>2 && islower(string[n-2]) && (string[n-1]=='A' || string[n-1]=='W'))
    string[n-1]='\0';
  WinHelp((HWND)Plugingetvalue(VAL_HWMAIN),apihelp,HELP_PARTIALKEY,(ulong)string);
  return 0;
};

int LoadMac(char *answer,ulong parm) {
  const char *nomore = "NO MORE";
  int  i,j;
  BOOL IsMacName;
  char key[10],line[TEXTLEN],buf[TEXTLEN];
  char *pdest;
  PCOMMAND_QUE pnew,p;

  pdest = MacroNames;
  IsMacName = FALSE;
  while(*pdest != '\0') {
    if(!strcmp(string,pdest)) {
      IsMacName = TRUE;
      break;
    }
    pdest += lstrlen(MacroNames)+1;
  }
  if(IsMacName == FALSE) {
    wsprintf(answer,"\"%s\" is invalid macro name!!",string);
    return(-1);
  }

  Addtolist(0,1," Loading Macro \"%s\"...",string);
  p = &cmdquehead;
  i = 0;
  do {
    wsprintf(key,"Line%d",i);
    GetPrivateProfileString(string,key,nomore,line,TEXTLEN,szMacDefFile);
    if(strcmp(line,nomore)) {
      if((pnew = (PCOMMAND_QUE)calloc(1,sizeof(COMMAND_QUE))) == NULL) {
        wsprintf(answer,"can't load Macro \"%s\"",string);
        FreeMacro();
        return -1;
      }
      // trim args
      wsprintf(buf,"%s",line);
      if(buf[0] != '\0') {
        for(j=0; j<lstrlen(buf); j++) {
          if(buf[j] == ' ') {
            buf[j] = '\0';
            break;
          }
        }
      }
      for(j=0; j<NumberOfCommand; j++) {
        if(!stricmp(buf,(pcmdlist+j)->cmd)) {
          wsprintf(pnew->cmd,"%s",line);
          pnew->n = i;
          break;
        }
      }
      pnew->next = NULL;
      p->next = pnew;
      p = p->next;
    }
    else {
      break;
    }
    i++;
  } while(1);
  Addtolist(0,1," Macro \"%s\" begins...",string);
  bMacExec = TRUE;
  return 0;
}
