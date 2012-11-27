#pragma once

#include <afxwin.h>
#include "resource.h"

// COpenLibraryDlg dialog

class COpenLibraryDlg : public CDialog
{
	DECLARE_DYNAMIC(COpenLibraryDlg)

public:
	COpenLibraryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COpenLibraryDlg();

// Dialog Data
	enum { IDD = IDD_OPENLIBRARY_DLG };

  CListBox m_list;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOk();
};
