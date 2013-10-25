// SecuriyModuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SecuriyModule.h"
#include "SecuriyModuleDlg.h"
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEdit *g_pEdit;
BOOL g_bDetect = FALSE;

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
// CSecuriyModuleDlg dialog

CSecuriyModuleDlg::CSecuriyModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSecuriyModuleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSecuriyModuleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSecuriyModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecuriyModuleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSecuriyModuleDlg, CDialog)
	//{{AFX_MSG_MAP(CSecuriyModuleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSecuriyModuleDlg message handlers

BOOL CSecuriyModuleDlg::OnInitDialog()
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
	g_pEdit = (CEdit *)GetDlgItem(IDC_EDIT_COUNTER);

	if (FindWindow(NULL, _T("for Testversion")))
		g_bDetect = TRUE;

	DWORD threadid = 0;
	CloseHandle(CreateThread(NULL, 0, MonitorThread, NULL, 0, &threadid));
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSecuriyModuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSecuriyModuleDlg::OnPaint() 
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
HCURSOR CSecuriyModuleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL ScanFile(char *szFileName, DWORD dwPid)
{
	BOOL bDetect = FALSE;

	_tcslwr(szFileName);
	if (_tcsstr(szFileName, "olly"))
	{
		bDetect = TRUE;		

		HANDLE hAttackerProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (hAttackerProcess)
			TerminateProcess(hAttackerProcess, -1);
	}

	return bDetect;
}

DWORD CALLBACK MonitorThread(LPVOID lpParam)
{
	BOOL bDetect = FALSE; 
	DWORD loop = 0;

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
				if (ScanFile(pe32.szExeFile, pe32.th32ProcessID) && !g_bDetect)
				{
					MessageBox(NULL, _T("Hacking Detect !"), _T("caution"), MB_OK);
					
					bDetect = TRUE;
					break;
				}					
			} 
			while (Process32Next(hProcessSnap, &pe32)); 
		}
		
		CloseHandle (hProcessSnap);
		
		CString str;
		str.Format(_T("%d"), loop);
		g_pEdit->SetWindowText(str);
		
		Sleep(1000);
		loop++;
	}

	if (bDetect || g_bDetect)
		TerminateProcess(GetCurrentProcess(), -1);

	return 0;
}