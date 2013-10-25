// MFCReversingDlg.h : header file
//

#if !defined(AFX_MFCREVERSINGDLG_H__1DFB67A7_17AD_4FDA_A827_8DA3E6D53684__INCLUDED_)
#define AFX_MFCREVERSINGDLG_H__1DFB67A7_17AD_4FDA_A827_8DA3E6D53684__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMFCReversingDlg dialog

class CMFCReversingDlg : public CDialog
{
// Construction
public:
	CMFCReversingDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMFCReversingDlg)
	enum { IDD = IDD_MFCREVERSING_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCReversingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMFCReversingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCREVERSINGDLG_H__1DFB67A7_17AD_4FDA_A827_8DA3E6D53684__INCLUDED_)
