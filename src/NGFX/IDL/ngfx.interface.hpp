#include "ngfx.structs.hpp"

namespace ngfx {
	interface Device;
    [[refcount("true")]]
    interface LabeledObject
    {
        void                setLabel(const char* label);
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
	
    struct ClearColor {
        float red;
        float green;
        float blue;
        float alpha;
    };

    interface Texture;
    struct RenderpassAttachmentDesc {
        Texture* texture;
        uint32 level;
        uint32 slice;
        uint32 depth;
        uint32 resolveLevel;
        uint32 resolveSlcie;
        uint32 resolveDepthPlane;
        LoadAction loadAction;
        StoreAction storeAction;
    };

    struct RenderpassColorAttachmentDesc : RenderpassAttachmentDesc {
        ClearColor clearColor;
    };

    struct RenderpassDepthAttachmentDesc : RenderpassAttachmentDesc {
        double clearDepth;
    };

    struct RenderpassStencilAttachmentDesc : RenderpassAttachmentDesc {
        uint32 clearStencil;
    };

    struct RenderpassDesc {
        array<RenderpassColorAttachmentDesc, 8> colorAttachments;
        RenderpassDepthAttachmentDesc depthAttachment;
        RenderpassStencilAttachmentDesc stencilAttachment;
        uint32 renderTargetArrayLenth;
        uint32 renderTargetWidth;
        uint32 renderTargetHeight;
	};

    interface Resource : LabeledObject {
		void*				map(uint64 offset, uint64 size);
		void				unmap(void* addr);
    };
	interface RaytracingAS : LabeledObject {

	};
	interface TextureView;
	[[vk("VkImage"), mtl("id<MTLTexture>")]]
    interface Texture : Resource {
		PixelFormat			format() const;
		TextureView*		newView(Result* result);
    };
	interface TextureView {
		const Texture*		texture() const;
		TextureUsage		usage() const;
	};
	interface BufferView;
	[[vk("VkBuffer"), mtl("id<MTLBuffer>")]]
    interface Buffer : Resource {
		BufferView*			newView(Result* result);
    };
	interface BufferView {
		const Buffer*		buffer() const;
		BufferUsage			usage() const;
	};
	[[vk("VkSampler"), mtl("id<MTLSampler>")]]
    interface Sampler : LabeledObject {

    };
    interface Shader : LabeledObject {

    };
	[[vk("VkFramebuffer"),refcount("true")]]
    interface Framebuffer : LabeledObject {

    };

	interface Drawable;

	[[vk("VkSwapchain"), refcount("true")]]
	interface PresentLayer
	{
		void				getDesc(PresentLayerDesc* desc) const;
		Device*				device();
		Drawable*			nextDrawable();
	};

	[[vk("VkSwapchain"), refcount("true")]]
	interface Drawable
	{
		int					drawableId() const;
		Texture*			texture();
		PresentLayer*		layer();
		void				present();
	};

	[[transient("frame"), refcount("true")]]
	interface BindGroup {
		void				setSampler(uint32 id, ShaderStage stage, const Sampler* sampler);
		/* void				setSamplers(uint32 id, ShaderStage stage, const array<Texture*>* sampler); */
		void				setTexture(uint32 id, ShaderStage stage, const TextureView* texture);
		/* void				setTextures(uint32 id, ShaderStage stage, const array<Texture*>* texture); */
		void				setBuffer(uint32 id, ShaderStage stage, const BufferView* buffer);
		void				setRaytracingAS(uint32 id, ShaderStage stage, const RaytracingAS* as);
	};
    interface Pipeline : LabeledObject {
		BindGroup*			newBindGroup(Result* result);
		Device*				device();
    };
    interface RenderPipeline : Pipeline {

    };
	
    interface ComputePipeline : Pipeline {

    };

    interface RaytracePipeline : Pipeline {
        
    };
	interface RenderEncoder;
	interface ComputeEncoder;
	interface RaytraceEncoder;
    interface ParallelEncoder;
    interface BlitEncoder;
	
    [[vk("VkCommandBuffer"), mtl("id<MTLCommandBuffer>")]]
    interface CommandBuffer : LabeledObject {
        RenderEncoder*      newRenderEncoder(const RenderpassDesc* desc, Result* result);
        ParallelEncoder*    newParallelRenderEncoder(const RenderpassDesc* desc, Result* result);
        ComputeEncoder*		newComputeEncoder(Result* result);
        BlitEncoder*		newBlitEncoder(Result* result);
        RaytraceEncoder*	newRaytraceEncoder(Result* result);
        Result				commit();
    };

