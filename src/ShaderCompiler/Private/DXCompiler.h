#pragma once
#ifndef __DXCompiler_h__
#define __DXCompiler_h__

#if K3DPLATFORM_OS_WIN && ENABLE_D3D12_BUILD

#include <d3dcommon.h>
#include <wrl/client.h>
#include <sstream>

namespace k3d 
{
	using namespace Microsoft::WRL;

	class DXCompiler : public k3d::IShCompiler
	{
	public:
		DXCompiler() {}
		~DXCompiler() override {}

		typedef ::k3d::SharedPtr<IShCompiler> Ptr;
		virtual k3d::shc::NGFXShaderCompileResult Compile(
			String const& src,
			NGFXShaderDesc const& inOp,
			NGFXShaderBundle & bundle) override;
		const char *				GetVersion();
	};
}
#endif
#endif