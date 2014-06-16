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
#include <discovery.h>
#include <yamicontainer.h>
#include <boost/thread.hpp>
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
  : home_system::service("player"),
  m_buffer(104857600)
{
  m_subtype = MEDIASUBTYPE_NULL;
}

CLibraryStream::~CLibraryStream()
{
  Clear();
}

void CLibraryStream::Clear()
{
  m_pos = m_len = 0;
}

void CLibraryStream::Append(BYTE* buff, int len)
{
  CAutoLock cAutoLock(&m_csLock);

  for (int i = 0; i < len; i++)
  {
    m_buffer.push_back(buff[i]);
  }

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
    params.set_string("destination", name());
    params.set_string("endpoint", YC.endpoint());

    std::auto_ptr<yami::outgoing_message> message(AGENT.send(DISCOVERY.get("dvb-source"), "dvb-source", "create_streaming_session", params));

    message->wait_for_completion();

    if (message->get_state() == yami::replied)
    {
      int session_id = message->get_reply().get_integer("session");
      params.clear();

      params.set_integer("session", session_id);

      AGENT.send(DISCOVERY.get("dvb-source"), "dvb-source", "start_streaming_session", params);
    }
    else
    {
      return false;
    }
  }
  catch (const home_system::service_not_found&)
  {
    return false;
  }

  if (sl.GetCount() != 2 || FAILED(GUIDFromCString(CString(sl.GetTail()), m_subtype)))
  {
    m_subtype = MEDIASUBTYPE_MPEG2_TRANSPORT;    // TODO: detect subtype
  }

  m_len = 0;
  while(m_len < 1048576)
  {
    yami::parameters params;
    params.set_integer("len", 65535);
    params.set_long_long("pos", m_len);
    std::auto_ptr<yami::outgoing_message> message(AGENT.send(DISCOVERY.get("dvb-source"), "dvb-source", "read", params));
    message->wait_for_completion();
    size_t len = 0;
    PBYTE buf = (PBYTE)message->get_reply().get_binary("payload", len);
    m_len += len;
    for (size_t i = 0; i < len; ++i)
    {
      m_buffer.push_back(buf[i]);
    }

    Sleep(10);
  }

  boost::thread(boost::ref(*this));

  return true;
}

void CLibraryStream::operator()()
{
  while(true)
  {
    yami::parameters params;
    params.set_integer("len", 655350);
    params.set_long_long("pos", m_len);
    std::auto_ptr<yami::outgoing_message> message(AGENT.send(DISCOVERY.get("dvb-source"), "dvb-source", "read", params));
    message->wait_for_completion();
    size_t len = 0;
    PBYTE buf = (PBYTE)message->get_reply().get_binary("payload", len);

    for (size_t i = 0; i < len; ++i)
    {
      m_buffer.push_back(buf[i]);
      m_len++;
    }

    Sleep(10);
  }
}

HRESULT CLibraryStream::SetPointer(LONGLONG llPos)
{
  CAutoLock cAutoLock(&m_csLock);

  if (llPos >= m_len)
  {
    return E_FAIL;
  }

  m_pos = llPos;

  return S_OK;
}

HRESULT CLibraryStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
  *pdwBytesRead = dwBytesToRead;

  while (dwBytesToRead > m_len - m_pos)
  {
    Sleep(1);
  }

  {
    CAutoLock cAutoLock(&m_csLock);
    for (size_t i = 0; i < *pdwBytesRead; ++i)
    {
      *pbBuffer = m_buffer[m_pos + i];
      pbBuffer++;
    }
  }

  return S_OK;
}

LONGLONG CLibraryStream::Size(LONGLONG* pSizeAvailable)
{
  CAutoLock cAutoLock(&m_csLock);
  if (pSizeAvailable)
  {
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
    }
    catch (const std::exception& e)
    {
    }
  }
  else
  {
    home_system::service::on_msg(im);
  }
}
