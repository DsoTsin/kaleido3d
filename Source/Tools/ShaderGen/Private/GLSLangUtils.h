#pragma once
#include <Interface/ICrossShaderCompiler.h>
#include <glslang/GlslangToSpv.h>

void sInitializeGlSlang();
void sFinializeGlSlang();

void initResources(TBuiltInResource &resources);
EShLanguage findLanguage(const rhi::EShaderType shader_type);
rhi::shc::EDataType glTypeToRHIAttribType(int glType);
rhi::shc::EDataType glTypeToRHIUniformType(int glType);
rhi::shc::EDataType glslangDataTypeToRHIDataType(const glslang::TType& type);
rhi::shc::EBindType glslangTypeToRHIType(const glslang::TBasicType& type);

void ExtractAttributeData(const glslang::TProgram& program, rhi::shc::Attributes& shAttributes);
void ExtractUniformData(rhi::EShaderType const& type, const glslang::TProgram& program, rhi::shc::BindingTable& outUniformLayout);
