// DllSapIpDlg.h : header file
//

#if !defined(AFX_DLLSAPIPDLG_H__51C47F26_A5B7_4383_B92A_CECC72C39A54__INCLUDED_)
#define AFX_DLLSAPIPDLG_H__51C47F26_A5B7_4383_B92A_CECC72C39A54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDllSapIpDlg dialog

class CDllSapIpDlg : public CDialog
{
// Construction
public:
	CDllSapIpDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDllSapIpDlg)
	enum { IDD = IDD_DLLSAPIP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDllSapIpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDllSapIpDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonInjection();
	afx_msg void OnButtonDllpath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLLSAPIPDLG_H__51C47F26_A5B7_4383_B92A_CECC72C39A54__INCLUDED_)