	[[refcount("true"), vk("VkQueue"), mtl("id<MTLQueue>")]]
    interface CommandQueue {
		CommandBuffer*		newCommandBuffer() [[transient("true")]];
    };
    interface CommandEncoder : LabeledObject{
		void				setPipeline(Pipeline* pipeline);
		void				setBindGroup(const BindGroup* bindGroup);
        void				endEncode();
    };
    interface RenderEncoder : CommandEncoder {
        void                setViewport(Viewport viewport);
		void                setViewports(int numViewports, const Viewport* pViewport);
        void                setScissors(int numScirssors, const Rect* pRects);
		void				setStencilRef(uint32 referenceValue);
        void                setDepthStencilState();
        void                setDepthClipMode(DepthClipMode clipMode);
		void				setDepthBias(float depthBias, float slopeScale, float clamp);
        void				drawPrimitives(PrimitiveType primType, 
								int vertexStart, int vertexCount, int instanceCount, int baseInstance);
        void                drawIndexedPrimitives(PrimitiveType primType,
								IndexType indexType, int indexCount, const Buffer* indexBuffer, int indexBufferOffset,
								int vertexStart, int vertexCount, int instanceCount, int baseInstance);
		void				drawIndirect(PrimitiveType primType, 
								const Buffer* buffer, uint64 offset, uint32 drawCount, uint32 stride);
        void				present(Drawable* drawable);
    };

    interface ComputeEncoder : CommandEncoder {
        void				dispatch(int x, int y, int z);
    };
    
    interface ParallelEncoder : CommandEncoder {
        void                setColorStoreAction(StoreAction storeAction, int index);
        void                setDepthStoreAction(StoreAction storeAction);
        void                setStencilAction(StoreAction storeAction);
        RenderEncoder*      subRenderEncoder(Result* result);
    };
    
    struct BufferStride {
        Buffer*     buffer;
        uint32      stride;
    };
    struct RaytracingAABBs
    {
        uint32 count;
        BufferStride aabbs;
    };
    
    struct RaytracingTriangles
    {
        uint32          vertexCount;
        BufferStride    vertices;
        VertexFormat    vertexFormat;
        uint32          indexCount;
        Buffer*         indices;
        IndexType       indexType;
        Buffer*         transforms;
    };
    
    struct RaytracingGeometryData
    {
        RaytracingAABBs aabbs;
        RaytracingTriangles triangles;
    };

    struct RaytracingGeometryDesc
    {
        RaytracingGeometryType type;
        RaytracingGeometryFlags flag;
        RaytracingGeometryData data;
    };

    struct RaytracingASDesc {
        AccelerationStructureType type;
        AccelerationStructureBuildFlag flag;
        uint32 instanceCount;
        uint32 geometryCount;
        const RaytracingGeometryDesc* pGeometries;
    };

    interface RaytraceEncoder : CommandEncoder {
		void				buildAS(RaytracingAS* src, RaytracingAS* dest, Buffer* scratch);
        void                copyAS(RaytracingAS* src, RaytracingAS* dest, AccelerationStructureCopyMode mode);
        void				traceRay(Buffer* rayGen, BufferStride miss, BufferStride hit, int width, int height);
    };
	[[vk("VkFence"), mtl("id<MTLFence>")]]
    interface Fence : LabeledObject {
    };
	[[vk("VkDevice"), mtl("id<MTLDevice>")]]
    interface Device : LabeledObject {
        DeviceType          getType() const;
		CommandQueue*		newQueue(Result* result);
        Shader*				newShader();
        RenderPipeline* newRenderPipeline(const RenderPipelineDesc* desc, Result* result) [[gen_rc("true")]];
		ComputePipeline*	newComputePipeline(const ComputePipelineDesc* desc, Result* result)[[gen_rc("true")]];
		RaytracePipeline*	newRaytracePipeline(const RaytracePipelineDesc* desc, Result* result)[[gen_rc("true")]];
		Texture*			newTexture(const TextureDesc* desc, StorageMode mode, Result* result)[[gen_rc("true")]];
        Buffer*				newBuffer(const BufferDesc* desc, StorageMode mode, Result* result)[[gen_rc("true")]];
		RaytracingAS*		newRaytracingAS(const RaytracingASDesc* rtDesc, Result* result)[[gen_rc("true")]];
		Sampler*			newSampler(const SamplerDesc* desc, Result* result)[[gen_rc("true")]];
        Fence*				newFence(Result* result);
        Result				wait();
    };
    [[refcount("true")]]
    interface Factory {
        int                 numDevices();
        Device*             getDevice(uint32 id);
		PresentLayer*		newPresentLayer(const PresentLayerDesc* desc, 
                                Device* device, PresentLayer* old, 
                                Result* result);
    };
}