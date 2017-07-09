#include <Kaleido3D.h>
#include "Public/ShaderCompiler.h"
#include <Core/LogUtil.h>
#include "GLSLCompiler.h"
#if K3DPLATFORM_OS_WIN && ENABLE_D3D12_BUILD
#include "DXCompiler.h"
#endif
#include "MetalCompiler.h"

using namespace k3d;

class ShaderCompilerModule : public k3d::IShModule
{
public:
	ShaderCompilerModule() {}
	~ShaderCompilerModule() override 
	{
		KLOG(Info, ShaderCompilerModule, "Destroying..");
	}

	void Start() override {}
	void Shutdown() override {}
	const char * Name() override { return "ShaderCompiler"; }

	IShCompiler::Ptr CreateShaderCompiler(NGFXRHIType const& rhiType) override
	{
		switch (rhiType)
		{
		case NGFX_RHI_VULKAN:
			return MakeShared<GLSLangCompiler>();
#if K3DPLATFORM_OS_WIN && ENABLE_D3D12_BUILD
		case NGFX_RHI_D3D12:
			break;
#endif
		case NGFX_RHI_METAL:
            return MakeShared<MetalCompiler>();
		}
		return nullptr;
	}

};

MODULE_IMPLEMENT(ShaderCompiler, ShaderCompilerModule)
