#include "vk_common.h"
#include "vk_commands.h"
#include "vk_format.h"

namespace vulkan {

	// ---- GpuCommandBuffer ----

    GpuCommandBuffer::GpuCommandBuffer(GpuQueue * queue)
	: command_pool_(VK_NULL_HANDLE)
	, command_buffer_(VK_NULL_HANDLE)
	, queue_(queue)
    {
		auto* device = queue->getDevice();
		VkCommandPoolCreateInfo poolInfo = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			queue->familyIndex(),
		};
		device->createCommandPool(&poolInfo, &command_pool_);

		VkCommandBufferAllocateInfo allocInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			command_pool_,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};
		device->allocateCommandBuffer(&allocInfo, &command_buffer_);

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		device->beginCommandBuffer(command_buffer_, &beginInfo);
    }
    
	GpuCommandBuffer::~GpuCommandBuffer()
    {
		auto* device = getDevice();
		if (command_pool_ != VK_NULL_HANDLE) {
			device->freeCommandBuffer(command_pool_, 1, &command_buffer_);
			device->destroyCommandPool(command_pool_);
			command_pool_ = VK_NULL_HANDLE;
			command_buffer_ = VK_NULL_HANDLE;
		}
    }

	GpuDevice* GpuCommandBuffer::getDevice() const
	{
		return queue_->getDevice();
	}

	void GpuCommandBuffer::setLabel(const char* label)
	{
		if (label) label_ = label;
	}

	const char* GpuCommandBuffer::label() const
	{
		return label_.empty() ? nullptr : label_.c_str();
	}

    ngfx::RenderEncoder* GpuCommandBuffer::newRenderEncoder(ngfx::Result* result)
    {
		if (result) *result = ngfx::Result::Ok;
        return new GpuRenderEncoder(this);
    }

    ngfx::ComputeEncoder* GpuCommandBuffer::newComputeEncoder(ngfx::Result* result)
    {
		if (result) *result = ngfx::Result::Ok;
        return new GpuComputeEncoder(this);
    }

    ngfx::BlitEncoder* GpuCommandBuffer::newBlitEncoder(ngfx::Result* result)
    {
		if (result) *result = ngfx::Result::Ok;
		return nullptr;
    }

    ngfx::ParallelEncoder* GpuCommandBuffer::newParallelRenderEncoder(ngfx::Result* result)
    {
		if (result) *result = ngfx::Result::Ok;
		return new GpuParallelRenderEncoder(this);
    }

    ngfx::RaytraceEncoder* GpuCommandBuffer::newRaytraceEncoder(ngfx::Result* result)
    {
		if (result) *result = ngfx::Result::Ok;
		return new GpuRaytracingEncoder(this);
    }

    ngfx::Result GpuCommandBuffer::commit()
    {
		auto* device = getDevice();
		device->endCommandBuffer(command_buffer_);

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command_buffer_;

		VkResult ret = device->queueSubmit(queue_->vkQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		return (ret == VK_SUCCESS) ? ngfx::Result::Ok : ngfx::Result::Failed;
    }

	// ---- GpuRenderEncoder ----

	GpuRenderEncoder::GpuRenderEncoder(GpuCommandBuffer* cmdBuf)
		: command_(cmdBuf)
	{
	}

	GpuRenderEncoder::~GpuRenderEncoder()
	{
	}

	void GpuRenderEncoder::setLabel(const char* label) { if (label) label_ = label; }
	const char* GpuRenderEncoder::label() const { return label_.empty() ? nullptr : label_.c_str(); }

	void GpuRenderEncoder::setPipeline(ngfx::Pipeline* pipeline)
	{
		if (!pipeline) return;
		auto* gpuPipeline = static_cast<GpuPipelineBase*>(pipeline);
		bound_pipeline_ = gpuPipeline->vkPipeline();
		bound_layout_ = gpuPipeline->vkPipelineLayout();
		auto* device = command_->getDevice();
		device->__CmdBindPipeline(command_->vkCmdBuf(), VK_PIPELINE_BIND_POINT_GRAPHICS, bound_pipeline_);
	}

	void GpuRenderEncoder::setBindGroup(const ngfx::BindGroup* bindGroup)
	{
		if (!bindGroup) return;
		auto* gpuGroup = static_cast<const GpuBindGroup*>(bindGroup);
		VkDescriptorSet set = gpuGroup->vkDescriptorSet();
		auto* device = command_->getDevice();
		device->__CmdBindDescriptorSets(command_->vkCmdBuf(), VK_PIPELINE_BIND_POINT_GRAPHICS,
			bound_layout_, 0, 1, &set, 0, nullptr);
	}

	void GpuRenderEncoder::setViewport(ngfx::Viewport viewport)
	{
		VkViewport vp = {};
		vp.x = viewport.x;
		vp.y = viewport.y;
		vp.width = viewport.width;
		vp.height = viewport.height;
		vp.minDepth = viewport.near;
		vp.maxDepth = viewport.far;
		auto* device = command_->getDevice();
		device->__CmdSetViewport(command_->vkCmdBuf(), 0, 1, &vp);
	}

	void GpuRenderEncoder::setViewports(int numViewports, const ngfx::Viewport* pViewport)
	{
		ngfx::Vec<VkViewport> vps;
		for (int i = 0; i < numViewports; ++i) {
			VkViewport vp = {};
			vp.x = pViewport[i].x;
			vp.y = pViewport[i].y;
			vp.width = pViewport[i].width;
			vp.height = pViewport[i].height;
			vp.minDepth = pViewport[i].near;
			vp.maxDepth = pViewport[i].far;
			vps.push(vp);
		}
		auto* device = command_->getDevice();
		device->__CmdSetViewport(command_->vkCmdBuf(), 0, (uint32_t)numViewports, &vps[0]);
	}

	void GpuRenderEncoder::setScissors(int numScissors, const ngfx::Rect* pRects)
	{
		ngfx::Vec<VkRect2D> rects;
		for (int i = 0; i < numScissors; ++i) {
			VkRect2D r = {};
			r.offset.x = (int32_t)pRects[i].x;
			r.offset.y = (int32_t)pRects[i].y;
			r.extent.width = pRects[i].width;
			r.extent.height = pRects[i].height;
			rects.push(r);
		}
		auto* device = command_->getDevice();
		device->__CmdSetScissor(command_->vkCmdBuf(), 0, (uint32_t)numScissors, &rects[0]);
	}

	void GpuRenderEncoder::setStencilRef()
	{
		auto* device = command_->getDevice();
		device->__CmdSetStencilReference(command_->vkCmdBuf(), VK_STENCIL_FACE_FRONT_AND_BACK, 0);
	}

	void GpuRenderEncoder::setDepthBias()
	{
		auto* device = command_->getDevice();
		device->__CmdSetDepthBias(command_->vkCmdBuf(), 0.0f, 0.0f, 0.0f);
	}

	void GpuRenderEncoder::drawPrimitives(ngfx::PrimitiveType primType,
		int vertexStart, int vertexCount, int instanceCount, int baseInstance)
	{
		auto* device = command_->getDevice();
		device->__CmdDraw(command_->vkCmdBuf(), vertexCount, instanceCount > 0 ? instanceCount : 1, vertexStart, baseInstance);
	}

	void GpuRenderEncoder::drawIndexedPrimitives(
		ngfx::PrimitiveType primType, ngfx::IndexType indexType, int indexCount,
		const ngfx::Buffer* indexBuffer, int indexBufferOffset,
		int vertexStart, int vertexCount, int instanceCount, int baseInstance)
	{
		if (indexBuffer) {
			auto* gpuBuf = static_cast<const GpuBuffer*>(indexBuffer);
			auto* device = command_->getDevice();
			device->__CmdBindIndexBuffer(command_->vkCmdBuf(), gpuBuf->vkBuffer(),
				(VkDeviceSize)indexBufferOffset, indexTypeToVk(indexType));
			device->__CmdDrawIndexed(command_->vkCmdBuf(), indexCount,
				instanceCount > 0 ? instanceCount : 1, 0, vertexStart, baseInstance);
		}
	}

	void GpuRenderEncoder::drawIndirect(ngfx::PrimitiveType primType,
		const ngfx::Buffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		if (!buffer) return;
		auto* gpuBuf = static_cast<const GpuBuffer*>(buffer);
		auto* device = command_->getDevice();
		device->__CmdDrawIndirect(command_->vkCmdBuf(), gpuBuf->vkBuffer(), offset, drawCount, stride);
	}

	void GpuRenderEncoder::present(ngfx::Drawable* drawable)
	{
		pending_drawable_ = static_cast<GpuDrawable*>(drawable);
	}

	void GpuRenderEncoder::endEncode()
	{
		if (pending_drawable_) {
			pending_drawable_->present();
			pending_drawable_ = nullptr;
		}
	}

	// ---- GpuComputeEncoder ----

	GpuComputeEncoder::GpuComputeEncoder(GpuCommandBuffer* cmdBuf)
		: command_(cmdBuf)
	{
	}

	GpuComputeEncoder::~GpuComputeEncoder()
	{
	}

	void GpuComputeEncoder::setLabel(const char* label) { if (label) label_ = label; }
	const char* GpuComputeEncoder::label() const { return label_.empty() ? nullptr : label_.c_str(); }

	void GpuComputeEncoder::setPipeline(ngfx::Pipeline* pipeline)
	{
		if (!pipeline) return;
		auto* gpuPipeline = static_cast<GpuPipelineBase*>(pipeline);
		bound_pipeline_ = gpuPipeline->vkPipeline();
		bound_layout_ = gpuPipeline->vkPipelineLayout();
		auto* device = command_->getDevice();
		device->__CmdBindPipeline(command_->vkCmdBuf(), VK_PIPELINE_BIND_POINT_COMPUTE, bound_pipeline_);
	}

	void GpuComputeEncoder::setBindGroup(const ngfx::BindGroup* bindGroup)
	{
		if (!bindGroup) return;
		auto* gpuGroup = static_cast<const GpuBindGroup*>(bindGroup);
		VkDescriptorSet set = gpuGroup->vkDescriptorSet();
		auto* device = command_->getDevice();
		device->__CmdBindDescriptorSets(command_->vkCmdBuf(), VK_PIPELINE_BIND_POINT_COMPUTE,
			bound_layout_, 0, 1, &set, 0, nullptr);
	}

	void GpuComputeEncoder::dispatch(int x, int y, int z)
	{
		auto* device = command_->getDevice();
		device->__CmdDispatch(command_->vkCmdBuf(), (uint32_t)x, (uint32_t)y, (uint32_t)z);
	}

	void GpuComputeEncoder::endEncode()
	{
	}

	// ---- GpuBlitEncoder ----

	GpuBlitEncoder::GpuBlitEncoder(GpuCommandBuffer* cmdBuf)
		: command_(cmdBuf)
	{
	}

	GpuBlitEncoder::~GpuBlitEncoder() {}

	void GpuBlitEncoder::setLabel(const char* label) { if (label) label_ = label; }
	const char* GpuBlitEncoder::label() const { return label_.empty() ? nullptr : label_.c_str(); }
	void GpuBlitEncoder::setPipeline(ngfx::Pipeline* pipeline) {}
	void GpuBlitEncoder::setBindGroup(const ngfx::BindGroup* bindGroup) {}
	void GpuBlitEncoder::endEncode() {}

	// ---- GpuParallelRenderEncoder ----

	GpuParallelRenderEncoder::GpuParallelRenderEncoder(GpuCommandBuffer* cmdBuf)
		: command_(cmdBuf)
	{
	}

	GpuParallelRenderEncoder::~GpuParallelRenderEncoder() {}

	void GpuParallelRenderEncoder::setLabel(const char* label) { if (label) label_ = label; }
	const char* GpuParallelRenderEncoder::label() const { return label_.empty() ? nullptr : label_.c_str(); }
	void GpuParallelRenderEncoder::setPipeline(ngfx::Pipeline* pipeline) {}
	void GpuParallelRenderEncoder::setBindGroup(const ngfx::BindGroup* bindGroup) {}
	void GpuParallelRenderEncoder::endEncode() {}

	ngfx::RenderEncoder* GpuParallelRenderEncoder::subRenderEncoder(ngfx::Result* result)
	{
		if (result) *result = ngfx::Result::Ok;
		return new GpuRenderEncoder(command_.get());
	}

	// ---- GpuRaytracingEncoder ----

	GpuRaytracingEncoder::GpuRaytracingEncoder(GpuCommandBuffer* cmdBuf)
		: command_(cmdBuf)
	{
	}

	GpuRaytracingEncoder::~GpuRaytracingEncoder() {}

	void GpuRaytracingEncoder::setLabel(const char* label) { if (label) label_ = label; }
	const char* GpuRaytracingEncoder::label() const { return label_.empty() ? nullptr : label_.c_str(); }
	void GpuRaytracingEncoder::setPipeline(ngfx::Pipeline* pipeline) {}
	void GpuRaytracingEncoder::setBindGroup(const ngfx::BindGroup* bindGroup) {}
	void GpuRaytracingEncoder::endEncode() {}

    void GpuRaytracingEncoder::buildAS(ngfx::RaytracingAS * src, ngfx::RaytracingAS * dest, ngfx::Buffer * scratch)
    {
    }

    void GpuRaytracingEncoder::copyAS(ngfx::RaytracingAS * src, ngfx::RaytracingAS * dest, ngfx::AccelerationStructureCopyMode mode)
    {
    }

    void GpuRaytracingEncoder::traceRay(ngfx::Buffer * rayGen, ngfx::BufferStride miss, ngfx::BufferStride hit, int width, int height)
    {
    }

    // ---- GpuQueue ----

	ngfx::CommandBuffer* GpuQueue::newCommandBuffer()
	{
		return new GpuCommandBuffer(this);
	}

	void GpuQueue::submit(GpuCommandBuffer* cmdBuf)
	{
		// Submit is handled by GpuCommandBuffer::commit()
	}

	// ---- CommandAllocator (legacy, kept for compat) ----

	CommandContext::CommandContext(CommandAllocator& allocator)
	: allocator_(allocator) {
	}

	CommandContext::~CommandContext()
	{
	}

	void CommandContext::Flush()
	{
	}

	void CommandContext::Reset()
	{
	}

	bool CommandContext::NextCommandType(CommandType& cmd_type)
	{
		return false;
	}

	CommandAllocator::CommandAllocator() {}
	CommandAllocator::~CommandAllocator() {}

	void* CommandAllocator::allocate(size_t size, size_t alignment)
	{
		return nullptr;
	}

	void CommandAllocator::free(void* ptr)
	{
	}

	void CommandAllocator::trimBlocks()
	{
	}
}
