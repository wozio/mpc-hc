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
#include "../../../../../common/src/service.h"

#define LibraryReaderPushName L"MPC Library Push Reader"

class __declspec(uuid("77CB350B-5908-4DA2-87E3-765CB021EA08"))
    CLibraryReaderPush
    : public CSource
    //, public IFileSourceFilter
    //, public IAMFilterMiscFlags
{
    CStringW m_fn;

public:
    CLibraryReaderPush(IUnknown* pUnk, HRESULT* phr);
    virtual ~CLibraryReaderPush();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IFileSourceFilter
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

    // IAMFilterMiscFlags
    STDMETHODIMP_(ULONG) GetMiscFlags();

    // CBaseFilter
    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);
};

class CLibraryStreamPush
    : public CSourceStream,
      //public CSourceSeeking,
      public home_system::service
{
    CCritSec m_cSharedState;

public:
    CLibraryStreamPush(CLibraryReaderPush* pParent, HRESULT* phr);
    virtual ~CLibraryStreamPush();

    //STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT FillBuffer(IMediaSample* pSample);
    //HRESULT CheckConnect(IPin* pPin);
    HRESULT GetMediaType(CMediaType* pmt);

    //STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);

    void on_msg(yami::incoming_message & im);
};
