#include "ShaderCompiler.h"
#include "GLSLCompiler.h"
#if _WIN32
#include "DXCompiler.h"
#endif
using namespace k3d;

k3d::IShaderCompiler * CreateShaderCompiler(k3d::EShaderLang lang) 
{
	switch (lang)
	{
	case EShaderLang::GLSL:
		return new GLSLCompiler;
#if _WIN32
	case EShaderLang::HLSL:
		return new DXCompiler;
#endif
	}
	return nullptr;
}
