/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageFileInfoClip.h"
#include <atlbase.h>
#include <qnetwork.h>
#include "DSUtil.h"
#include "WinAPIUtils.h"


// CPPageFileInfoClip dialog

IMPLEMENT_DYNAMIC(CPPageFileInfoClip, CPropertyPage)
CPPageFileInfoClip::CPPageFileInfoClip(CString path, IFilterGraph* pFG, IFileSourceFilter* pFSF)
    : CPropertyPage(CPPageFileInfoClip::IDD, CPPageFileInfoClip::IDD)
    , m_fn(path)
    , m_path(path)
    , m_clip(ResStr(IDS_AG_NONE))
    , m_author(ResStr(IDS_AG_NONE))
    , m_copyright(ResStr(IDS_AG_NONE))
    , m_rating(ResStr(IDS_AG_NONE))
    , m_location_str(ResStr(IDS_AG_NONE))
    , m_hIcon(nullptr)
{
    if (pFSF) {
        LPOLESTR pFN;
        if (SUCCEEDED(pFSF->GetCurFile(&pFN, nullptr))) {
            m_fn = pFN;
            CoTaskMemFree(pFN);
        }
    }

    bool fEmpty = true;
    BeginEnumFilters(pFG, pEF, pBF) {
        if (CComQIPtr<IAMMediaContent, &IID_IAMMediaContent> pAMMC = pBF) {
            CComBSTR bstr;
            if (SUCCEEDED(pAMMC->get_Title(&bstr)) && bstr.Length()) {
                m_clip = bstr.m_str;
                fEmpty = false;
            }
            bstr.Empty();
            if (SUCCEEDED(pAMMC->get_AuthorName(&bstr)) && bstr.Length()) {
                m_author = bstr.m_str;
                fEmpty = false;
            }
            bstr.Empty();
            if (SUCCEEDED(pAMMC->get_Copyright(&bstr)) && bstr.Length()) {
                m_copyright = bstr.m_str;
                fEmpty = false;
            }
            bstr.Empty();
            if (SUCCEEDED(pAMMC->get_Rating(&bstr)) && bstr.Length()) {
                m_rating = bstr.m_str;
                fEmpty = false;
            }
            bstr.Empty();
            if (SUCCEEDED(pAMMC->get_Description(&bstr)) && bstr.Length()) {
                m_desctext = bstr.m_str;
                fEmpty = false;
            }
            bstr.Empty();
            if (!fEmpty) {
                break;
            }
        }
    }
    EndEnumFilters;
}

CPPageFileInfoClip::~CPPageFileInfoClip()
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
    }
}

BOOL CPPageFileInfoClip::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_LBUTTONDBLCLK && pMsg->hwnd == m_location.m_hWnd && !m_location_str.IsEmpty()) {
        CString path = m_location_str;
        if (path[path.GetLength() - 1] != '\\') {
            path += _T("\\");
        }
        path += m_fn;

        if (ExploreToFile(path)) {
            return TRUE;
        }
    }

    m_tooltip.RelayEvent(pMsg);

    return __super::PreTranslateMessage(pMsg);
}


void CPPageFileInfoClip::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEFAULTICON, m_icon);
    DDX_Text(pDX, IDC_EDIT1, m_fn);
    DDX_Text(pDX, IDC_EDIT4, m_clip);
    DDX_Text(pDX, IDC_EDIT3, m_author);
    DDX_Text(pDX, IDC_EDIT2, m_copyright);
    DDX_Text(pDX, IDC_EDIT5, m_rating);
    DDX_Control(pDX, IDC_EDIT6, m_location);
    DDX_Control(pDX, IDC_EDIT7, m_desc);
}

#define SETPAGEFOCUS (WM_APP + 252) // arbitrary number, can be changed if necessary

BEGIN_MESSAGE_MAP(CPPageFileInfoClip, CPropertyPage)
    ON_MESSAGE(SETPAGEFOCUS, OnSetPageFocus)
END_MESSAGE_MAP()


// CPPageFileInfoClip message handlers

BOOL CPPageFileInfoClip::OnInitDialog()
{
    __super::OnInitDialog();

    if (m_path.IsEmpty()) {
        m_path = m_fn;
    }

    m_fn.TrimRight('/');
    int i = std::max(m_fn.ReverseFind('\\'), m_fn.ReverseFind('/'));
    if (i >= 0 && i < m_fn.GetLength() - 1) {
        m_location_str = m_fn.Left(i);
        m_fn = m_fn.Mid(i + 1);

        if (m_location_str.GetLength() == 2 && m_location_str[1] == ':') {
            m_location_str += '\\';
        }
    }

    m_hIcon = LoadIcon(m_fn, false);
    if (m_hIcon) {
        m_icon.SetIcon(m_hIcon);
    }

    m_location.SetWindowText(m_location_str);

    m_tooltip.Create(this, TTS_NOPREFIX | TTS_ALWAYSTIP);

    m_tooltip.SetDelayTime(TTDT_INITIAL, 0);
    m_tooltip.SetDelayTime(TTDT_AUTOPOP, 2500);
    m_tooltip.SetDelayTime(TTDT_RESHOW, 0);

    if (FileExists(m_path)) {
        m_tooltip.AddTool(&m_location, IDS_TOOLTIP_EXPLORE_TO_FILE);
    }

    m_desc.SetWindowText(m_desctext);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageFileInfoClip::OnSetActive()
{
    BOOL ret = __super::OnSetActive();

    PostMessage(SETPAGEFOCUS, 0, 0L);

    return ret;
}

LRESULT CPPageFileInfoClip::OnSetPageFocus(WPARAM wParam, LPARAM lParam)
{
    CPropertySheet* psheet = (CPropertySheet*) GetParent();
    psheet->GetTabControl()->SetFocus();

    return 0;
}