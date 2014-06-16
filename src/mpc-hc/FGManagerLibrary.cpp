#include "stdafx.h"

#include <mpeg2data.h>

#include "moreuuids.h"
#include "FGManagerLibrary.h"
#include "MainFrm.h"
#include "SysVersion.h"
#include "../filters/switcher/AudioSwitcher/AudioSwitcher.h"
#include "../filters/reader/LibraryReaderPush/LibraryReaderPush.h"

/// Format, Video MPEG2
static const MPEG2VIDEOINFO sMpv_fmt = {
  {
    // hdr
    {0, 0, 720, 576},           // rcSource
    {0, 0, 0, 0},               // rcTarget
    0,                          // dwBitRate
    0,                          // dwBitErrorRate
    0,                          // AvgTimePerFrame
    0,                          // dwInterlaceFlags
    0,                          // dwCopyProtectFlags
    0,                          // dwPictAspectRatioX
    0,                          // dwPictAspectRatioY
    {0},                        // dwControlFlag & dwReserved1
    0,                          // dwReserved2
    {
      // bmiHeader
      sizeof(BITMAPINFOHEADER),// biSize
        720,                    // biWidth
        576                     // biHeight
    }
    // implicitly sets the others fields to 0
  }
};

/// Media type, Video MPEG2
static const AM_MEDIA_TYPE mt_Mpv = {
  MEDIATYPE_Video,                // majortype
  MEDIASUBTYPE_MPEG2_VIDEO,       // subtype
  FALSE,                          // bFixedSizeSamples
  TRUE,                           // bTemporalCompression
  0,                              // lSampleSize
  FORMAT_MPEG2Video,              // formattype
  NULL,                           // pUnk
  sizeof(sMpv_fmt),               // cbFormat
  (LPBYTE)& sMpv_fmt              // pbFormat
};

#define FCC_h264 MAKEFOURCC('h', '2', '6', '4')

/// Format, Video H264
static const VIDEOINFOHEADER2 vih2_H264 = {
  {0, 0, 0, 0},                   // rcSource
  {0, 0, 0, 0},                   // rcTarget
  0,                              // dwBitRate,
  0,                              // dwBitErrorRate
  0,                              // AvgTimePerFrame
  0,                              // dwInterlaceFlags
  0,                              // dwCopyProtectFlags
  0,                              // dwPictAspectRatioX
  0,                              // dwPictAspectRatioY
  {0},                            // dwControlFlag & dwReserved1
  0,                              // dwReserved2
  {
    // bmiHeader
    sizeof(BITMAPINFOHEADER),   // biSize
      //720,                        // biWidth
      //576,                        // biHeight
      1920,                       // biWidth
      1080,                       // biHeight
      0,                          // biPlanes
      0,                          // biBitCount
      FCC_h264                    // biCompression
  }
  // implicitly sets the others fields to 0
};

/// Media type, Video H264
static const AM_MEDIA_TYPE mt_H264 = {
  MEDIATYPE_Video,                // majortype
  MEDIASUBTYPE_H264,              // subtype
  FALSE,                          // bFixedSizeSamples
  TRUE,                           // bTemporalCompression
  1,                              // lSampleSize
  FORMAT_VideoInfo2,              // formattype
  NULL,                           // pUnk
  sizeof(vih2_H264),              // cbFormat
  (LPBYTE)& vih2_H264             // pbFormat
};

/// Format, Audio (common)
static const WAVEFORMATEX wf_Audio = {
  WAVE_FORMAT_PCM,                // wFormatTag
  2,                              // nChannels
  48000,                          // nSamplesPerSec
  4 * 48000,                      // nAvgBytesPerSec
  4,                              // nBlockAlign
  16,                             // wBitsPerSample
  0                               // cbSize
};

/// Media type, Audio MPEG2
static const AM_MEDIA_TYPE mt_Mpa = {
  MEDIATYPE_Audio,                // majortype
  MEDIASUBTYPE_MPEG2_AUDIO,       // subtype
  TRUE,                           // bFixedSizeSamples
  FALSE,                          // bTemporalCompression
  0,                              // lSampleSize
  FORMAT_WaveFormatEx,            // formattype
  NULL,                           // pUnk
  sizeof(wf_Audio),               // cbFormat
  (LPBYTE)& wf_Audio              // pbFormat
};

