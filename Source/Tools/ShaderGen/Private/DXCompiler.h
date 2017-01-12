#pragma once
#ifndef __DXCompiler_h__
#define __DXCompiler_h__

#if _WIN32

#include <d3dcommon.h>
#include <wrl/client.h>
#include <sstream>

namespace k3d 
{
	using namespace Microsoft::WRL;

	class DXCompiler : public rhi::IShCompiler
	{
	public:
		DXCompiler() {}
		~DXCompiler() override {}

		typedef ::k3d::SharedPtr<IShCompiler> Ptr;
		virtual rhi::shc::EResult Compile(
			String const& src,
			rhi::ShaderDesc const& inOp,
			rhi::ShaderBundle & bundle) override;
		const char *				GetVersion();
	};
}
#endif
#endif