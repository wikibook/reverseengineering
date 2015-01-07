// DllSapIpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DllSapIp.h"
#include "DllSapIpDlg.h"
#include "TraceBC.h"

#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CButton *g_pButtonGoGoXing;

typedef struct injectionparam{
	TCHAR szTartExe[MAX_PATH];
	TCHAR szInjectionDll[MAX_PATH];
} INJECTION_PARAM;



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDllSapIpDlg dialog

CDllSapIpDlg::CDllSapIpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDllSapIpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDllSapIpDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDllSapIpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDllSapIpDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDllSapIpDlg, CDialog)
	//{{AFX_MSG_MAP(CDllSapIpDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INJECTION, OnButtonInjection)
	ON_BN_CLICKED(IDC_BUTTON_DLLPATH, OnButtonDllpath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDllSapIpDlg message handlers

BOOL CDllSapIpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	CEdit *pEditProcessName = (CEdit *)GetDlgItem(IDC_EDIT_PROCESSNAME);
	pEditProcessName->SetWindowText(_T("test.exe"));
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDllSapIpDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDllSapIpDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDllSapIpDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void InjectionDll(DWORD pid, LPCSTR dll)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (NULL == hProcess)
		return;
	
	LPVOID lpAddr = VirtualAllocEx(hProcess, NULL, strlen(dll)+1, MEM_COMMIT, PAGE_READWRITE);
	if (lpAddr)
	{
		WriteProcessMemory(hProcess, lpAddr, dll, strlen(dll), NULL);
		
		LPTHREAD_START_ROUTINE pfnLoadLibraryA = 
			(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), _T("LoadLibraryA"));
		if (pfnLoadLibraryA)
		{
			HANDLE hThread = CreateRemoteThread(hProcess,
				NULL,
				0,
				pfnLoadLibraryA,
				lpAddr,
				0,
				NULL);
			if (hThread)
			{
				WaitForSingleObject(hThread, INFINITE);
				CloseHandle(hThread);
			}
		}
		
		VirtualFreeEx(hProcess, lpAddr, 0, MEM_RELEASE);
	}
	
	CloseHandle(hProcess);
}

UINT InjectionThread(LPVOID pParam)
{
	INJECTION_PARAM *pInject = (INJECTION_PARAM *)pParam;
	BOOL bDetect = FALSE;

	while (1) 
	{
		if (bDetect)
			break;

		HANDLE hProcessSnap = NULL;
		PROCESSENTRY32 pe32      = {0,}; 
		
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
		
		if (hProcessSnap == INVALID_HANDLE_VALUE) 
			return 0; 
		
		pe32.dwSize = sizeof(PROCESSENTRY32); 
		
		if (Process32First(hProcessSnap, &pe32)) 
		{	
			do 
			{
				if (strcmpi(pe32.szExeFile, pInject->szTartExe) == 0)
				{				
					HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
					if (hProcess)
					{
						// Dll injection
						InjectionDll(pe32.th32ProcessID, pInject->szInjectionDll);

						g_pButtonGoGoXing->SetWindowText(_T("Injection Success"));

						bDetect = TRUE;
					}
					CloseHandle(hProcess);						
				}					
			} 
			while (Process32Next(hProcessSnap, &pe32)); 
		}
		
		CloseHandle (hProcessSnap);
		
		Sleep(500);
	}

	AfxMessageBox(_T("Injection Success"));

	delete [] pInject;
	g_pButtonGoGoXing->EnableWindow(TRUE);
	g_pButtonGoGoXing->SetWindowText(_T("GoGoXing"));
	
	return 0;
}

void CDllSapIpDlg::OnButtonInjection() 
{
	// TODO: Add your control notification handler code here
	g_pButtonGoGoXing = (CButton *)GetDlgItem(IDC_BUTTON_INJECTION);

	CEdit *pEditProcessName = (CEdit *)GetDlgItem(IDC_EDIT_PROCESSNAME);
	CEdit *pEditDllName = (CEdit *)GetDlgItem(IDC_EDIT_DLL);

	CString szProcessName;
	CString szDllName;
	pEditProcessName->GetWindowText(szProcessName);
	pEditDllName->GetWindowText(szDllName);

	if (szProcessName.IsEmpty() || szDllName.IsEmpty())
	{
		AfxMessageBox(_T("input name"));
		return;
	}

	
	if (!_tcschr(szDllName.GetBuffer(0), '\\'))
	{
		AfxMessageBox(_T("input dll full path"));
		return;
	}

	INJECTION_PARAM *pInject;
	pInject = new INJECTION_PARAM;

	_tcscpy(pInject->szTartExe, szProcessName.GetBuffer(0));
	_tcscpy(pInject->szInjectionDll, szDllName.GetBuffer(0));	

	g_pButtonGoGoXing->EnableWindow(FALSE);
	g_pButtonGoGoXing->SetWindowText(_T("Waiting process"));

	::AfxBeginThread(InjectionThread, (LPVOID)pInject);	
}

void CDllSapIpDlg::OnButtonDllpath() 
{
	// TODO: Add your control notification handler code here
	CFileDialog Dlg(TRUE, _T("DLL"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
		_T("All Files(*.*)|*.*||"), this);
	
	if (Dlg.DoModal() != IDOK)
		return;
	
	CString csSourceFile;
	
	csSourceFile = Dlg.GetPathName(); // Full °æ·Î

	CEdit *g_pEditExePath = (CEdit *)GetDlgItem(IDC_EDIT_DLL);
	g_pEditExePath->SetWindowText(csSourceFile);
}