/// Media type, Audio AC3
static const AM_MEDIA_TYPE mt_Ac3 = {
  MEDIATYPE_Audio,                // majortype
  MEDIASUBTYPE_DOLBY_AC3,         // subtype
  TRUE,                           // bFixedSizeSamples
  FALSE,                          // bTemporalCompression
  0,                              // lSampleSize
  FORMAT_WaveFormatEx,            // formattype
  NULL,                           // pUnk
  sizeof(wf_Audio),               // cbFormat
  (LPBYTE)& wf_Audio,             // pbFormat
};

/// Media type, Audio EAC3
static const AM_MEDIA_TYPE mt_Eac3 = {
  MEDIATYPE_Audio,                // majortype
  MEDIASUBTYPE_DOLBY_DDPLUS,      // subtype
  TRUE,                           // bFixedSizeSamples
  FALSE,                          // bTemporalCompression
  0,                              // lSampleSize
  FORMAT_WaveFormatEx,            // formattype
  NULL,                           // pUnk
  sizeof(wf_Audio),               // cbFormat
  (LPBYTE)& wf_Audio,             // pbFormat
};

static const SUBTITLEINFO SubFormat = { 0, "", L"" };

/// Media type, subtitle
static const AM_MEDIA_TYPE mt_Subtitle = {
  MEDIATYPE_Subtitle,             // majortype
  MEDIASUBTYPE_DVB_SUBTITLES,     // subtype
  FALSE,                          // bFixedSizeSamples
  FALSE,                          // bTemporalCompression
  0,                              // lSampleSize
  FORMAT_None,                    // formattype
  NULL,                           // pUnk
  sizeof(SubFormat),              // cbFormat
  (LPBYTE)& SubFormat             // pbFormat
};

CFGManagerLibrary::CFGManagerLibrary(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd)
  : CFGManagerPlayer(pName, pUnk, hWnd)
{
  m_DVBStreams[DVB_MPV]  = CLibraryStream2(L"mpv",  &mt_Mpv);
  m_DVBStreams[DVB_H264] = CLibraryStream2(L"h264", &mt_H264);
  m_DVBStreams[DVB_MPA]  = CLibraryStream2(L"mpa",  &mt_Mpa);
  m_DVBStreams[DVB_AC3]  = CLibraryStream2(L"ac3",  &mt_Ac3);
  m_DVBStreams[DVB_EAC3] = CLibraryStream2(L"eac3", &mt_Eac3);

  // Warning : MEDIA_ELEMENTARY_STREAM didn't work for subtitles with Windows XP!
  if (SysVersion::IsVistaOrLater()) {
    m_DVBStreams[DVB_SUB] = CLibraryStream2(L"sub", &mt_Subtitle/*, false, MEDIA_TRANSPORT_PAYLOAD*/);
  } else {
    m_DVBStreams[DVB_SUB] = CLibraryStream2(L"sub", &mt_Subtitle, false, MEDIA_TRANSPORT_PAYLOAD);
  }

  m_nCurVideoType = DVB_H264;
  m_nCurAudioType = DVB_EAC3;
  m_fHideWindow = false;

  // Hack : remove audio switcher !
  POSITION pos = m_transform.GetHeadPosition();
  while (pos) {
    CFGFilter* pFGF = m_transform.GetAt(pos);
    if (pFGF->GetCLSID() == __uuidof(CAudioSwitcherFilter)) {
      m_transform.RemoveAt(pos);
      delete pFGF;
      break;
    }
    m_transform.GetNext(pos);
  }
}

CFGManagerLibrary::~CFGManagerLibrary()
{
  m_DVBStreams.RemoveAll();
}

