// libFuzzer harness: DirectXTK XACT wave bank (.xwb) reader (Audio/WaveBankReader.cpp).
// WaveBankReader only exposes a file API, so each input is written to a scratch file under
// $TMPDIR and opened with WaveBankReader::Open; then every entry is queried through the
// public accessors (format expansion, wave data, seek tables, metadata, name lookup).
#include "pch.h"
#include "WaveBankReader.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

static volatile uint32_t g_sink;
static std::wstring g_path;
static std::string g_cpath;

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    const char* tmp = getenv("TMPDIR");
    std::string dir = (tmp && *tmp) ? tmp : "/tmp";
    std::string tmpl = dir + "/fuzz_xwb_XXXXXX";
    std::vector<char> t(tmpl.begin(), tmpl.end());
    t.push_back(0);
    int fd = mkstemp(t.data());
    if (fd < 0) abort();
    close(fd);
    g_cpath = t.data();
    g_path.assign(g_cpath.begin(), g_cpath.end());
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FILE* f = fopen(g_cpath.c_str(), "wb");
    if (!f) abort();
    if (size && fwrite(data, 1, size, f) != size) abort();
    fclose(f);

    DirectX::WaveBankReader wb;
    if (FAILED(wb.Open(g_path.c_str())))
        return 0;
    wb.WaitOnPrepare();

    uint32_t acc = wb.Count() + wb.BankAudioSize() + wb.GetWaveAlignment();
    acc += wb.HasNames() + wb.IsStreamingBank();
    if (const char* n = wb.BankName()) acc += (uint32_t)strlen(n);
    acc += wb.Find("a");

    const uint32_t count = wb.Count() < 64 ? wb.Count() : 64;
    for (uint32_t i = 0; i < count; ++i)
    {
        alignas(8) uint8_t fmtbuf[64] = {};
        auto wfx = reinterpret_cast<WAVEFORMATEX*>(fmtbuf);
        if (SUCCEEDED(wb.GetFormat(i, wfx, sizeof(fmtbuf))))
            acc += wfx->wFormatTag + wfx->nChannels + wfx->cbSize;

        DirectX::WaveBankReader::Metadata md = {};
        if (SUCCEEDED(wb.GetMetadata(i, md)))
            acc += md.duration + md.loopStart + md.loopLength;

        const uint32_t* seek = nullptr;
        uint32_t seekCount = 0, tag = 0;
        if (SUCCEEDED(wb.GetSeekTable(i, &seek, seekCount, tag)) && seek)
            for (uint32_t s = 0; s < seekCount; ++s)
            {
                uint32_t v;
                memcpy(&v, reinterpret_cast<const uint8_t*>(seek) + s * sizeof(uint32_t), sizeof(v));
                acc += v;
            }

        if (!wb.IsStreamingBank())
        {
            const uint8_t* wave = nullptr;
            uint32_t waveSize = 0;
            if (SUCCEEDED(wb.GetWaveData(i, &wave, waveSize)) && waveSize)
                acc += wave[0] + wave[waveSize - 1];
        }
    }
    g_sink = acc;
    return 0;
}
