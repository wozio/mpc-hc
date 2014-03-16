#include "stdafx.h"
#include "LibraryReaderPush.h"
#include "../../../DSUtil/DSUtil.h"
#include "../../../../../common/src/discovery.h"
#include "../../../../../common/src/yamicontainer.h"
#include "../../../../../common/src/logger.h"
#include <thread>
#include <fstream>

#define BUFSIZE 18800

using namespace std;

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

// CLibraryStreamPush

CLibraryStreamPush::CLibraryStreamPush(CLibraryReaderPush* pParent, HRESULT* phr)
  : CSourceStream(NAME("LibraryPushStream"), phr, pParent, L"Output"),
  //CSourceSeeking(NAME("LibraryPushStream"), (IPin*)this, phr, &m_cSharedState),
  home_system::service("player"),
  buffer_(100),
  buffer_len_(0),
  buffer_pos_(0),
  tmp_len_(0)
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
  pProperties->cbBuffer = BUFSIZE;

  ALLOCATOR_PROPERTIES Actual;
  if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual)))
  {
    return hr;
  }

  if (Actual.cbBuffer < pProperties->cbBuffer)
  {
    return E_FAIL;
  }
  ASSERT(Actual.cBuffers == pProperties->cBuffers);

  return NOERROR;
}

HRESULT CLibraryStreamPush::FillBuffer(IMediaSample* pSample)
{
  while(buffer_len_ == 0 || buffer_pos_ == buffer_len_)
  {
    Sleep(1);
  }
  size_t size = pSample->GetSize();
  
  BYTE* buf;
  pSample->GetPointer(&buf);

  CAutoLock lock(&buffer_lock_);
  if (size > buffer_len_)
  {
    size = buffer_len_;
  }

  size_t read_size = size;

  while (read_size != 0)
  {
    size_t buffer_index = buffer_pos_ / BUFSIZE;
    size_t local_buf_pos = buffer_pos_ - buffer_index * BUFSIZE;
    size_t to_read;
    if (read_size >= BUFSIZE)
    {
      to_read = BUFSIZE - local_buf_pos;
    }
    else
    {
      to_read = read_size;
    }
    memcpy(buf, buffer_[buffer_index].get() + local_buf_pos, to_read);
    buffer_pos_ += to_read;
    read_size -= to_read;
    buf += to_read;
  }
  pSample->SetActualDataLength(size);

  //LOG("buffer_pos="<<buffer_pos_);

  return S_OK;
}

HRESULT CLibraryStreamPush::GetMediaType(CMediaType* pmt)
{
  LOG("GetMediaType");
  CAutoLock lock(m_pFilter->pStateLock());

  pmt->SetType(&MEDIATYPE_Stream);
  pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_TRANSPORT);

  return NOERROR;
}

HRESULT CLibraryStreamPush::OnThreadStartPlay()
{
  yami::parameters params;
                
  params.set_integer("channel", 950);
  params.set_string("destination", "player");
  params.set_string("endpoint", YC.endpoint());

  auto_ptr<yami::outgoing_message> message(AGENT.send(DISCOVERY.get("tv"), "tv", "create_client_session", params));

  message->wait_for_completion(1000);
                
  if (message->get_state() == yami::replied)
  {
    return S_FALSE;
  }

  return S_OK;
}

void CLibraryStreamPush::on_msg(yami::incoming_message & im)
{
  if (im.get_message_name() == "stream_part")
  {
    try
    {
      size_t len = 0;
      BYTE* buf = (BYTE*)im.get_parameters().get_binary("payload", len);

      // loop on arrived data until all gets copied into buffer
      // some may be left in temporary buffer until next portion of data arrives
      
      while (len != 0)
      {
        //LOG("len= " << len << " buffer_len_=" << buffer_len_ << " tmp_len_=" << tmp_len_ << " buffer_pos_=" << buffer_pos_);

        size_t available_in_tmp = BUFSIZE - tmp_len_;
        
        if (tmp_len_ == 0)
        {
          tmp_buf_.reset(new BYTE[BUFSIZE]);
        }

        if (len >= available_in_tmp)
        {
          // if arrived data fills remaining space in temporary buffer
          // copy all it fits into temporary buffer
          memcpy(tmp_buf_.get() + tmp_len_, buf, available_in_tmp);

          len -= available_in_tmp;
          buf += available_in_tmp;

          // add it to main buffer
          CAutoLock lock(&buffer_lock_);
          buffer_.push_back(tmp_buf_);
          buffer_len_ += BUFSIZE;
          tmp_len_ = 0;
        }
        else
        {
          // copy into temporary buffer
          memcpy(tmp_buf_.get() + tmp_len_, buf, len);
          tmp_len_ += len;
          len = 0;
        }
      }

      // removing already read elements
      while (buffer_pos_ >= BUFSIZE)
      {
        CAutoLock lock(&buffer_lock_);
        buffer_.erase_begin(1);
        buffer_pos_ -= BUFSIZE;
        buffer_len_ -= BUFSIZE;
      }
      //LOG("buffer_len_=" << buffer_len_ << " tmp_len_=" << tmp_len_ << " buffer_pos_=" << buffer_pos_);
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
