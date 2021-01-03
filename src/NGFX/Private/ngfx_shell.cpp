#include "ngfx_shell.h"

namespace ngfxu
{
	PresentLayer Factory::newPresentLayer(ngfx::PresentLayerDesc const& desc, Device device)
	{
		return PresentLayer(ptr_->newPresentLayer(&desc, device.iptr(), nullptr, nullptr));
	}
	Drawable Factory::getDrawable()
	{
		return Drawable(nullptr);
	}
	RenderCommandEncoder ParallelRenderEncoder::subEncoder()
	{
		return RenderCommandEncoder(ptr_->subRenderEncoder(nullptr));
	}
	void ParallelRenderEncoder::endEncode()
	{
	}
	RenderCommandEncoder CommandBuffer::newRenderEncoder(ngfx::RenderpassDesc const& rpDesc)
	{
		return RenderCommandEncoder(ptr_->newRenderEncoder(nullptr));
	}
	ParallelRenderEncoder CommandBuffer::newParallelRenderEncoder(ngfx::RenderpassDesc const & rpDesc)
	{
		return ParallelRenderEncoder(ptr_->newParallelRenderEncoder(nullptr));
	}
	ComputeEncoder CommandBuffer::newComputeEncoder()
	{
		return ComputeEncoder(ptr_->newComputeEncoder(nullptr));
	}
	void CommandBuffer::commit()
	{
	}
	void CommandBuffer::release()
	{
	}
	CommandBuffer CommandQueue::obtainCommandBuffer()
	{
		return CommandBuffer(ptr_->newCommandBuffer());
	}
	void RenderCommandEncoder::updateFence(Fence fence)
	{
	}
	void RenderCommandEncoder::waitForFence(Fence fence)
	{
	}
    void RenderCommandEncoder::drawPrimitive(ngfx::PrimitiveType primType, int vertexStart, int vertexCount, int instanceCount, int baseInstance)
    {
        ptr_->drawPrimitives(primType, vertexStart, vertexCount, instanceCount, baseInstance);
    }
    void RenderCommandEncoder::drawIndexedPrimitives(ngfx::PrimitiveType primType, ngfx::IndexType indexType, int indexCount, const Buffer* indexBuffer, int indexBufferOffset, int vertexStart, int vertexCount, int instanceCount, int baseInstance)
    {
        ptr_->drawIndexedPrimitives(primType, indexType, indexCount, indexBuffer->iptr(), indexBufferOffset, vertexStart, vertexCount, instanceCount, baseInstance);
    }
    void RenderCommandEncoder::drawIndirect(ngfx::PrimitiveType primType, const Buffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
    {
        ptr_->drawIndirect(primType, buffer->iptr(), offset, drawCount, stride);
    }

	void RenderCommandEncoder::presentDrawable(Drawable const & drawable)
	{

	}
	void RenderCommandEncoder::endEncode()
	{
        ptr_->endEncode();
	}
	void ComputeEncoder::setPipeline(const ComputePipeline& pipeline)
	{

	}
	void ComputeEncoder::updateFence(Fence fence)
	{
	}
	void ComputeEncoder::waitForFence(Fence fence)
	{
		
	}
	void ComputeEncoder::dispatch(int x, int y, int z)
	{
        ptr_->dispatch(x, y, z);
	}
	void ComputeEncoder::endEncode()
	{
        ptr_->endEncode();
	}
	RenderPipeline Device::newRenderPipeline(const ngfx::RenderPipelineDesc& desc)
	{

		return RenderPipeline();
	}
	ComputePipeline Device::newComputePipeline(const ngfx::ComputePipelineDesc& desc)
	{
		return ComputePipeline(ptr_->newComputePipeline(&desc, nullptr));
	}
	Fence Device::newFence()
	{
		return Fence(ptr_->newFence(nullptr));
	}
	void Device::wait()
	{
	}
	Drawable PresentLayer::nextDrawable()
	{
		return Drawable(ptr_->nextDrawable());
	}

	Texture Drawable::texture()
	{
		return Texture(ptr_->texture());
	}
}