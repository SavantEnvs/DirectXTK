// Link stand-in for the two SpriteBatch symbols Src/SpriteFont.cpp references from DrawString.
// The real Src/SpriteBatch.cpp needs compiled HLSL shaders and a live D3D11 context, so it is not
// part of the Linux build; the spritefont harness only parses fonts and lays out glyphs and never
// draws. Reaching either function is a harness bug, so both abort.
#include "pch.h"
#include "SpriteBatch.h"

#include <cstdio>
#include <cstdlib>

using namespace DirectX;

// never instantiated here (no SpriteBatch is ever constructed); only completes the type for ~SpriteBatch
class SpriteBatch::Impl
{};

SpriteBatch::~SpriteBatch()
{
    fputs("SpriteBatch is not available in the Linux build\n", stderr);
    abort();
}

void XM_CALLCONV SpriteBatch::Draw(ID3D11ShaderResourceView*, FXMVECTOR, RECT const*, FXMVECTOR, float, FXMVECTOR,
    GXMVECTOR, SpriteEffects, float)
{
    fputs("SpriteBatch::Draw is not available in the Linux build\n", stderr);
    abort();
}
