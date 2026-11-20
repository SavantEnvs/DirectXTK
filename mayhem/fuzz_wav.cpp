// libFuzzer harness: DirectXTK WAV/RIFF parser (Audio/WAVFileReader.cpp) via its public
// in-memory API LoadWAVAudioInMemoryEx / LoadWAVAudioInMemory.
#include "pch.h"
#include "WAVFileReader.h"

#include <cstdint>
#include <cstring>
#include <memory>

static volatile uint32_t g_sink;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // exact-size heap copy so ASan sees any over-read past the input
    std::unique_ptr<uint8_t[]> buf(new uint8_t[size ? size : 1]);
    if (size) memcpy(buf.get(), data, size);

    DirectX::WAVData wav = {};
    if (SUCCEEDED(DirectX::LoadWAVAudioInMemoryEx(buf.get(), size, wav)))
    {
        uint32_t acc = wav.wfx->wFormatTag + wav.wfx->nChannels + wav.wfx->nBlockAlign;
        for (uint32_t i = 0; i < wav.audioBytes; ++i)
            acc += wav.startAudio[i];
        for (uint32_t i = 0; i < wav.seekCount; ++i)
        {
            // the seek table points into the caller's buffer and may be unaligned
            uint32_t v;
            memcpy(&v, reinterpret_cast<const uint8_t*>(wav.seek) + i * sizeof(uint32_t), sizeof(v));
            acc += v;
        }
        acc += wav.loopStart + wav.loopLength;
        g_sink = acc;
    }

    const WAVEFORMATEX* wfx = nullptr;
    const uint8_t* start = nullptr;
    uint32_t bytes = 0;
    if (SUCCEEDED(DirectX::LoadWAVAudioInMemory(buf.get(), size, &wfx, &start, &bytes)))
        g_sink = wfx->nSamplesPerSec + (bytes ? start[bytes - 1] : 0);
    return 0;
}
