#include "Public/ShaderCompiler.h"
#include "GLSLCompiler.h"
#if _WIN32
#include "DXCompiler.h"
#endif
#include "MetalCompiler.h"

using namespace k3d;
using namespace rhi;

class ShaderCompilerModule : public rhi::IShModule
{
public:
	ShaderCompilerModule() {}
	~ShaderCompilerModule() override {}

	void Start() override {}
	void Shutdown() override {}
	const char * Name() override { return "ShaderCompiler"; }

	IShCompiler::Ptr CreateShaderCompiler(ERHIType const& rhiType) override
	{
		switch (rhiType)
		{
		case ERHI_Vulkan:
			return MakeShared<GLSLangCompiler>();
#if K3DPLATFORM_OS_WIN
		case ERHI_D3D12:
			break;
#endif
		case ERHI_Metal:
            return MakeShared<MetalCompiler>();
		}
		return nullptr;
	}

};

MODULE_IMPLEMENT(ShaderCompiler, ShaderCompilerModule)
