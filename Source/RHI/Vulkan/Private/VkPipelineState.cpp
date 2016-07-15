#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"
#include "VkUtils.h"

#include <set>

K3D_VK_BEGIN

PipelineStateObject::PipelineStateObject(Device::Ptr pDevice, rhi::PipelineDesc const & desc, PipelineLayout * ppl)
	: DeviceChild(pDevice)
	, m_Pipeline(VK_NULL_HANDLE)
	, m_PipelineCache(VK_NULL_HANDLE)
	, m_RenderPass(VK_NULL_HANDLE)
	, m_GfxCreateInfo{}
	, m_PipelineLayout(ppl)
{
	memset(&m_GfxCreateInfo, 0, sizeof(m_GfxCreateInfo));
	InitWithDesc(desc);
}

/**
 * @class	PipelineStateObject
 */
PipelineStateObject::PipelineStateObject(Device* pDevice)
	: DeviceChild(pDevice)
	, m_Pipeline(VK_NULL_HANDLE)
	, m_PipelineCache(VK_NULL_HANDLE)
	, m_RenderPass(VK_NULL_HANDLE)
{
	memset(&m_GfxCreateInfo, 0, sizeof(m_GfxCreateInfo));
}

PipelineStateObject::~PipelineStateObject()
{
	Destroy();
}


void PipelineStateObject::BindRenderPass(VkRenderPass RenderPass)
{
	this->m_RenderPass = RenderPass;
}


VkShaderModule CreateShaderModule(VkDevice Device, rhi::ShaderByteCode const& ShaderBytes)
{
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo moduleCreateInfo;

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;

	moduleCreateInfo.codeSize = ShaderBytes.Count()*sizeof(uint32);
	moduleCreateInfo.pCode = ShaderBytes.Data();
	moduleCreateInfo.flags = 0;
	K3D_VK_VERIFY(vkCreateShaderModule(Device, &moduleCreateInfo, NULL, &shaderModule));
	return shaderModule;
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
	K3D_VK_VERIFY(shaderStage.module != NULL);
	m_ShaderStageInfos.push_back(shaderStage);
}

void PipelineStateObject::SetLayout(rhi::IPipelineLayout * PipelineLayout)
{

}

