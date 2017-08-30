#pragma once
#include <Core/Interface/ICrossShaderCompiler.h>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <spirv_cross/spirv_glsl.hpp>

NGFXShaderDataType spirTypeToRHIAttribType(const spirv_cross::SPIRType& spirType);
NGFXShaderDataType spirTypeToGlslUniformDataType(const spirv_cross::SPIRType& spirType);
spv::ExecutionModel rhiShaderStageToSpvModel(NGFXShaderType const& type);
void ExtractAttributeData(spirv_cross::CompilerGLSL const& backCompiler, NGFXShaderAttributes & outShaderAttributes);
void ExtractUniformData(NGFXShaderType shaderStage, spirv_cross::CompilerGLSL const& backCompiler, NGFXShaderBindingTable& outUniformLayout);
