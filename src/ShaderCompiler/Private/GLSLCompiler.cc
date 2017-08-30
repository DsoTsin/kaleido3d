#if USE_GLSLANG
#include <SPIRV/GlslangToSpv.h>
#else
#include <shaderc/shaderc.hpp>
#endif
#include <Core/Kaleido3D.h>
#include <Core/LogUtil.h>
#include <algorithm>

#include "GLSLCompiler.h"
#include "GLSLangUtils.h"
#include "SPIRVCrossUtils.h"

namespace k3d {
#if USE_GLSLANG
using namespace ::glslang;
#endif

NGFXShaderSemantic
attributeNameToSemantic(std::string const& attribName)
{
  if (attribName == "POSITION")
  {
    return NGFX_SEMANTIC_POSITION;
  }
  if (attribName == "COLOR")
  {
    return NGFX_SEMANTIC_COLOR;
  }
  if (attribName == "NORMAL")
  {
    return NGFX_SEMANTIC_NORMAL;
  }

  return NGFX_SEMANTIC_POSITION;
}

GLSLangCompiler::GLSLangCompiler()
{
  sInitializeGlSlang();
}

GLSLangCompiler::~GLSLangCompiler()
{
  sFinializeGlSlang();
}

  NGFXShaderCompileResult
  GLSLangCompiler::Compile(const String& src,
                           const NGFXShaderDesc& inOp,
                           NGFXShaderBundle& bundle)
  {
    if (inOp.Format == NGFX_SHADER_FORMAT_TEXT)
    {
      if (inOp.Lang == NGFX_SHADER_LANG_METALSL)
        return NGFX_SHADER_COMPILE_FAILED;

      EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
      switch (inOp.Lang)
      {
      case NGFX_SHADER_LANG_ESSL:
      case NGFX_SHADER_LANG_GLSL:
      case NGFX_SHADER_LANG_VKGLSL:
        break;
      case NGFX_SHADER_LANG_HLSL:
        messages =
          (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgReadHlsl);
        break;
      default:
        break;
      }
      TProgram& program = *new TProgram;
      TBuiltInResource Resources;
      initResources(Resources);

      const char* shaderStrings[1];
      EShLanguage stage = findLanguage(inOp.Stage);
      TShader* shader = new TShader(stage);

      shaderStrings[0] = src.CStr();
      shader->setStrings(shaderStrings, 1);
      shader->setEntryPoint(inOp.EntryFunction.CStr());

      if (!shader->parse(&Resources, 100, false, messages))
      {
        puts(shader->getInfoLog());
        puts(shader->getInfoDebugLog());
        return NGFX_SHADER_COMPILE_FAILED;
      }
      program.addShader(shader);
      if (!program.link(messages))
      {
        puts(program.getInfoLog());
        puts(program.getInfoDebugLog());
        return NGFX_SHADER_COMPILE_FAILED;
      }
      std::vector<unsigned int> spirv;
      GlslangToSpv(*program.getIntermediate(stage), spirv);

      bundle.RawData = {spirv.data(), spirv.size() * sizeof(uint32)};
      bundle.Desc = inOp;
      bundle.Desc.Format = NGFX_SHADER_FORMAT_BYTE_CODE;

      if (program.buildReflection())
      {
        ExtractAttributeData(program, bundle.Attributes);
        ExtractUniformData(inOp.Stage, program, bundle.BindingTable);
      }
      else
      {
        return NGFX_SHADER_COMPILE_FAILED;
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
    return NGFX_SHADER_COMPILE_OK;
}
}
