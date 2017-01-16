#pragma once
#include <Interface/ICrossShaderCompiler.h>
#include <spirv2cross/spirv_cross.hpp>
#include <spirv2cross/spirv_msl.hpp>
#include <spirv2cross/spirv_glsl.hpp>

rhi::shc::EDataType spirTypeToRHIAttribType(const spirv_cross::SPIRType& spirType);
rhi::shc::EDataType spirTypeToGlslUniformDataType(const spirv_cross::SPIRType& spirType);

void ExtractAttributeData(spirv_cross::CompilerGLSL const& backCompiler, rhi::shc::Attributes & outShaderAttributes);
void ExtractUniformData(rhi::EShaderType shaderStage, spirv_cross::CompilerGLSL const& backCompiler, rhi::shc::BindingTable& outUniformLayout);