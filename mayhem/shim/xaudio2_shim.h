// Declaration-only stand-ins for the XAudio2 / X3DAudio / MMDevice types that Inc/Audio.h and
// Audio/SoundCommon.h name. The fuzzed parsers (WaveBankReader, WAVFileReader) never call them;
// nothing here is implemented.
#pragma once
#include "windows.h"
#include "mmreg.h"
#include <map>

typedef struct _FILETIME { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME;

inline uint32_t _byteswap_ulong(uint32_t v) { return __builtin_bswap32(v); }
inline uint16_t _byteswap_ushort(uint16_t v) { return __builtin_bswap16(v); }
inline uint64_t _byteswap_uint64(uint64_t v) { return __builtin_bswap64(v); }

template <size_t N>
inline int strncpy_s(char (&dst)[N], const char* src, size_t count)
{
    size_t n = strnlen(src, count < N - 1 ? count : N - 1);
    memcpy(dst, src, n);
    dst[n] = 0;
    return 0;
}

#define ERROR_MORE_DATA 234L
#define ERROR_NO_DATA 232L

typedef enum _AUDIO_STREAM_CATEGORY { AudioCategory_Other = 0, AudioCategory_GameEffects = 6 } AUDIO_STREAM_CATEGORY;

#define XAUDIO2_MAX_AUDIO_CHANNELS 64
#define XAUDIO2_MAX_VOLUME_LEVEL 16777216.0f
#define XAUDIO2_VOICE_NOSAMPLESPLAYED 0x0100
#define XAUDIO2_PLAY_TAILS 0x0020
#define XAUDIO2_COMMIT_NOW 0
inline float XAudio2SemitonesToFrequencyRatio(float s) { return __builtin_powf(2.0f, s / 12.0f); }


typedef struct XAUDIO2_BUFFER { uint32_t Flags; uint32_t AudioBytes; const BYTE* pAudioData; uint32_t PlayBegin; uint32_t PlayLength; uint32_t LoopBegin; uint32_t LoopLength; uint32_t LoopCount; void* pContext; } XAUDIO2_BUFFER;
typedef struct XAUDIO2_BUFFER_WMA { const uint32_t* pDecodedPacketCumulativeBytes; uint32_t PacketCount; } XAUDIO2_BUFFER_WMA;
typedef struct XAUDIO2_VOICE_STATE { void* pCurrentBufferContext; uint32_t BuffersQueued; uint64_t SamplesPlayed; } XAUDIO2_VOICE_STATE;
typedef struct XAUDIO2FX_REVERB_PARAMETERS { float WetDryMix; } XAUDIO2FX_REVERB_PARAMETERS;

struct IXAudio2Voice
{
    virtual void DestroyVoice() = 0;
    virtual HRESULT SetVolume(float, uint32_t = 0) = 0;
    virtual ~IXAudio2Voice() = default;
};
struct IXAudio2SourceVoice : IXAudio2Voice
{
    virtual HRESULT Start(uint32_t = 0, uint32_t = 0) = 0;
    virtual HRESULT Stop(uint32_t = 0, uint32_t = 0) = 0;
    virtual HRESULT FlushSourceBuffers() = 0;
    virtual HRESULT ExitLoop(uint32_t = 0) = 0;
    virtual void GetState(XAUDIO2_VOICE_STATE*, uint32_t = 0) = 0;
    virtual HRESULT SetFrequencyRatio(float, uint32_t = 0) = 0;
};
struct IXAudio2SubmixVoice : IXAudio2Voice {};
struct IXAudio2MasteringVoice : IXAudio2Voice {};
struct IXAudio2 {};

typedef BYTE X3DAUDIO_HANDLE[20];
#define X3DAUDIO_PI 3.141592654f
#define X3DAUDIO_CALCULATE_ZEROCENTER 0x00010000
#define X3DAUDIO_CALCULATE_REDIRECT_TO_LFE 0x00020000
typedef struct X3DAUDIO_VECTOR { float x, y, z; } X3DAUDIO_VECTOR;
typedef struct X3DAUDIO_DISTANCE_CURVE_POINT { float Distance; float DSPSetting; } X3DAUDIO_DISTANCE_CURVE_POINT;
typedef struct X3DAUDIO_DISTANCE_CURVE { X3DAUDIO_DISTANCE_CURVE_POINT* pPoints; uint32_t PointCount; } X3DAUDIO_DISTANCE_CURVE;
typedef struct X3DAUDIO_CONE { float InnerAngle, OuterAngle, InnerVolume, OuterVolume, InnerLPF, OuterLPF, InnerReverb, OuterReverb; } X3DAUDIO_CONE;
typedef struct X3DAUDIO_LISTENER { X3DAUDIO_VECTOR OrientFront; X3DAUDIO_VECTOR OrientTop; X3DAUDIO_VECTOR Position; X3DAUDIO_VECTOR Velocity; X3DAUDIO_CONE* pCone; } X3DAUDIO_LISTENER;
typedef struct X3DAUDIO_EMITTER {
    X3DAUDIO_CONE* pCone; X3DAUDIO_VECTOR OrientFront; X3DAUDIO_VECTOR OrientTop; X3DAUDIO_VECTOR Position; X3DAUDIO_VECTOR Velocity;
    float InnerRadius; float InnerRadiusAngle; uint32_t ChannelCount; float ChannelRadius; float* pChannelAzimuths;
    X3DAUDIO_DISTANCE_CURVE* pVolumeCurve; X3DAUDIO_DISTANCE_CURVE* pLFECurve; X3DAUDIO_DISTANCE_CURVE* pLPFDirectCurve;
    X3DAUDIO_DISTANCE_CURVE* pLPFReverbCurve; X3DAUDIO_DISTANCE_CURVE* pReverbCurve; float CurveDistanceScaler; float DopplerScaler;
} X3DAUDIO_EMITTER;
typedef struct X3DAUDIO_DSP_SETTINGS {
    float* pMatrixCoefficients; float* pDelayTimes; uint32_t SrcChannelCount; uint32_t DstChannelCount;
    float LPFDirectCoefficient; float LPFReverbCoefficient; float ReverbLevel; float DopplerFactor;
    float EmitterToListenerAngle; float EmitterToListenerDistance; float EmitterVelocityComponent; float ListenerVelocityComponent;
} X3DAUDIO_DSP_SETTINGS;
