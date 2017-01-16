#if USE_GLSLANG
#include <glslang/GlslangToSpv.h>
#else
#include <shaderc/shaderc.hpp>
#endif
#include "Kaleido3D.h"
#include <algorithm>
#include <Core/LogUtil.h>

#include "GLSLCompiler.h"
#include "GLSLangUtils.h"
#include "SPIRVCrossUtils.h"

namespace k3d {
#if USE_GLSLANG
	using namespace ::glslang;
#endif
	using namespace rhi::shc;

	ESemantic attributeNameToSemantic(std::string const& attribName)
	{
		if (attribName == "POSITION")
		{
			return ESemantic::EPOSITION;
		}
		else if (attribName == "COLOR")
		{
			return ESemantic::ECOLOR;
		}
		else if (attribName == "NORMAL")
		{
			return ESemantic::ENORMAL;
		}

		return ESemantic::ENumSemanics;
	}
	   
	GLSLangCompiler::GLSLangCompiler()
    {
        sInitializeGlSlang();
    }
    
	GLSLangCompiler::~GLSLangCompiler()
    {
        sFinializeGlSlang();
    }
    
    rhi::shc::EResult
	GLSLangCompiler::Compile(const String &src, const rhi::ShaderDesc &inOp, rhi::ShaderBundle &bundle)
    {
        if(inOp.Format == rhi::EShaderFormat::EShFmt_Text)
        {
            if(inOp.Lang == rhi::EShLang_MetalSL)
                return rhi::shc::E_Failed;
            
            EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
            switch(inOp.Lang)
            {
                case rhi::EShLang_ESSL:
                case rhi::EShLang_GLSL:
                case rhi::EShLang_VkGLSL:
                    break;
                case rhi::EShLang_HLSL:
                    messages = (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgReadHlsl);
                    break;
                default:
                    break;
            }
            glslang::TProgram& program = *new glslang::TProgram;
            TBuiltInResource Resources;
            initResources(Resources);
            
            const char *shaderStrings[1];
            EShLanguage stage = findLanguage(inOp.Stage);
            glslang::TShader* shader = new glslang::TShader(stage);
            
            shaderStrings[0] = src.CStr();
            shader->setStrings(shaderStrings, 1);
            shader->setEntryPoint(inOp.EntryFunction.CStr());
            
            if (!shader->parse(&Resources, 100, false, messages)) {
                puts(shader->getInfoLog());
                puts(shader->getInfoDebugLog());
                return rhi::shc::E_Failed;
            }
            program.addShader(shader);
            if (!program.link(messages)) {
                puts(program.getInfoLog());
                puts(program.getInfoDebugLog());
                return rhi::shc::E_Failed;
            }
            std::vector<unsigned int> spirv;
            glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
            
            bundle.RawData = {spirv.data(), spirv.size() * sizeof(uint32)};
            bundle.Desc = inOp;
			bundle.Desc.Format = rhi::EShFmt_ByteCode;
            
            if(program.buildReflection())
            {
				ExtractAttributeData(program, bundle.Attributes);
				ExtractUniformData(inOp.Stage, program, bundle.BindingTable);
            }
			else
			{
				return rhi::shc::E_Failed;
			}
        }
		else // byteCode reflection
		{
			const uint32* begin = reinterpret_cast<const uint32*>(src.Data());
			size_t count = src.Length() / 4;
			SPIRV_T spirv(count);
			spirv.assign(begin, begin + count);
			spirv_cross::CompilerGLSL glslangCompiler(spirv);
			ExtractAttributeData(glslangCompiler, bundle.Attributes);
			ExtractUniformData(inOp.Stage, glslangCompiler, bundle.BindingTable);
			bundle.RawData = src;
			bundle.Desc = inOp;
		}
        return rhi::shc::E_Ok;
    }
    
}
