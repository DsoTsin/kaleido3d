#pragma once
#include <Interface/ICrossShaderCompiler.h>
#include <spirv2cross/spirv_cross.hpp>
#include <spirv2cross/spirv_msl.hpp>
#include <spirv2cross/spirv_glsl.hpp>

NGFXShaderDataType spirTypeToRHIAttribType(const spirv_cross::SPIRType& spirType);
NGFXShaderDataType spirTypeToGlslUniformDataType(const spirv_cross::SPIRType& spirType);
spv::ExecutionModel rhiShaderStageToSpvModel(NGFXShaderType const& type);
void ExtractAttributeData(spirv_cross::CompilerGLSL const& backCompiler, NGFXShaderAttributes & outShaderAttributes);
void ExtractUniformData(NGFXShaderType shaderStage, spirv_cross::CompilerGLSL const& backCompiler, NGFXShaderBindingTable& outUniformLayout);