HRESULT CFGManagerLibrary::ConnectFilters(IBaseFilter* pOutFilter, IBaseFilter* pInFilter)
{
  HRESULT hr = VFW_E_CANNOT_CONNECT;
  BeginEnumPins(pOutFilter, pEP, pOutPin) {
    if (S_OK == IsPinDirection(pOutPin, PINDIR_OUTPUT)
      && S_OK != IsPinConnected(pOutPin)) {
        BeginEnumPins(pInFilter, pEP, pInPin) {
          if (S_OK == IsPinDirection(pInPin, PINDIR_INPUT)
            && S_OK != IsPinConnected(pInPin)) {
              hr = this->ConnectDirect(pOutPin, pInPin, NULL);

#ifdef _DEBUG
              PIN_INFO InfoPinIn, InfoPinOut;
              FILTER_INFO InfoFilterIn, InfoFilterOut;
              pInPin->QueryPinInfo (&InfoPinIn);
              pOutPin->QueryPinInfo (&InfoPinOut);
              InfoPinIn.pFilter->QueryFilterInfo(&InfoFilterIn);
              InfoPinOut.pFilter->QueryFilterInfo(&InfoFilterOut);

              InfoPinIn.pFilter->Release();
              InfoPinOut.pFilter->Release();
#endif
              if (SUCCEEDED(hr)) {
                return hr;
              }
          }
        }
        EndEnumPins;
    }
  }
  EndEnumPins;

  return hr;
}

STDMETHODIMP CFGManagerLibrary::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
  HRESULT hr;
  const CAppSettings& s = AfxGetAppSettings();

  CFGFilter* pFGF = new CFGFilterInternal<CLibraryReaderPush>();
  CComPtr<IBaseFilter> pSource;
  CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
  pFGF->Create(&pSource, pUnks);
  if (FAILED(hr = AddFilter(pSource, L"Library Push Source")))
  {
    return VFW_E_CANNOT_LOAD_SOURCE_FILTER;
  }

  // MPEG 2 demux
  CComPtr<IBaseFilter> pMpeg2Demux;
  // Create Mpeg2 demux
  if (FAILED(hr = CreateMicrosoftDemux(pSource, pMpeg2Demux)))
  {
    return hr;
  }

  return S_OK;
}

STDMETHODIMP CFGManagerLibrary::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
  // Bypass CFGManagerPlayer limitation (IMediaSeeking for Mpeg2 demux)
  return CFGManagerCustom::ConnectDirect(pPinOut, pPinIn, pmt);
}

//STDMETHODIMP CFGManagerLibrary::SetChannel(int nChannelPrefNumber)
//{
//    HRESULT hr = E_INVALIDARG;
//    CAppSettings& s = AfxGetAppSettings();
//    CDVBChannel* pChannel = s.FindChannelByPref(nChannelPrefNumber);
//
//    if (pChannel != NULL) {
//        hr = SetChannelInternal(pChannel);
//
//        if (SUCCEEDED(hr)) {
//            s.nDVBLastChannel = nChannelPrefNumber;
//        }
//    }
//
//    return hr;
//}
//
//STDMETHODIMP CFGManagerLibrary::SetAudio(int nAudioIndex)
//{
//    return E_NOTIMPL;
//}
//
//STDMETHODIMP CFGManagerLibrary::SetFrequency(ULONG freq)
//{
//    HRESULT hr;
//    const CAppSettings& s = AfxGetAppSettings();
//    CheckPointer(m_pBDAControl, E_FAIL);
//    CheckPointer(m_pBDAFreq, E_FAIL);
//
//    CheckAndLog(m_pBDAControl->StartChanges(), _T("BDA: Setfrequency StartChanges"));
//    CheckAndLog(m_pBDAFreq->put_Bandwidth(s.iBDABandwidth), _T("BDA: Setfrequency put_Bandwidth"));
//    CheckAndLog(m_pBDAFreq->put_Frequency(freq), _T("BDA: Setfrequency put_Frequency"));
//    CheckAndLog(m_pBDAControl->CheckChanges(), _T("BDA: Setfrequency CheckChanges"));
//    CheckAndLog(m_pBDAControl->CommitChanges(), _T("BDA: Setfrequency CommitChanges"));
//
//    return hr;
//}
//
//STDMETHODIMP CFGManagerLibrary::Scan(ULONG ulFrequency, HWND hWnd)
//{
//    CMpeg2DataParser Parser(m_DVBStreams[DVB_PSI].GetFilter());
//
//    Parser.ParseSDT(ulFrequency);
//    Parser.ParsePAT();
//    Parser.ParseNIT();
//
//    POSITION pos = Parser.Channels.GetStartPosition();
//    while (pos) {
//        CDVBChannel& Channel = Parser.Channels.GetNextValue(pos);
//        if (Channel.HasName()) {
//            ::SendMessage(hWnd, WM_TUNER_NEW_CHANNEL, 0, (LPARAM)(LPCTSTR)Channel.ToString());
//        }
//    }
//
//    return S_OK;
//}
//
//STDMETHODIMP CFGManagerLibrary::GetStats(BOOLEAN& bPresent, BOOLEAN& bLocked, LONG& lStrength, LONG& lQuality)
//{
//    HRESULT hr;
//    CheckPointer(m_pBDAStats, E_UNEXPECTED);
//
//    CheckNoLog(m_pBDAStats->get_SignalPresent(&bPresent));
//    CheckNoLog(m_pBDAStats->get_SignalLocked(&bLocked));
//    CheckNoLog(m_pBDAStats->get_SignalStrength(&lStrength));
//    CheckNoLog(m_pBDAStats->get_SignalQuality(&lQuality));
//
//    return S_OK;
//}

