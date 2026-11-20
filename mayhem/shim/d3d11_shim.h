// Declaration-level stand-in for the D3D11/DXGI/WRL surface that DirectXTK's DDS loader names
// (Src/DDSTextureLoader.cpp, Src/LoaderHelpers.h, Inc/DirectXHelpers.h). Interfaces are abstract
// classes with only the methods those sources call; the fuzz harness implements a stub device.
// Enum values and struct layouts follow the Windows SDK headers.
#pragma once
#include "windows.h"
#include <utility>

// ---- COM basics ---------------------------------------------------------------
#define REFIID const IID&
#define STDMETHOD(m) virtual HRESULT m
#define STDMETHOD_(t, m) virtual t m
#define __uuidof(x) (x##_shim_iid())
struct IUnknown
{
    virtual HRESULT QueryInterface(REFIID, void**) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual ~IUnknown() = default;
};
struct IWICStream : IUnknown {};

namespace Microsoft { namespace WRL {
template <typename T> class ComPtr
{
    T* p = nullptr;
    void rel() { if (p) { T* t = p; p = nullptr; t->Release(); } }
public:
    ComPtr() noexcept = default;
    ComPtr(std::nullptr_t) noexcept {}
    ComPtr(T* o) noexcept : p(o) { if (p) p->AddRef(); }
    ComPtr(const ComPtr& o) noexcept : p(o.p) { if (p) p->AddRef(); }
    ComPtr(ComPtr&& o) noexcept : p(o.p) { o.p = nullptr; }
    ~ComPtr() { rel(); }
    ComPtr& operator=(const ComPtr& o) noexcept { if (this != &o) { if (o.p) o.p->AddRef(); rel(); p = o.p; } return *this; }
    ComPtr& operator=(ComPtr&& o) noexcept { if (this != &o) { rel(); p = o.p; o.p = nullptr; } return *this; }
    ComPtr& operator=(std::nullptr_t) noexcept { rel(); return *this; }
    T* Get() const noexcept { return p; }
    T* operator->() const noexcept { return p; }
    explicit operator bool() const noexcept { return p != nullptr; }
    T** GetAddressOf() noexcept { return &p; }
    T** ReleaseAndGetAddressOf() noexcept { rel(); return &p; }
    T** operator&() noexcept { rel(); return &p; } // as WRL: &ptr releases and yields the slot
    T* Detach() noexcept { T* t = p; p = nullptr; return t; }
    void Attach(T* o) noexcept { rel(); p = o; }
    void Reset() noexcept { rel(); }
    void Swap(ComPtr& o) noexcept { std::swap(p, o.p); }
    HRESULT CopyTo(T** out) const noexcept { if (p) p->AddRef(); *out = p; return S_OK; }
};
}} // namespace Microsoft::WRL

// ---- files (LoaderHelpers auto_delete_file; only used by the save paths) -------
#define FileDispositionInfo ((FILE_INFO_BY_HANDLE_CLASS)4)
typedef struct _FILE_DISPOSITION_INFO { BOOL DeleteFile; } FILE_DISPOSITION_INFO;
inline BOOL SetFileInformationByHandle(HANDLE, FILE_INFO_BY_HANDLE_CLASS, void*, DWORD) { return FALSE; }
inline BOOL DeleteFileW(const wchar_t*) { return FALSE; }

