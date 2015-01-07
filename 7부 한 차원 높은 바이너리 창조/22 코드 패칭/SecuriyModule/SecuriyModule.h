// SecuriyModule.h : main header file for the SECURIYMODULE application
//

#if !defined(AFX_SECURIYMODULE_H__7738B785_AE5A_4E40_987B_A8ADA6BF0F86__INCLUDED_)
#define AFX_SECURIYMODULE_H__7738B785_AE5A_4E40_987B_A8ADA6BF0F86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSecuriyModuleApp:
// See SecuriyModule.cpp for the implementation of this class
//

class CSecuriyModuleApp : public CWinApp
{
public:
	CSecuriyModuleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSecuriyModuleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSecuriyModuleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECURIYMODULE_H__7738B785_AE5A_4E40_987B_A8ADA6BF0F86__INCLUDED_)
