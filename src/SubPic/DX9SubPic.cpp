/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include "DX9SubPic.h"
#include <vmr9.h>
#include <algorithm>

//
// CDX9SubPic
//

CDX9SubPic::CDX9SubPic(IDirect3DSurface9* pSurface, CDX9SubPicAllocator* pAllocator, bool bExternalRenderer)
    : m_pSurface(pSurface)
    , m_pAllocator(pAllocator)
    , m_bExternalRenderer(bExternalRenderer)
{
    D3DSURFACE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    if (SUCCEEDED(m_pSurface->GetDesc(&desc))) {
        m_maxsize.SetSize(desc.Width, desc.Height);
        m_rcDirty.SetRect(0, 0, desc.Width, desc.Height);
    }
}

CDX9SubPic::~CDX9SubPic()
{
    {
        CAutoLock Lock(&CDX9SubPicAllocator::ms_SurfaceQueueLock);
        // Add surface to cache
        if (m_pAllocator) {
            for (POSITION pos = m_pAllocator->m_AllocatedSurfaces.GetHeadPosition(); pos;) {
                POSITION ThisPos = pos;
                CDX9SubPic* pSubPic = m_pAllocator->m_AllocatedSurfaces.GetNext(pos);
                if (pSubPic == this) {
                    m_pAllocator->m_AllocatedSurfaces.RemoveAt(ThisPos);
                    break;
                }
            }
            m_pAllocator->m_FreeSurfaces.AddTail(m_pSurface);
        }
    }
}


// ISubPic

STDMETHODIMP_(void*) CDX9SubPic::GetObject()
{
    CComPtr<IDirect3DTexture9> pTexture;
    if (SUCCEEDED(m_pSurface->GetContainer(IID_PPV_ARGS(&pTexture)))) {
        return (void*)(IDirect3DTexture9*)pTexture;
    }

    return nullptr;
}

STDMETHODIMP CDX9SubPic::GetDesc(SubPicDesc& spd)
{
    D3DSURFACE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    if (FAILED(m_pSurface->GetDesc(&desc))) {
        return E_FAIL;
    }

    spd.type = 0;
    spd.w = m_size.cx;
    spd.h = m_size.cy;
    spd.bpp =
        desc.Format == D3DFMT_A8R8G8B8 ? 32 :
        desc.Format == D3DFMT_A4R4G4B4 ? 16 : 0;
    spd.pitch = 0;
    spd.bits = nullptr;
    spd.vidrect = m_vidrect;

    return S_OK;
}

STDMETHODIMP CDX9SubPic::CopyTo(ISubPic* pSubPic)
{
    HRESULT hr;
    if (FAILED(hr = __super::CopyTo(pSubPic))) {
        return hr;
    }

    if (m_rcDirty.IsRectEmpty()) {
        return S_FALSE;
    }

    CComPtr<IDirect3DDevice9> pD3DDev;
    if (!m_pSurface || FAILED(m_pSurface->GetDevice(&pD3DDev)) || !pD3DDev) {
        return E_FAIL;
    }

    IDirect3DTexture9* pSrcTex = (IDirect3DTexture9*)GetObject();
    CComPtr<IDirect3DSurface9> pSrcSurf;
    pSrcTex->GetSurfaceLevel(0, &pSrcSurf);
    D3DSURFACE_DESC srcDesc;
    pSrcSurf->GetDesc(&srcDesc);

    IDirect3DTexture9* pDstTex = (IDirect3DTexture9*)pSubPic->GetObject();
    CComPtr<IDirect3DSurface9> pDstSurf;
    pDstTex->GetSurfaceLevel(0, &pDstSurf);
    D3DSURFACE_DESC dstDesc;
    pDstSurf->GetDesc(&dstDesc);

    RECT r;
    SetRect(&r, 0, 0, std::min(srcDesc.Width, dstDesc.Width), std::min(srcDesc.Height, dstDesc.Height));
    POINT p = { 0, 0 };
    hr = pD3DDev->UpdateSurface(pSrcSurf, &r, pDstSurf, &p);
    //  ASSERT (SUCCEEDED (hr));

    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}

