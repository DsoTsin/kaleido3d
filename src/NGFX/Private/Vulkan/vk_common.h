/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#if _WIN32
#define VK_KHR_win32_surface 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif

#include <stdint.h>
#include <ngfx.h>
#include "volk.h"
#define VULKAN_HPP_NO_EXCEPTIONS 1
//#include "vulkan/vulkan.hpp"
#include "CoreMinimal.h"
#define check(x) assert(x)

#define USE_CUSTOM_ALLOCATOR 0
#if USE_CUSTOM_ALLOCATOR
#define NGFXVK_ALLOCATOR &gAllocationCallbacks
#else
#define NGFXVK_ALLOCATOR  nullptr
#endif

#define VK_FN_STRNAME(n)		"vk" #n

#define VK_PROTO_FN(name)		PFN_vk##name __##name
#define VK_PROTO_FN_ZERO(name)	__##name = NULL;
#define VK_CALL(name, ...)		__##name(__VAR_ARGS__)
#define VK_FN_RSV(name)			__##name = (PFN_vk##name)soloader_.ResolveSymbol(VK_FN_STRNAME(name))

#define VK_INST_FN_RSV(name)	__##name = (PFN_vk##name)__GetInstanceProcAddr(instance_, VK_FN_STRNAME(name))

#if _MSC_VER
#define NGFX_EXPORT __declspec(dllexport)
#else 
#define NGFX_EXPORT __attribute__((visibility("default"))) 
#endif

typedef void(*ngfx_LogCallback)(int level, const char* msg);
namespace vulkan {
	class GpuDevice;
	class GpuBuffer;
	class GpuTexture;
	class GpuDrawable;

	template <class T>
	class iptr
	{
	public:
		iptr(T * pObj) : ptr_(pObj) {}
		iptr(iptr<T> const& Other) : ptr_(Other.ptr_) {
			if (ptr_) ptr_->retain_internal();
		}
		iptr() : ptr_(nullptr) {}
		~iptr() {
			if (ptr_) {
				ptr_->release_internal();
			    ptr_ = nullptr;
			}
		}
		T& operator*() const { return *ptr_; }
		T* operator->() const { return ptr_; }
		explicit operator bool() const { return ptr_ != nullptr; }
		void swap(iptr& Other) {
			T * const pValue = Other.ptr_;
			Other.ptr_ = ptr_;
			ptr_ = pValue;
		}
        iptr& operator=(const iptr& Other) {
			typedef iptr<T> ThisType;
			if (&Other != this) {
				ThisType(Other).swap(*this);
			}
			return *this;
		}
		T* get() const { return ptr_; }
		T** getAddressOf() { return &ptr_;}
	private:
		T* ptr_;
	};

    class Allocator
    {
    public:
        static void*   alloc(void* usr, size_t size, size_t align, VkSystemAllocationScope scope);
        static void*   realloc(void* usr, void* origin, size_t size, size_t align, VkSystemAllocationScope scope);
        static void    free(void* usr, void* ptr);

        static void    notifyAlloc(void* usr, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);
        static void    notifyFree(void* usr, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);

    public:
        Allocator();
        ~Allocator();

        void*   alloc(size_t size, size_t align, VkSystemAllocationScope scope);
        void*   realloc(void* origin, size_t size, size_t align, VkSystemAllocationScope scope);
        void    free(void* ptr);

        void    notifyAlloc(size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);
        void    notifyFree(size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);

    private:
    };

    class GpuAllocator
    {
        using MemProps = VkPhysicalDeviceMemoryProperties;
    public:
		struct MemoryItem
		{
			VkDeviceMemory mem;
			VkDeviceSize offset;
		};

        GpuAllocator(GpuDevice* device);
        ~GpuAllocator();

		uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, bool& memTypeFound);

		ngfx::Result allocateForBuffer(VkBuffer buffer, ngfx::StorageMode mode, MemoryItem& memItem);
		ngfx::Result allocateForImage(VkImage image, ngfx::StorageMode mode, MemoryItem& memItem);
		/*
		 * also need to know scratch buffer size 
		 */
		ngfx::Result allocateForAccelerationStructure(VkAccelerationStructureNV accelerationStructure, ngfx::StorageMode mode, MemoryItem& memItem);