void PipelineStateObject::Finalize()
{
	if (VK_NULL_HANDLE != m_Pipeline)
		return;
	/*VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	K3D_VK_VERIFY(vkCreatePipelineCache(GetRawDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));*/
	if (GetType() == rhi::EPSO_Graphics) 
	{
		K3D_VK_VERIFY(vkCmd::CreateGraphicsPipelines(GetRawDevice(), VK_NULL_HANDLE, 1, &m_GfxCreateInfo, nullptr, &m_Pipeline));
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

// Init Shaders
std::vector<VkPipelineShaderStageCreateInfo> 
GetShaderStageInfo(VkDevice device, rhi::PipelineDesc const & desc)
{
	std::vector<VkPipelineShaderStageCreateInfo> infos;
	for (uint32 i = 0; i < rhi::EShaderType::ShaderTypeNum; i++)
	{
		const rhi::ShaderByteCode& code = desc.Shaders[i];
		if (code.Count() > 0)
		{
			infos.push_back({
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
				g_ShaderType[i], CreateShaderModule(device, code), "main"
			});
		}
	}
	return infos;
}

void PipelineStateObject::InitWithDesc(rhi::PipelineDesc const & desc)
{
	m_ShaderStageInfos = GetShaderStageInfo(GetRawDevice(), desc);
	// Init PrimType
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO , nullptr, 0,
		g_PrimitiveTopology[desc.PrimitiveTopology] , VK_FALSE
	};

	// Init RasterState
	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, NULL };
	rasterizationState.polygonMode = g_FillMode[desc.Rasterizer.FillMode];
	rasterizationState.cullMode = g_CullMode[desc.Rasterizer.CullMode];
	rasterizationState.frontFace = desc.Rasterizer.FrontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthClampEnable = desc.Rasterizer.DepthClipEnable ? VK_TRUE : VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	// Init DepthStencilState
	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, NULL };
	depthStencilState.depthTestEnable = desc.DepthStencil.DepthEnable ? VK_TRUE : VK_FALSE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = g_ComparisonFunc[desc.DepthStencil.DepthFunc];
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = g_StencilOp[desc.DepthStencil.BackFace.StencilFailOp];
	depthStencilState.back.passOp = g_StencilOp[desc.DepthStencil.BackFace.StencilPassOp];
	depthStencilState.back.compareOp = g_ComparisonFunc[desc.DepthStencil.BackFace.StencilFunc];
	depthStencilState.stencilTestEnable = desc.DepthStencil.StencilEnable ? VK_TRUE : VK_FALSE;
	depthStencilState.front = depthStencilState.back;

	// Init BlendState
	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, NULL };
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = desc.Blend.Enable ? VK_TRUE : VK_FALSE;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = blendAttachmentState;

	struct VIADLess {
		bool operator() (const VkVertexInputBindingDescription& lhs, const VkVertexInputBindingDescription& rhs) const {
			return lhs.binding < rhs.binding || lhs.stride < rhs.stride || lhs.inputRate < rhs.inputRate;
		}
	};
	std::set <VkVertexInputBindingDescription, VIADLess> bindings;
	// Init VertexLayout
	for (uint32 i = 0; i < desc.VertexLayout.Count(); i++)
	{
		rhi::VertexDeclaration const& vertDec = desc.VertexLayout[i];
		VkVertexInputBindingDescription bindingDesc = { vertDec.BindID, vertDec.Stride, VK_VERTEX_INPUT_RATE_VERTEX };
		VkVertexInputAttributeDescription attribDesc = { vertDec.AttributeIndex, vertDec.BindID, g_VertexFormatTable[vertDec.Format], vertDec.OffSet };
		bindings.insert(bindingDesc);
		m_AttributeDescriptions.push_back(attribDesc);
	}
	m_BindingDescriptions.assign(bindings.begin(), bindings.end());

	VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, NULL };
	vertexInputState.vertexBindingDescriptionCount = m_BindingDescriptions.size();
	vertexInputState.pVertexBindingDescriptions = m_BindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = (uint32)m_AttributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions = m_AttributeDescriptions.data();

	VkPipelineViewportStateCreateInfo vpInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	vpInfo.viewportCount = 1;
	vpInfo.scissorCount = 1;

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = (uint32)dynamicStateEnables.size();

	VkPipelineMultisampleStateCreateInfo msInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	msInfo.pSampleMask = NULL;
	msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	this->m_GfxCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	this->m_GfxCreateInfo.stageCount = (uint32)m_ShaderStageInfos.size();
	this->m_GfxCreateInfo.pStages = m_ShaderStageInfos.data();

	this->m_GfxCreateInfo.pInputAssemblyState = &inputAssemblyState;
	this->m_GfxCreateInfo.pRasterizationState = &rasterizationState;
	this->m_GfxCreateInfo.pDepthStencilState = &depthStencilState;
	this->m_GfxCreateInfo.pColorBlendState = &colorBlendState;
	this->m_GfxCreateInfo.pVertexInputState = &vertexInputState;
	this->m_GfxCreateInfo.pDynamicState = &dynamicState;
	this->m_GfxCreateInfo.pMultisampleState = &msInfo;
	this->m_GfxCreateInfo.pViewportState = &vpInfo;

	m_RenderPass = RHIRoot::GetViewport(0)->GetRenderPass();

	this->m_GfxCreateInfo.renderPass = m_RenderPass;
	this->m_GfxCreateInfo.layout = m_PipelineLayout->m_PipelineLayout;

	// Finalize
	Finalize();
}

void PipelineStateObject::Destroy()
{
	for (auto iter : m_ShaderStageInfos)
	{
		if(iter.module)
		{
			vkDestroyShaderModule(GetRawDevice(), iter.module, nullptr);
		}
	}
	//vkDestroyPipelineCache(GetRawDevice(), m_PipelineCache, nullptr);
	vkDestroyPipeline(GetRawDevice(), m_Pipeline, nullptr);
	VKLOG(Info, "PipelineStateObject-Destroyed..");
}

K3D_VK_END