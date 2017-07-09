#include <Kaleido3D.h>
#include "GLSLangUtils.h"
#include <glslang/MachineIndependent/gl_types.h>

using namespace ::glslang;
using namespace ::k3d;

void sInitializeGlSlang()
{
#if USE_GLSLANG
    static bool sGlSlangIntialized = false;
    if (!sGlSlangIntialized) {
        glslang::InitializeProcess();
        sGlSlangIntialized = true;
    }
#endif
}

void sFinializeGlSlang()
{
#if USE_GLSLANG
    static bool sGlSlangFinalized = false;
    if (!sGlSlangFinalized) {
        glslang::FinalizeProcess();
        sGlSlangFinalized = true;
    }
#endif
}

NGFXShaderDataType glTypeToRHIAttribType(int glType)
{
  switch (glType)
  {
  case GL_FLOAT:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT;
  case GL_FLOAT_VEC2:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT2;
  case GL_FLOAT_VEC3:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT3;
  case GL_FLOAT_VEC4:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT4;
  case GL_INT:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT;
  case GL_INT_VEC2:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT2;
  case GL_INT_VEC3:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT3;
  case GL_INT_VEC4:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT4;
  case GL_UNSIGNED_INT:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT;
  case GL_UNSIGNED_INT_VEC2:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT2;
  case GL_UNSIGNED_INT_VEC3:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT3;
  case GL_UNSIGNED_INT_VEC4:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT4;
  }
  return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
}


NGFXShaderDataType glTypeToRHIUniformType(int glType)
{
  switch (glType)
  {
  case GL_FLOAT:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT;
  case GL_FLOAT_VEC2:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT2;
  case GL_FLOAT_VEC3:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT3;
  case GL_FLOAT_VEC4:
    return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT4;
  case GL_INT:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT;
  case GL_INT_VEC2:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT2;
  case GL_INT_VEC3:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT3;
  case GL_INT_VEC4:
    return NGFXShaderDataType::NGFX_SHADER_VAR_INT4;
  case GL_UNSIGNED_INT:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT;
  case GL_UNSIGNED_INT_VEC2:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT2;
  case GL_UNSIGNED_INT_VEC3:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT3;
  case GL_UNSIGNED_INT_VEC4:
    return NGFXShaderDataType::NGFX_SHADER_VAR_UINT4;
  }
  return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
}

NGFXShaderDataType glslangDataTypeToRHIDataType(const TType& type)
{
	switch (type.getBasicType()) {
	case EbtSampler:
  {
    switch (type.getQualifier().layoutFormat)
    {
    case ElfRgba32f:
      return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT4;
    }
  }
	case EbtStruct:
	case EbtBlock:
	case EbtVoid:
		return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
	default:
		break;
	}

	if (type.isVector()) {
		int offset = type.getVectorSize() - 2;
		switch (type.getBasicType()) {
		case EbtFloat:      return (NGFXShaderDataType)((int)NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT2 + offset);
		//case EbtDouble:     return GL_DOUBLE_VEC2 + offset;
#ifdef AMD_EXTENSIONS
		//case EbtFloat16:    return GL_FLOAT16_VEC2_NV + offset;
#endif
		case EbtInt:        return (NGFXShaderDataType)((int)NGFXShaderDataType::NGFX_SHADER_VAR_INT2 + offset);
		case EbtUint:       return (NGFXShaderDataType)((int)NGFXShaderDataType::NGFX_SHADER_VAR_UINT2 + offset);
		//case EbtInt64:      return GL_INT64_ARB + offset;
		//case EbtUint64:     return GL_UNSIGNED_INT64_ARB + offset;
		case EbtBool:       return (NGFXShaderDataType)((int)NGFXShaderDataType::NGFX_SHADER_VAR_BOOL2 + offset);
		//case EbtAtomicUint: return GL_UNSIGNED_INT_ATOMIC_COUNTER + offset;
		default:            return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
		}
	}
	if (type.isMatrix()) 
	{
		switch (type.getBasicType())
		{
		case EbtFloat:
			switch (type.getMatrixCols())
			{
			case 2:
				switch (type.getMatrixRows())
				{
				case 2:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT2;
				case 3:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT2X3;
				case 4:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT2X4;
				default:   return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
				}
			case 3:
				switch (type.getMatrixRows())
				{
				case 2:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT3X2;
				case 3:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT3;
				case 4:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT3X4;
				default:   return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
				}
			case 4:
				switch (type.getMatrixRows())
				{
				case 2:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT4X2;
				case 3:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT4X3;
				case 4:    return NGFXShaderDataType::NGFX_SHADER_VAR_MAT4;
				default:   return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
				}
			}
		}
	}
	if (type.getVectorSize() == 1) {
		switch (type.getBasicType()) {
		case EbtFloat:      return NGFXShaderDataType::NGFX_SHADER_VAR_FLOAT;
		case EbtInt:        return NGFXShaderDataType::NGFX_SHADER_VAR_INT;
		case EbtUint:       return NGFXShaderDataType::NGFX_SHADER_VAR_UINT;
		case EbtBool:       return NGFXShaderDataType::NGFX_SHADER_VAR_BOOL;
		default:   return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
		}
	}
	return NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN;
}

