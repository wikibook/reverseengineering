#ifndef __CMDEXEC_H
#define __CMDEXEC_H

void FreeMacro(void);
int IsHex(const char* str);
int Express(char *answer,ulong parm);
int Addwtch(char *answer,ulong parm);
int Dasmcmd(char *answer,ulong parm);
int Dorigin(char *answer,ulong parm);
int Dumpcmd(char *answer,ulong parm);
int Stakcmd(char *answer,ulong parm);
int Assembl(char *answer,ulong parm);
int LineAsm(char *answer,ulong parm);
int LineDsm(char *answer,ulong parm);
int Commlab(char *answer,ulong parm);
int Breakpt(char *answer,ulong parm);
int Brkname(char *answer,ulong parm);
int Delbkpt(char *answer,ulong parm);
int Membkpt(char *answer,ulong parm);
int Hwbreak(char *answer,ulong parm);
int Runprog(char *answer,ulong parm);
int Stepper(char *answer,ulong parm);
int Tracing(char *answer,ulong parm);
int Keystrk(char *answer,ulong parm);
int Ctrlkey(char *answer,ulong parm);
int CtlDasm(char *answer,ulong parm);
int Altkeyd(char *answer,ulong parm);
int Openexe(char *answer,ulong parm);
int Heeeelp(char *answer,ulong parm);
int LoadMac(char *answer,ulong parm);

// Prototype for function that executes command. Must return 0 on success and
// -1 if command is invalid and should not be added to the history. Anything
// written to answer will be displayed in error/message line of the command
// line window, input parameter parm is the corresponding field from cmdlist.
typedef int t_exefunc(char *answer,ulong parm);

typedef struct t_command { // Descriptor of command
  char           *cmd;       // Command (must be uppercase)
  char           *args;      // List of arguments
  ulong          parm;       // Parameter
  t_exefunc      *func;      // Responsible function
  char           *info;      // Command information
} t_command;

typedef struct t_cmdque {
  int  n;
  char cmd[TEXTLEN];
  struct t_cmdque *next;
} COMMAND_QUE, *PCOMMAND_QUE;

#endif //__CMDEXEC_H