// ---- DXGI -------------------------------------------------------------------
typedef enum DXGI_FORMAT : uint32_t {
    DXGI_FORMAT_UNKNOWN = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R32G32B32A32_UINT = 3,
    DXGI_FORMAT_R32G32B32A32_SINT = 4,
    DXGI_FORMAT_R32G32B32_TYPELESS = 5,
    DXGI_FORMAT_R32G32B32_FLOAT = 6,
    DXGI_FORMAT_R32G32B32_UINT = 7,
    DXGI_FORMAT_R32G32B32_SINT = 8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM = 11,
    DXGI_FORMAT_R16G16B16A16_UINT = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM = 13,
    DXGI_FORMAT_R16G16B16A16_SINT = 14,
    DXGI_FORMAT_R32G32_TYPELESS = 15,
    DXGI_FORMAT_R32G32_FLOAT = 16,
    DXGI_FORMAT_R32G32_UINT = 17,
    DXGI_FORMAT_R32G32_SINT = 18,
    DXGI_FORMAT_R32G8X24_TYPELESS = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM = 24,
    DXGI_FORMAT_R10G10B10A2_UINT = 25,
    DXGI_FORMAT_R11G11B10_FLOAT = 26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
    DXGI_FORMAT_R8G8B8A8_UINT = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM = 31,
    DXGI_FORMAT_R8G8B8A8_SINT = 32,
    DXGI_FORMAT_R16G16_TYPELESS = 33,
    DXGI_FORMAT_R16G16_FLOAT = 34,
    DXGI_FORMAT_R16G16_UNORM = 35,
    DXGI_FORMAT_R16G16_UINT = 36,
    DXGI_FORMAT_R16G16_SNORM = 37,
    DXGI_FORMAT_R16G16_SINT = 38,
    DXGI_FORMAT_R32_TYPELESS = 39,
    DXGI_FORMAT_D32_FLOAT = 40,
    DXGI_FORMAT_R32_FLOAT = 41,
    DXGI_FORMAT_R32_UINT = 42,
    DXGI_FORMAT_R32_SINT = 43,
    DXGI_FORMAT_R24G8_TYPELESS = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
    DXGI_FORMAT_R8G8_TYPELESS = 48,
    DXGI_FORMAT_R8G8_UNORM = 49,
    DXGI_FORMAT_R8G8_UINT = 50,
    DXGI_FORMAT_R8G8_SNORM = 51,
    DXGI_FORMAT_R8G8_SINT = 52,
    DXGI_FORMAT_R16_TYPELESS = 53,
    DXGI_FORMAT_R16_FLOAT = 54,
    DXGI_FORMAT_D16_UNORM = 55,
    DXGI_FORMAT_R16_UNORM = 56,
    DXGI_FORMAT_R16_UINT = 57,
    DXGI_FORMAT_R16_SNORM = 58,
    DXGI_FORMAT_R16_SINT = 59,
    DXGI_FORMAT_R8_TYPELESS = 60,
    DXGI_FORMAT_R8_UNORM = 61,
    DXGI_FORMAT_R8_UINT = 62,
    DXGI_FORMAT_R8_SNORM = 63,
    DXGI_FORMAT_R8_SINT = 64,
    DXGI_FORMAT_A8_UNORM = 65,
    DXGI_FORMAT_R1_UNORM = 66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
    DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
    DXGI_FORMAT_BC1_TYPELESS = 70,
    DXGI_FORMAT_BC1_UNORM = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB = 72,
    DXGI_FORMAT_BC2_TYPELESS = 73,
    DXGI_FORMAT_BC2_UNORM = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB = 75,
    DXGI_FORMAT_BC3_TYPELESS = 76,
    DXGI_FORMAT_BC3_UNORM = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB = 78,
    DXGI_FORMAT_BC4_TYPELESS = 79,
    DXGI_FORMAT_BC4_UNORM = 80,
    DXGI_FORMAT_BC4_SNORM = 81,
    DXGI_FORMAT_BC5_TYPELESS = 82,
    DXGI_FORMAT_BC5_UNORM = 83,
    DXGI_FORMAT_BC5_SNORM = 84,
    DXGI_FORMAT_B5G6R5_UNORM = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM = 88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
    DXGI_FORMAT_BC6H_TYPELESS = 94,
    DXGI_FORMAT_BC6H_UF16 = 95,
    DXGI_FORMAT_BC6H_SF16 = 96,
    DXGI_FORMAT_BC7_TYPELESS = 97,
    DXGI_FORMAT_BC7_UNORM = 98,
    DXGI_FORMAT_BC7_UNORM_SRGB = 99,
    DXGI_FORMAT_AYUV = 100,
    DXGI_FORMAT_Y410 = 101,
    DXGI_FORMAT_Y416 = 102,
    DXGI_FORMAT_NV12 = 103,
    DXGI_FORMAT_P010 = 104,
    DXGI_FORMAT_P016 = 105,
    DXGI_FORMAT_420_OPAQUE = 106,
    DXGI_FORMAT_YUY2 = 107,
    DXGI_FORMAT_Y210 = 108,
    DXGI_FORMAT_Y216 = 109,
    DXGI_FORMAT_NV11 = 110,
    DXGI_FORMAT_AI44 = 111,
    DXGI_FORMAT_IA44 = 112,
    DXGI_FORMAT_P8 = 113,
    DXGI_FORMAT_A8P8 = 114,
    DXGI_FORMAT_B4G4R4A4_UNORM = 115,
    DXGI_FORMAT_P208 = 130,
    DXGI_FORMAT_V208 = 131,
    DXGI_FORMAT_V408 = 132,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE = 189,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,
    DXGI_FORMAT_A4B4G4R4_UNORM = 191,
    DXGI_FORMAT_FORCE_UINT = 0xffffffff
} DXGI_FORMAT;
typedef struct DXGI_SAMPLE_DESC { UINT Count; UINT Quality; } DXGI_SAMPLE_DESC;

