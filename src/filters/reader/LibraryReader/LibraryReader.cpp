/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "LibraryReader.h"
#include "../../../DSUtil/DSUtil.h"
#include "../../../../../common/src/discovery.h"
#include "../../../../../common/src/yamicontainer.h"
#include <sstream>

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CLibraryReader), LibraryReaderName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CLibraryReader>, NULL, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(_T("udp"), 0, _T("Source Filter"), CStringFromGUID(__uuidof(CLibraryReader)));
    SetRegKeyValue(_T("tévé"), 0, _T("Source Filter"), CStringFromGUID(__uuidof(CLibraryReader)));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    // TODO

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

#define BUFF_SIZE (256 * 1024)
#define BUFF_SIZE_FIRST (4 * BUFF_SIZE)

//
// CLibraryReader
//

CLibraryReader::CLibraryReader(IUnknown* pUnk, HRESULT* phr)
    : CAsyncReader(NAME("CLibraryReader"), pUnk, &m_stream, phr, __uuidof(this))
{
    if (phr) {
        *phr = S_OK;
    }
}

CLibraryReader::~CLibraryReader()
{
}

STDMETHODIMP CLibraryReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFileSourceFilter)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IFileSourceFilter

STDMETHODIMP CLibraryReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    if (!m_stream.Load(pszFileName)) {
        return E_FAIL;
    }

    m_fn = pszFileName;

    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = m_stream.GetSubType();
    m_mt = mt;

    return S_OK;
}

STDMETHODIMP CLibraryReader::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    if (!ppszFileName) {
        return E_POINTER;
    }

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// CLibraryStream

CLibraryStream::CLibraryStream()
  : home_system::service("player")
{
    m_port = 0;
    m_socket = INVALID_SOCKET;
    m_subtype = MEDIASUBTYPE_NULL;
}

CLibraryStream::~CLibraryStream()
{
    Clear();
}

void CLibraryStream::Clear()
{
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    while (!m_packets.IsEmpty()) {
        delete m_packets.RemoveHead();
    }
    m_pos = m_len = 0;
    m_drop = false;
}

void CLibraryStream::Append(BYTE* buff, int len)
{
    CAutoLock cAutoLock(&m_csLock);

    if (m_packets.GetCount() > 1) {
        __int64 size = m_packets.GetTail()->m_end - m_packets.GetHead()->m_start;

        if (!m_drop && (m_pos >= BUFF_SIZE_FIRST && size >= BUFF_SIZE_FIRST || size >= 2 * BUFF_SIZE_FIRST)) {
            m_drop = true;
            TRACE(_T("DROP ON\n"));
        } else if (m_drop && size <= BUFF_SIZE_FIRST) {
            m_drop = false;
            TRACE(_T("DROP OFF\n"));
        }

        if (m_drop) {
            return;
        }
    }

    m_packets.AddTail(DNew packet_t(buff, m_len, m_len + len));
    m_len += len;
}

bool CLibraryStream::Load(const WCHAR* fnw)
{
    Clear();

    CStringW url = CStringW(fnw);

    CAtlList<CStringW> sl;
    Explode(url, sl, ':');

    CStringW protocol = sl.RemoveHead();
    if (protocol != L"hsl") return false;

    try
    {
      yami::parameters params;
                
      params.set_long_long("local_channel", 37830072008705);
      params.set_string("destination", "player");
      params.set_string("endpoint", YC.endpoint_);

      std::auto_ptr<yami::outgoing_message> message(YC.agent_.send(DISCOVERY.get("dvb-source"), "dvb-source", "create_streaming_session", params));
                
      message->wait_for_completion(1000);
                
      if (message->get_state() == yami::replied)
      {
        int session_id = message->get_reply().get_integer("session");
        params.clear();
                
        params.set_integer("session", session_id);

        YC.agent_.send(DISCOVERY.get("dvb-source"), "dvb-source", "start_streaming_session", params);
      }
      else
      {
        return false;
      }
    }
    catch (const home_system::service_not_found& e)
    {
      TRACE(_T("Service not found"));
      return false;
    }


    if (sl.GetCount() != 2 || FAILED(GUIDFromCString(CString(sl.GetTail()), m_subtype))) {
        m_subtype = MEDIASUBTYPE_NULL;    // TODO: detect subtype
    }

    clock_t start = clock();
    while (clock() - start < 3000 && m_len < 1000000) {
        Sleep(100);
    }

    return true;
}

HRESULT CLibraryStream::SetPointer(LONGLONG llPos)
{
    CAutoLock cAutoLock(&m_csLock);

    if (m_packets.IsEmpty() && llPos != 0
            || !m_packets.IsEmpty() && llPos < m_packets.GetHead()->m_start
            || !m_packets.IsEmpty() && llPos > m_packets.GetTail()->m_end) {
        TRACE(_T("CLibraryStream: SetPointer error\n"));
        return E_FAIL;
    }

    m_pos = llPos;

    return S_OK;
}

HRESULT CLibraryStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
    CAutoLock cAutoLock(&m_csLock);

    DWORD len = dwBytesToRead;
    BYTE* ptr = pbBuffer;

    while (len > 0 && !m_packets.IsEmpty()) {
        POSITION pos = m_packets.GetHeadPosition();
        while (pos && len > 0) {
            packet_t* p = m_packets.GetNext(pos);

            if (p->m_start <= m_pos && m_pos < p->m_end) {
                DWORD size;

                if (m_pos < p->m_start) {
                    ASSERT(0);
                    size = (DWORD)min(len, p->m_start - m_pos);
                    memset(ptr, 0, size);
                } else {
                    size = (DWORD)min(len, p->m_end - m_pos);
                    memcpy(ptr, &p->m_buff[m_pos - p->m_start], size);
                }

                m_pos += size;

                ptr += size;
                len -= size;
            }

            if (p->m_end <= m_pos - 2048 && BUFF_SIZE_FIRST <= m_pos) {
                while (m_packets.GetHeadPosition() != pos) {
                    delete m_packets.RemoveHead();
                }
            }

        }
    }

    if (pdwBytesRead) {
        *pdwBytesRead = ptr - pbBuffer;
    }

    return S_OK;
}

LONGLONG CLibraryStream::Size(LONGLONG* pSizeAvailable)
{
    CAutoLock cAutoLock(&m_csLock);
    if (pSizeAvailable) {
        *pSizeAvailable = m_len;
    }
    return 0;
}

DWORD CLibraryStream::Alignment()
{
    return 1;
}

void CLibraryStream::Lock()
{
    m_csLock.Lock();
}

void CLibraryStream::Unlock()
{
    m_csLock.Unlock();
}

void CLibraryStream::on_msg(yami::incoming_message & im)
{
  if (im.get_message_name() == "stream_part")
  {
    try
    {
      size_t len = 0;
      const void* buf = im.get_parameters().get_binary("payload", len);
      Append((BYTE*)buf, len);
      std::wstringstream s;
      s << L"Appended " << len << L" bytes. Current buffer size: " << m_len << std::endl;
      TRACE(s.str().c_str());
    }
    catch (const std::exception& e)
    {
      TRACE(_T("CLibraryStream: Exception\n"));
    }
  }
  else
  {
    home_system::service::on_msg(im);
  }
}

CLibraryStream::packet_t::packet_t(BYTE* p, __int64 start, __int64 end)
    : m_start(start)
    , m_end(end)
{
    size_t size = (size_t)(end - start);
    m_buff = DNew BYTE[size];
    memcpy(m_buff, p, size);
}