STDMETHODIMP CDX9SubPic::ClearDirtyRect(DWORD color)
{
    if (m_rcDirty.IsRectEmpty()) {
        return S_FALSE;
    }

    CComPtr<IDirect3DDevice9> pD3DDev;
    if (!m_pSurface || FAILED(m_pSurface->GetDevice(&pD3DDev)) || !pD3DDev) {
        return E_FAIL;
    }

    SubPicDesc spd;
    if (SUCCEEDED(Lock(spd))) {
        int h = m_rcDirty.Height();

        BYTE* ptr = spd.bits + spd.pitch * m_rcDirty.top + (m_rcDirty.left * spd.bpp >> 3);

        if (spd.bpp == 16) {
            while (h-- > 0) {
                memsetw(ptr, (unsigned short)color, 2 * m_rcDirty.Width());
                ptr += spd.pitch;
            }
        } else if (spd.bpp == 32) {
            while (h-- > 0) {
                memsetd(ptr, color, 4 * m_rcDirty.Width());
                ptr += spd.pitch;
            }
        }
        /*
                DWORD* ptr = (DWORD*)bm.bits;
                DWORD* end = ptr + bm.h*bm.wBytes/4;
                while (ptr < end) *ptr++ = color;
        */
        Unlock(nullptr);
    }

    //      HRESULT hr = pD3DDev->ColorFill(m_pSurface, m_rcDirty, color);

    m_rcDirty.SetRectEmpty();

    return S_OK;
}

STDMETHODIMP CDX9SubPic::Lock(SubPicDesc& spd)
{
    D3DSURFACE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    if (FAILED(m_pSurface->GetDesc(&desc))) {
        return E_FAIL;
    }

    D3DLOCKED_RECT LockedRect;
    ZeroMemory(&LockedRect, sizeof(LockedRect));
    if (FAILED(m_pSurface->LockRect(&LockedRect, nullptr, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK))) {
        return E_FAIL;
    }

    spd.type = 0;
    spd.w = m_size.cx;
    spd.h = m_size.cy;
    spd.bpp =
        desc.Format == D3DFMT_A8R8G8B8 ? 32 :
        desc.Format == D3DFMT_A4R4G4B4 ? 16 : 0;
    spd.pitch = LockedRect.Pitch;
    spd.bits = (BYTE*)LockedRect.pBits;
    spd.vidrect = m_vidrect;

    return S_OK;
}

STDMETHODIMP CDX9SubPic::Unlock(RECT* pDirtyRect)
{
    HRESULT hr = m_pSurface->UnlockRect();

    if (pDirtyRect) {
        m_rcDirty = *pDirtyRect;
        if (!((CRect*)pDirtyRect)->IsRectEmpty()) {
            m_rcDirty.InflateRect(1, 1);
            m_rcDirty.left &= ~127;
            m_rcDirty.top &= ~63;
            m_rcDirty.right = (m_rcDirty.right + 127) & ~127;
            m_rcDirty.bottom = (m_rcDirty.bottom + 63) & ~63;
            m_rcDirty &= CRect(CPoint(0, 0), m_size);
        }
    } else {
        m_rcDirty = CRect(CPoint(0, 0), m_size);
    }

    CComPtr<IDirect3DTexture9> pTexture = (IDirect3DTexture9*)GetObject();
    if (pTexture && pDirtyRect && !((CRect*)pDirtyRect)->IsRectEmpty()) {
        hr = pTexture->AddDirtyRect(&m_rcDirty);
    }

    return S_OK;
}

