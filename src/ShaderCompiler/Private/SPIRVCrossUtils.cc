#include <Kaleido3D.h>
#include "SPIRVCrossUtils.h"
#include <algorithm>
#include <Core/LogUtil.h>

using namespace k3d;

NGFXShaderDataType spirTypeToRHIAttribType(const spirv_cross::SPIRType& spirType)
{
	NGFXShaderDataType result = NGFX_SHADER_VAR_UNKNOWN;
	switch (spirType.basetype)
	{
	case spirv_cross::SPIRType::Boolean:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_BOOL; break;
		case 2: result = NGFX_SHADER_VAR_BOOL2; break;
		case 3: result = NGFX_SHADER_VAR_BOOL3; break;
		case 4: result = NGFX_SHADER_VAR_BOOL4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Int:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_INT; break;
		case 2: result = NGFX_SHADER_VAR_INT2; break;
		case 3: result = NGFX_SHADER_VAR_INT3; break;
		case 4: result = NGFX_SHADER_VAR_INT4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::UInt:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_UINT; break;
		case 2: result = NGFX_SHADER_VAR_UINT2; break;
		case 3: result = NGFX_SHADER_VAR_UINT3; break;
		case 4: result = NGFX_SHADER_VAR_UINT4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Float:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_FLOAT; break;
		case 2: result = NGFX_SHADER_VAR_FLOAT2; break;
		case 3: result = NGFX_SHADER_VAR_FLOAT3; break;
		case 4: result = NGFX_SHADER_VAR_FLOAT4; break;
		}
	}
	break;
	}
	return result;
}

spv::ExecutionModel rhiShaderStageToSpvModel(NGFXShaderType const& type)
{
	switch(type)
	{
		case NGFX_SHADER_TYPE_VERTEX:
			return spv::ExecutionModelVertex;
		case NGFX_SHADER_TYPE_FRAGMENT:
			return spv::ExecutionModelFragment;
	}
	return spv::ExecutionModelVertex;
}