NGFXShaderBindType glslangTypeToRHIType(const TBasicType& type)
{
	switch (type) 
	{
	case EbtSampler:
		return NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLER;
	case EbtStruct:
	case EbtBlock:
		return NGFXShaderBindType::NGFX_SHADER_BIND_BLOCK;
	case EbtVoid:
		return NGFXShaderBindType::NGFX_SHADER_BIND_UNDEFINED;
	default:
		break;
	}
	return NGFXShaderBindType::NGFX_SHADER_BIND_BLOCK;
}

void initResources(TBuiltInResource &resources)
{
	resources.maxLights = 32;
	resources.maxClipPlanes = 6;
	resources.maxTextureUnits = 32;
	resources.maxTextureCoords = 32;
	resources.maxVertexAttribs = 64;
	resources.maxVertexUniformComponents = 4096;
	resources.maxVaryingFloats = 64;
	resources.maxVertexTextureImageUnits = 32;
	resources.maxCombinedTextureImageUnits = 80;
	resources.maxTextureImageUnits = 32;
	resources.maxFragmentUniformComponents = 4096;
	resources.maxDrawBuffers = 32;
	resources.maxVertexUniformVectors = 128;
	resources.maxVaryingVectors = 8;
	resources.maxFragmentUniformVectors = 16;
	resources.maxVertexOutputVectors = 16;
	resources.maxFragmentInputVectors = 15;
	resources.minProgramTexelOffset = -8;
	resources.maxProgramTexelOffset = 7;
	resources.maxClipDistances = 8;
	resources.maxComputeWorkGroupCountX = 65535;
	resources.maxComputeWorkGroupCountY = 65535;
	resources.maxComputeWorkGroupCountZ = 65535;
	resources.maxComputeWorkGroupSizeX = 1024;
	resources.maxComputeWorkGroupSizeY = 1024;
	resources.maxComputeWorkGroupSizeZ = 64;
	resources.maxComputeUniformComponents = 1024;
	resources.maxComputeTextureImageUnits = 16;
	resources.maxComputeImageUniforms = 8;
	resources.maxComputeAtomicCounters = 8;
	resources.maxComputeAtomicCounterBuffers = 1;
	resources.maxVaryingComponents = 60;
	resources.maxVertexOutputComponents = 64;
	resources.maxGeometryInputComponents = 64;
	resources.maxGeometryOutputComponents = 128;
	resources.maxFragmentInputComponents = 128;
	resources.maxImageUnits = 8;
	resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resources.maxCombinedShaderOutputResources = 8;
	resources.maxImageSamples = 0;
	resources.maxVertexImageUniforms = 0;
	resources.maxTessControlImageUniforms = 0;
	resources.maxTessEvaluationImageUniforms = 0;
	resources.maxGeometryImageUniforms = 0;
	resources.maxFragmentImageUniforms = 8;
	resources.maxCombinedImageUniforms = 8;
	resources.maxGeometryTextureImageUnits = 16;
	resources.maxGeometryOutputVertices = 256;
	resources.maxGeometryTotalOutputComponents = 1024;
	resources.maxGeometryUniformComponents = 1024;
	resources.maxGeometryVaryingComponents = 64;
	resources.maxTessControlInputComponents = 128;
	resources.maxTessControlOutputComponents = 128;
	resources.maxTessControlTextureImageUnits = 16;
	resources.maxTessControlUniformComponents = 1024;
	resources.maxTessControlTotalOutputComponents = 4096;
	resources.maxTessEvaluationInputComponents = 128;
	resources.maxTessEvaluationOutputComponents = 128;
	resources.maxTessEvaluationTextureImageUnits = 16;
	resources.maxTessEvaluationUniformComponents = 1024;
	resources.maxTessPatchComponents = 120;
	resources.maxPatchVertices = 32;
	resources.maxTessGenLevel = 64;
	resources.maxViewports = 16;
	resources.maxVertexAtomicCounters = 0;
	resources.maxTessControlAtomicCounters = 0;
	resources.maxTessEvaluationAtomicCounters = 0;
	resources.maxGeometryAtomicCounters = 0;
	resources.maxFragmentAtomicCounters = 8;
	resources.maxCombinedAtomicCounters = 8;
	resources.maxAtomicCounterBindings = 1;
	resources.maxVertexAtomicCounterBuffers = 0;
	resources.maxTessControlAtomicCounterBuffers = 0;
	resources.maxTessEvaluationAtomicCounterBuffers = 0;
	resources.maxGeometryAtomicCounterBuffers = 0;
	resources.maxFragmentAtomicCounterBuffers = 1;
	resources.maxCombinedAtomicCounterBuffers = 1;
	resources.maxAtomicCounterBufferSize = 16384;
	resources.maxTransformFeedbackBuffers = 4;
	resources.maxTransformFeedbackInterleavedComponents = 64;
	resources.maxCullDistances = 8;
	resources.maxCombinedClipAndCullDistances = 8;
	resources.maxSamples = 4;
	resources.limits.nonInductiveForLoops = 1;
	resources.limits.whileLoops = 1;
	resources.limits.doWhileLoops = 1;
	resources.limits.generalUniformIndexing = 1;
	resources.limits.generalAttributeMatrixVectorIndexing = 1;
	resources.limits.generalVaryingIndexing = 1;
	resources.limits.generalSamplerIndexing = 1;
	resources.limits.generalVariableIndexing = 1;
	resources.limits.generalConstantMatrixVectorIndexing = 1;
}