		void freeBuffer(VkBuffer buffer, MemoryItem const& item);
		void freeImage(VkImage buffer, MemoryItem const& item);
		void freeAccelerationStructure(VkAccelerationStructureNV accelerationStructure, MemoryItem const& item);

		friend class GpuDevice;
    private:
		void init();

        GpuDevice*                  device_;
		MemProps                    device_mem_props_ = {};
    };

    extern Allocator                gAllocator;
    extern VkAllocationCallbacks    gAllocationCallbacks;

    struct QueueInfo
    {
        int32_t queueFamilyIndex;
        VkQueue queue;
    };

    struct QueuesInfo
    {
        QueueInfo graphics;
        QueueInfo compute;
        QueueInfo transfer;
		int32_t presentQueueFamilyIndex;
    };

	class ExtensionProps : public ngfx::Vec<VkExtensionProperties>
	{
	public:
		bool hasExtension(const char* extName) const;
	};

	class LayerProps : public ngfx::Vec<VkLayerProperties>
	{
	public:
		bool hasLayer(const char* layerName) const;
	};

	class GpuFactory : public ngfx::Factory
	{
		using LayerMap = ngfx::HashMap<std::string, VkLayerProperties>;
		using ExtensionMap = ngfx::HashMap<std::string, VkExtensionProperties>;
	public:
        GpuFactory(VkInstance instance, bool enable_debug, ngfx_LogCallback log_call);
        ~GpuFactory() override;
		void initDevices();

		void                        enumerateExtensions();

		void						getDeviceProps(VkPhysicalDevice device,
										VkPhysicalDeviceProperties& deviceProps,
										VkPhysicalDeviceFeatures& deviceFeatures,
										ngfx::Vec<VkQueueFamilyProperties>& queueProps);
		void						getDeviceExtensions(VkPhysicalDevice device, ngfx::Vec<VkExtensionProperties>& extProps);
		void						getDeviceLayers(VkPhysicalDevice device, ngfx::Vec<VkLayerProperties>& layerProps);

		bool                        hasLayer(std::string const& layer_name) const;
		bool                        hasExtension(std::string const& extension_name) const;

		void						checkNonUniformIndexing(VkPhysicalDevice device, bool& nonUniformIndex);
		void						checkNVRaytracing(VkPhysicalDevice device, bool& nvRaytracing);

        int                         numDevices() override;
        ngfx::Device *              getDevice(ngfx::uint32 id) override;
		ngfx::PresentLayer *		newPresentLayer(const ngfx::PresentLayerDesc* desc, 
										ngfx::Device* device, ngfx::PresentLayer* old, ngfx::Result* result) override;
        bool                        debuggable() const;

		VkBool32					report(
										VkDebugReportFlagsEXT                       flags,
										VkDebugReportObjectTypeEXT                  objectType,
										uint64_t                                    object,
										size_t                                      location,
										int32_t                                     messageCode,
										const char*                                 pLayerPrefix,
										const char*                                 pMessage);
		void						printLogStr(int level, const char* msg);
	
		static VkBool32				debugReport(
										VkDebugReportFlagsEXT                       flags,
										VkDebugReportObjectTypeEXT                  objectType,
										uint64_t                                    object,
										size_t                                      location,
										int32_t                                     messageCode,
										const char*                                 pLayerPrefix,
										const char*                                 pMessage,
										void*                                       pUserData);

		friend class GpuDevice;
	public:
		void destroySurface(VkSurfaceKHR surface);

	private:
		void enumPhysicalDevices();
		void loadInstanceFunctions();
		void resolveInstanceFunctions();

	private:
		k3d::os::LibraryLoader		soloader_;
		LayerMap                    layer_props_;
		ExtensionMap                ext_props_;
		VkInstance					instance_;
		VkDebugReportCallbackEXT	debug_report_callback_;
        bool                        debug_enable_;
        ngfx_LogCallback            log_call_;
        ngfx::Vec<iptr<GpuDevice>>  devices_;
		ngfx::Vec<VkPhysicalDevice> physical_devices_;

	private: // instance procs here
#include "instance_procs.inl"
	};

	// check video decode/encode support, raytracing support, bindless support
	class GpuDevice : public ngfx::Device/*, public VolkDeviceTable*/
	{
        using RtProps = VkPhysicalDeviceRayTracingPropertiesNV;
        using HWProps = VkPhysicalDeviceProperties;
        using HWFeatures = VkPhysicalDeviceFeatures;
        using QueueProps = ngfx::Vec<VkQueueFamilyProperties>;
        using ExtensionMap = ngfx::HashMap<std::string, VkExtensionProperties>;

	public:
		GpuDevice(VkPhysicalDevice device, GpuFactory* factory);
		~GpuDevice() override;

        void                        setLabel(const char * label) override;
        const char*                 label() const override;
        ngfx::DeviceType            getType() const override;
		// Compute, Graphics, Transfer (Async Resource Upload via vkCmdFillBuffer), Sparse Bind, Adreno GPU only supports Gfx & Compute
		ngfx::CommandQueue *		newQueue(ngfx::Result * result) override;
		ngfx::Shader *				newShader() override;
		ngfx::Renderpass *			newRenderpass(const ngfx::RenderpassDesc * desc, ngfx::Result * result) override;
		ngfx::ComputePipeline *		newComputePipeline(const ngfx::ComputePipelineDesc * desc, ngfx::Result * result) override;
		ngfx::RaytracePipeline *	newRaytracePipeline(const ngfx::RaytracePipelineDesc * desc, ngfx::Result * result) override;
		ngfx::Texture *				newTexture(const ngfx::TextureDesc * desc, ngfx::StorageMode mode, ngfx::Result * result) override;
		ngfx::Buffer *				newBuffer(const ngfx::BufferDesc * desc, ngfx::StorageMode mode, ngfx::Result * result) override;
		ngfx::RaytracingAS *		newRaytracingAS(const ngfx::RaytracingASDesc * rtDesc, ngfx::Result * result) override;
		ngfx::Sampler *				newSampler(const ngfx::SamplerDesc * desc, ngfx::Result * result) override;
		ngfx::Fence *				newFence(ngfx::Result * result) override;
		ngfx::Result				wait() override;

        bool                        isValid() const { return device_ != VK_NULL_HANDLE; }

		GpuAllocator&				getAllocator();

        friend class                GpuCommandBuffer;
        friend class                GpuFactory;
		friend class                GpuPresentLayer;

    public:
		void						getPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties& memProps);

		VkResult					createSwapchain(const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain);
		void						destroySwapchain(VkSwapchainKHR swapchain);
		VkResult					getSwapchainImages(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
		VkResult					acquireNextImage(VkSwapchainKHR swapchain, uint64_t timeOut, VkSemaphore semaphore, VkFence fence, uint32_t* imageIndex);

		VkResult					allocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory);
		void						freeMemory(VkDeviceMemory memory);

		void*						mapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags);
		void						flushMappedMemory(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
		void						invalidMappedMemory(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
		void						unmapMemory(VkDeviceMemory mem);

		VkResult					createBuffer(const VkBufferCreateInfo* pCreateInfo, VkBuffer* pBuffer);
		void						destroyBuffer(VkBuffer buffer);
		void						getBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);

		VkResult					createImage(const VkImageCreateInfo* pCreateInfo, VkImage* pImage);
		void						destroyImage(VkImage image);
		void						getImageMemoryRequirements(VkImage image, VkMemoryRequirements* pMemoryRequirements);
		
		VkResult					createSampler(const VkSamplerCreateInfo* pCreateInfo, VkSampler* pSampler);
		void						destroySampler(VkSampler sampler);

		// VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV
		// VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV
		// VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV
		void						getAccelerationStructureMemoryRequirements(VkAccelerationStructureNV accelerationStructure, 
										VkAccelerationStructureMemoryRequirementsTypeNV type, 
										VkMemoryRequirements2KHR* pMemoryRequirements);
        VkResult                    createAccelerationStructure(const VkAccelerationStructureCreateInfoNV* pCreateInfo, 
                                        const VkAllocationCallbacks* pAllocator, 
                                        VkAccelerationStructureNV* pAccelerationStructure);
        void                        destroyAccelerationStructure(VkAccelerationStructureNV accelerationStructure,
                                        const VkAllocationCallbacks * pAllocator);
        VkMemoryRequirements        getAccelerationStructureMemorySize(VkAccelerationStructureNV accel);

        VkResult                    createFence(const VkFenceCreateInfo& info, VkFence* pFence);
		void                        destroyFence(VkFence fence);

		VkResult					createSemaphore(const VkSemaphoreCreateInfo& info, VkSemaphore* pSemaphore);
		void						destroySemaphore(VkSemaphore semaphore);

		VkResult					createCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, VkCommandPool* pCommandPool);
		void						destroyCommandPool(VkCommandPool commandPool);
		VkResult					resetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags flags);
		VkResult					allocateCommandBuffer(const VkCommandBufferAllocateInfo* info, VkCommandBuffer* cmds);
		void						freeCommandBuffer(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);

		VkResult					queueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);

        struct SurfaceInfo
        {
            uint32_t				present_family_index;
            uint32_t				min_images;
            uint32_t				max_images;
            ngfx::Vec<VkPresentModeKHR> present_modes;
            ngfx::Vec<VkSurfaceFormatKHR> formats;
        };

        bool                        querySurfaceInfo(VkSurfaceKHR surface, SurfaceInfo& info);

		int							getQueueFamilyIndex(QueueProps& queueProps, VkQueueFlags queueFlag);

	private:
        void                        createDevice();
		void						loadDeviceFunctions();

        VkPhysicalDevice            physical_device_    = VK_NULL_HANDLE;
		VkDevice					device_             = VK_NULL_HANDLE;

		QueuesInfo					queues_info_		= {};

        ngfx::DeviceType            device_type_        = ngfx::DeviceType::Integrate;
        bool                        is_mobile_gpu_      = false;
        bool                        support_raytracing_ = false;
        RtProps                     ray_tracing_props_  = {};
		GpuAllocator				mem_alloc_;

		iptr<GpuFactory>            factory_;

	private: // device procs table
