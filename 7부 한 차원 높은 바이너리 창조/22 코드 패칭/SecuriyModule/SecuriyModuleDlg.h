// SecuriyModuleDlg.h : header file
//

#if !defined(AFX_SECURIYMODULEDLG_H__EDD83297_13ED_45C5_A32E_92ACD3621230__INCLUDED_)
#define AFX_SECURIYMODULEDLG_H__EDD83297_13ED_45C5_A32E_92ACD3621230__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSecuriyModuleDlg dialog

class CSecuriyModuleDlg : public CDialog
{
// Construction
public:
	CSecuriyModuleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSecuriyModuleDlg)
	enum { IDD = IDD_SECURIYMODULE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSecuriyModuleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSecuriyModuleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECURIYMODULEDLG_H__EDD83297_13ED_45C5_A32E_92ACD3621230__INCLUDED_)

DWORD CALLBACK MonitorThread(LPVOID lpParam);
