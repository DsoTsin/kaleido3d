#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"

K3D_VK_BEGIN

/**
* @class	PipelineLayout
*/
void PipelineLayout::Create(rhi::ShaderParamLayout const & ParamLayout)
{

}

void PipelineLayout::Finalize(rhi::IDevice *)
{
	//vkCreatePipelineLayout()
}

/**
 * @class	PipelineStateObject
 */
PipelineStateObject::PipelineStateObject(Device* pDevice)
	: DeviceChild(pDevice)
{

}

PipelineStateObject::~PipelineStateObject()
{
	Log::Out(LogLevel::Info, "PipelineStateObject", "Destroying..");
	vkDestroyPipelineCache(GetRawDevice(), m_PipelineCache, nullptr);
	vkDestroyPipeline(GetRawDevice(), m_Pipeline, nullptr);
}


void PipelineStateObject::BindRenderPass(VkRenderPass RenderPass)
{
	this->m_RenderPass = RenderPass;
}

VkShaderModule CreateShaderModule(VkDevice Device, ::k3d::IShaderCompilerOutput * ShaderBytes)
{
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo moduleCreateInfo;

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;

	moduleCreateInfo.codeSize = ShaderBytes->GetByteCount();
	moduleCreateInfo.pCode = (uint32_t*)ShaderBytes->GetShaderBytes();
	moduleCreateInfo.flags = 0;
	K3D_VK_VERIFY(vkCreateShaderModule(Device, &moduleCreateInfo, NULL, &shaderModule));
	return shaderModule;
}

void PipelineStateObject::SetShader(rhi::EShaderType ShaderType, ::k3d::IShaderCompilerOutput * ShaderBytes)
{
	auto sm = CreateShaderModule(GetRawDevice(), ShaderBytes); 
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = g_ShaderType[ShaderType];
	shaderStage.module = sm;
	shaderStage.pName = "main"; // todo : make param
	assert(shaderStage.module != NULL);
	//ShaderModule * shaderModule = static_cast<ShaderModule*>(ShaderBytes);
	m_ShaderStageInfos.push_back(shaderStage);
}

void PipelineStateObject::SetLayout(rhi::IPipelineLayout * PipelineLayout)
{

}

void PipelineStateObject::Finalize()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	K3D_VK_VERIFY(vkCreatePipelineCache(GetRawDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
	if (GetType() == rhi::EPSO_Graphics) 
	{
		m_GfxCreateInfo.stageCount = (uint32)m_ShaderStageInfos.size();
		m_GfxCreateInfo.pStages = m_ShaderStageInfos.data();
		K3D_VK_VERIFY(vkCreateGraphicsPipelines(GetRawDevice(), m_PipelineCache, 1, &m_GfxCreateInfo, nullptr, &m_Pipeline));
	}
	else
	{
		m_CptCreateInfo.stage = m_ShaderStageInfos[0];
		K3D_VK_VERIFY(vkCreateComputePipelines(GetRawDevice(), m_PipelineCache, 1, &m_CptCreateInfo, nullptr, &m_Pipeline));
	}
}

void PipelineStateObject::SetRasterizerState(const rhi::RasterizerState& rasterState)
{
	m_RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// Solid polygon mode
	m_RasterizationState.polygonMode = g_FillMode[rasterState.FillMode];
	// No culling
	m_RasterizationState.cullMode = g_CullMode[rasterState.CullMode];
	m_RasterizationState.frontFace = rasterState.FrontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
	m_RasterizationState.depthClampEnable = rasterState.DepthClipEnable ? VK_TRUE : VK_FALSE;
	m_RasterizationState.rasterizerDiscardEnable = VK_FALSE;
	m_RasterizationState.depthBiasEnable = VK_FALSE;
	this->m_GfxCreateInfo.pRasterizationState = &m_RasterizationState;
}

// One blend attachment state
// Blending is not used in this example
// TODO
void PipelineStateObject::SetBlendState(const rhi::BlendState& blendState)
{
	m_ColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = blendState.Enable ? VK_TRUE : VK_FALSE;
	m_ColorBlendState.attachmentCount = 1;
	m_ColorBlendState.pAttachments = blendAttachmentState;
	this->m_GfxCreateInfo.pColorBlendState = &m_ColorBlendState;
}

void PipelineStateObject::SetDepthStencilState(const rhi::DepthStencilState& depthStencilState)
{
	m_DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_DepthStencilState.depthTestEnable = depthStencilState.DepthEnable ? VK_TRUE : VK_FALSE;
	m_DepthStencilState.depthWriteEnable = VK_TRUE;
	m_DepthStencilState.depthCompareOp = g_ComparisonFunc[depthStencilState.DepthFunc];
	m_DepthStencilState.depthBoundsTestEnable = VK_FALSE;
	m_DepthStencilState.back.failOp = g_StencilOp[depthStencilState.BackFace.StencilFailOp];
	m_DepthStencilState.back.passOp = g_StencilOp[depthStencilState.BackFace.StencilPassOp];
	m_DepthStencilState.back.compareOp = g_ComparisonFunc[depthStencilState.BackFace.StencilFunc];
	m_DepthStencilState.stencilTestEnable = depthStencilState.StencilEnable ? VK_TRUE : VK_FALSE;
	m_DepthStencilState.front = m_DepthStencilState.back;
	this->m_GfxCreateInfo.pDepthStencilState = &m_DepthStencilState;
}

void PipelineStateObject::SetSampler(rhi::ISampler*)
{

}

void PipelineStateObject::SetVertexInputLayout(rhi::VertexDeclaration const* vertDecs, uint32 Count)
{
	K3D_ASSERT(vertDecs && Count > 0 && m_BindingDescriptions.empty());
	/*
	if(!m_BindingDescriptions.empty()) 
	{ 
		m_BindingDescriptions.swap(decltype(m_BindingDescriptions)());
		m_AttributeDescriptions.swap(decltype(m_AttributeDescriptions)());
	}
	*/
	for (uint32 i = 0; i < Count; i++)
	{
		rhi::VertexDeclaration const& vertDec = vertDecs[i];
		VkVertexInputBindingDescription bindingDesc = { vertDec.BindID, vertDec.Stride, VK_VERTEX_INPUT_RATE_VERTEX };
		VkVertexInputAttributeDescription attribDesc = { vertDec.AttributeIndex, vertDec.BindID, g_VertexFormatTable[vertDec.Format], vertDec.OffSet };
		m_BindingDescriptions.push_back(bindingDesc);
		m_AttributeDescriptions.push_back(attribDesc);
	}
	m_VertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_VertexInputState.pNext = NULL;
	m_VertexInputState.vertexBindingDescriptionCount = (uint32)m_BindingDescriptions.size();
	m_VertexInputState.pVertexBindingDescriptions = m_BindingDescriptions.data();
	m_VertexInputState.vertexAttributeDescriptionCount = (uint32)m_AttributeDescriptions.size();
	m_VertexInputState.pVertexAttributeDescriptions = m_AttributeDescriptions.data();
	this->m_GfxCreateInfo.pVertexInputState = &m_VertexInputState;
}

void PipelineStateObject::SetPrimitiveTopology(const rhi::EPrimitiveType Type)
{
	m_InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_InputAssemblyState.topology = g_PrimitiveTopology[Type];
	this->m_GfxCreateInfo.pInputAssemblyState = &m_InputAssemblyState;
}

void PipelineStateObject::SetRenderTargetFormat(const rhi::RenderTargetFormat &)
{

}

K3D_VK_END