// IAMStreamSelect
STDMETHODIMP CFGManagerLibrary::Count(DWORD* pcStreams)
{
  /*CheckPointer(pcStreams, E_POINTER);
  CAppSettings& s = AfxGetAppSettings();
  CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
  */
  *pcStreams = 0;

  /*if (pChannel != 0) {
  *pcStreams = pChannel->GetAudioCount() + pChannel->GetSubtitleCount();
  }
  */
  return S_OK;
}

STDMETHODIMP CFGManagerLibrary::Enable(long lIndex, DWORD dwFlags)
{
  HRESULT hr = E_INVALIDARG;
  /*CAppSettings& s = AfxGetAppSettings();
  CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
  DVBStreamInfo* pStreamInfo = NULL;
  CDVBStream* pStream = NULL;
  FILTER_STATE nState;

  if (pChannel) {
  if (lIndex >= 0 && lIndex < pChannel->GetAudioCount()) {
  pStreamInfo = pChannel->GetAudio(lIndex);
  pStream = &m_DVBStreams[pStreamInfo->Type];
  if (pStream && pStreamInfo) {
  nState = GetState();
  if (m_nCurAudioType != pStreamInfo->Type) {
  SwitchStream(m_nCurAudioType, pStreamInfo->Type);
  }
  pStream->Map(pStreamInfo->PID);
  ChangeState((FILTER_STATE)nState);

  hr = S_OK;
  }
  } else if (lIndex > 0 && lIndex < pChannel->GetAudioCount() + pChannel->GetSubtitleCount()) {
  pStreamInfo = pChannel->GetSubtitle(lIndex - pChannel->GetAudioCount());

  if (pStreamInfo) {
  m_DVBStreams[DVB_SUB].Map(pStreamInfo->PID);
  hr = S_OK;
  }
  }
  }*/

  return hr;
}

STDMETHODIMP CFGManagerLibrary::Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk)
{
  HRESULT hr = E_INVALIDARG;
  //CAppSettings& s = AfxGetAppSettings();
  //CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
  //DVBStreamInfo* pStreamInfo = NULL;
  //CDVBStream* pStream = NULL;
  //CDVBStream* pCurrentStream = NULL;

  //if (pChannel) {
  //    if (lIndex >= 0 && lIndex < pChannel->GetAudioCount()) {
  //        pCurrentStream = &m_DVBStreams[m_nCurAudioType];
  //        pStreamInfo = pChannel->GetAudio(lIndex);
  //        if (pStreamInfo) {
  //            pStream = &m_DVBStreams[pStreamInfo->Type];
  //        }
  //        if (pdwGroup) {
  //            *pdwGroup = 1;    // Audio group
  //        }
  //    } else if (lIndex > 0 && lIndex < pChannel->GetAudioCount() + pChannel->GetSubtitleCount()) {
  //        pCurrentStream = &m_DVBStreams[DVB_SUB];
  //        pStreamInfo = pChannel->GetSubtitle(lIndex - pChannel->GetAudioCount());
  //        if (pStreamInfo) {
  //            pStream = &m_DVBStreams[pStreamInfo->Type];
  //        }
  //        if (pdwGroup) {
  //            *pdwGroup = 2;    // Subtitle group
  //        }
  //    }

  //    if (pStreamInfo && pStream && pCurrentStream) {
  //        if (ppmt) {
  //            *ppmt = CreateMediaType(pStream->GetMediaType());
  //        }
  //        if (pdwFlags) {
  //            *pdwFlags = (pCurrentStream->GetMappedPID() == pStreamInfo->PID) ? AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE : 0;
  //        }
  //        if (plcid) {
  //            *plcid = pStreamInfo->GetLCID();
  //        }
  //        if (ppObject) {
  //            *ppObject = NULL;
  //        }
  //        if (ppUnk) {
  //            *ppUnk = NULL;
  //        }
  //        if (ppszName) {
  //            CStringW str;

  //            str = StreamTypeToName(pStreamInfo->PesType);

  //            *ppszName = (WCHAR*)CoTaskMemAlloc((str.GetLength() + 1) * sizeof(WCHAR));
  //            if (*ppszName == NULL) {
  //                return E_OUTOFMEMORY;
  //            }
  //            wcscpy_s(*ppszName, str.GetLength() + 1, str);
  //        }

  //        hr = S_OK;
  //    }
  //}

  return hr;
}

