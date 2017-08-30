#pragma once
#include <Core/Interface/ICrossShaderCompiler.h>
#include <SPIRV/GlslangToSpv.h>

void sInitializeGlSlang();
void sFinializeGlSlang();

void initResources(TBuiltInResource &resources);
EShLanguage findLanguage(const NGFXShaderType shader_type);
NGFXShaderDataType glTypeToRHIAttribType(int glType);
NGFXShaderDataType glTypeToRHIUniformType(int glType);
NGFXShaderDataType glslangDataTypeToRHIDataType(const glslang::TType& type);
NGFXShaderBindType glslangTypeToRHIType(const glslang::TBasicType& type);

void ExtractAttributeData(const glslang::TProgram& program, NGFXShaderAttributes& shAttributes);
void ExtractUniformData(NGFXShaderType const& type, const glslang::TProgram& program, NGFXShaderBindingTable& outUniformLayout);
