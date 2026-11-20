// libFuzzer harness: DirectXTK DDS texture loader (Src/DDSTextureLoader.cpp + Src/LoaderHelpers.h)
// via its public in-memory API CreateDDSTextureFromMemoryEx, on a stub D3D11 device that reads
// every byte of initial data the loader hands it (mayhem/d3d11_stub_device.h).
#include "pch.h"
#include "DDSTextureLoader.h"
#include "d3d11_stub_device.h"

#include <cstdint>
#include <cstring>
#include <memory>

static volatile uint32_t g_sink;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // exact-size heap copy so ASan sees any over-read past the input
    std::unique_ptr<uint8_t[]> buf(new uint8_t[size ? size : 1]);
    if (size) memcpy(buf.get(), data, size);

    auto dev = new stubd3d::Device();
    {
        ID3D11Resource* tex = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        DirectX::DDS_ALPHA_MODE alpha = DirectX::DDS_ALPHA_MODE_UNKNOWN;
        if (SUCCEEDED(DirectX::CreateDDSTextureFromMemoryEx(dev, buf.get(), size, 0, D3D11_USAGE_DEFAULT,
                D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::DDS_LOADER_DEFAULT, &tex, &srv, &alpha)))
            g_sink = dev->checksum + alpha;
        if (srv) srv->Release();
        if (tex) tex->Release();
    }
    {
        // no view, small maxsize (mip skipping) and forced sRGB
        ID3D11Resource* tex = nullptr;
        if (SUCCEEDED(DirectX::CreateDDSTextureFromMemoryEx(dev, buf.get(), size, 64, D3D11_USAGE_DEFAULT,
                D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::DDS_LOADER_FORCE_SRGB, &tex, nullptr, nullptr)))
            g_sink = dev->checksum;
        if (tex) tex->Release();
    }
    dev->Release();
    return 0;
}
