CommandBar : Plugin for OllyDbg
                                     by Gigapede

summary:
  I remodeled command line plugin to a bar type in the lowest position of a main window.
  I add some commands and a candidate command indication function.
  About a command, please refer to a help of command line plugin.

what's new in v3.00.108
  I implement macro-function.

  Please put "macro.def" in a folder the same as a plugin folder.

  About a macro-form, please refer to the contents of macro.def.

  I moved settings in CmdBar.ini from ollydbg.ini.

  In initialization of plugin, it gets all macro-names by GetPrivateProfileSectionNames API.

  Buffer size for macro names is written in CmdBar.ini and gets it in an initialization process.

  If macro increases, and a buffer for plugin name was not enough, please edit CmdBar.ini directly to ajust the size.
    example: Macro Name Buffer Size=256


  I prepared for macro of a function the same as DebugPlugin as a sample in a macro-definition file.
  Please refer to it.
