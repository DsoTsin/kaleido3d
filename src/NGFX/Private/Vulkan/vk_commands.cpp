#include "vk_common.h"
#include "vk_commands.h"
/*
 Buffer views and texture views are created when encoding commands.
 When binding to fix slot,
 create the right resource view for slot, 
 then insert cooresponding resource barrier in need.
 */
namespace vulkan {
	CommandContext::CommandContext(CommandAllocator& allocator)
	: allocator_(allocator) {
	}

	CommandContext::~CommandContext()
	{
	}

	void CommandContext::Flush()
	{
		CommandType cmd_type = CommandType::Noop;
		while (NextCommandType(cmd_type))
		{
			switch (cmd_type)
			{
			case CommandType::Noop:
				break;
			case CommandType::SetPipeline:
				NextCommand<GfxCmdSetPipeline>();
				break;
			case CommandType::SetBindGroup:
				NextCommand<GfxCmdSetBindGroup>();
				break;
			case CommandType::SetIAVertexBuffer:
				NextCommand<GfxCmdSetIAVertexBuffer>();
				break;
			case CommandType::SetViewport:
				NextCommand<GfxCmdSetViewport>();
				break;
			case CommandType::SetViewports:
				NextCommand<GfxCmdSetViewports>();
				break;
			case CommandType::SetScissors:
				NextCommand<GfxCmdSetScissors>();
				break;
			case CommandType::SetStencilRef:
				NextCommand<GfxCmdSetStencilRef>();
				break;
				//case CommandType::SetBlendFactor:
				//	NextCommand<GfxCmdSetPipeline>();
				//	break;
			case CommandType::SetShadingRate:
				NextCommand<GfxCmdSetShadingRate>();
				break;
			case CommandType::DrawPrimitives:
				NextCommand<GfxCmdDrawPrimitives>();
				break;
			case CommandType::DrawIndexedPrimitives:
				NextCommand<GfxCmdDrawIndexedPrimitives>();
				break;
			case CommandType::DrawIndirect:
				NextCommand<GfxCmdDrawIndirect>();
				break;
			case CommandType::Present:
				NextCommand<GfxCmdPresent>();
				break;
			case CommandType::DispatchCompute:
				NextCommand<GfxCmdDispatchCompute>();
				//vkCmdDispatch();
				break;
				/*case CommandType::DispatchRays:
					break;
				case CommandType::DispatchMesh:
					break;
				case CommandType::NumMaxCommands:
					break;*/
			default:
				break;
			}
		}
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

	RenderEncoder::RenderEncoder(CommandContext& context)
		: context_(context)
	{
	}

	RenderEncoder::~RenderEncoder()
	{

	}

	void RenderEncoder::setPipeline(ngfx::Pipeline* pipeline)
	{

	}

	void RenderEncoder::setBindGroup(const ngfx::BindGroup* bindGroup)
	{

	}

	void RenderEncoder::drawPrimitives(ngfx::PrimitiveType primType, int vertexStart, int vertexCount, int instanceCount, int baseInstance)
	{
		auto& arg = context_.NewCmd<GfxCmdDrawPrimitives>();
		arg.primType = primType;
		arg.vertexStart = vertexStart;
		arg.vertexCount = vertexCount;
		arg.instanceCount = instanceCount;
		arg.baseInstance = baseInstance;
		// flush cmds?
	}

	void RenderEncoder::drawIndexedPrimitives(ngfx::PrimitiveType primType, 
		ngfx::IndexType indexType, int indexCount, const ngfx::Buffer* indexBuffer, int indexBufferOffset, 
		int vertexStart, int vertexCount, int instanceCount, int baseInstance)
	{
		auto& arg = context_.NewCmd<GfxCmdDrawIndexedPrimitives>();
		arg.primType = primType;
		arg.indexType = indexType;
		arg.indexCount = indexCount;
		arg.indexBuffer = indexBuffer;
		arg.indexBufferOffset = indexBufferOffset;
		arg.vertexStart = vertexStart;
		arg.vertexCount = vertexCount;
		arg.instanceCount = instanceCount;
		arg.baseInstance = baseInstance;
		// flush cmds?
	}

	void RenderEncoder::drawIndirect(ngfx::PrimitiveType primType, const ngfx::Buffer * buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
	{
		auto& arg = context_.NewCmd<GfxCmdDrawIndirect>();
		arg.primType = primType;
		arg.buffer = buffer;
		arg.offset = offset;
		arg.drawCount = drawCount;
		arg.stride = stride;
		// flush cmds?
	}

	void RenderEncoder::endEncode()
	{
		context_.Flush();
	}

	ComputeEncoder::ComputeEncoder(CommandContext& context)
		: context_(context)
	{

	}

	ComputeEncoder::~ComputeEncoder()
	{

	}

	void ComputeEncoder::setPipeline(ngfx::Pipeline* pipeline)
	{

	}

	void ComputeEncoder::setBindGroup(const ngfx::BindGroup* bindGroup)
	{

	}

	void ComputeEncoder::dispatch(int thread_group_x, int thread_group_y, int thread_group_z)
	{
		auto& arg = context_.NewCmd<GfxCmdDispatchCompute>();
		arg.threadGroupX = thread_group_x;
		arg.threadGroupY = thread_group_y;
		arg.threadGroupZ = thread_group_z;
	}

	void ComputeEncoder::endEncode()
	{

	}

    GpuCommandBuffer::GpuCommandBuffer(GpuQueue * queue)
	: encoder_context_(nullptr)
	, command_buffer_(VK_NULL_HANDLE)
	, queue_(queue)
    {
		
    }
    
	GpuCommandBuffer::~GpuCommandBuffer()
    {
    }

	void GpuCommandBuffer::setLabel(const char* label)
	{
	}

	const char* GpuCommandBuffer::label() const
	{
		return nullptr;
	}

    ngfx::RenderEncoder* GpuCommandBuffer::newRenderEncoder(ngfx::Result* result)
    {
        return nullptr;
    }

    ngfx::ComputeEncoder* GpuCommandBuffer::newComputeEncoder(ngfx::Result* result)
    {
        return nullptr;
    }

    ngfx::BlitEncoder* GpuCommandBuffer::newBlitEncoder(ngfx::Result* result)
    {
        return nullptr;
    }

    ngfx::ParallelEncoder* GpuCommandBuffer::newParallelRenderEncoder(ngfx::Result* result)
    {
        return nullptr;
    }

    ngfx::RaytraceEncoder* GpuCommandBuffer::newRaytraceEncoder(ngfx::Result* result)
    {
        return nullptr;
    }

    ngfx::Result GpuCommandBuffer::commit()
    {
		//
		queue_->submit(this);

        return ngfx::Result();
    }

	/*
	typedef enum VkCommandPoolCreateFlagBits {
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT = 0x00000001,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = 0x00000002,
		VK_COMMAND_POOL_CREATE_PROTECTED_BIT = 0x00000004,
		VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	} VkCommandPoolCreateFlagBits;
	typedef VkFlags VkCommandPoolCreateFlags;

	typedef enum VkCommandPoolResetFlagBits {
		VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT = 0x00000001,
		VK_COMMAND_POOL_RESET_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	} VkCommandPoolResetFlagBits;
	typedef VkFlags VkCommandPoolResetFlags;

	typedef enum VkCommandBufferUsageFlagBits {
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = 0x00000001,
		VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = 0x00000002,
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = 0x00000004,
		VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	} VkCommandBufferUsageFlagBits;
	typedef VkFlags VkCommandBufferUsageFlags;

	typedef enum VkCommandBufferResetFlagBits {
		VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT = 0x00000001,
		VK_COMMAND_BUFFER_RESET_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	} VkCommandBufferResetFlagBits;
	typedef VkFlags VkCommandBufferResetFlags;
	*/
	ngfx::CommandBuffer* GpuQueue::newCommandBuffer()
	{
		// or lazy init?
		VkCommandPoolCreateInfo cinfo = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, // flags, one shot
			0, // queueFamilyIndex
		};
		VkCommandPool pool;
		device_->createCommandPool(&cinfo, &pool);
		VkCommandBufferAllocateInfo ainfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			pool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, // level
			1, // count
		};
		VkCommandBuffer cmdbuf;
		device_->allocateCommandBuffer(&ainfo, &cmdbuf);
		return new vulkan::GpuCommandBuffer(this);
	}

	void GpuQueue::submit(GpuCommandBuffer* cmdBuf)
	{
		VkSubmitInfo sinfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,

		};
		VkSubmitInfo2KHR sinfo2 = {

		};
		VkFence fence = VK_NULL_HANDLE;
		// fence is an optional handle to a fence to be signaled once all submitted command buffers have completed execution.
		// If fence is not VK_NULL_HANDLE, it defines a fence signal operation.
		device_->queueSubmit(queue_, 1, &sinfo, fence);
	}
}