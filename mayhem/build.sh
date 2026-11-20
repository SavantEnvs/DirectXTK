#!/usr/bin/env bash
#
# mayhem/build.sh — build DirectXTK's binary-asset parsers for Linux and their fuzz harnesses.
#
# DirectXTK is a Windows-only (D3D11/XAudio2) library with no Linux build system, so this compiles
# just the parser sources (Audio/WAVFileReader.cpp, Audio/WaveBankReader.cpp, Src/DDSTextureLoader.cpp)
# directly, against the Win32 shim in mayhem/shim (types, SAL no-ops, POSIX-backed CreateFile2/ReadFile,
# declaration-only XAudio2/X3DAudio types for Inc/Audio.h, a minimal D3D11/DXGI/WRL surface for the
# DDS loader; the harness supplies a stub device, mayhem/d3d11_stub_device.h) and DirectXMath from /opt/toolchains/deps (Inc/Audio.h
# needs it). Upstream sources are not modified.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${DIRECTXMATH_INC:=/opt/toolchains/deps/directxmath-6ac6a3c/src/Inc}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"
OUT="$SRC/mayhem/build-out"
mkdir -p "$OUT/obj"

INC=(-I"$SRC/mayhem/shim" -I"$SRC/mayhem" -I"$SRC/Audio" -I"$SRC/Src" -I"$SRC/Inc" -I"$DIRECTXMATH_INC")
CXXSTD=(-std=c++17 -Wno-pragma-pack -Wno-unknown-pragmas)
LIBSRC=(Audio/WAVFileReader.cpp Audio/WaveBankReader.cpp Src/DDSTextureLoader.cpp Src/SpriteFont.cpp Src/BinaryReader.cpp
  mayhem/spritebatch_stub.cpp)

# 1) Sanitized, instrumented library objects (shared by every harness).
FUZZ_FLAGS="$SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize=fuzzer-no-link"
LIBOBJ=()
for s in "${LIBSRC[@]}"; do
  o="$OUT/obj/$(basename "${s%.cpp}").o"
  # shellcheck disable=SC2086
  $CXX "${CXXSTD[@]}" $FUZZ_FLAGS "${INC[@]}" -c "$s" -o "$o"
  LIBOBJ+=("$o")
done
# shellcheck disable=SC2086
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c mayhem/lsan_off.cc -o "$OUT/obj/lsan_off.o"
# shellcheck disable=SC2086
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OUT/obj/standalone_main.o"

# 2) Each harness: libFuzzer binary + standalone reproducer.
for h in wav xwb dds spritefont; do
  # shellcheck disable=SC2086
  $CXX "${CXXSTD[@]}" $FUZZ_FLAGS "${INC[@]}" -c "mayhem/fuzz_$h.cpp" -o "$OUT/obj/fuzz_$h.o"
  # shellcheck disable=SC2086
  $CXX $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE "$OUT/obj/fuzz_$h.o" "${LIBOBJ[@]}" \
    "$OUT/obj/lsan_off.o" -o "$OUT/fuzz_$h"
  # shellcheck disable=SC2086
  $CXX $SANITIZER_FLAGS $DEBUG_FLAGS "$OUT/obj/fuzz_$h.o" "${LIBOBJ[@]}" "$OUT/obj/lsan_off.o" \
    "$OUT/obj/standalone_main.o" -o "$OUT/fuzz_$h-standalone"
done

# 3) Known-answer test runner with normal flags (no sanitizers), for mayhem/test.sh.
# shellcheck disable=SC2086
$CXX "${CXXSTD[@]}" -O1 $COVERAGE_FLAGS "${INC[@]}" "${LIBSRC[@]}" mayhem/selftest.cpp \
  -o "$OUT/directxtk_selftest"

ls -la "$OUT"
