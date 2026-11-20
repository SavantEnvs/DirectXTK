// Stub ID3D11Device for driving DirectXTK's DDS loader on Linux (mayhem/shim/d3d11_shim.h).
// Every Create* copies each byte its initial data claims to cover, sized from the descriptor
// (row/slice pitch per subresource), so ASan reports any loader size miscalculation. Descriptors
// the real D3D11 runtime rejects (a zero dimension, mip or array count) are rejected the same way.
#pragma once
#include "d3d11_shim.h"

#include <cstdlib>
#include <cstring>
#include <vector>

namespace stubd3d {

template <class Base> struct Obj : Base
{
    ULONG refs = 1;
    HRESULT QueryInterface(REFIID, void** out) override { *out = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return ++refs; }
    ULONG Release() override { ULONG r = --refs; if (!r) delete this; return r; }
};
template <class Base> struct Child : Obj<Base>
{
    void GetDevice(ID3D11Device** d) override { *d = nullptr; }
    HRESULT SetPrivateData(REFIID, UINT, const void*) override { return S_OK; }
};

template <class Iface, class Desc, D3D11_RESOURCE_DIMENSION Dim> struct Tex : Child<Iface>
{
    Desc desc;
    explicit Tex(const Desc& d) : desc(d) {}
    void GetType(D3D11_RESOURCE_DIMENSION* dim) override { *dim = Dim; }
    void GetDesc(Desc* d) override { *d = desc; }
};
using Tex1D = Tex<ID3D11Texture1D, D3D11_TEXTURE1D_DESC, D3D11_RESOURCE_DIMENSION_TEXTURE1D>;
using Tex2D = Tex<ID3D11Texture2D, D3D11_TEXTURE2D_DESC, D3D11_RESOURCE_DIMENSION_TEXTURE2D>;
using Tex3D = Tex<ID3D11Texture3D, D3D11_TEXTURE3D_DESC, D3D11_RESOURCE_DIMENSION_TEXTURE3D>;

struct SRV : Child<ID3D11ShaderResourceView>
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ID3D11Resource* res;
    SRV(const D3D11_SHADER_RESOURCE_VIEW_DESC& d, ID3D11Resource* r) : desc(d), res(r) { res->AddRef(); }
    ~SRV() override { res->Release(); }
    void GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC* d) override { *d = desc; }
    void GetResource(ID3D11Resource** r) override { res->AddRef(); *r = res; }
};

struct Device : Obj<ID3D11Device>
{
    // what the loader last asked for (read by the self-test)
    D3D11_RESOURCE_DIMENSION lastDim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    D3D11_TEXTURE1D_DESC last1D = {};
    D3D11_TEXTURE2D_DESC last2D = {};
    D3D11_TEXTURE3D_DESC last3D = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC lastSRV = {};
    size_t bytesRead = 0;
    uint32_t checksum = 0;
    std::vector<uint8_t> scratch;

    void consume(const void* p, size_t n)
    {
        if (!n) return;
        if (!p) abort(); // initial data promised but missing
        scratch.resize(n);
        memcpy(scratch.data(), p, n);
        for (size_t i = 0; i < n; i += 61) checksum = checksum * 31 + scratch[i];
        checksum += scratch[n - 1];
        bytesRead += n;
    }
    HRESULT CreateTexture1D(const D3D11_TEXTURE1D_DESC* d, const D3D11_SUBRESOURCE_DATA* init, ID3D11Texture1D** out) override
    {
        lastDim = D3D11_RESOURCE_DIMENSION_TEXTURE1D; last1D = *d;
        if (!d->Width || !d->MipLevels || !d->ArraySize) return E_INVALIDARG; // as the D3D11 runtime
        if (init)
            for (UINT a = 0; a < d->ArraySize; ++a)
                for (UINT m = 0; m < d->MipLevels; ++m)
                    consume(init[a * d->MipLevels + m].pSysMem, init[a * d->MipLevels + m].SysMemPitch);
        if (out) *out = new Tex1D(*d);
        return S_OK;
    }
    HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC* d, const D3D11_SUBRESOURCE_DATA* init, ID3D11Texture2D** out) override
    {
        lastDim = D3D11_RESOURCE_DIMENSION_TEXTURE2D; last2D = *d;
        if (!d->Width || !d->Height || !d->MipLevels || !d->ArraySize) return E_INVALIDARG; // as the D3D11 runtime
        if (init)
            for (UINT a = 0; a < d->ArraySize; ++a)
                for (UINT m = 0; m < d->MipLevels; ++m)
                    consume(init[a * d->MipLevels + m].pSysMem, init[a * d->MipLevels + m].SysMemSlicePitch);
        if (out) *out = new Tex2D(*d);
        return S_OK;
    }
    HRESULT CreateTexture3D(const D3D11_TEXTURE3D_DESC* d, const D3D11_SUBRESOURCE_DATA* init, ID3D11Texture3D** out) override
    {
        lastDim = D3D11_RESOURCE_DIMENSION_TEXTURE3D; last3D = *d;
        if (!d->Width || !d->Height || !d->Depth || !d->MipLevels) return E_INVALIDARG; // as the D3D11 runtime
        if (init)
            for (UINT m = 0; m < d->MipLevels; ++m)
            {
                UINT depth = d->Depth >> m;
                if (!depth) depth = 1;
                consume(init[m].pSysMem, size_t(init[m].SysMemSlicePitch) * depth);
            }
        if (out) *out = new Tex3D(*d);
        return S_OK;
    }
    HRESULT CreateShaderResourceView(ID3D11Resource* r, const D3D11_SHADER_RESOURCE_VIEW_DESC* d, ID3D11ShaderResourceView** out) override
    {
        if (!r || !d) return E_INVALIDARG;
        lastSRV = *d;
        if (out) *out = new SRV(*d, r);
        return S_OK;
    }
    HRESULT CheckFormatSupport(DXGI_FORMAT, UINT* s) override { *s = 0xFFFFFFFFu; return S_OK; }
    D3D_FEATURE_LEVEL GetFeatureLevel() override { return D3D_FEATURE_LEVEL_11_0; }
    void GetImmediateContext(ID3D11DeviceContext** c) override { *c = nullptr; }
};

} // namespace stubd3d