//STDMETHODIMP CFGManagerLibrary::NonDelegatingQueryInterface(REFIID riid, void** ppv)
//{
//  CheckPointer(ppv, E_POINTER);
//
//  return
//    //QI(IAMStreamSelect)
//    __super::NonDelegatingQueryInterface(riid, ppv);
//}

HRESULT CFGManagerLibrary::CreateMicrosoftDemux(IBaseFilter* pSource, CComPtr<IBaseFilter>& pMpeg2Demux)
{
  CComPtr<IMpeg2Demultiplexer> pDemux;
  HRESULT hr;

  CheckNoLog(pMpeg2Demux.CoCreateInstance(CLSID_MPEG2Demultiplexer, NULL, CLSCTX_INPROC_SERVER));
  CheckNoLog(AddFilter(pMpeg2Demux, _T("MPEG-2 Demultiplexer")));
  CheckNoLog(ConnectFilters(pSource, pMpeg2Demux));
  CheckNoLog(pMpeg2Demux->QueryInterface(IID_IMpeg2Demultiplexer, (void**)&pDemux));

  // Cleanup unnecessary pins
  //for (int i=0; i<6; i++)
  //{
  //  CStringW strPin;
  //  strPin.Format(L"%d", i);
  //  pDemux->DeleteOutputPin((LPWSTR)(LPCWSTR)strPin);
  //}

  POSITION pos = m_DVBStreams.GetStartPosition();
  while (pos) {
    CComPtr<IPin> pPin;
    DVB_STREAM_TYPE nType = m_DVBStreams.GetNextKey(pos);
    CLibraryStream2& Stream = m_DVBStreams[nType];

    if (nType != DVB_EPG) { // Hack: DVB_EPG not required
      if (!Stream.GetFindExisting() ||
        (pPin = FindPin(pMpeg2Demux, PINDIR_OUTPUT, Stream.GetMediaType())) == NULL) {
          CheckNoLog(pDemux->CreateOutputPin((AM_MEDIA_TYPE*)Stream.GetMediaType(), Stream.GetName(), &pPin));
      }

      if (nType == m_nCurVideoType || nType == m_nCurAudioType) {
        CheckNoLog(Connect(pPin, NULL, true));
        Stream.SetPin(pPin);
      } else {
        /*CheckNoLog(Connect(pPin, NULL, false));
        Stream.SetPin(pPin);
        LOG("Filter connected to Demux for media type " << nType);*/
      }
      if (nType == DVB_H264) {
        m_pPin_h264 = pPin;  // Demux h264 output pin
      }
    }
  }

  m_DVBStreams[DVB_H264].Map(102);
  m_DVBStreams[DVB_EAC3].Map(104);

  return S_OK;
}