// ---- D3D11 enums/constants ----------------------------------------------------
typedef enum D3D_FEATURE_LEVEL {
    D3D_FEATURE_LEVEL_9_1 = 0x9100, D3D_FEATURE_LEVEL_9_2 = 0x9200, D3D_FEATURE_LEVEL_9_3 = 0x9300,
    D3D_FEATURE_LEVEL_10_0 = 0xa000, D3D_FEATURE_LEVEL_10_1 = 0xa100, D3D_FEATURE_LEVEL_11_0 = 0xb000,
    D3D_FEATURE_LEVEL_11_1 = 0xb100
} D3D_FEATURE_LEVEL;
typedef enum D3D11_USAGE { D3D11_USAGE_DEFAULT = 0, D3D11_USAGE_IMMUTABLE = 1, D3D11_USAGE_DYNAMIC = 2, D3D11_USAGE_STAGING = 3 } D3D11_USAGE;
typedef enum D3D11_RESOURCE_DIMENSION {
    D3D11_RESOURCE_DIMENSION_UNKNOWN = 0, D3D11_RESOURCE_DIMENSION_BUFFER = 1, D3D11_RESOURCE_DIMENSION_TEXTURE1D = 2,
    D3D11_RESOURCE_DIMENSION_TEXTURE2D = 3, D3D11_RESOURCE_DIMENSION_TEXTURE3D = 4
} D3D11_RESOURCE_DIMENSION;
typedef enum D3D_SRV_DIMENSION {
    D3D_SRV_DIMENSION_UNKNOWN = 0, D3D_SRV_DIMENSION_BUFFER = 1, D3D_SRV_DIMENSION_TEXTURE1D = 2, D3D_SRV_DIMENSION_TEXTURE1DARRAY = 3,
    D3D_SRV_DIMENSION_TEXTURE2D = 4, D3D_SRV_DIMENSION_TEXTURE2DARRAY = 5, D3D_SRV_DIMENSION_TEXTURE2DMS = 6,
    D3D_SRV_DIMENSION_TEXTURE2DMSARRAY = 7, D3D_SRV_DIMENSION_TEXTURE3D = 8, D3D_SRV_DIMENSION_TEXTURECUBE = 9,
    D3D_SRV_DIMENSION_TEXTURECUBEARRAY = 10, D3D_SRV_DIMENSION_BUFFEREX = 11,
    D3D11_SRV_DIMENSION_UNKNOWN = 0, D3D11_SRV_DIMENSION_BUFFER = 1, D3D11_SRV_DIMENSION_TEXTURE1D = 2, D3D11_SRV_DIMENSION_TEXTURE1DARRAY = 3,
    D3D11_SRV_DIMENSION_TEXTURE2D = 4, D3D11_SRV_DIMENSION_TEXTURE2DARRAY = 5, D3D11_SRV_DIMENSION_TEXTURE2DMS = 6,
    D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY = 7, D3D11_SRV_DIMENSION_TEXTURE3D = 8, D3D11_SRV_DIMENSION_TEXTURECUBE = 9,
    D3D11_SRV_DIMENSION_TEXTURECUBEARRAY = 10, D3D11_SRV_DIMENSION_BUFFEREX = 11
} D3D_SRV_DIMENSION;
typedef D3D_SRV_DIMENSION D3D11_SRV_DIMENSION;
typedef enum D3D11_BIND_FLAG { D3D11_BIND_VERTEX_BUFFER = 0x1, D3D11_BIND_INDEX_BUFFER = 0x2, D3D11_BIND_CONSTANT_BUFFER = 0x4,
    D3D11_BIND_SHADER_RESOURCE = 0x8, D3D11_BIND_STREAM_OUTPUT = 0x10, D3D11_BIND_RENDER_TARGET = 0x20,
    D3D11_BIND_DEPTH_STENCIL = 0x40, D3D11_BIND_UNORDERED_ACCESS = 0x80 } D3D11_BIND_FLAG;
