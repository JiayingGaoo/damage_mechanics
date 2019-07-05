// single_nnDlg.h : header file
//

#if !defined(AFX_SINGLE_NNDLG_H__CC86FD5B_3602_4CDB_B17B_EEF1B367C552__INCLUDED_)
#define AFX_SINGLE_NNDLG_H__CC86FD5B_3602_4CDB_B17B_EEF1B367C552__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSingle_nnDlg dialog

class CSingle_nnDlg : public CDialog
{
// Construction
public:
	void ReadTrainData();
	CSingle_nnDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSingle_nnDlg)
	enum { IDD = IDD_SINGLE_NN_DIALOG };
	CString	m_train_data_file_path;
	int		m_input_dimen;
	int		m_hidden_dimen;
	int		m_output_dimen;
	double	m_learn_rate;
	int		m_activate_fun_id;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSingle_nnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSingle_nnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoadData();
	afx_msg void OnTrain();
	afx_msg void OnTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SINGLE_NNDLG_H__CC86FD5B_3602_4CDB_B17B_EEF1B367C552__INCLUDED_)
