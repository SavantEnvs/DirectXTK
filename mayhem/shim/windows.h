// Minimal Win32 shim so DirectXTK's binary-asset parsers compile and run on Linux.
// Only types/APIs the parsers use; file APIs are implemented synchronously over POSIX fds.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cwchar>
#include <climits>
// Windows is LLP64: LONG and LONG_MAX are 32-bit (SpriteFont.cpp initialises a RECT with LONG_MAX).
#undef LONG_MAX
#define LONG_MAX 2147483647L
#undef LONG_MIN
#define LONG_MIN (-LONG_MAX - 1L)
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#define _WIN32_WINNT_WIN10 0x0A00
#define _WIN32_WINNT_WIN8 0x0602
#define _WIN32_WINNT_WINBLUE 0x0603
#define WINAPI
#define __cdecl
#define __stdcall
#define CALLBACK
#define STDMETHODCALLTYPE
#define UNREFERENCED_PARAMETER(x) (void)(x)

#include "sal.h"
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef int INT;
typedef unsigned int UINT;
typedef int BOOL;
typedef int32_t HRESULT;
typedef void* HANDLE;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef DWORD* LPDWORD;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef const wchar_t* LPCWSTR;
typedef const char* LPCSTR;
typedef uintptr_t ULONG_PTR;
typedef size_t SIZE_T;
typedef float FLOAT;

typedef union _LARGE_INTEGER { struct { DWORD LowPart; LONG HighPart; }; LONGLONG QuadPart; } LARGE_INTEGER;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define INFINITE 0xFFFFFFFFu

typedef struct _GUID { uint32_t Data1; uint16_t Data2; uint16_t Data3; uint8_t Data4[8]; } GUID;
typedef GUID IID;
inline bool operator==(const GUID& a, const GUID& b) { return memcmp(&a, &b, sizeof(GUID)) == 0; }
inline bool operator!=(const GUID& a, const GUID& b) { return !(a == b); }