STDMETHODIMP CDX9SubPic::AlphaBlt(RECT* pSrc, RECT* pDst, SubPicDesc* pTarget)
{
    ASSERT(pTarget == nullptr);

    if (!pSrc || !pDst) {
        return E_POINTER;
    }

    CRect src(*pSrc), dst(*pDst);

    CComPtr<IDirect3DDevice9> pD3DDev;
    CComPtr<IDirect3DTexture9> pTexture = (IDirect3DTexture9*)GetObject();
    if (!pTexture || FAILED(pTexture->GetDevice(&pD3DDev)) || !pD3DDev) {
        return E_NOINTERFACE;
    }

    HRESULT hr;

    D3DSURFACE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    if (FAILED(pTexture->GetLevelDesc(0, &desc)) /*|| desc.Type != D3DRTYPE_TEXTURE*/) {
        return E_FAIL;
    }

    float w = (float)desc.Width;
    float h = (float)desc.Height;

    // Be careful with the code that follows. Some compilers (e.g. Visual Studio 2012) used to miscompile
    // it in some cases (namely x64 with optimizations /O2 /Ot). This bug led pVertices not to be correctly
    // initialized and thus the subtitles weren't shown.
    struct {
        float x, y, z, rhw;
        float tu, tv;
    } pVertices[] = {
        {float(dst.left),  float(dst.top),    0.5f, 2.0f, float(src.left)  / w, float(src.top) / h},
        {float(dst.right), float(dst.top),    0.5f, 2.0f, float(src.right) / w, float(src.top) / h},
        {float(dst.left),  float(dst.bottom), 0.5f, 2.0f, float(src.left)  / w, float(src.bottom) / h},
        {float(dst.right), float(dst.bottom), 0.5f, 2.0f, float(src.right) / w, float(src.bottom) / h},
    };

    for (size_t i = 0; i < _countof(pVertices); i++) {
        pVertices[i].x -= 0.5f;
        pVertices[i].y -= 0.5f;
    }

    hr = pD3DDev->SetTexture(0, pTexture);

    // GetRenderState fails for devices created with D3DCREATE_PUREDEVICE
    // so we need to provide default values in case GetRenderState fails
    DWORD abe, sb, db;
    if (FAILED(pD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &abe))) {
        abe = FALSE;
    }
    if (FAILED(pD3DDev->GetRenderState(D3DRS_SRCBLEND, &sb))) {
        sb = D3DBLEND_ONE;
    }
    if (FAILED(pD3DDev->GetRenderState(D3DRS_DESTBLEND, &db))) {
        db = D3DBLEND_ZERO;
    }

    hr = pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    hr = pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    hr = pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
    hr = pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    hr = pD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // pre-multiplied src and ...
    hr = pD3DDev->SetRenderState(D3DRS_DESTBLEND, m_invAlpha ? D3DBLEND_INVSRCALPHA : D3DBLEND_SRCALPHA); // ... inverse alpha channel for dst

    hr = pD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    hr = pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = pD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    if (src == dst) {
        hr = pD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        hr = pD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    } else {
        hr = pD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        hr = pD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    }
    hr = pD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    hr = pD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
    hr = pD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
    hr = pD3DDev->SetSamplerState(0, D3DSAMP_BORDERCOLOR, m_invAlpha ? 0x00000000 : 0xFF000000);

    /*//

    D3DCAPS9 d3dcaps9;
    hr = pD3DDev->GetDeviceCaps(&d3dcaps9);
    if (d3dcaps9.AlphaCmpCaps & D3DPCMPCAPS_LESS)
    {
        hr = pD3DDev->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x000000FE);
        hr = pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        hr = pD3DDev->SetRenderState(D3DRS_ALPHAFUNC, D3DPCMPCAPS_LESS);
    }

    *///

    hr = pD3DDev->SetPixelShader(nullptr);

    if (m_bExternalRenderer && FAILED(hr = pD3DDev->BeginScene())) {
        return E_FAIL;
    }

    hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
    hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

    if (m_bExternalRenderer) {
        hr = pD3DDev->EndScene();
    }

    pD3DDev->SetTexture(0, nullptr);

    pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, abe);
    pD3DDev->SetRenderState(D3DRS_SRCBLEND, sb);
    pD3DDev->SetRenderState(D3DRS_DESTBLEND, db);

    return S_OK;
}

//
// CDX9SubPicAllocator
//

