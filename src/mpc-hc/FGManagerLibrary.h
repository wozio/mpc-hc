/*
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

//#include <bdatypes.h>
//#include <bdamedia.h>
//#include <bdaiface.h>
#include "FGManager.h"

class CLibraryStream2
{
public:
    CLibraryStream2()
        : m_ulMappedPID(0)
        , m_pmt(0)
        , m_bFindExisting(0) {
    }

    CLibraryStream2(LPWSTR strName, const AM_MEDIA_TYPE* pmt, bool bFindExisting = false, MEDIA_SAMPLE_CONTENT nMsc = MEDIA_ELEMENTARY_STREAM) :
        m_Name(strName),
        m_bFindExisting(bFindExisting),
        m_pmt(pmt),
        m_nMsc(nMsc),
        m_ulMappedPID(0)
    {}

    LPWSTR GetName() { return m_Name; };
    const AM_MEDIA_TYPE* GetMediaType() { return m_pmt; };
    bool GetFindExisting() const { return m_bFindExisting; };
    IBaseFilter* GetFilter() { return m_pFilter; };

    void SetPin(IPin* pPin) {
        CComPtr<IPin> pPinOut;
        PIN_INFO PinInfo;

        m_pMap = pPin;
        if (m_pMap &&
                SUCCEEDED(pPin->ConnectedTo(&pPinOut)) &&
                SUCCEEDED(pPinOut->QueryPinInfo(&PinInfo))) {
            m_pFilter.Attach(PinInfo.pFilter);
        }
    }

    HRESULT Map(ULONG ulPID) {
        CheckPointer(m_pMap, E_UNEXPECTED);
        ClearMaps();
        m_ulMappedPID = ulPID;
        return m_pMap->MapPID(1, &ulPID, m_nMsc);
    }

    HRESULT Unmap(ULONG ulPID) {
        CheckPointer(m_pMap, E_UNEXPECTED);
        m_ulMappedPID = 0;
        return m_pMap->UnmapPID(1, &ulPID);
    }

    ULONG GetMappedPID() const { return m_ulMappedPID; }

private:
    CComQIPtr<IMPEG2PIDMap> m_pMap;
    CComPtr<IBaseFilter>    m_pFilter;
    const AM_MEDIA_TYPE*    m_pmt;
    bool                    m_bFindExisting;
    LPWSTR                  m_Name;
    MEDIA_SAMPLE_CONTENT    m_nMsc;
    ULONG                   m_ulMappedPID;

    void ClearMaps() {
        HRESULT hr;
        CComPtr<IEnumPIDMap> pEnumMap;

        if (SUCCEEDED(hr = m_pMap->EnumPIDMap(&pEnumMap))) {
            PID_MAP maps[8];
            ULONG   nbPids = 0;

            if (pEnumMap->Next(_countof(maps), maps, &nbPids) == S_OK) {
                for (ULONG i = 0; i < nbPids; i++) {
                    ULONG pid = maps[i].ulPID;

                    m_pMap->UnmapPID(1, &pid);
                }
            }
        }
    }
};

class CFGManagerLibrary : public CFGManagerPlayer, /*IBDATuner, */IAMStreamSelect
{
public:
    CFGManagerLibrary(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd);
    ~CFGManagerLibrary();

    // IGraphBuilder
    STDMETHODIMP RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList);

    // IFilterGraph
    STDMETHODIMP ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt);

    // IAMStreamSelect
    STDMETHODIMP Count(DWORD* pcStreams);
    STDMETHODIMP Enable(long lIndex, DWORD dwFlags);
    STDMETHODIMP Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk);

    DECLARE_IUNKNOWN;
    //STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
    //STDMETHODIMP UpdatePSI(PresentFollowing& NowNext);

private:

    CAtlMap<DVB_STREAM_TYPE, CLibraryStream2> m_DVBStreams;

    DVB_STREAM_TYPE m_nCurVideoType;
    DVB_STREAM_TYPE m_nCurAudioType;
    CString         m_BDANetworkProvider;
    bool            m_fHideWindow;
    CComPtr<IPin>   m_pPin_h264;

    HRESULT         ConnectFilters(IBaseFilter* pOutFiter, IBaseFilter* pInFilter);
    HRESULT         CreateMicrosoftDemux(IBaseFilter* pReceiver, CComPtr<IBaseFilter>& pMpeg2Demux);
    HRESULT         ChangeState(FILTER_STATE niuy80Requested);
    FILTER_STATE    GetState();

    void Sleep(unsigned int mseconds) {
        clock_t goal = mseconds + clock();
        while (goal > clock()) {
            ;
        }
    }
};
