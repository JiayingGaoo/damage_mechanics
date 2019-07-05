// single_nn.h : main header file for the SINGLE_NN application
//

#if !defined(AFX_SINGLE_NN_H__961BE6BE_6174_4718_9D44_00BF84241AA8__INCLUDED_)
#define AFX_SINGLE_NN_H__961BE6BE_6174_4718_9D44_00BF84241AA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSingle_nnApp:
// See single_nn.cpp for the implementation of this class
//

class CSingle_nnApp : public CWinApp
{
public:
	CSingle_nnApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSingle_nnApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSingle_nnApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SINGLE_NN_H__961BE6BE_6174_4718_9D44_00BF84241AA8__INCLUDED_)
