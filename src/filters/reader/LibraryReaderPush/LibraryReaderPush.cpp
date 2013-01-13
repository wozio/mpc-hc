#include "stdafx.h"
#include "LibraryReaderPush.h"
#include "../../../DSUtil/DSUtil.h"
#include "../../../../../common/src/discovery.h"
#include "../../../../../common/src/yamicontainer.h"
#include "../../../../../common/src/logger.h"
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
  {&__uuidof(CLibraryReaderPush), LibraryReaderName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
  {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CLibraryReaderPush>, NULL, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
  SetRegKeyValue(_T("udp"), 0, _T("Source Filter"), CStringFromGUID(__uuidof(CLibraryReaderPush)));
  SetRegKeyValue(_T("tévé"), 0, _T("Source Filter"), CStringFromGUID(__uuidof(CLibraryReaderPush)));

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
// CLibraryReaderPush
//

CLibraryReaderPush::CLibraryReaderPush(IUnknown* pUnk, HRESULT* phr)
  : CSource(NAME("CLibraryReaderPush"), pUnk, __uuidof(this))
{
  LOG("CLibraryReaderPush Created");
  *phr = S_OK;
  if (!(DNew CLibraryStreamPush(this, phr)))
  {
    *phr = E_OUTOFMEMORY;
  }
}

CLibraryReaderPush::~CLibraryReaderPush()
{
  LOG("CLibraryReaderPush Destroyed");
}

STDMETHODIMP CLibraryReaderPush::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
  CheckPointer(ppv, E_POINTER);

  return
    QI(IAMFilterMiscFlags)
    __super::NonDelegatingQueryInterface(riid, ppv);
}

// IAMFilterMiscFlags

ULONG CLibraryReaderPush::GetMiscFlags()
{
    return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

STDMETHODIMP CLibraryReaderPush::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));
    wcscpy_s(pInfo->achName, LibraryReaderPushName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

// CLibraryStreamPush

CLibraryStreamPush::CLibraryStreamPush(CLibraryReaderPush* pParent, HRESULT* phr)
: CSourceStream(NAME("LibraryPushStream"), phr, pParent, L"Output"),
  //CSourceSeeking(NAME("LibraryPushStream"), (IPin*)this, phr, &m_cSharedState),
  home_system::service("player")
{
  LOG("CLibraryStreamPush Created");
}

CLibraryStreamPush::~CLibraryStreamPush()
{
  LOG("CLibraryStreamPush Destroyed");
}

HRESULT CLibraryStreamPush::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
  LOG("DecideBufferSize");
    //    CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = 18800;

    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CLibraryStreamPush::FillBuffer(IMediaSample* pSample)
{
  LOG("FillBuffer");
    HRESULT hr = S_OK;
    return hr;
}

HRESULT CLibraryStreamPush::GetMediaType(CMediaType* pmt)
{
  LOG("GetMediaType");
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    pmt->SetType(&MEDIATYPE_Stream);
    pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_TRANSPORT);

    return NOERROR;
}

void CLibraryStreamPush::on_msg(yami::incoming_message & im)
{
  LOG("On message: " << im.get_message_name().c_str());
  if (im.get_message_name() == "stream_part")
  {
    try
    {
      size_t len = 0;
      const void* buf = im.get_parameters().get_binary("payload", len);

      LOG("Received " << len << L" bytes");
    }
    catch (const std::exception& e)
    {
      LOG("CLibraryStreamPush: Exception: " << e.what());
    }
  }
  else
  {
    home_system::service::on_msg(im);
  }
}
