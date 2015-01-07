; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDllSapIpDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "DllSapIp.h"

ClassCount=4
Class1=CDllSapIpApp
Class2=CDllSapIpDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_DLLSAPIP_DIALOG

[CLS:CDllSapIpApp]
Type=0
HeaderFile=DllSapIp.h
ImplementationFile=DllSapIp.cpp
Filter=N

[CLS:CDllSapIpDlg]
Type=0
HeaderFile=DllSapIpDlg.h
ImplementationFile=DllSapIpDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=DllSapIpDlg.h
ImplementationFile=DllSapIpDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_DLLSAPIP_DIALOG]
Type=1
Class=CDllSapIpDlg
ControlCount=6
Control1=IDC_EDIT_PROCESSNAME,edit,1350631552
Control2=IDC_EDIT_DLL,edit,1350631552
Control3=IDC_BUTTON_INJECTION,button,1342242817
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_BUTTON_DLLPATH,button,1342242816

