#include <Kaleido3D.h>
#include "MetalCompiler.h"
#include <Core/Utils/MD5.h>
#include <Core/Os.h>
#include "GLSLangUtils.h"
#include "SPIRVCrossUtils.h"

#define METAL_BIN_DIR_MACOS "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/usr/bin/"
#define METAL_BIN_DIR_IOS "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/usr/bin/"
#define METAL_COMPILE_TMP "/.metaltmp/"
#define COMPILE_OPTION "-arch air64 -emit-llvm -c"

#include <string.h>

using namespace std;

namespace k3d
{
    NGFXShaderCompileResult mtlCompile(string const& source, String & metalIR);

    MetalCompiler::MetalCompiler()
    {
        sInitializeGlSlang();
    }
    
    MetalCompiler::~MetalCompiler()
    {
        sFinializeGlSlang();
    }

  NGFXShaderCompileResult MetalCompiler::Compile(String const& src, NGFXShaderDesc const& inOp, NGFXShaderBundle& bundle)
  {
    if (inOp.Format == NGFX_SHADER_FORMAT_TEXT)
    {
      if (inOp.Lang == NGFX_SHADER_LANG_METALSL)
      {
        if (m_IsMac)
        {
        }
        else // iOS
        {
        }
      }
      else // process hlsl or glsl
      {
        bool canConvertToMetalSL = false;
        switch (inOp.Lang)
        {
        case NGFX_SHADER_LANG_ESSL:
        case NGFX_SHADER_LANG_GLSL:
        case NGFX_SHADER_LANG_HLSL:
        case NGFX_SHADER_LANG_VKGLSL:
          canConvertToMetalSL = true;
          break;
        default:
          break;
        }
        if (canConvertToMetalSL)
        {
          EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
          switch (inOp.Lang)
          {
          case NGFX_SHADER_LANG_ESSL:
          case NGFX_SHADER_LANG_GLSL:
          case NGFX_SHADER_LANG_VKGLSL:
            break;
          case NGFX_SHADER_LANG_HLSL:
            messages = (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules | EShMsgReadHlsl);
            break;
          default:
            break;
          }
          glslang::TProgram& program = *new glslang::TProgram;
          TBuiltInResource Resources;
          initResources(Resources);

          const char* shaderStrings[1];
          EShLanguage stage = findLanguage(inOp.Stage);
          glslang::TShader* shader = new glslang::TShader(stage);

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
          vector<unsigned int> spirv;
          GlslangToSpv(*program.getIntermediate(stage), spirv);

          if (program.buildReflection())
          {
            ExtractAttributeData(program, bundle.Attributes);
            ExtractUniformData(inOp.Stage, program, bundle.BindingTable);
          }
          else
          {
            return NGFX_SHADER_COMPILE_FAILED;
          }
          uint32 bufferLoc = 0;
          vector<spirv_cross::MSLVertexAttr> vertAttrs;
          for (auto& attr : bundle.Attributes)
          {
            spirv_cross::MSLVertexAttr vAttrib;
            vAttrib.location = attr.VarLocation;
            vAttrib.msl_buffer = attr.VarBindingPoint;
            vertAttrs.push_back(vAttrib);
            bufferLoc = attr.VarBindingPoint;
          }
          vector<spirv_cross::MSLResourceBinding> resBindings;
          for (auto& binding : bundle.BindingTable.Bindings)
          {
            if (binding.VarType == NGFX_SHADER_BIND_BLOCK)
            {
              bufferLoc ++;
              spirv_cross::MSLResourceBinding resBind;
              resBind.stage = rhiShaderStageToSpvModel(binding.VarStage);
              resBind.desc_set = 0;
              resBind.binding = binding.VarNumber;
              resBind.msl_buffer = bufferLoc;
              resBindings.push_back(resBind);
            }
          }
          auto metalc = make_unique<spirv_cross::CompilerMSL>(spirv);
          spirv_cross::CompilerMSL::Options config;
          config.flip_vert_y = false;
          config.entry_point_name = inOp.EntryFunction.CStr();
          auto result = metalc->compile(config, &vertAttrs, &resBindings);
          if (m_IsMac)
          {
            auto ret = mtlCompile(result, bundle.RawData);
            if (ret == NGFX_SHADER_COMPILE_FAILED)
              return ret;
            bundle.Desc = inOp;
            bundle.Desc.Format = NGFX_SHADER_FORMAT_BYTE_CODE;
            bundle.Desc.Lang = NGFX_SHADER_LANG_METALSL;
          }
          else
          {
            bundle.RawData = {result.c_str()};
            bundle.Desc = inOp;
            bundle.Desc.Format = NGFX_SHADER_FORMAT_TEXT;
            bundle.Desc.Lang = NGFX_SHADER_LANG_METALSL;
          }
        }
      }
    }
    else
    {
      if (inOp.Lang == NGFX_SHADER_LANG_METALSL)
      {
      }
      else
      {
      }
    }
    return NGFX_SHADER_COMPILE_OK;
  }
    
    const char*
    MetalCompiler::GetVersion()
    {
        return "Metal";
    }
    
    NGFXShaderCompileResult mtlCompile(string const& source, String & metalIR)
    {
#if K3DPLATFORM_OS_MAC 
        MD5 md5(source);
        auto name = md5.toString();
        auto intermediate = string(".") + METAL_COMPILE_TMP;
        Os::Path::MakeDir(intermediate.c_str());
        auto tmpSh = intermediate + name + ".metal";
        auto tmpAr = intermediate + name + ".air";
        auto tmpLib = intermediate + name + ".metallib";
        Os::File shSrcFile(tmpSh.c_str());
        shSrcFile.Open(IOWrite);
        shSrcFile.Write(source.data(), source.size());
        shSrcFile.Close();
        auto mcc = string(METAL_BIN_DIR_MACOS) + "metal";
        auto mlink = string(METAL_BIN_DIR_MACOS) + "metallib";
        auto ccmd = mcc + " -arch air64 -c -o " + tmpAr + " " + tmpSh;
        auto ret = system(ccmd.c_str());
        if(ret)
        {
            return NGFX_SHADER_COMPILE_FAILED;
        }
        auto lcmd = mlink + " -split-module -o " + tmpLib + " " + tmpAr;
        ret = system(lcmd.c_str());
        if(ret)
        {
            return NGFX_SHADER_COMPILE_FAILED;
        }
        Os::MemMapFile bcFile;
        bcFile.Open(tmpLib.c_str(), IORead);
        metalIR = { bcFile.FileData(), (size_t)bcFile.GetSize() };
        bcFile.Close();
        //Os::Remove(intermediate.c_str());
#endif
        return NGFX_SHADER_COMPILE_OK;
    }
}