#include "device_procs.inl"
	};

    class GpuPresentLayer : public ngfx::PresentLayer {
    public:
		GpuPresentLayer(GpuDevice* device, VkSurfaceKHR surface, VkSwapchainKHR swapchain);
        ~GpuPresentLayer() override;

		void						getDesc(ngfx::PresentLayerDesc* desc) const override;
		ngfx::Device *				device() override;
		ngfx::Drawable *			nextDrawable() override;

    private:
		VkSurfaceKHR				surface_;
        VkSwapchainKHR              swapchain_;
        iptr<GpuDevice>             device_;
    };

	class GpuDrawable : public ngfx::Drawable
	{
	public:
		GpuDrawable(GpuPresentLayer* layer);
		~GpuDrawable() override;

		int					drawableId() const override;
		ngfx::Texture*		texture() override;
		ngfx::PresentLayer* layer() override;
		void				present() override;

	private:
		iptr<GpuPresentLayer> layer_;
		int image_id_;
	};

	class GpuCommandBuffer;

    class GpuQueue : public ngfx::CommandQueue
	{
	public:
        GpuQueue(VkQueue queue, GpuDevice* device);
        ~GpuQueue() override;

		ngfx::CommandBuffer*        newCommandBuffer() override;
		void						submit(GpuCommandBuffer * cmdBuf);

    private:
        VkQueue                     queue_;
        iptr<GpuDevice>             device_;
	};

    class GpuFence : public ngfx::Fence {
    public:
        GpuFence(GpuDevice* device);
        ~GpuFence() override;

        void                        setLabel(const char * label) override;
        const char*                 label() const override;
    private:
        VkFence                     fence_;
        iptr<GpuDevice>             device_;
    };

	class GpuCommandBuffer : public ngfx::CommandBuffer
	{
	public:
        GpuCommandBuffer(GpuQueue* queue);
        ~GpuCommandBuffer() override;

		void                        setLabel(const char* label) override;
		const char*					label() const override;

        ngfx::RenderEncoder *       newRenderEncoder(ngfx::Result * result) override;
        ngfx::ComputeEncoder *      newComputeEncoder(ngfx::Result * result) override;
        ngfx::BlitEncoder *         newBlitEncoder(ngfx::Result* result) override;
        ngfx::ParallelEncoder *     newParallelRenderEncoder(ngfx::Result* result) override;
        ngfx::RaytraceEncoder *     newRaytraceEncoder(ngfx::Result * result) override;
        ngfx::Result                commit() override;
    private:
		class CommandEncoder*		encoder_context_;

        VkCommandBuffer             command_buffer_;
        iptr<GpuQueue>              queue_;
	};

    // Raytracing Commands support
    class GpuRaytracingEncoder : public ngfx::RaytraceEncoder
    {
    public:

        void buildAS(ngfx::RaytracingAS * src, ngfx::RaytracingAS * dest, ngfx::Buffer * scratch) override;
        void copyAS(ngfx::RaytracingAS * src, ngfx::RaytracingAS * dest, ngfx::AccelerationStructureCopyMode mode) override;
        void traceRay(ngfx::Buffer * rayGen, ngfx::BufferStride miss, ngfx::BufferStride hit, int width, int height) override;
    private:
        iptr<GpuCommandBuffer>      command_;
    };

    class ComputeEncoder final : public ngfx::ComputeEncoder
    {
    public:
		ComputeEncoder(class CommandContext& context);
		~ComputeEncoder() override;

		void						setPipeline(ngfx::Pipeline* pipeline) override;
		void						setBindGroup(const ngfx::BindGroup* bindGroup) override;
		void						dispatch(int thread_group_x, int thread_group_y, int thread_group_z) override;
		void						endEncode() override;

	private:
		CommandContext&				context_;
		iptr<GpuCommandBuffer>      command_;
    };

	class ParallelRenderEncoder final : public ngfx::ParallelEncoder 
	{
    public:

		ngfx::RenderEncoder* subRenderEncoder(ngfx::Result * result) override;

    private:
        iptr<GpuCommandBuffer> command_;
	};

    class RenderEncoder final : public ngfx::RenderEncoder
    {
    public:
		RenderEncoder(CommandContext& context);
		~RenderEncoder() override;

		void						setPipeline(ngfx::Pipeline* pipeline) override;
		void						setBindGroup(const ngfx::BindGroup* bindGroup) override;

		void						drawPrimitives(ngfx::PrimitiveType primType, 
										int vertexStart, int vertexCount, int instanceCount, int baseInstance) override;
		void						drawIndexedPrimitives(
										ngfx::PrimitiveType primType, ngfx::IndexType indexType, int indexCount, 
										const ngfx::Buffer* indexBuffer, int indexBufferOffset,
										int vertexStart, int vertexCount, int instanceCount, int baseInstance) override;
		void						drawIndirect(ngfx::PrimitiveType primType, 
										const ngfx::Buffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) override;
		void						endEncode() override;

	private:
		CommandContext&				context_;
		iptr<GpuCommandBuffer>      command_;
    };

    class GpuPipelineBase : public ngfx::Pipeline {
    public:
        GpuPipelineBase(GpuDevice* device);
        virtual ~GpuPipelineBase() override;

    protected:
        VkPipeline                  pipeline_       = VK_NULL_HANDLE;
        VkPipelineCache             pipeline_cache_ = VK_NULL_HANDLE;
        iptr<GpuDevice>             device_;
    };

    class GpuRaytracingPipeline : public GpuPipelineBase {
        using CreateInfo = VkRayTracingPipelineCreateInfoNV;
    public:
        GpuRaytracingPipeline(const ngfx::RaytracePipelineDesc* desc, GpuDevice* device);
        ~GpuRaytracingPipeline() override;

    private:
        CreateInfo                  create_info_    = {};
        ngfx::RaytracePipelineDesc  desc_           = {};
    };

    class GpuRenderPipeline : public GpuPipelineBase {
        using CreateInfo = VkGraphicsPipelineCreateInfo;
    public:
		GpuRenderPipeline(GpuDevice* device, ngfx::RenderPipelineDesc const& desc);
		~GpuRenderPipeline() final override;

    private:
        CreateInfo                  create_info_    = {};
        ngfx::RenderPipelineDesc    desc_           = {};
		bool						lazy_create_	= false;
    };

    class GpuComputePipeline : public GpuPipelineBase {
        using CreateInfo = VkComputePipelineCreateInfo;
	public:
		GpuComputePipeline(GpuDevice* device, ngfx::ComputePipelineDesc const& desc);
		~GpuComputePipeline() final override;

    private:
        CreateInfo                  create_info_    = {};
		ngfx::ComputePipelineDesc   desc_			= {};
		bool						lazy_create_	= false;
    };

    class GpuBuffer : public ngfx::Buffer
    {
    public:
        GpuBuffer(const ngfx::BufferDesc& desc, VkBuffer buffer, GpuAllocator::MemoryItem inMemItem, GpuDevice* device);
        ~GpuBuffer() override;
        ngfx::BufferView*           newView(ngfx::Result * result) override;
        bool                        isValid() const { return buffer_ != VK_NULL_HANDLE; }
		// ~ resource interface
		void*						map(ngfx::uint64 offset, ngfx::uint64 size) override;
		void						unmap(void * addr) override;
		// ~ end resource interface

		// ~ gfx object interface
		void						setLabel(const char* label) override;
		const char*					label() const override;
		// ~ end gfx object interface
    private:
        VkBuffer                    buffer_ = VK_NULL_HANDLE;
        VkBufferCreateInfo          create_info_;
        ngfx::BufferDesc            desc_;
		GpuAllocator::MemoryItem	mem_item_ = {};
		//<list of resource views implicit & explicit>
        iptr<GpuDevice>             device_;
    };

    class GpuTexture : public ngfx::Texture
    {
    public:
        GpuTexture(const ngfx::TextureDesc& desc, VkImage image, GpuAllocator::MemoryItem inMemItem, GpuDevice* device);
        ~GpuTexture() override;

		ngfx::PixelFormat			format() const override;
        ngfx::TextureView*          newView(ngfx::Result * result) override;
        bool                        isValid() const { return texture_ != VK_NULL_HANDLE; }

		// ~ resource interface
		void*						map(ngfx::uint64 offset, ngfx::uint64 size) override;
		void						unmap(void * addr) override;
		// ~ end resource interface

		// ~ gfx object interface
		void						setLabel(const char* label) override;
		const char*					label() const override;
		// ~ end gfx object interface

    private:
        VkImage                     texture_        = VK_NULL_HANDLE;
        VkImageCreateInfo           create_info_    = {};
        ngfx::TextureDesc           desc_           = {};
		GpuAllocator::MemoryItem	mem_item_		= {};
        iptr<GpuDevice>             device_;
    };

    class GpuSampler : public ngfx::Sampler
    {
    public:
        GpuSampler(const ngfx::SamplerDesc& desc, GpuDevice* device);
        ~GpuSampler() override;
        bool                        isValid() const { return sampler_ != VK_NULL_HANDLE; }
        void                        setLabel(const char* label);
        const char*                 label() const override;
    private:
        VkSampler                   sampler_        = VK_NULL_HANDLE;
        VkSamplerCreateInfo         create_info_    = {};
        ngfx::SamplerDesc           desc_           = {};
        iptr<GpuDevice>             device_;
    };

    class GpuRaytracingAccelerationStructure : public ngfx::RaytracingAS
    {
        using CreateInfo = VkAccelerationStructureCreateInfoNV;
    public:
        GpuRaytracingAccelerationStructure(
			const ngfx::RaytracingASDesc& desc,
			VkAccelerationStructureNV inAs, 
			GpuAllocator::MemoryItem const& memItem,
			GpuDevice* device);
        ~GpuRaytracingAccelerationStructure() override;

        bool                        isValid() const { return acceleration_structure_ != VK_NULL_HANDLE; }
        void                        setLabel(const char* label);
        const char*                 label() const override;

    private:
        VkAccelerationStructureNV   acceleration_structure_ = VK_NULL_HANDLE;
		GpuAllocator::MemoryItem	mem_item_				= {};
        ngfx::RaytracingASDesc      desc_                   = {};
        CreateInfo                  create_info_            = {};
        iptr<GpuDevice>             device_;
    };
}

#include "vk_instance.inl"
#include "vk_device.inl"

extern "C" 
{
    NGFX_EXPORT ngfx::Factory* CreateFactory(bool debug_layer_enable, ngfx_LogCallback log_call);
}