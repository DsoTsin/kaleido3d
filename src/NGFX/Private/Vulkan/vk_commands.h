#pragma once

#include <stdint.h>
#include "linear_array.h"

namespace vulkan
{
	enum class CommandType : uint8_t 
	{
		Noop,
		SetPipeline,	// bind pipeline state
		SetBindGroup,	// bind resources
		SetIAVertexBuffer,
		//~ dynamic states
		SetViewport,
		SetViewports,
		SetScissors,
		SetStencilRef,
		//SetDepthBias,
		SetBlendFactor,
		SetShadingRate,
		//~ End dynamic states
		DrawPrimitives,
		DrawIndexedPrimitives,
		DrawIndirect,
		Present,
		DispatchCompute,
		DispatchComputeIndirect,
		DispatchRays,
		DispatchMesh,
		NumMaxCommands
	};

	struct GfxCommand
	{
		virtual ~GfxCommand() {}
	};

#define CMD_BEGIN(type) struct GfxCmd##type : public GfxCommand {\
	static constexpr CommandType cmd_id = CommandType::type;
#define CMD_END };

	CMD_BEGIN(SetPipeline)

	CMD_END

	CMD_BEGIN(SetBindGroup)

	CMD_END

	CMD_BEGIN(SetIAVertexBuffer)

	CMD_END

	CMD_BEGIN(SetViewport)

	CMD_END

	CMD_BEGIN(SetViewports)

	CMD_END

	CMD_BEGIN(SetScissors)

	CMD_END

	CMD_BEGIN(SetStencilRef)

	CMD_END

	CMD_BEGIN(SetShadingRate)

	CMD_END

	CMD_BEGIN(DrawPrimitives)
		ngfx::PrimitiveType primType;
		int vertexStart;
		int vertexCount;
		int instanceCount;
		int baseInstance;
	CMD_END

	CMD_BEGIN(DrawIndexedPrimitives)
		ngfx::PrimitiveType primType;
		ngfx::IndexType indexType;
		int indexCount;
		const ngfx::Buffer* indexBuffer;
		int indexBufferOffset;
		int vertexStart;
		int vertexCount;
		int instanceCount;
		int baseInstance;
	CMD_END

	CMD_BEGIN(DrawIndirect)
		ngfx::PrimitiveType primType;
		const ngfx::Buffer* buffer;
		uint64_t offset;
		uint32_t drawCount;
		uint32_t stride;
	CMD_END

	CMD_BEGIN(Present)

	CMD_END

	CMD_BEGIN(DispatchCompute)
		int32_t threadGroupX;
		int32_t threadGroupY;
		int32_t threadGroupZ;
	CMD_END

	class CommandContext
	{
	public:
		CommandContext(class CommandAllocator& allocator);
		~CommandContext();

		template <typename T, typename ...Args>
		T& NewCmd(Args...args)
		{
			void* cmdAddr = allocator_.allocate(sizeof(T), alignof(T));

			auto Cmd = new (cmdAddr) T(args...);
			return *Cmd;
		}

		bool NextCommandType(CommandType& cmd_type);

		template <typename T>
		T& NextCommand()
		{
			return *(T*)nullptr;
		}

		void Reset();
		void Flush();

	private:
		VkCommandPool			cmd_pool_;
		VkCommandBuffer			cmd_buf_;
		CommandAllocator&		allocator_;
	};

	class CommandAllocator
	{
	public:
		// default block is 4MB
		static constexpr size_t PreAllocBlockSize = 4 * 1024 * 1024;

		struct Block
		{
			void*	ptr;
			size_t	size;
			int		nextBlockId;
		};

		CommandAllocator();
		~CommandAllocator();

		void*	allocate(size_t size, size_t alignment);
		void	free(void* ptr);

		void	trimBlocks();

	private:
		LinearArray<Block, 4> allocated_blocks_;
		LinearArray<Block, 4> freed_blocks_;
	};
}