typedef enum D3D11_CPU_ACCESS_FLAG { D3D11_CPU_ACCESS_WRITE = 0x10000, D3D11_CPU_ACCESS_READ = 0x20000 } D3D11_CPU_ACCESS_FLAG;
typedef enum D3D11_RESOURCE_MISC_FLAG { D3D11_RESOURCE_MISC_GENERATE_MIPS = 0x1, D3D11_RESOURCE_MISC_SHARED = 0x2,
    D3D11_RESOURCE_MISC_TEXTURECUBE = 0x4 } D3D11_RESOURCE_MISC_FLAG;
typedef enum D3D11_FORMAT_SUPPORT { D3D11_FORMAT_SUPPORT_TEXTURE1D = 0x10, D3D11_FORMAT_SUPPORT_TEXTURE2D = 0x20,
    D3D11_FORMAT_SUPPORT_TEXTURE3D = 0x40, D3D11_FORMAT_SUPPORT_TEXTURECUBE = 0x80, D3D11_FORMAT_SUPPORT_MIP = 0x1000,
    D3D11_FORMAT_SUPPORT_MIP_AUTOGEN = 0x2000, D3D11_FORMAT_SUPPORT_RENDER_TARGET = 0x4000 } D3D11_FORMAT_SUPPORT;
typedef enum D3D11_MAP { D3D11_MAP_READ = 1, D3D11_MAP_WRITE = 2, D3D11_MAP_READ_WRITE = 3, D3D11_MAP_WRITE_DISCARD = 4,
    D3D11_MAP_WRITE_NO_OVERWRITE = 5 } D3D11_MAP;
#define D3D11_REQ_MIP_LEVELS 15
#define D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION 2048
#define D3D11_REQ_TEXTURE1D_U_DIMENSION 16384
#define D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION 2048
#define D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION 16384
#define D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION 2048
#define D3D11_REQ_TEXTURECUBE_DIMENSION 16384
#define D3D10_REQ_MIP_LEVELS 14
#define D3D10_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION 512
#define D3D10_REQ_TEXTURE1D_U_DIMENSION 8192
#define D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION 512
#define D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION 8192
#define D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION 2048
#define D3D10_REQ_TEXTURECUBE_DIMENSION 8192
inline UINT D3D11CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT MipLevels) { return MipSlice + ArraySlice * MipLevels; }