NGFXShaderDataType spirTypeToGlslUniformDataType(const spirv_cross::SPIRType& spirType)
{
	NGFXShaderDataType result = NGFX_SHADER_VAR_UNKNOWN;
	switch (spirType.basetype)
	{
	case spirv_cross::SPIRType::Boolean:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_BOOL; break;
		case 2: result = NGFX_SHADER_VAR_BOOL2; break;
		case 3: result = NGFX_SHADER_VAR_BOOL3; break;
		case 4: result = NGFX_SHADER_VAR_BOOL4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Int:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_INT; break;
		case 2: result = NGFX_SHADER_VAR_INT2; break;
		case 3: result = NGFX_SHADER_VAR_INT3; break;
		case 4: result = NGFX_SHADER_VAR_INT4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::UInt:
	{
		switch (spirType.vecsize)
		{
		case 1: result = NGFX_SHADER_VAR_UINT; break;
		case 2: result = NGFX_SHADER_VAR_UINT2; break;
		case 3: result = NGFX_SHADER_VAR_UINT3; break;
		case 4: result = NGFX_SHADER_VAR_UINT4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Float:
	{
		switch (spirType.columns)
		{
		case 1:
		{
			switch (spirType.vecsize)
			{
			case 1: result = NGFX_SHADER_VAR_FLOAT; break;
			case 2: result = NGFX_SHADER_VAR_FLOAT2; break;
			case 3: result = NGFX_SHADER_VAR_FLOAT3; break;
			case 4: result = NGFX_SHADER_VAR_FLOAT4; break;
			}
		}
		break;

		case 2:
		{
			switch (spirType.vecsize)
			{
			case 2: result = NGFX_SHADER_VAR_MAT2; break;
			case 3: result = NGFX_SHADER_VAR_MAT2X3; break;
			case 4: result = NGFX_SHADER_VAR_MAT2X4; break;
			}
		}
		break;

		case 3:
		{
			switch (spirType.vecsize)
			{
			case 2: result = NGFX_SHADER_VAR_MAT3X2; break;
			case 3: result = NGFX_SHADER_VAR_MAT3; break;
			case 4: result = NGFX_SHADER_VAR_MAT3X4; break;
			}
		}
		break;

		case 4:
		{
			switch (spirType.vecsize)
			{
			case 2: result = NGFX_SHADER_VAR_MAT4X2; break;
			case 3: result = NGFX_SHADER_VAR_MAT4X3; break;
			case 4: result = NGFX_SHADER_VAR_MAT4; break;
			}
		}
		break;
		}
	}
	break;
	}
	return result;
}

void ExtractAttributeData(spirv_cross::CompilerGLSL const& backCompiler,
	NGFXShaderAttributes & outShaderAttributes)
{
	for (auto& res : backCompiler.get_shader_resources().stage_inputs)
	{
		spirv_cross::SPIRType	spirType = backCompiler.get_type(res.type_id);
		std::string				attrName = res.name;
		uint32					attrLocation = backCompiler.get_decoration(res.id, spv::DecorationLocation);
		uint32					attrBinding = 0;

		NGFXShaderDataType				attrDataType = spirTypeToRHIAttribType(spirType);
		NGFXShaderSemantic				attrSemantic = NGFX_SEMANTIC_POSITION;

		if (NGFX_SHADER_VAR_UNKNOWN == attrDataType) {
			KLOG(Error, "GLSLCompiler", "Unable to determine data type for vertex shader attr %s", attrName.c_str());
			continue;
		}

		auto it = std::find_if(
			std::begin(outShaderAttributes), std::end(outShaderAttributes),
			[attrName](const NGFXShaderAttribute& elem) -> bool
		{
			return elem.VarName == String(attrName.c_str());
		}
		);

		if (std::end(outShaderAttributes) != it)
		{
			NGFXShaderAttribute& attr = *it;
			attr.VarLocation = attrLocation;
			attr.VarBindingPoint = attrBinding;
			attr.VarType = attrDataType;
			attr.VarSemantic = attrSemantic;
		}
		else
		{
			NGFXShaderAttribute attr = { attrName.c_str(), attrSemantic, attrDataType, attrLocation, attrBinding, 0 };
			outShaderAttributes.Append(attr);
		}
	}

	std::sort(
		std::begin(outShaderAttributes), std::end(outShaderAttributes),
		[](const NGFXShaderAttribute& a, const NGFXShaderAttribute& b) -> bool
	{
		return a.VarLocation < b.VarLocation;
	}
	);
}

void ExtractBlock(
	NGFXShaderType shaderType,
	const spirv_cross::Resource& res,
	const spirv_cross::CompilerGLSL& backCompiler,
	NGFXShaderBindingTable & outUniformLayout)
{
	const uint64 kBlockMask = (1ULL << spv::DecorationBlock) | (1ULL << spv::DecorationBufferBlock);

	auto&    typeInfo = backCompiler.get_type(res.type_id);
	bool     isPushConstant = (spv::StorageClassPushConstant == backCompiler.get_storage_class(res.id));
	bool     isBlock = (0 != (backCompiler.get_decoration_mask(typeInfo.self) & kBlockMask));
	uint32 	typeId = ((!isPushConstant && isBlock) ? res.type_id : res.id);

	auto            bindingType = isPushConstant ? NGFX_SHADER_BIND_CONSTANTS : NGFX_SHADER_BIND_BLOCK;
	std::string     bindingName = backCompiler.get_name(res.id);
	uint32			bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
	uint32			bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
	NGFXShaderType bindingStage = shaderType;
	//		NGFXShaderBinding			binding;
	outUniformLayout.AddBinding({ bindingType, bindingName, bindingStage, bindingNumber })
		.AddSet(bindingSet);

	for (uint32 index = 0; index < typeInfo.member_types.size(); ++index) {
		std::string memberName = backCompiler.get_member_name(res.type_id, index);
		uint32      memberId = typeInfo.member_types[index];
		auto&		memberTypeInfo = backCompiler.get_type(memberId);
		uint32      memberOffset = backCompiler.get_member_decoration(res.type_id, index, spv::DecorationOffset);
		uint32      memberArraySize = memberTypeInfo.array.empty() ? 1 : memberTypeInfo.array[0];
		NGFXShaderDataType	memberDataType = spirTypeToGlslUniformDataType(memberTypeInfo);

		std::string uniformName = bindingName + "." + memberName;
		outUniformLayout.AddUniform({ memberDataType, uniformName.c_str(), memberOffset, memberArraySize });
	}

	// Block size
	//size_t blockSize = backCompiler.get_declared_struct_size(typeInfo);

	//// Round up to next multiple of 16			
	//size_t paddedSize = (blockSize + 15) & (~15);
	//if ((blockSize != paddedSize) && (!isPushConstant)) {
	//	Log::Out(LogLevel::Info, "GLSLCompiler", "Padded uniform block " << bindingName << " from " << blockSize << " bytes to " << paddedSize << " bytes");
	//}

	// Use block size for push constants and padded size for UBOs
	//outUniformLayout->setBlockSizeBytes(bindingName, isPushConstant ? blockSize : paddedSize);
	//outUniformLayout->sortByOffset();
}

void ExtractUniformData(
	NGFXShaderType shaderStage,
	spirv_cross::CompilerGLSL const& backCompiler,
	NGFXShaderBindingTable & outUniformLayout)
{
	// Extract uniform blocks from all shader stages
	for (auto& res : backCompiler.get_shader_resources().uniform_buffers) {
		ExtractBlock(shaderStage, res, backCompiler, outUniformLayout);
	}

	// Extract samplers from all shader stages
	for (auto& res : backCompiler.get_shader_resources().sampled_images) {
		std::string         bindingName = backCompiler.get_name(res.id);
		uint32              bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
		uint32              bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		NGFXShaderType	bindingStage = shaderStage;

		outUniformLayout.AddBinding({ NGFX_SHADER_BIND_SAMPLER, bindingName, bindingStage, bindingNumber }).AddSet(bindingSet);
		//outUniformLayout->addSet(bindingSet, CHANGES_DONTCARE);
	}

	// Extract storage images from all shader stages - but probably only just compute
	for (auto& res : backCompiler.get_shader_resources().storage_images) {
		std::string				bindingName = backCompiler.get_name(res.id);
		uint32					bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
		uint32					bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		NGFXShaderType		bindingStage = shaderStage;

		outUniformLayout.AddBinding({ NGFX_SHADER_BIND_STORAGE_IMAGE, bindingName, bindingStage, bindingNumber }).AddSet(bindingSet);
	}

	// Extract storage buffers from all shader stages - but probably only just compute
	for (auto& res : backCompiler.get_shader_resources().storage_buffers) {
		std::string				bindingName = backCompiler.get_name(res.id);
		uint32					bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
		uint32					bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		NGFXShaderType		bindingStage = shaderStage;

		outUniformLayout.AddBinding({ NGFX_SHADER_BIND_STORAGE_BUFFER, bindingName, bindingStage, bindingNumber }).AddSet(bindingSet);
	}

	// Extract push constants from all shader stages
	for (auto& res : backCompiler.get_shader_resources().push_constant_buffers) {
		ExtractBlock(shaderStage, res, backCompiler, outUniformLayout);
	}
}