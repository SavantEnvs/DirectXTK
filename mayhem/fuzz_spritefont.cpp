// libFuzzer harness: DirectXTK .spritefont reader (Src/SpriteFont.cpp + Src/BinaryReader.cpp) via
// the public SpriteFont(device, dataBlob, dataSize) constructor on the stub D3D11 device
// (mayhem/d3d11_stub_device.h reads every byte of the glyph-sheet texture it is handed), then the
// glyph-layout queries (MeasureString / MeasureDrawBounds / FindGlyph) over the parsed glyphs.
#include "pch.h"
#include "SpriteFont.h"
#include "d3d11_stub_device.h"

#include <cstdint>
#include <cstring>
#include <exception>
#include <memory>

static volatile float g_sink;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::unique_ptr<uint8_t[]> buf(new uint8_t[size ? size : 1]);
    if (size) memcpy(buf.get(), data, size);

    auto dev = new stubd3d::Device();
    try
    {
        DirectX::SpriteFont font(dev, buf.get(), size);
        float acc = font.GetLineSpacing();
        acc += DirectX::XMVectorGetX(font.MeasureString(L"Ab c\nxyz?"));
        acc += DirectX::XMVectorGetY(font.MeasureString("Hello,\n w\xc3\xb6rld", false));
        RECT r = font.MeasureDrawBounds(L"A\tb\n c", DirectX::XMFLOAT2(1.f, 2.f), false);
        acc += float(r.right) - float(r.left);  // float math: the RECT can span more than LONG range
        acc += font.ContainsCharacter(L'x') ? 1.f : 0.f;
        if (font.ContainsCharacter(L'A'))
            acc += font.FindGlyph(L'A')->XAdvance;
        g_sink = acc;
    }
    catch (const std::exception&)
    {
        // malformed fonts are rejected with exceptions (std::runtime_error, com_exception, ...)
    }
    dev->Release();
    return 0;
}