// ---- D3D11 structs ------------------------------------------------------------
typedef struct D3D11_SUBRESOURCE_DATA { const void* pSysMem; UINT SysMemPitch; UINT SysMemSlicePitch; } D3D11_SUBRESOURCE_DATA;
typedef struct D3D11_MAPPED_SUBRESOURCE { void* pData; UINT RowPitch; UINT DepthPitch; } D3D11_MAPPED_SUBRESOURCE;
typedef struct D3D11_BOX { UINT left, top, front, right, bottom, back; } D3D11_BOX;
typedef struct D3D11_TEXTURE1D_DESC { UINT Width; UINT MipLevels; UINT ArraySize; DXGI_FORMAT Format; D3D11_USAGE Usage;
    UINT BindFlags; UINT CPUAccessFlags; UINT MiscFlags; } D3D11_TEXTURE1D_DESC;
typedef struct D3D11_TEXTURE2D_DESC { UINT Width; UINT Height; UINT MipLevels; UINT ArraySize; DXGI_FORMAT Format;
    DXGI_SAMPLE_DESC SampleDesc; D3D11_USAGE Usage; UINT BindFlags; UINT CPUAccessFlags; UINT MiscFlags; } D3D11_TEXTURE2D_DESC;
typedef struct D3D11_TEXTURE3D_DESC { UINT Width; UINT Height; UINT Depth; UINT MipLevels; DXGI_FORMAT Format; D3D11_USAGE Usage;
    UINT BindFlags; UINT CPUAccessFlags; UINT MiscFlags; } D3D11_TEXTURE3D_DESC;
typedef struct D3D11_SHADER_RESOURCE_VIEW_DESC
{
    DXGI_FORMAT Format;
    D3D11_SRV_DIMENSION ViewDimension;
    union
    {
        struct { UINT FirstElement; UINT NumElements; } Buffer;
        struct { UINT MostDetailedMip; UINT MipLevels; } Texture1D;
        struct { UINT MostDetailedMip; UINT MipLevels; UINT FirstArraySlice; UINT ArraySize; } Texture1DArray;
        struct { UINT MostDetailedMip; UINT MipLevels; } Texture2D;
        struct { UINT MostDetailedMip; UINT MipLevels; UINT FirstArraySlice; UINT ArraySize; } Texture2DArray;
        struct { UINT MostDetailedMip; UINT MipLevels; } Texture3D;
        struct { UINT MostDetailedMip; UINT MipLevels; } TextureCube;
        struct { UINT MostDetailedMip; UINT MipLevels; UINT First2DArrayFace; UINT NumCubes; } TextureCubeArray;
    };
} D3D11_SHADER_RESOURCE_VIEW_DESC;
typedef struct D3D11_INPUT_ELEMENT_DESC { const char* SemanticName; UINT SemanticIndex; DXGI_FORMAT Format; UINT InputSlot;
    UINT AlignedByteOffset; UINT InputSlotClass; UINT InstanceDataStepRate; } D3D11_INPUT_ELEMENT_DESC;

