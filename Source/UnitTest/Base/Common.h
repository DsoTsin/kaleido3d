#pragma once

#include <Kaleido3D.h>
#include <Core/Os.h>
#include <Core/Bundle.h>
#include <KTL/String.hpp>
#include <KTL/Archive.hpp>
#include <Core/WebSocket.h>
#include <Core/LogUtil.h>

#include <KTL/SharedPtr.hpp>
#include <KTL/DynArray.hpp>

#include <Tools/ShaderGen/Public/ShaderCompiler.h>

#include <Renderer/FontRenderer.h>

#include <memory>
#include <thread>

using threadptr = std::shared_ptr<std::thread>;

#define JOIN_TESTS(...) \
threadptr tests[] = { __VA_ARGS__ }; \
for (auto t : tests) \
{ \
	t->join(); \
}


extern threadptr TestBundle();

extern threadptr TestOs();

extern threadptr TestFontManager();

extern threadptr TestString();

extern threadptr TestWebSocket();

extern threadptr TestShaderCompiler();

extern threadptr TestSharedPtr();

extern threadptr TestDynArrray();
