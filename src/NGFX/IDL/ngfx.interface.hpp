#include "ngfx.structs.hpp"

namespace ngfx {
	interface Device;
    [[refcount("true")]]
    interface LabeledObject
    {
        void                set_label(const char* label);
        const char*         label() const;
    };
    [[refcount("true")]]
    interface Blob {
        const void*			data() const;
        uint64				length() const;
    };
    [[refcount("true")]]
    interface Function {
        const char*			entry() const;
        const Blob*			bundle() const;
    };
    struct RenderPipelineDesc {
        RasterizerState     rasterizer;
        BlendState          blend;
        DepthStencilState   depthStencil;
        VertexInputState    input;

        PixelFormat         depthStencilFormat;

		Function*			vertex;
		Function*			pixel;
		Function*			geometry;
		Function*			domain;
		Function*			hull;
        uint32              deviceMask;
    };
    struct ComputePipelineDesc {
		Function*			function;
		uint32				deviceMask;
    };
    struct RaytracePipelineDesc {
		uint32				maxTraceRecurseDepth;
		array<Function*>	functions;
    };
	struct RenderpassDesc {

	};
    interface Resource : LabeledObject {
		void*				map(uint64 offset, uint64 size);
		void				unmap(void* addr);
    };
	interface RaytracingAS : LabeledObject {

	};
	interface TextureView;
	[[vulkan("VkImage"), metal("id<MTLTexture>")]]
    interface Texture : Resource {
		PixelFormat			format() const;
		TextureView*		newView(Result* result);
    };
	interface TextureView {
		const Texture*		texture() const;
		TextureUsage		usage() const;
	};
	interface BufferView;
	[[vulkan("VkBuffer"), metal("id<MTLBuffer>")]]
    interface Buffer : Resource {
		BufferView*			newView(Result* result);
    };
	interface BufferView {
		const Buffer*		buffer() const;
		BufferUsage			usage() const;
	};
	[[vulkan("VkSampler"), metal("id<MTLSampler>")]]
    interface Sampler : LabeledObject {

    };
    interface Shader : LabeledObject {

    };
	[[vulkan("VkFramebuffer"),refcount("true")]]
    interface Framebuffer : LabeledObject {

    };
	[[vulkan("VkSwapchain"),refcount("true")]]
    interface Swapchain {
		Texture*			currentTexture();
    };
	[[transient("frame"), refcount("true")]]
	interface BindGroup {
		void				set_sampler(uint32 id, ShaderStage stage, const Sampler* sampler);
		void				set_texture(uint32 id, ShaderStage stage, const TextureView* texture);
		void				set_buffer(uint32 id, ShaderStage stage, const BufferView* buffer);
		void				set_raytracing_as(uint32 id, ShaderStage stage, const RaytracingAS* as);
	};
    interface Pipeline : LabeledObject {
		BindGroup*			newBindGroup(Result* result);
		Device*				device();
    };
    interface RenderPipeline : Pipeline {

    };
	[[vulkan("VkRenderpass"), refcount("true")]]
	interface Renderpass {
		RenderPipeline*		newRenderPipeline(const RenderPipelineDesc* desc, Result* result);
	};
    interface ComputePipeline : Pipeline {

    };
    interface RaytracePipeline : Pipeline {
        
    };
	interface RenderEncoder;
	interface ComputeEncoder;
	interface RaytraceEncoder;
	[[vulkan("VkCommandBuffer"), metal("id<MTLCommandBuffer>")]]
    interface CommandBuffer : LabeledObject {
        RenderEncoder *		newRenderEncoder(Result* result);
        ComputeEncoder*		newComputeEncoder(Result* result);
		Result				newBlitEncoder();
        Result				newParallelRenderEncoder();
        RaytraceEncoder*	newRaytraceEncoder(Result* result);
        Result				commit();
    };
	[[refcount("true"), vulkan("VkQueue"), metal("id<MTLQueue>")]]
    interface CommandQueue {
		CommandBuffer*		newCommandBuffer() [[transient("true")]];
    };
    interface CommandEncoder : LabeledObject{
		void				setPipeline(Pipeline* pipeline);
		void				setBindGroup(const BindGroup* bindGroup);
        void				endEncode();
    };
    interface RenderEncoder : CommandEncoder {
        void				draw();
        void				present(Swapchain* swapchain);
    };
    interface ComputeEncoder : CommandEncoder {
        void				dispatch(int x, int y, int z);
    };
    interface RaytraceEncoder : CommandEncoder {
		void				buildAS();
        void				traceRay(int width, int height);
    };
	[[vulkan("VkFence"), metal("id<MTLFence>")]]
    interface Fence : LabeledObject {
        void				signal();
    };
	[[vulkan("VkDevice"), metal("id<MTLDevice>")]]
    interface Device : LabeledObject {
        DeviceType          getType() const;
		CommandQueue*		newQueue();
        Shader*				newShader();
        Renderpass*			newRenderpass(const RenderpassDesc* desc, Result* result) [[gen_rc("true")]];
		ComputePipeline*	newComputePipeline(const ComputePipelineDesc* desc, Result* result)[[gen_rc("true")]];
		RaytracePipeline*	newRaytracePipeline(const RaytracePipelineDesc* desc, Result* result)[[gen_rc("true")]];
		Texture*			newTexture(const TextureDesc* desc, StorageMode mode, Result* result)[[gen_rc("true")]];
        Buffer*				newBuffer(const BufferDesc* desc, StorageMode mode, Result* result)[[gen_rc("true")]];
		RaytracingAS*		newRaytracingAS(const RaytracingASDesc* rtDesc, Result* result)[[gen_rc("true")]];
		Sampler*			newSampler(const SamplerDesc* desc, Result* result)[[gen_rc("true")]];
        Fence*				newFence();
        Result				wait();
    };
    [[refcount("true")]]
    interface Factory {
        Swapchain*			newSwapchain(void* handle, void* reserved);
        int                 numDevices();
        Device*             getDevice(uint32 id);
    };
}