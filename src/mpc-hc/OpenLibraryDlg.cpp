// OpenLibraryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OpenLibraryDlg.h"
#include "discovery.h"

using namespace std;

// COpenLibraryDlg dialog

IMPLEMENT_DYNAMIC(COpenLibraryDlg, CDialog)

COpenLibraryDlg::COpenLibraryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenLibraryDlg::IDD, pParent)
{
}

COpenLibraryDlg::~COpenLibraryDlg()
{
}

void COpenLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST1, m_list);
}

BOOL COpenLibraryDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  map<string, string> services;
  DISCOVERY.get_all(services);
  map<string, string>::iterator i = services.begin();
  for (; i != services.end(); ++i)
  {
    CString s(i->first.c_str());
    m_list.AddString(s);
  }

  return FALSE;
}


BEGIN_MESSAGE_MAP(COpenLibraryDlg, CDialog)
  ON_BN_CLICKED(IDOK, &COpenLibraryDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// COpenLibraryDlg message handlers


void COpenLibraryDlg::OnBnClickedOk()
{
  // TODO: Add your control notification handler code here
  CDialog::OnOK();
}
