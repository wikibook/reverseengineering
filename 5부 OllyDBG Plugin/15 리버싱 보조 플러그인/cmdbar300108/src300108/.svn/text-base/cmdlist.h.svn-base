#ifndef __CMDLIST_H
#define __CMDLIST_H

#define Y 1
#define N 0

ulong     address;              // Operands A/a (address expression)
t_result  value;                // Operands V/v (expression)
t_result  lvalue;               // Operand  L   (lvalue address)
char      string[TEXTLEN];      // Operand  S   (any string)
int       count;                // Operands N/n (count)
char      cmd[TEXTLEN];         // Command  (uppercase)

//static t_command cmdlist[] = {
t_command cmdlist[] = {
  // Disassembler commands.
  { "AT",    "A",  0,                 Dasmcmd, "Disassemble at address"                    },   // Disassemble at address
  { "FOLLOW","A",  0,                 Dasmcmd, "Disassemble at address"                    },   // Ditto
  { "ORIG",  "",   0,                 Dorigin, "Disassemble at EIP"                        },   // Disassemble at EIP
  { "*",     "",   0,                 Dorigin, "Disassemble at EIP"                        },   // Ditto
  // Dump and stack commands.
  { "D",     "A",  0,                 Dumpcmd, "Dump at address"                           },   // Dump at address
  { "DUMP",  "A",  0,                 Dumpcmd, "Dump at address"                           },   // Dump at address
  { "DA",    "a",  DU_DISASM|0x0011,  Dumpcmd, "Dump as disassembly"                       },   // Dump as disassembly
  { "DB",    "a",  DU_HEXTEXT|0x0101, Dumpcmd, "Dump in hex byte format"                   },   // Dump in hex byte format
  { "DC",    "a",  DU_TEXT|0x0401,    Dumpcmd, "Dump in ASCII format"                      },   // Dump in ASCII format
  { "DD",    "a",  DU_ADDR|0x0014,    Dumpcmd, "Dump in stack format"                      },   // Dump in stack format
  { "DU",    "a",  DU_UNICODE|0x0402, Dumpcmd, "Dump in UNICODE format"                    },   // Dump in UNICODE format
  { "DW",    "a",  DU_IHEX|0x0082,    Dumpcmd, "Dump in hex word format"                   },   // Dump in hex word format
  { "STK",   "A",  0,                 Stakcmd, "Go to address in stack"                    },   // Go to address in stack
  // Assembling commands.
  { "A",     "AS", 0,                 Assembl, "Assemble at address"                       },   // Assemble at address
  // Comments and labels.
  { "L",     "AS", NM_LABEL,          Commlab, "Label at address"                          },   // Label at address
  { ":",     "AS", NM_LABEL,          Commlab, "Label at address"                          },   // Ditto
  { "C",     "AS", NM_COMMENT,        Commlab, "Comment at address"                        },   // Comment at address
  // Breakpoint commands.
  { "BP",    "AS", 0,                 Breakpt, "Break with condition"                      },   // Break with condition
  { "BPX",   "S",  0,                 Brkname, "Break on all calls"                        },   // Break on all calls
  { "BPD",   "S",  1,                 Brkname, "Delete break on all calls"                 },   // Delete break on all calls
  { "BC",    "A",  0,                 Delbkpt, "Delete breakpoint"                         },   // Delete breakpoint
  { "MR",    "Av", MEMBP_READ,        Membkpt, "Memory breakpt on access"                  },   // Memory breakpt on access
  { "MW",    "Av", MEMBP_WRITE,       Membkpt, "Memory breakpt on write"                   },   // Memory breakpt on write
  { "MD",    "",   0,                 Membkpt, "Remove memory breakpoint"                  },   // Remove memory breakpoint
  { "HR",    "A",  HB_ACCESS,         Hwbreak, "HW break on access"                        },   // HW break on access
  { "HW",    "A",  HB_WRITE,          Hwbreak, "HW break on write"                         },   // HW break on write
  { "HE",    "A",  HB_CODE,           Hwbreak, "HW break on execution"                     },   // HW break on execution
  { "HD",    "a",  HB_FREE,           Hwbreak, "Remove HW breakpoint"                      },   // Remove HW breakpoint
  // Stepping and tracing commands.
  { "STOP",  "",   VK_F12,            Keystrk, "Pause execution"                           },   // Pause execution
  { "PAUSE", "",   VK_F12,            Keystrk, "Pause execution"                           },   // Ditto
  { "RUN",   "",   0,                 Runprog, "Run program"                               },   // Run program
  { "G",     "a",  0,                 Runprog, "Run till address"                          },   // Run till address
  { "GE",    "a",  1,                 Runprog, "Run and pass exception"                    },   // Run and pass exception
  { "S",     "",   0,                 Stepper, "Step into"                                 },   // Step into
  { "SI",    "",   0,                 Stepper, "Step into"                                 },   // Ditto
  { "SO",    "",   1,                 Stepper, "Step over"                                 },   // Step over
  { "T",     "a",  0,                 Tracing, "Trace in till address"                     },   // Trace in till address
  { "TI",    "a",  0,                 Tracing, "Trace in till address"                     },   // Ditto
  { "TO",    "a",  1,                 Tracing, "Trace over till address"                   },   // Trace over till address
  { "TC",    "S",  0,                 Tracing, "Trace in till condition"                   },   // Trace in till condition
  { "TOC",   "S",  1,                 Tracing, "Trace over till condition"                 },   // Trace over till condition
  { "TR",    "",   VK_F9,             Ctrlkey, "Till return"                               },   // Till return
  { "TU",    "",   VK_F9,             Altkeyd, "Till user code"                            },   // Till user code
  // Table window commands.
  { "LOG",   "",   'L',               Altkeyd, "View Log window"                           },   // View Log window
  { "MOD",   "",   'E',               Altkeyd, "View Modules window"                       },   // View Modules window
  { "MEM",   "",   'M',               Altkeyd, "View Memory window"                        },   // View Memory window
  { "CPU",   "",   'C',               Altkeyd, "View CPU window"                           },   // View CPU window
  { "CS",    "",   'K',               Altkeyd, "View Call Stack"                           },   // View Call Stack
  { "BRK",   "",   'B',               Altkeyd, "View Breakpoints window"                   },   // View Breakpoints window
  { "OPT",   "",   'O',               Altkeyd, "Open Options"                              },   // Open Options
  // Application commands.
  { "EXIT",  "",   'X',               Altkeyd, "Quit OllyDbg"                              },   // Quit OllyDbg
  { "QUIT",  "",   'X',               Altkeyd, "Quit OllyDbg"                              },   // Ditto
  // Miscellaneous commands.
  { "OPEN",  "S",  0,                 Openexe, "Open executable file"                      },   // Open executable file
  { "CLOSE", "",   VK_F2,             Altkeyd, "Close executable"                          },   // Close executable
  { "RST",   "",   VK_F2,             Ctrlkey, "Restart current program"                   },   // Restart current program
  { "HELP",  "S",  0,                 Heeeelp, "Help on API function"                      },   // Help on API function
  { "H",     "S",  0,                 Heeeelp, "Help on API function"                      },   // Ditto
  // My commands.
  { "ASM",   "S",  0,                 LineAsm, "Assemble (if command needs it's own addres, \"ASM COMMAND\;ADDRESS\")"},   // Assemble
  { "DASM",  "S",  0,                 LineDsm, "Disassemble immediate opcode"              },   // Dump at address
  { "FR",    "",   'R',               CtlDasm, "Find reference to selected command/address"},   // Find Reference to selected command/address
  { "AC",    "",   'A',               CtlDasm, "Analyse code"                              },   // Analyse Code
  { "SN",    "",   'N',               CtlDasm, "Search for Name(label) in current module"  },   // Search for Name(label) in current module
  { "SOB",   "",   'O',               CtlDasm, "Scan object files"                         },   // Scan OBJect files
  // Macro command
  { "MAC",   "S",  0,                 LoadMac, "Execute Macro Command"                     },   // Execute Macro Command
  // Calculator and watch commands.
  { "CALC",  "V",  0,                 Express, "Estimate expression"                       },   // Estimate expression
  { "?",     "V",  0,                 Express, "Estimate expression"                       },   // Ditto
  { "WATCH", "S",  0,                 Addwtch, "Add watch expression"                      },   // Add watch expression
  { "W",     "S",  0,                 Addwtch, "Add watch expression"                      },   // Ditto
  { "",      "V",  0,                 Express, "Estimate expression"                       }    // Ditto
};
#endif // __CMDLIST_H