// ---- D3D11 interfaces (only the methods the DDS loader path calls) --------------
struct ID3D11Device;
struct ID3D11DeviceChild : IUnknown
{
    virtual void GetDevice(ID3D11Device** dev) = 0;
    virtual HRESULT SetPrivateData(REFIID, UINT, const void*) = 0;
};
struct ID3D11Resource : ID3D11DeviceChild
{
    virtual void GetType(D3D11_RESOURCE_DIMENSION* dim) = 0;
};
struct ID3D11Texture1D : ID3D11Resource { virtual void GetDesc(D3D11_TEXTURE1D_DESC*) = 0; };
struct ID3D11Texture2D : ID3D11Resource { virtual void GetDesc(D3D11_TEXTURE2D_DESC*) = 0; };
struct ID3D11Texture3D : ID3D11Resource { virtual void GetDesc(D3D11_TEXTURE3D_DESC*) = 0; };
struct ID3D11View : ID3D11DeviceChild { virtual void GetResource(ID3D11Resource**) = 0; };
struct ID3D11ShaderResourceView : ID3D11View { virtual void GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC*) = 0; };
struct ID3D11InputLayout : ID3D11DeviceChild {};
struct ID3D11DeviceContext : ID3D11DeviceChild
{
    virtual HRESULT Map(ID3D11Resource*, UINT, D3D11_MAP, UINT, D3D11_MAPPED_SUBRESOURCE*) = 0;
    virtual void Unmap(ID3D11Resource*, UINT) = 0;
    virtual void UpdateSubresource(ID3D11Resource*, UINT, const D3D11_BOX*, const void*, UINT, UINT) = 0;
    virtual void CopySubresourceRegion(ID3D11Resource*, UINT, UINT, UINT, UINT, ID3D11Resource*, UINT, const D3D11_BOX*) = 0;
    virtual void GenerateMips(ID3D11ShaderResourceView*) = 0;
};
struct ID3D11Device : IUnknown
{
    virtual HRESULT CreateTexture1D(const D3D11_TEXTURE1D_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Texture1D**) = 0;
    virtual HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Texture2D**) = 0;
    virtual HRESULT CreateTexture3D(const D3D11_TEXTURE3D_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Texture3D**) = 0;
    virtual HRESULT CreateShaderResourceView(ID3D11Resource*, const D3D11_SHADER_RESOURCE_VIEW_DESC*, ID3D11ShaderResourceView**) = 0;
    virtual HRESULT CheckFormatSupport(DXGI_FORMAT, UINT*) = 0;
    virtual D3D_FEATURE_LEVEL GetFeatureLevel() = 0;
    virtual void GetImmediateContext(ID3D11DeviceContext**) = 0;
};

// ---- extra declarations for Inc/SpriteBatch.h / Src/SpriteFont.cpp -----------------------------
typedef struct tagRECT { LONG left; LONG top; LONG right; LONG bottom; } RECT;
typedef enum DXGI_MODE_ROTATION { DXGI_MODE_ROTATION_UNSPECIFIED = 0, DXGI_MODE_ROTATION_IDENTITY = 1,
    DXGI_MODE_ROTATION_ROTATE90 = 2, DXGI_MODE_ROTATION_ROTATE180 = 3, DXGI_MODE_ROTATION_ROTATE270 = 4 } DXGI_MODE_ROTATION;
typedef struct D3D11_VIEWPORT { FLOAT TopLeftX, TopLeftY, Width, Height, MinDepth, MaxDepth; } D3D11_VIEWPORT;
struct ID3D11BlendState : ID3D11DeviceChild {};
struct ID3D11SamplerState : ID3D11DeviceChild {};
struct ID3D11DepthStencilState : ID3D11DeviceChild {};
struct ID3D11RasterizerState : ID3D11DeviceChild {};
struct CD3D11_SHADER_RESOURCE_VIEW_DESC : D3D11_SHADER_RESOURCE_VIEW_DESC
{
    CD3D11_SHADER_RESOURCE_VIEW_DESC() = default;
    explicit CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION dim, DXGI_FORMAT fmt = DXGI_FORMAT_UNKNOWN,
        UINT mostDetailedMip = 0, UINT mipLevels = UINT(-1), UINT firstArraySlice = 0, UINT arraySize = UINT(-1))
    {
        *static_cast<D3D11_SHADER_RESOURCE_VIEW_DESC*>(this) = {};
        Format = fmt;
        ViewDimension = dim;
        Texture2DArray.MostDetailedMip = mostDetailedMip;
        Texture2DArray.MipLevels = mipLevels;
        Texture2DArray.FirstArraySlice = firstArraySlice;
        Texture2DArray.ArraySize = arraySize;
    }
};
