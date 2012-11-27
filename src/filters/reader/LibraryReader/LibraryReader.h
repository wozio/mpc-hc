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

#pragma once

#include <atlbase.h>
#include "AsyncReader/asyncio.h"
#include "AsyncReader/asyncrdr.h"
#include "../../../../../common/src/service.h"

#define LibraryReaderName L"MPC Library Reader"

class CLibraryStream : public CAsyncStream, public home_system::service
{
private:
    CCritSec m_csLock;

    class packet_t
    {
    public:
        BYTE* m_buff;
        __int64 m_start, m_end;
        packet_t(BYTE* p, __int64 start, __int64 end);
        virtual ~packet_t() {
            delete [] m_buff;
        }
    };

    int m_port;
    CString m_ip;
    SOCKET m_socket;
    GUID m_subtype;
    __int64 m_pos, m_len;
    bool m_drop;
    CAtlList<packet_t*> m_packets;

    void Clear();
    void Append(BYTE* buff, int len);

    enum { CMD_EXIT, CMD_RUN };
    DWORD ThreadProc();

    void on_msg(yami::incoming_message & im);

public:
    CLibraryStream();
    virtual ~CLibraryStream();

    bool Load(const WCHAR* fnw);
    const GUID& GetSubType() { return m_subtype; }

    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG* pSizeAvailable);
    DWORD Alignment();
    void Lock();
    void Unlock();
};

class __declspec(uuid("357E0F9C-0D74-45ED-899F-F1AC7CA6B0F4"))
    CLibraryReader
    : public CAsyncReader
    , public IFileSourceFilter
{
    CLibraryStream m_stream;
    CStringW m_fn;

public:
    CLibraryReader(IUnknown* pUnk, HRESULT* phr);
    ~CLibraryReader();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IFileSourceFilter
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);
};
