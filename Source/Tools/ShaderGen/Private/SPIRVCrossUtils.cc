#include <Kaleido3D.h>
#include "SPIRVCrossUtils.h"
#include <algorithm>
#include <Core/LogUtil.h>

using namespace rhi::shc;

rhi::shc::EDataType spirTypeToRHIAttribType(const spirv_cross::SPIRType& spirType)
{
	EDataType result = EDataType::EUnknown;
	switch (spirType.basetype)
	{
	case spirv_cross::SPIRType::Boolean:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EBool; break;
		case 2: result = EDataType::EBool2; break;
		case 3: result = EDataType::EBool3; break;
		case 4: result = EDataType::EBool4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Int:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EInt; break;
		case 2: result = EDataType::EInt2; break;
		case 3: result = EDataType::EInt3; break;
		case 4: result = EDataType::EInt4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::UInt:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EUInt; break;
		case 2: result = EDataType::EUInt2; break;
		case 3: result = EDataType::EUInt3; break;
		case 4: result = EDataType::EUInt4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Float:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EFloat; break;
		case 2: result = EDataType::EFloat2; break;
		case 3: result = EDataType::EFloat3; break;
		case 4: result = EDataType::EFloat4; break;
		}
	}
	break;
	}
	return result;
}


rhi::shc::EDataType spirTypeToGlslUniformDataType(const spirv_cross::SPIRType& spirType)
{
	EDataType result = EDataType::EUnknown;
	switch (spirType.basetype)
	{
	case spirv_cross::SPIRType::Boolean:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EBool; break;
		case 2: result = EDataType::EBool2; break;
		case 3: result = EDataType::EBool3; break;
		case 4: result = EDataType::EBool4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::Int:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EInt; break;
		case 2: result = EDataType::EInt2; break;
		case 3: result = EDataType::EInt3; break;
		case 4: result = EDataType::EInt4; break;
		}
	}
	break;

	case spirv_cross::SPIRType::UInt:
	{
		switch (spirType.vecsize)
		{
		case 1: result = EDataType::EUInt; break;
		case 2: result = EDataType::EUInt2; break;
		case 3: result = EDataType::EUInt3; break;
		case 4: result = EDataType::EUInt4; break;
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
			case 1: result = EDataType::EFloat; break;
			case 2: result = EDataType::EFloat2; break;
			case 3: result = EDataType::EFloat3; break;
			case 4: result = EDataType::EFloat4; break;
			}
		}
		break;

		case 2:
		{
			switch (spirType.vecsize)
			{
			case 2: result = EDataType::EMat2; break;
			case 3: result = EDataType::EMat2x3; break;
			case 4: result = EDataType::EMat2x4; break;
			}
		}
		break;

		case 3:
		{
			switch (spirType.vecsize)
			{
			case 2: result = EDataType::EMat3x2; break;
			case 3: result = EDataType::EMat3; break;
			case 4: result = EDataType::EMat3x4; break;
			}
		}
		break;

		case 4:
		{
			switch (spirType.vecsize)
			{
			case 2: result = EDataType::EMat4x2; break;
			case 3: result = EDataType::EMat4x3; break;
			case 4: result = EDataType::EMat4; break;
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
	rhi::shc::Attributes & outShaderAttributes)
{
	for (auto& res : backCompiler.get_shader_resources().stage_inputs)
	{
		spirv_cross::SPIRType	spirType = backCompiler.get_type(res.type_id);
		std::string				attrName = res.name;
		uint32					attrLocation = backCompiler.get_decoration(res.id, spv::DecorationLocation);
		uint32					attrBinding = 0;

		EDataType				attrDataType = spirTypeToRHIAttribType(spirType);
		ESemantic				attrSemantic = ESemantic::ENumSemanics;

		if (EDataType::EUnknown == attrDataType) {
			KLOG(Error, "GLSLCompiler", "Unable to determine data type for vertex shader attr %s", attrName.c_str());
			continue;
		}

		auto it = std::find_if(
			std::begin(outShaderAttributes), std::end(outShaderAttributes),
			[attrName](const Attribute& elem) -> bool
		{
			return elem.VarName == rhi::String(attrName.c_str());
		}
		);

		if (std::end(outShaderAttributes) != it)
		{
			Attribute& attr = *it;
			attr.VarLocation = attrLocation;
			attr.VarBindingPoint = attrBinding;
			attr.VarType = attrDataType;
			attr.VarSemantic = attrSemantic;
		}
		else
		{
			Attribute attr = { attrName.c_str(), attrSemantic, attrDataType, attrLocation, attrBinding, 0 };
			outShaderAttributes.Append(attr);
		}
	}

	std::sort(
		std::begin(outShaderAttributes), std::end(outShaderAttributes),
		[](const Attribute& a, const Attribute& b) -> bool
	{
		return a.VarLocation < b.VarLocation;
	}
	);
}

void ExtractBlock(
	rhi::EShaderType shaderType,
	const spirv_cross::Resource& res,
	const spirv_cross::CompilerGLSL& backCompiler,
	BindingTable & outUniformLayout)
{
	const uint64 kBlockMask = (1ULL << spv::DecorationBlock) | (1ULL << spv::DecorationBufferBlock);

	auto&    typeInfo = backCompiler.get_type(res.type_id);
	bool     isPushConstant = (spv::StorageClassPushConstant == backCompiler.get_storage_class(res.id));
	bool     isBlock = (0 != (backCompiler.get_decoration_mask(typeInfo.self) & kBlockMask));
	uint32 	typeId = ((!isPushConstant && isBlock) ? res.type_id : res.id);

	auto            bindingType = isPushConstant ? EBindType::EConstants : EBindType::EBlock;
	std::string     bindingName = backCompiler.get_name(res.id);
	uint32			bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
	uint32			bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
	rhi::EShaderType bindingStage = shaderType;
	//		Binding			binding;
	outUniformLayout.AddBinding({ bindingType, bindingName, bindingStage, bindingNumber })
		.AddSet(bindingSet);

	for (uint32 index = 0; index < typeInfo.member_types.size(); ++index) {
		std::string memberName = backCompiler.get_member_name(res.type_id, index);
		uint32      memberId = typeInfo.member_types[index];
		auto&		memberTypeInfo = backCompiler.get_type(memberId);
		uint32      memberOffset = backCompiler.get_member_decoration(res.type_id, index, spv::DecorationOffset);
		uint32      memberArraySize = memberTypeInfo.array.empty() ? 1 : memberTypeInfo.array[0];
		EDataType	memberDataType = spirTypeToGlslUniformDataType(memberTypeInfo);

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
	rhi::EShaderType shaderStage,
	spirv_cross::CompilerGLSL const& backCompiler,
	BindingTable & outUniformLayout)
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
		rhi::EShaderType	bindingStage = shaderStage;

		outUniformLayout.AddBinding({ EBindType::ESampler, bindingName, bindingStage, bindingNumber }).AddSet(bindingSet);
		//outUniformLayout->addSet(bindingSet, CHANGES_DONTCARE);
	}

	// Extract storage images from all shader stages - but probably only just compute
	for (auto& res : backCompiler.get_shader_resources().storage_images) {
		std::string				bindingName = backCompiler.get_name(res.id);
		uint32					bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
		uint32					bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		rhi::EShaderType		bindingStage = shaderStage;

		outUniformLayout.AddBinding({ EBindType::EStorageImage, bindingName, bindingStage, bindingNumber }).AddSet(bindingSet);
	}

	// Extract storage buffers from all shader stages - but probably only just compute
	for (auto& res : backCompiler.get_shader_resources().storage_buffers) {
		std::string				bindingName = backCompiler.get_name(res.id);
		uint32					bindingNumber = backCompiler.get_decoration(res.id, spv::DecorationBinding);
		uint32					bindingSet = backCompiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		rhi::EShaderType		bindingStage = shaderStage;

		outUniformLayout.AddBinding({ EBindType::EStorageBuffer, bindingName, bindingStage, bindingNumber }).AddSet(bindingSet);
	}

	// Extract push constants from all shader stages
	for (auto& res : backCompiler.get_shader_resources().push_constant_buffers) {
		ExtractBlock(shaderStage, res, backCompiler, outUniformLayout);
	}
}