#include "vk_common.h"
#include "vk_format.h"

namespace vulkan
{
	// ---- GpuRenderPipeline ----

	GpuRenderPipeline::GpuRenderPipeline(GpuDevice* device, ngfx::RenderPipelineDesc const& desc, VkRenderPass renderPass)
		: GpuPipelineBase(device)
		, desc_(desc)
	{
		build(desc, renderPass);
	}

	GpuRenderPipeline::~GpuRenderPipeline()
	{
	}

	void GpuRenderPipeline::build(ngfx::RenderPipelineDesc const& desc, VkRenderPass renderPass)
	{
		ngfx::Vec<VkPipelineShaderStageCreateInfo> shaderStages;
		ngfx::Vec<VkShaderModule> shaderModules;

		auto makeStage = [&](ngfx::Function* fn, VkShaderStageFlagBits stage) {
			if (!fn || !fn->bundle()) return;
			VkShaderModuleCreateInfo moduleInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			moduleInfo.codeSize = (size_t)fn->bundle()->length();
			moduleInfo.pCode = (const uint32_t*)fn->bundle()->data();
			VkShaderModule mod = VK_NULL_HANDLE;
			if (device_->createShaderModule(&moduleInfo, &mod) == VK_SUCCESS) {
				shaderModules.push(mod);
				VkPipelineShaderStageCreateInfo stageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
				stageInfo.stage = stage;
				stageInfo.module = mod;
				stageInfo.pName = fn->entry() ? fn->entry() : "main";
				shaderStages.push(stageInfo);
			}
		};

		makeStage(desc.vertex, VK_SHADER_STAGE_VERTEX_BIT);
		makeStage(desc.pixel, VK_SHADER_STAGE_FRAGMENT_BIT);
		makeStage(desc.geometry, VK_SHADER_STAGE_GEOMETRY_BIT);
		makeStage(desc.domain, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
		makeStage(desc.hull, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);

		ngfx::Vec<VkVertexInputBindingDescription> bindings;
		ngfx::Vec<VkVertexInputAttributeDescription> attributes;

		for (size_t i = 0; i < desc.input.layouts.num(); ++i) {
			VkVertexInputBindingDescription bind = {};
			bind.binding = (uint32_t)i;
			bind.stride = desc.input.layouts.at(i).stride;
			bind.inputRate = inputRateToVk(desc.input.layouts.at(i).inputRate);
			bindings.push(bind);
		}
		for (size_t i = 0; i < desc.input.attributes.num(); ++i) {
			VkVertexInputAttributeDescription attr = {};
			attr.location = (uint32_t)i;
			attr.binding = desc.input.attributes.at(i).slot;
			attr.format = vertexFormatToVk((ngfx::VertexFormat)desc.input.attributes.at(i).format);
			attr.offset = desc.input.attributes.at(i).offset;
			attributes.push(attr);
		}

		VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInput.vertexBindingDescriptionCount = (uint32_t)bindings.num();
		vertexInput.pVertexBindingDescriptions = bindings.num() > 0 ? &bindings[0] : nullptr;
		vertexInput.vertexAttributeDescriptionCount = (uint32_t)attributes.num();
		vertexInput.pVertexAttributeDescriptions = attributes.num() > 0 ? &attributes[0] : nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = fillModeToVk(desc.rasterizer.fillMode);
		rasterizer.cullMode = cullModeToVk(desc.rasterizer.cullMode);
		rasterizer.frontFace = desc.rasterizer.frontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = (desc.rasterizer.depthBias != 0.0f) ? VK_TRUE : VK_FALSE;
		rasterizer.depthBiasConstantFactor = desc.rasterizer.depthBias;
		rasterizer.depthBiasClamp = desc.rasterizer.depthBiasClamp;
		rasterizer.depthBiasSlopeFactor = desc.rasterizer.depthBiasSlope;
		rasterizer.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.rasterizationSamples = multisampleToVk(desc.rasterizer.multisample);

		VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencil.depthTestEnable = desc.depthStencil.depthTest ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = (desc.depthStencil.depthWriteMask == ngfx::DepthWriteMask::All) ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = compareOpToVk(desc.depthStencil.depthFunction);
		depthStencil.stencilTestEnable = desc.depthStencil.stencilTest ? VK_TRUE : VK_FALSE;
		depthStencil.front.failOp = stencilOpToVk(desc.depthStencil.front.stencilFailOp);
		depthStencil.front.passOp = stencilOpToVk(desc.depthStencil.front.stencilPassOp);
		depthStencil.front.depthFailOp = stencilOpToVk(desc.depthStencil.front.depthStencilFailOp);
		depthStencil.front.compareOp = compareOpToVk(desc.depthStencil.front.stencilFunc);
		depthStencil.front.compareMask = desc.depthStencil.front.compareMask;
		depthStencil.front.writeMask = desc.depthStencil.front.writeMask;
		depthStencil.front.reference = desc.depthStencil.front.reference;
		depthStencil.back.failOp = stencilOpToVk(desc.depthStencil.back.stencilFailOp);
		depthStencil.back.passOp = stencilOpToVk(desc.depthStencil.back.stencilPassOp);
		depthStencil.back.depthFailOp = stencilOpToVk(desc.depthStencil.back.depthStencilFailOp);
		depthStencil.back.compareOp = compareOpToVk(desc.depthStencil.back.stencilFunc);
		depthStencil.back.compareMask = desc.depthStencil.back.compareMask;
		depthStencil.back.writeMask = desc.depthStencil.back.writeMask;
		depthStencil.back.reference = desc.depthStencil.back.reference;

		ngfx::Vec<VkPipelineColorBlendAttachmentState> colorAttachments;
		for (size_t i = 0; i < desc.blend.renderTargets.num(); ++i) {
			auto const& rt = desc.blend.renderTargets.at(i);
			VkPipelineColorBlendAttachmentState att = {};
			att.blendEnable = rt.blendEnable ? VK_TRUE : VK_FALSE;
			att.srcColorBlendFactor = blendFactorToVk(rt.srcColor);
			att.dstColorBlendFactor = blendFactorToVk(rt.destColor);
			att.colorBlendOp = blendOpToVk(rt.colorOp);
			att.srcAlphaBlendFactor = blendFactorToVk(rt.srcAlpha);
			att.dstAlphaBlendFactor = blendFactorToVk(rt.destAlpha);
			att.alphaBlendOp = blendOpToVk(rt.alphaOp);
			att.colorWriteMask = rt.colorWriteMask & 0xF;
			colorAttachments.push(att);
		}
		if (colorAttachments.empty()) {
			VkPipelineColorBlendAttachmentState defaultAtt = {};
			defaultAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorAttachments.push(defaultAtt);
		}

		VkPipelineColorBlendStateCreateInfo colorBlend = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlend.attachmentCount = (uint32_t)colorAttachments.num();
		colorBlend.pAttachments = &colorAttachments[0];

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_STENCIL_REFERENCE,
			VK_DYNAMIC_STATE_DEPTH_BIAS,
		};
		VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = 4;
		dynamicState.pDynamicStates = dynamicStates;

		VkDescriptorSetLayoutCreateInfo dslInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		device_->createDescriptorSetLayout(&dslInfo, &descriptor_set_layout_);

		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &descriptor_set_layout_;
		device_->createPipelineLayout(&layoutInfo, &pipeline_layout_);

		VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.stageCount = (uint32_t)shaderStages.num();
		pipelineInfo.pStages = shaderStages.num() > 0 ? &shaderStages[0] : nullptr;
		pipelineInfo.pVertexInputState = &vertexInput;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlend;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline_layout_;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		device_->createGraphicsPipelines(pipeline_cache_, 1, &pipelineInfo, &pipeline_);

		for (size_t i = 0; i < shaderModules.num(); ++i) {
			device_->destroyShaderModule(shaderModules.at(i));
		}
	}