//HRESULT CFGManagerLibrary::SetChannelInternal(CDVBChannel* pChannel)
//{
//    HRESULT hr = S_OK;
//    bool fRadioToTV = false;
//
//    int nState = GetState();
//
//    if (pChannel->GetVideoPID() != 0) {
//        SwitchStream(m_nCurVideoType, pChannel->GetVideoType());
//        if (m_fHideWindow) {
//            fRadioToTV = true;
//        }
//    } else {
//        m_fHideWindow = true;
//        ((CMainFrame*)AfxGetMainWnd())->HideVideoWindow(m_fHideWindow);
//    }
//
//    SwitchStream(m_nCurAudioType, pChannel->GetDefaultAudioType());
//
//    // Re-connection needed for H264 to allow different formats of H264 channels
//    // Makes possible switching between H264 channels of different resolutions
//    // and/or between interlaced and progressive.
//
//    if (m_nCurVideoType == DVB_H264) {
//        CComPtr<IPin> pInPin;
//        m_pPin_h264->ConnectedTo(&pInPin);
//        m_pPin_h264->Disconnect();
//        pInPin->Disconnect();
//        ConnectDirect(m_pPin_h264, pInPin, NULL);
//    }
//
//    CheckNoLog(SetFrequency(pChannel->GetFrequency()));
//    if (pChannel->GetVideoPID() != 0) {
//        CheckNoLog(m_DVBStreams[m_nCurVideoType].Map(pChannel->GetVideoPID()));
//    }
//
//    CheckNoLog(m_DVBStreams[m_nCurAudioType].Map(pChannel->GetDefaultAudioPID()));
//
//    if (GetState() == State_Stopped) {
//        hr = ChangeState((FILTER_STATE)nState);
//    }
//
//    if (fRadioToTV) {
//        m_fHideWindow = false;
//        Sleep(1800);
//        ((CMainFrame*)AfxGetMainWnd())->HideVideoWindow(m_fHideWindow);
//    }
//
//    // TODO : remove sub later!
//    //  CheckNoLog(m_DVBStreams[DVB_SUB].Map (pChannel->GetDefaultSubtitlePID()));
//
//    return hr;
//}

//HRESULT CFGManagerLibrary::SwitchStream(DVB_STREAM_TYPE& nOldType, DVB_STREAM_TYPE nNewType)
//{
//    if ((nNewType != nOldType) || (nNewType == DVB_H264)) {
//        CComPtr<IBaseFilter> pFGOld = m_DVBStreams[nOldType].GetFilter();
//        CComPtr<IBaseFilter> pFGNew = m_DVBStreams[nNewType].GetFilter();
//        CComPtr<IPin> pOldOut = GetFirstPin(pFGOld, PINDIR_OUTPUT);
//        CComPtr<IPin> pInPin;
//        CComPtr<IPin> pNewOut = GetFirstPin(pFGNew,  PINDIR_OUTPUT);
//
//        if (GetState() != State_Stopped) {
//            ChangeState(State_Stopped);
//        }
//        pOldOut->ConnectedTo(&pInPin);
//        Disconnect(pOldOut);
//        Disconnect(pInPin);
//        ConnectDirect(pNewOut, pInPin, NULL);
//        nOldType = nNewType;
//    }
//    return S_OK;
//}
//

HRESULT CFGManagerLibrary::ChangeState(FILTER_STATE nRequested)
{
  HRESULT hr = S_OK;
  OAFilterState nState = nRequested + 1;

  CComPtr<IMediaControl> pMC;
  QueryInterface(__uuidof(IMediaControl), (void**) &pMC);
  pMC->GetState(500, &nState);
  if (nState != nRequested) {
    switch (nRequested) {
    case State_Stopped: {
      if (SUCCEEDED(hr = pMC->Stop())) {
        ((CMainFrame*)AfxGetMainWnd())->KillTimersStop();
      }
      return hr;
                        }
    case State_Paused: {
      return pMC->Pause();
                       }
    case State_Running: {
      int iCount = 0;
      hr = S_FALSE;
      while ((hr == S_FALSE) && (iCount++ < 10)) {
        hr = pMC->Run();
        if (hr == S_FALSE) {
          Sleep(50);
        }
      }
      if (SUCCEEDED(hr)) {
        ((CMainFrame*)AfxGetMainWnd())->SetTimersPlay();
      }
      return hr;
                        }
    }
  }
  return hr;
}

FILTER_STATE CFGManagerLibrary::GetState()
{
  CComPtr<IMediaControl> pMC;
  OAFilterState nState;
  QueryInterface(__uuidof(IMediaControl), (void**) &pMC);
  pMC->GetState(500, &nState);

  return (FILTER_STATE) nState;
}