#define S_OK ((HRESULT)0)
#define S_FALSE ((HRESULT)1)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_POINTER ((HRESULT)0x80004003L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#define E_ABORT ((HRESULT)0x80004004L)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_INVALID_HANDLE 6L
#define ERROR_NOT_ENOUGH_MEMORY 8L
#define ERROR_INVALID_DATA 13L
#define ERROR_HANDLE_EOF 38L
#define ERROR_NOT_SUPPORTED 50L
#define ERROR_INVALID_PARAMETER 87L
#define ERROR_ARITHMETIC_OVERFLOW 534L
#define ERROR_IO_PENDING 997L
#define ERROR_IO_INCOMPLETE 996L
#define ERROR_FILE_TOO_LARGE 223L
#define ERROR_NOT_FOUND 1168L
#define FACILITY_WIN32 7
inline HRESULT HRESULT_FROM_WIN32(unsigned long x)
{
    return (HRESULT)(x) <= 0 ? (HRESULT)(x) : (HRESULT)(((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000);
}

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                                          \
    (static_cast<uint32_t>(static_cast<uint8_t>(ch0)) | (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8) \
        | (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16) | (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24))

template <size_t N>
inline int sprintf_s(char (&buf)[N], const char* fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    int r = vsnprintf(buf, N, fmt, a);
    va_end(a);
    return r;
}
template <size_t N>
inline int vsprintf_s(char (&buf)[N], const char* fmt, va_list a)
{
    return vsnprintf(buf, N, fmt, a);
}
inline void OutputDebugStringA(const char*) {}

// ---- thread-local last error -------------------------------------------------
inline DWORD& shim_last_error() { static thread_local DWORD e = 0; return e; }
inline DWORD GetLastError() { return shim_last_error(); }
inline void SetLastError(DWORD e) { shim_last_error() = e; }

// ---- memory ------------------------------------------------------------------
#define MEM_RELEASE 0x8000
inline BOOL VirtualFree(void* p, size_t, DWORD) { free(p); return TRUE; }

// ---- files (synchronous POSIX implementation) --------------------------------
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define GENERIC_READ 0x80000000u
#define FILE_SHARE_READ 0x1u
#define OPEN_EXISTING 3
#define FILE_FLAG_OVERLAPPED 0x40000000u
#define FILE_FLAG_NO_BUFFERING 0x20000000u
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000u
#define FILE_ATTRIBUTE_NORMAL 0x80u
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define CREATE_EVENT_MANUAL_RESET 0x1
#define EVENT_MODIFY_STATE 0x2
#define SYNCHRONIZE 0x00100000L
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258

struct shim_handle { int fd; bool is_event; };

typedef struct _CREATEFILE2_EXTENDED_PARAMETERS {
    DWORD dwSize; DWORD dwFileAttributes; DWORD dwFileFlags; DWORD dwSecurityQosFlags;
    void* lpSecurityAttributes; HANDLE hTemplateFile;
} CREATEFILE2_EXTENDED_PARAMETERS;

typedef struct _OVERLAPPED {
    ULONG_PTR Internal; ULONG_PTR InternalHigh;
    union { struct { DWORD Offset; DWORD OffsetHigh; }; void* Pointer; };
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef enum _FILE_INFO_BY_HANDLE_CLASS { FileBasicInfo = 0, FileStandardInfo = 1 } FILE_INFO_BY_HANDLE_CLASS;
typedef struct _FILE_STANDARD_INFO {
    LARGE_INTEGER AllocationSize; LARGE_INTEGER EndOfFile; DWORD NumberOfLinks; BOOL DeletePending; BOOL Directory;
} FILE_STANDARD_INFO;

inline HANDLE CreateFile2(const wchar_t* name, DWORD, DWORD, DWORD, CREATEFILE2_EXTENDED_PARAMETERS*)
{
    char path[4096];
    size_t n = wcstombs(path, name, sizeof(path) - 1);
    if (n == (size_t)-1) { SetLastError(ERROR_FILE_NOT_FOUND); return INVALID_HANDLE_VALUE; }
    path[n] = 0;
    int fd = open(path, O_RDONLY);
    if (fd < 0) { SetLastError(ERROR_FILE_NOT_FOUND); return INVALID_HANDLE_VALUE; }
    return new shim_handle{ fd, false };
}
inline BOOL CloseHandle(HANDLE h)
{
    if (!h || h == INVALID_HANDLE_VALUE) return FALSE;
    auto s = static_cast<shim_handle*>(h);
    if (!s->is_event) close(s->fd);
    delete s;
    return TRUE;
}
inline HANDLE CreateEventEx(void*, const wchar_t*, DWORD, DWORD) { return new shim_handle{ -1, true }; }
inline DWORD WaitForSingleObjectEx(HANDLE, DWORD, BOOL) { return WAIT_OBJECT_0; }
inline BOOL GetFileInformationByHandleEx(HANDLE h, FILE_INFO_BY_HANDLE_CLASS, void* out, DWORD)
{
    struct stat st;
    if (fstat(static_cast<shim_handle*>(h)->fd, &st) != 0) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
    auto fi = static_cast<FILE_STANDARD_INFO*>(out);
    memset(fi, 0, sizeof(*fi));
    fi->EndOfFile.QuadPart = st.st_size;
    fi->AllocationSize.QuadPart = st.st_size;
    return TRUE;
}
// Overlapped reads complete synchronously; the byte count is kept in InternalHigh
// for GetOverlappedResultEx, as on Windows.
inline BOOL ReadFile(HANDLE h, void* buf, DWORD len, DWORD* read, OVERLAPPED* ov)
{
    int fd = static_cast<shim_handle*>(h)->fd;
    ssize_t r;
    if (ov)
    {
        off_t off = (off_t)(((uint64_t)ov->OffsetHigh << 32) | ov->Offset);
        r = pread(fd, buf, len, off);
    }
    else
        r = ::read(fd, buf, len);
    if (r < 0) { SetLastError(ERROR_INVALID_DATA); return FALSE; }
    if (ov) { ov->Internal = 0; ov->InternalHigh = (ULONG_PTR)r; }
    if (read) *read = (DWORD)r;
    if (ov && r == 0 && len > 0) { SetLastError(ERROR_HANDLE_EOF); return FALSE; }
    return TRUE;
}
inline BOOL GetOverlappedResultEx(HANDLE, OVERLAPPED* ov, DWORD* bytes, DWORD, BOOL)
{
    if (bytes) *bytes = (DWORD)ov->InternalHigh;
    return TRUE;
}
inline BOOL GetOverlappedResult(HANDLE h, OVERLAPPED* ov, DWORD* bytes, BOOL w) { return GetOverlappedResultEx(h, ov, bytes, 0, w); }
inline BOOL SetFilePointerEx(HANDLE h, LARGE_INTEGER dist, LARGE_INTEGER* newpos, DWORD method)
{
    off_t r = lseek(static_cast<shim_handle*>(h)->fd, (off_t)dist.QuadPart,
        method == FILE_BEGIN ? SEEK_SET : method == FILE_CURRENT ? SEEK_CUR : SEEK_END);
    if (r < 0) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
    if (newpos) newpos->QuadPart = r;
    return TRUE;
}
inline BOOL CancelIo(HANDLE) { return TRUE; }

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(T)                                                                            \
    inline constexpr T operator|(T a, T b) noexcept { return T(static_cast<unsigned>(a) | static_cast<unsigned>(b)); } \
    inline T& operator|=(T& a, T b) noexcept { return a = a | b; }                                                    \
    inline constexpr T operator&(T a, T b) noexcept { return T(static_cast<unsigned>(a) & static_cast<unsigned>(b)); } \
    inline T& operator&=(T& a, T b) noexcept { return a = a & b; }                                                    \
    inline constexpr T operator~(T a) noexcept { return T(~static_cast<unsigned>(a)); }                              \
    inline constexpr T operator^(T a, T b) noexcept { return T(static_cast<unsigned>(a) ^ static_cast<unsigned>(b)); } \
    inline T& operator^=(T& a, T b) noexcept { return a = a ^ b; }
#endif

// ---- UTF-8 -> wchar_t (SpriteFont's char* overloads) ---------------------------------------------
#include <cwctype>
#define CP_UTF8 65001
#define ERROR_INSUFFICIENT_BUFFER 122L
#define ERROR_NO_UNICODE_TRANSLATION 1113L
// Strict UTF-8 decoder with MultiByteToWideChar's buffer contract (cbMultiByte == -1: NUL included).
inline int MultiByteToWideChar(UINT, DWORD, const char* src, int srcLen, wchar_t* dst, int dstLen)
{
    const unsigned char* s = reinterpret_cast<const unsigned char*>(src);
    size_t n = (srcLen < 0) ? strlen(src) + 1 : size_t(srcLen);
    int out = 0;
    for (size_t i = 0; i < n;)
    {
        uint32_t c = s[i];
        int extra = c < 0x80 ? 0 : (c & 0xE0) == 0xC0 ? 1 : (c & 0xF0) == 0xE0 ? 2 : (c & 0xF8) == 0xF0 ? 3 : -1;
        if (extra < 0 || i + extra >= n + (extra ? 0 : 1)) { SetLastError(ERROR_NO_UNICODE_TRANSLATION); return 0; }
        if (extra) c &= (0x3F >> extra);
        for (int k = 1; k <= extra; ++k)
        {
            if ((s[i + k] & 0xC0) != 0x80) { SetLastError(ERROR_NO_UNICODE_TRANSLATION); return 0; }
            c = (c << 6) | (s[i + k] & 0x3F);
        }
        i += size_t(extra) + 1;
        if (dstLen)
        {
            if (out >= dstLen) { SetLastError(ERROR_INSUFFICIENT_BUFFER); return 0; }
            dst[out] = wchar_t(c);
        }
        ++out;
    }
    return out;
}