	// ---- GpuComputePipeline ----

	GpuComputePipeline::GpuComputePipeline(GpuDevice* device, ngfx::ComputePipelineDesc const& desc)
		: GpuPipelineBase(device)
		, desc_(desc)
	{
		build(desc);
	}

	GpuComputePipeline::~GpuComputePipeline()
	{
	}

	void GpuComputePipeline::build(ngfx::ComputePipelineDesc const& desc)
	{
		if (!desc.function || !desc.function->bundle()) return;

		VkShaderModuleCreateInfo moduleInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		moduleInfo.codeSize = (size_t)desc.function->bundle()->length();
		moduleInfo.pCode = (const uint32_t*)desc.function->bundle()->data();
		VkShaderModule mod = VK_NULL_HANDLE;
		if (device_->createShaderModule(&moduleInfo, &mod) != VK_SUCCESS) return;

		VkDescriptorSetLayoutCreateInfo dslInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		device_->createDescriptorSetLayout(&dslInfo, &descriptor_set_layout_);

		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &descriptor_set_layout_;
		device_->createPipelineLayout(&layoutInfo, &pipeline_layout_);

		VkComputePipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		pipelineInfo.stage.module = mod;
		pipelineInfo.stage.pName = desc.function->entry() ? desc.function->entry() : "main";
		pipelineInfo.layout = pipeline_layout_;

		device_->createComputePipelines(pipeline_cache_, 1, &pipelineInfo, &pipeline_);
		device_->destroyShaderModule(mod);
	}
}
