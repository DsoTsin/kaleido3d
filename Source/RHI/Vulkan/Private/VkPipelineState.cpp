#include "VkCommon.h"
#include "Public/VkRHI.h"

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

void PipelineStateObject::SetShader(rhi::EShaderType ShaderType, rhi::IShaderBytes * ShaderBytes)
{

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
		K3D_VK_VERIFY(vkCreateGraphicsPipelines(GetRawDevice(), m_PipelineCache, 1, &m_GfxCreateInfo, nullptr, &m_Pipeline));
	}
	else
	{
		K3D_VK_VERIFY(vkCreateComputePipelines(GetRawDevice(), m_PipelineCache, 1, &m_CptCreateInfo, nullptr, &m_Pipeline));
	}
}

GraphicsPSO::GraphicsPSO(Device::Ptr pDevice)
	: PipelineStateObject(pDevice)
{

}

GraphicsPSO::~GraphicsPSO()
{

}

void	GraphicsPSO::SetRasterizerState(const rhi::RasterizerState&)
{
	m_RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// Solid polygon mode
	m_RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	// No culling
	m_RasterizationState.cullMode = VK_CULL_MODE_NONE;
	m_RasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_RasterizationState.depthClampEnable = VK_FALSE;
	m_RasterizationState.rasterizerDiscardEnable = VK_FALSE;
	m_RasterizationState.depthBiasEnable = VK_FALSE;
	this->m_GfxCreateInfo.pRasterizationState = &m_RasterizationState;
}

void	GraphicsPSO::SetBlendState(const rhi::BlendState&)
{
	m_ColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	// One blend attachment state
	// Blending is not used in this example
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = VK_FALSE;
	m_ColorBlendState.attachmentCount = 1;
	m_ColorBlendState.pAttachments = blendAttachmentState;
	this->m_GfxCreateInfo.pColorBlendState = &m_ColorBlendState;
}

void	GraphicsPSO::SetDepthStencilState(const rhi::DepthStencilState&)
{
	m_DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_DepthStencilState.depthTestEnable = VK_TRUE;
	m_DepthStencilState.depthWriteEnable = VK_TRUE;
	m_DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	m_DepthStencilState.depthBoundsTestEnable = VK_FALSE;
	m_DepthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	m_DepthStencilState.stencilTestEnable = VK_FALSE;
	m_DepthStencilState.front = m_DepthStencilState.back;
	this->m_GfxCreateInfo.pDepthStencilState = &m_DepthStencilState;
}

void	GraphicsPSO::SetSampler(rhi::ISampler*)
{

}

void	GraphicsPSO::SetVertexInputLayout(rhi::VertexDeclaration *, uint32 Count)
{

}

void	GraphicsPSO::SetPrimitiveTopology(const rhi::EPrimitiveType)
{
	m_InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	// This pipeline renders vertex data as triangle lists
	m_InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	this->m_GfxCreateInfo.pInputAssemblyState = &m_InputAssemblyState;
}

void	GraphicsPSO::SetRenderTargetFormat(const rhi::RenderTargetFormat &)
{

}

ComputePSO::ComputePSO(Device::Ptr pDevice)
	: PipelineStateObject(pDevice)
{

}

ComputePSO::~ComputePSO()
{

}

K3D_VK_END