EShLanguage findLanguage(const NGFXShaderType shader_type)
{
  switch (shader_type)
  {
  case NGFX_SHADER_TYPE_VERTEX:
    return EShLangVertex;

  case NGFX_SHADER_TYPE_HULL:
    return EShLangTessControl;

  case NGFX_SHADER_TYPE_DOMAIN:
    return EShLangTessEvaluation;

  case NGFX_SHADER_TYPE_GEOMETRY:
    return EShLangGeometry;

  case NGFX_SHADER_TYPE_FRAGMENT:
    return EShLangFragment;

  case NGFX_SHADER_TYPE_COMPUTE:
    return EShLangCompute;

  default:
    return EShLangVertex;
  }
}

void ExtractAttributeData(const TProgram& program, NGFXShaderAttributes& shAttributes)
{
  auto numAttrs = program.getNumLiveAttributes();
  if (numAttrs > 0)
  {
    for (uint32 i = 0; i < numAttrs; i++)
    {
      auto name = program.getAttributeName(i);
      auto type = program.getAttributeType(i);
      shAttributes.Append({name, NGFX_SEMANTIC_POSITION, glTypeToRHIAttribType(type), i, 0, 0});
    }
  }
}

void ExtractUniformData(NGFXShaderType const& stype, const TProgram& program, NGFXShaderBindingTable& outUniformLayout)
{
  auto numUniforms = program.getNumLiveUniformVariables();
  for (int i = 0; i < numUniforms; i++)
  {
    auto name = program.getUniformName(i);
    auto index = program.getUniformIndex(name);
    auto type = program.getUniformTType(index);
    bool isImage = type->isImage();
    auto baseType = type->getBasicType();
    auto qualifier = type->getQualifier();
    if (qualifier.hasBinding())
    {
      NGFXShaderBindType bind = glslangTypeToRHIType(baseType);
      if (baseType == EbtSampler)
      {
        if (type->getSampler().isCombined())
        {
          bind = NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLER_IMAGE_COMBINE;
        }
        switch (type->getSampler().dim)
        {
        case Esd1D:
        case Esd2D:
        case Esd3D:
        case EsdCube:
        case EsdRect:
          bind = NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLED_IMAGE;
          break;
        }
        if (type->isImage())
        {
          switch (qualifier.storage)
          {
          case EvqUniform:
            bind = NGFXShaderBindType::NGFX_SHADER_BIND_RWTEXEL_BUFFER;
            break;
          case EvqBuffer:
            bind = NGFXShaderBindType::NGFX_SHADER_BIND_RWTEXEL_BUFFER;
            break;
          }
        }
      }
      outUniformLayout.AddBinding({bind, name, stype, qualifier.layoutBinding});
    }
    if (qualifier.hasSet())
    {
      outUniformLayout.AddSet(qualifier.layoutSet);
    }
    if (baseType == EbtSampler)
    {
      auto sampler = type->getSampler();
      if (!sampler.combined)
      {
        outUniformLayout.AddUniform({glslangDataTypeToRHIDataType(*type), name, qualifier.hasOffset() ? (uint32)qualifier.layoutOffset : 0, /*type->getArraySizes()*/});
      }
    }
    else
    {
      outUniformLayout.AddUniform({glslangDataTypeToRHIDataType(*type), name, qualifier.hasOffset() ? (uint32)qualifier.layoutOffset : 0, /*type->getArraySizes()*/});
    }
  }
  auto numUniformBlocks = program.getNumLiveUniformBlocks();
  for (int i = 0; i < numUniformBlocks; i++)
  {
    auto block = program.getUniformBlockTType(i);
    auto name = program.getUniformBlockName(i);
    auto bindNum = program.getUniformBlockIndex(i);
    auto bType = block->getBasicType();
    auto qualifier = block->getQualifier();
    if (qualifier.hasSet())
    {
      outUniformLayout.AddSet(qualifier.layoutSet);
    }
    if (qualifier.hasBinding())
    {
      outUniformLayout.AddBinding({glslangTypeToRHIType(bType), name, stype, (uint32)qualifier.layoutBinding});
    }
    switch (bType)
    {
    case EbtString:
      break;
    }
  }
}
