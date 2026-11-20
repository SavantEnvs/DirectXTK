// Known-answer tests for the DirectXTK WAV and XWB readers and the DDS loader, built with normal flags by
// mayhem/build.sh and run by mayhem/test.sh. Prints one "PASS <name>" / "FAIL <name>" line
// per check; test.sh counts them.
#include "pch.h"
#include "WAVFileReader.h"
#include "WaveBankReader.h"
#include "DDSTextureLoader.h"
#include "d3d11_stub_device.h"
#include "SpriteFont.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

static int g_fail = 0;
static void check(const char* name, bool ok)
{
    printf("%s %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok) ++g_fail;
}
static std::vector<uint8_t> slurp(const std::string& p)
{
    std::ifstream f(p, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(f), {});
}
static std::wstring wide(const std::string& s) { return std::wstring(s.begin(), s.end()); }

int main(int argc, char** argv)
{
    if (argc < 5) { fprintf(stderr, "usage: %s <wav-dir> <xwb-dir> <dds-dir> <spritefont-dir>\n", argv[0]); return 2; }
    const std::string sd = argv[4];
    const std::string wd = argv[1], xd = argv[2], dd = argv[3];
    using namespace DirectX;

    {
        auto b = slurp(wd + "/pcm16.wav");
        WAVData w = {};
        HRESULT hr = LoadWAVAudioInMemoryEx(b.data(), b.size(), w);
        check("wav_pcm16_ok", SUCCEEDED(hr));
        check("wav_pcm16_fmt", SUCCEEDED(hr) && w.wfx->wFormatTag == WAVE_FORMAT_PCM && w.wfx->nChannels == 1
            && w.wfx->nSamplesPerSec == 22050 && w.wfx->wBitsPerSample == 16);
        check("wav_pcm16_data", SUCCEEDED(hr) && w.audioBytes == 512 && w.startAudio[0] == 0 && w.startAudio[255] == 255
            && w.loopLength == 0);
    }
    {
        auto b = slurp(wd + "/pcm16_loop.wav");
        WAVData w = {};
        HRESULT hr = LoadWAVAudioInMemoryEx(b.data(), b.size(), w);
        check("wav_loop", SUCCEEDED(hr) && w.loopStart == 16 && w.loopLength == 200 - 16 + 1);
    }
    {
        auto b = slurp(wd + "/float_ext.wav");
        WAVData w = {};
        HRESULT hr = LoadWAVAudioInMemoryEx(b.data(), b.size(), w);
        check("wav_extensible", SUCCEEDED(hr) && w.wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && w.wfx->nChannels == 2
            && w.audioBytes == 64);
    }
    {
        auto b = slurp(wd + "/xwma.wav");
        WAVData w = {};
        HRESULT hr = LoadWAVAudioInMemoryEx(b.data(), b.size(), w);
        check("wav_xwma_seek", SUCCEEDED(hr) && w.seekCount == 4 && w.seek && w.seek[3] == 16384);
    }
    {
        auto b = slurp(wd + "/pcm16.wav");
        b[8] = 'X'; // not a WAVE form
        WAVData w = {};
        check("wav_reject_bad_form", FAILED(LoadWAVAudioInMemoryEx(b.data(), b.size(), w)));
        check("wav_reject_truncated", FAILED(LoadWAVAudioInMemoryEx(b.data(), 20, w)));
    }
    {
        WaveBankReader wb;
        HRESULT hr = wb.Open(wide(xd + "/pcm16_names.xwb").c_str());
        wb.WaitOnPrepare();
        check("xwb_open", SUCCEEDED(hr));
        check("xwb_count_name", SUCCEEDED(hr) && wb.Count() == 2 && wb.HasNames() && strcmp(wb.BankName(), "fuzzbank") == 0);
        check("xwb_find", SUCCEEDED(hr) && wb.Find("wave1") == 1 && wb.Find("nope") == uint32_t(-1));
        alignas(8) uint8_t fb[64] = {};
        auto wfx = reinterpret_cast<WAVEFORMATEX*>(fb);
        check("xwb_format", SUCCEEDED(hr) && SUCCEEDED(wb.GetFormat(1, wfx, sizeof(fb))) && wfx->wFormatTag == WAVE_FORMAT_PCM
            && wfx->nSamplesPerSec == 22050 && wfx->wBitsPerSample == 16);
        const uint8_t* d = nullptr;
        uint32_t n = 0;
        check("xwb_wavedata", SUCCEEDED(hr) && SUCCEEDED(wb.GetWaveData(1, &d, n)) && n == 64 && d[1] == 8);
    }
    {
        WaveBankReader wb;
        HRESULT hr = wb.Open(wide(xd + "/compact.xwb").c_str());
        wb.WaitOnPrepare();
        WaveBankReader::Metadata md = {};
        check("xwb_compact", SUCCEEDED(hr) && wb.Count() == 3 && SUCCEEDED(wb.GetMetadata(2, md)) && md.lengthBytes == 64);
    }
    {
        WaveBankReader wb;
        check("xwb_reject_missing", FAILED(wb.Open(L"/nonexistent/x.xwb")));
    }
    {
        auto load = [&](const char* name, stubd3d::Device* dev, DDS_ALPHA_MODE* alpha) {
            auto b = slurp(dd + "/" + name);
            ID3D11Resource* tex = nullptr;
            ID3D11ShaderResourceView* srv = nullptr;
            HRESULT hr = CreateDDSTextureFromMemoryEx(dev, b.data(), b.size(), 0, D3D11_USAGE_DEFAULT,
                D3D11_BIND_SHADER_RESOURCE, 0, 0, DDS_LOADER_DEFAULT, &tex, &srv, alpha);
            if (srv) srv->Release();
            if (tex) tex->Release();
            return hr;
        };
        {
            auto dev = new stubd3d::Device();
            HRESULT hr = load("rgba8_mips.dds", dev, nullptr);
            check("dds_rgba8_mips", SUCCEEDED(hr) && dev->lastDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D && dev->last2D.Width == 8
                && dev->last2D.Height == 4 && dev->last2D.MipLevels == 4 && dev->last2D.Format == DXGI_FORMAT_R8G8B8A8_UNORM
                && dev->bytesRead == 8 * 4 * 4 + 4 * 2 * 4 + 2 * 4 + 4);
            dev->Release();
        }
        {
            auto dev = new stubd3d::Device();
            HRESULT hr = load("cube_dx10.dds", dev, nullptr);
            check("dds_cubemap", SUCCEEDED(hr) && dev->last2D.ArraySize == 6 && (dev->last2D.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
                && dev->lastSRV.ViewDimension == D3D11_SRV_DIMENSION_TEXTURECUBE);
            dev->Release();
        }
        {
            auto dev = new stubd3d::Device();
            HRESULT hr = load("volume.dds", dev, nullptr);
            check("dds_volume", SUCCEEDED(hr) && dev->lastDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D && dev->last3D.Depth == 2
                && dev->last3D.MipLevels == 2 && dev->bytesRead == 4 * 4 * 2 * 4 + 2 * 2 * 4);
            dev->Release();
        }
        {
            auto dev = new stubd3d::Device();
            DDS_ALPHA_MODE alpha = DDS_ALPHA_MODE_CUSTOM;
            HRESULT hr = load("dxt1.dds", dev, &alpha);
            check("dds_dxt1", SUCCEEDED(hr) && dev->last2D.Format == DXGI_FORMAT_BC1_UNORM && dev->bytesRead == 32);
            dev->Release();
        }
        {
            auto dev = new stubd3d::Device();
            auto b = slurp(dd + "/dxt1.dds");
            ID3D11Resource* tex = nullptr;
            check("dds_reject_truncated", FAILED(CreateDDSTextureFromMemoryEx(dev, b.data(), b.size() - 8, 0, D3D11_USAGE_DEFAULT,
                D3D11_BIND_SHADER_RESOURCE, 0, 0, DDS_LOADER_DEFAULT, &tex, nullptr, nullptr)) && tex == nullptr);
            dev->Release();
        }
    }
    {
        // rgba8.spritefont: 7 glyphs " ?ABabc", rect width 4 / height 6, XOffset 0, YOffset 1, XAdvance 5,
        // line spacing 8, 32x8 R8G8B8A8 sheet (mayhem/directxtk_spritefont/testsuite)
        auto b = slurp(sd + "/rgba8.spritefont");
        auto dev = new stubd3d::Device();
        try
        {
            DirectX::SpriteFont font(dev, b.data(), b.size());
            check("font_glyphs", font.ContainsCharacter(L'A') && font.ContainsCharacter(L'c') && !font.ContainsCharacter(L'Z')
                && font.GetLineSpacing() == 8.f && font.GetDefaultCharacter() == L'?');
            auto g = font.FindGlyph(L'b');
            check("font_glyph_rect", g && g->Character == 'b' && g->Subrect.left == 20 && g->Subrect.right == 24 && g->XAdvance == 5.f);
            check("font_texture", dev->lastDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D && dev->last2D.Width == 32
                && dev->last2D.Height == 8 && dev->last2D.Format == DXGI_FORMAT_R8G8B8A8_UNORM && dev->bytesRead == 32 * 4 * 8);
            float w = DirectX::XMVectorGetX(font.MeasureString(L"AB"));
            float h = DirectX::XMVectorGetY(font.MeasureString(L"A\nB"));
            printf("font_measure: w=%g h=%g\n", w, h);
            check("font_measure", w == 13.f && h == 16.f); // 4 + 5 advance + 4; two lines of spacing 8
        }
        catch (const std::exception& e)
        {
            printf("font exception: %s\n", e.what());
            check("font_glyphs", false);
        }
        dev->Release();
        auto bad = slurp(sd + "/rgba8.spritefont");
        bad[0] = 'X';
        auto dev2 = new stubd3d::Device();
        bool threw = false;
        try { DirectX::SpriteFont f(dev2, bad.data(), bad.size()); } catch (const std::exception&) { threw = true; }
        check("font_reject_magic", threw);
        dev2->Release();
    }
    return g_fail ? 1 : 0;
}