CDX9SubPicAllocator::CDX9SubPicAllocator(IDirect3DDevice9* pD3DDev, SIZE maxsize, bool fPow2Textures, bool bExternalRenderer)
    : CSubPicAllocatorImpl(maxsize, true, fPow2Textures)
    , m_pD3DDev(pD3DDev)
    , m_maxsize(maxsize)
    , m_bExternalRenderer(bExternalRenderer)
{
}

CCritSec CDX9SubPicAllocator::ms_SurfaceQueueLock;

CDX9SubPicAllocator::~CDX9SubPicAllocator()
{
    ClearCache();
}

void CDX9SubPicAllocator::GetStats(int& _nFree, int& _nAlloc)
{
    CAutoLock Lock(&ms_SurfaceQueueLock);
    _nFree = (int)m_FreeSurfaces.GetCount();
    _nAlloc = (int)m_AllocatedSurfaces.GetCount();
}

void CDX9SubPicAllocator::ClearCache()
{
    {
        // Clear the allocator of any remaining subpics
        CAutoLock Lock(&ms_SurfaceQueueLock);
        for (POSITION pos = m_AllocatedSurfaces.GetHeadPosition(); pos;) {
            CDX9SubPic* pSubPic = m_AllocatedSurfaces.GetNext(pos);
            pSubPic->m_pAllocator = nullptr;
        }
        m_AllocatedSurfaces.RemoveAll();
        m_FreeSurfaces.RemoveAll();
    }
}

// ISubPicAllocator

STDMETHODIMP CDX9SubPicAllocator::ChangeDevice(IUnknown* pDev)
{
    ClearCache();
    CComQIPtr<IDirect3DDevice9> pD3DDev = pDev;
    if (!pD3DDev) {
        return E_NOINTERFACE;
    }

    CAutoLock cAutoLock(this);
    m_pD3DDev = pD3DDev;

    return __super::ChangeDevice(pDev);
}

STDMETHODIMP CDX9SubPicAllocator::SetMaxTextureSize(SIZE MaxTextureSize)
{
    ClearCache();
    m_maxsize = MaxTextureSize;
    SetCurSize(MaxTextureSize);
    return S_OK;
}

// ISubPicAllocatorImpl

bool CDX9SubPicAllocator::Alloc(bool fStatic, ISubPic** ppSubPic)
{
    if (!ppSubPic) {
        return false;
    }

    CAutoLock cAutoLock(this);

    *ppSubPic = nullptr;

    CComPtr<IDirect3DSurface9> pSurface;

    int Width = m_maxsize.cx;
    int Height = m_maxsize.cy;

    if (m_fPow2Textures && Width < 1024 && Height < 1024) {
        Width = Height = 1;
        while (Width < m_maxsize.cx) {
            Width <<= 1;
        }
        while (Height < m_maxsize.cy) {
            Height <<= 1;
        }
    }
    if (!fStatic) {
        CAutoLock cAutoLock2(&ms_SurfaceQueueLock);
        POSITION FreeSurf = m_FreeSurfaces.GetHeadPosition();
        if (FreeSurf) {
            pSurface = m_FreeSurfaces.GetHead();
            m_FreeSurfaces.RemoveHead();
        }
    }

    if (!pSurface) {
        CComPtr<IDirect3DTexture9> pTexture;
        if (FAILED(m_pD3DDev->CreateTexture(Width, Height, 1, 0, D3DFMT_A8R8G8B8, fStatic ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT, &pTexture, nullptr))) {
            return false;
        }

        if (FAILED(pTexture->GetSurfaceLevel(0, &pSurface))) {
            return false;
        }
    }

    try {
        *ppSubPic = DEBUG_NEW CDX9SubPic(pSurface, fStatic ? nullptr : this, m_bExternalRenderer);
    } catch (std::bad_alloc) {
        return false;
    }

    (*ppSubPic)->AddRef();

    if (!fStatic) {
        CAutoLock cAutoLock3(&ms_SurfaceQueueLock);
        m_AllocatedSurfaces.AddHead((CDX9SubPic*)*ppSubPic);
    }

    return true;
}
