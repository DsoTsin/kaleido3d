#include <Kaleido3D.h>
#include <KTL/Allocator.hpp>
#if K3DPLATFORM_OS_WIN
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif K3DPLATFORM_OS_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif
#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "ngfx.h"
#include <vector>

using namespace ngfx;

#define VULKAN_ALLOCATOR nullptr

VkFormat ConvertPixelFormatToVulkanEnum(PixelFormat const& e) {
  switch (e) {
  case PixelFormat::RGBA16Uint:
    return VK_FORMAT_R16G16B16A16_UINT;
  case PixelFormat::RGBA32Float:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case PixelFormat::RGBA8UNorm:
    return VK_FORMAT_R8G8B8A8_UNORM;
  case PixelFormat::RGBA8UNorm_sRGB:
    return VK_FORMAT_R8G8B8A8_SNORM;
  case PixelFormat::R11G11B10Float:
    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
  case PixelFormat::D32Float:
    return VK_FORMAT_D32_SFLOAT;
  case PixelFormat::RGB32Float:
    return VK_FORMAT_R32G32B32_SFLOAT;
  }
}
VkAttachmentLoadOp ConvertLoadActionToVulkanEnum(LoadAction const& e) {
  switch (e) {
  case LoadAction::Load:
    return VK_ATTACHMENT_LOAD_OP_LOAD;
  case LoadAction::Clear:
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  case LoadAction::DontCare:
    return VK_ATTACHMENT_LOAD_OP_CLEAR;
  }
}
VkAttachmentStoreOp ConvertStoreActionToVulkanEnum(StoreAction const& e) {
  switch (e) {
  case StoreAction::Store:
    return VK_ATTACHMENT_STORE_OP_STORE;
  case StoreAction::DontCare:
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
  }
}
VkPrimitiveTopology ConvertPrimitiveTypeToVulkanEnum(PrimitiveType const& e) {
  switch (e) {
  case PrimitiveType::Points:
    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  case PrimitiveType::Lines:
    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  case PrimitiveType::Triangles:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  case PrimitiveType::TriangleStrips:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  }
}
VkBlendOp ConvertBlendOperationToVulkanEnum(BlendOperation const& e) {
  switch (e) {
  case BlendOperation::Add:
    return VK_BLEND_OP_ADD;
  case BlendOperation::Sub:
    return VK_BLEND_OP_SUBTRACT;
  }
}
VkBlendFactor ConvertBlendTypeToVulkanEnum(BlendType const& e) {
  switch (e) {
  case BlendType::Zero:
    return VK_BLEND_FACTOR_ZERO;
  case BlendType::One:
    return VK_BLEND_FACTOR_ONE;
  case BlendType::SrcColor:
    return VK_BLEND_FACTOR_SRC_COLOR;
  case BlendType::DstColor:
    return VK_BLEND_FACTOR_DST_COLOR;
  case BlendType::SrcAlpha:
    return VK_BLEND_FACTOR_SRC_ALPHA;
  case BlendType::DstAlpha:
    return VK_BLEND_FACTOR_DST_ALPHA;
  }
}
VkStencilOp ConvertStencilOperationToVulkanEnum(StencilOperation const& e) {
  switch (e) {
  case StencilOperation::Keep:
    return VK_STENCIL_OP_KEEP;
  case StencilOperation::Zero:
    return VK_STENCIL_OP_ZERO;
  case StencilOperation::Replace:
    return VK_STENCIL_OP_REPLACE;
  case StencilOperation::Invert:
    return VK_STENCIL_OP_INVERT;
  case StencilOperation::Increment:
    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
  case StencilOperation::Decrement:
    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
  }
}
VkCompareOp ConvertComparisonFunctionToVulkanEnum(ComparisonFunction const& e) {
  switch (e) {
  case ComparisonFunction::Never:
    return VK_COMPARE_OP_NEVER;
  case ComparisonFunction::Less:
    return VK_COMPARE_OP_LESS;
  case ComparisonFunction::Equal:
    return VK_COMPARE_OP_EQUAL;
  case ComparisonFunction::LessEqual:
    return VK_COMPARE_OP_LESS_OR_EQUAL;
  case ComparisonFunction::Greater:
    return VK_COMPARE_OP_GREATER;
  case ComparisonFunction::NotEqual:
    return VK_COMPARE_OP_NOT_EQUAL;
  case ComparisonFunction::GreaterEqual:
    return VK_COMPARE_OP_GREATER_OR_EQUAL;
  case ComparisonFunction::Always:
    return VK_COMPARE_OP_ALWAYS;
  }
}
VkPolygonMode ConvertFillModeToVulkanEnum(FillMode const& e) {
  switch (e) {
  case FillMode::Wire:
    return VK_POLYGON_MODE_LINE;
  case FillMode::Solid:
    return VK_POLYGON_MODE_FILL;
  }
}
VkCullModeFlagBits ConvertCullModeToVulkanEnum(CullMode const& e) {
  switch (e) {
  case CullMode::None:
    return VK_CULL_MODE_NONE;
  case CullMode::Front:
    return VK_CULL_MODE_FRONT_BIT;
  case CullMode::Back:
    return VK_CULL_MODE_BACK_BIT;
  }
}
VkFilter ConvertFilterModeToVulkanEnum(FilterMode const& e) {
  switch (e) {
  case FilterMode::Point:
    return VK_FILTER_NEAREST;
  case FilterMode::Linear:
    return VK_FILTER_LINEAR;
  }
}
VkSamplerAddressMode ConvertAddressModeToVulkanEnum(AddressMode const& e) {
  switch (e) {
  case AddressMode::Wrap:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case AddressMode::Mirror:
    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case AddressMode::Clamp:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case AddressMode::Border:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  case AddressMode::MirrorOnce:
    return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
  }
}

class VulkanCommandBuffer : public CommandBuffer
{
protected:
  class VulkanQueue* OwningRoot;
private:
  VkCommandBuffer Handle = VK_NULL_HANDLE;
public:
  void Commit(Fence * pFence) override;
  struct RenderCommandEncoder * RenderCommandEncoder() override;
  struct ComputeCommandEncoder * ComputeCommandEncoder() override;
  struct ParallelRenderCommandEncoder * ParallelCommandEncoder() override;
  VulkanCommandBuffer(VulkanQueue* pQueue);
  ~VulkanCommandBuffer();
};

class VulkanQueue : public CommandQueue
{
public:
  VulkanQueue(class VulkanDevice* pDevice);
  ~VulkanQueue() override;
  struct CommandBuffer * CommandBuffer() override;
  VulkanDevice* OwningRoot;
private:
  VkQueue Handle = VK_NULL_HANDLE;
protected:
};

class VulkanRenderPass : public RenderPass
{
protected:
  VulkanDevice* OwningRoot;
private:
  VkRenderPass Handle = VK_NULL_HANDLE;
};

template <class T>
class TPipeline : public T
{
public:
  using Super = typename TPipeline<T>;

  TPipeline(VulkanDevice * pDevice) : OwningDevice(pDevice) {}
  virtual ~TPipeline() override;

protected:
  VulkanDevice* OwningDevice = nullptr;

  VkPipeline Handle = VK_NULL_HANDLE;
  VkPipelineCache Cache = VK_NULL_HANDLE;

private:
};

class VulkanRenderPipeline : public TPipeline<RenderPipeline>
{
public:
  VulkanRenderPipeline(VulkanDevice* pDevice, const RenderPipelineDesc* pDesc, RenderPass * pRenderPass, PipelineLayout * pLayout);
  ~VulkanRenderPipeline() override;
  Result GetDesc(RenderPipelineDesc * pDesc);
protected:
private:
};

class VulkanComputePipeline : public TPipeline<ComputePipeline>
{
public:
  VulkanComputePipeline(VulkanDevice* pDevice, Function* pComputeFunc, PipelineLayout * pLayout);
  ~VulkanComputePipeline();
};

void VulkanCommandBuffer::Commit(Fence * pFence)
{
}

RenderCommandEncoder * VulkanCommandBuffer::RenderCommandEncoder()
{
  return nullptr;
}

ComputeCommandEncoder * VulkanCommandBuffer::ComputeCommandEncoder()
{
  return nullptr;
}

ParallelRenderCommandEncoder * VulkanCommandBuffer::ParallelCommandEncoder()
{
  return nullptr;
}

VulkanCommandBuffer::VulkanCommandBuffer(VulkanQueue* pQueue)
  : OwningRoot(pQueue)
{
  OwningRoot->AddInternalRef();

}

VulkanCommandBuffer::~VulkanCommandBuffer()
{

  OwningRoot->ReleaseInternal();
}

class VulkanSampler : public Sampler
{
protected:
  VulkanDevice* OwningRoot;
private:
  VkSampler Handle = VK_NULL_HANDLE;
public:
  VulkanSampler(VulkanDevice* pDevice, const SamplerDesc* pDesc);
  ~VulkanSampler();
};

template<class RHIObj>
struct ResTrait
{
};

template<>
struct ResTrait<Buffer>
{
  typedef VkBuffer Obj;
  typedef VkBufferView View;
  typedef VkBufferUsageFlags UsageFlags;
  typedef VkBufferCreateInfo CreateInfo;
  typedef VkBufferViewCreateInfo ViewCreateInfo;
  typedef VkDescriptorBufferInfo DescriptorInfo;
  static decltype(vkCreateBufferView)* CreateView;
  static decltype(vkDestroyBufferView)* DestroyView;
  static decltype(vkCreateBuffer)* Create;
  static decltype(vmaCreateBuffer)* vmaCreate;
  static decltype(vkDestroyBuffer)* Destroy;
  static decltype(vmaDestroyBuffer)* vmaDestroy;
  static decltype(vkGetBufferMemoryRequirements)* GetMemoryInfo;
  static decltype(vkBindBufferMemory)* BindMemory;
};

template<>
struct ResTrait<Texture>
{
  typedef VkImage Obj;
  typedef VkImageView View;
  typedef VkImageUsageFlags UsageFlags;
  typedef VkImageCreateInfo CreateInfo;
  typedef VkImageViewCreateInfo ViewCreateInfo;
  typedef VkDescriptorImageInfo DescriptorInfo;
  static decltype(vkCreateImageView)* CreateView;
  static decltype(vkDestroyImageView)* DestroyView;
  static decltype(vkCreateImage)* Create;
  static decltype(vmaCreateImage)* vmaCreate;
  static decltype(vkDestroyImage)* Destroy;
  static decltype(vmaDestroyImage)* vmaDestroy;
  static decltype(vkGetImageMemoryRequirements)* GetMemoryInfo;
  static decltype(vkBindImageMemory)* BindMemory;
};

// Buffer functors
decltype(vkCreateBufferView)* ResTrait<Buffer>::CreateView =
&vkCreateBufferView;
decltype(vkDestroyBufferView)* ResTrait<Buffer>::DestroyView =
&vkDestroyBufferView;
decltype(vkCreateBuffer)* ResTrait<Buffer>::Create = &vkCreateBuffer;
decltype(vmaCreateBuffer)* ResTrait<Buffer>::vmaCreate = &vmaCreateBuffer;
decltype(vkDestroyBuffer)* ResTrait<Buffer>::Destroy = &vkDestroyBuffer;
decltype(vmaDestroyBuffer)* ResTrait<Buffer>::vmaDestroy = &vmaDestroyBuffer;
decltype(vkGetBufferMemoryRequirements)* ResTrait<Buffer>::GetMemoryInfo =
&vkGetBufferMemoryRequirements;
decltype(vkBindBufferMemory)* ResTrait<Buffer>::BindMemory =
&vkBindBufferMemory;
// Texture
decltype(vkCreateImageView)* ResTrait<Texture>::CreateView = &vkCreateImageView;
decltype(vkDestroyImageView)* ResTrait<Texture>::DestroyView =
&vkDestroyImageView;
decltype(vkCreateImage)* ResTrait<Texture>::Create = &vkCreateImage;
decltype(vmaCreateImage)* ResTrait<Texture>::vmaCreate = &vmaCreateImage;
decltype(vkDestroyImage)* ResTrait<Texture>::Destroy = &vkDestroyImage;
decltype(vmaDestroyImage)* ResTrait<Texture>::vmaDestroy = &vmaDestroyImage;
decltype(vkGetImageMemoryRequirements)* ResTrait<Texture>::GetMemoryInfo =
&vkGetImageMemoryRequirements;
decltype(vkBindImageMemory)* ResTrait<Texture>::BindMemory = &vkBindImageMemory;


template<class TRHIResObj>
class TResource : public TRHIResObj
{
  friend class CommandBufferImpl;
public:
  using TObj = typename ResTrait<TRHIResObj>::Obj;
  using ResInfo = typename ResTrait<TRHIResObj>::CreateInfo;
  using Super = TResource<TRHIResObj>;

  void * Map(uint64_t offset, uint64_t size) override;
  void UnMap() override;

  VulkanDevice* OwningDevice;

  TResource(VulkanDevice* Device) : OwningDevice(Device) {}
  virtual ~TResource();

protected:
  VkMappedMemoryRange MappedMemoryRange = {};
  
  ResInfo Info = {};
  TObj Handle = VK_NULL_HANDLE;
};

class VulkanBuffer : public TResource<Buffer>
{
public:
  VulkanBuffer(VulkanDevice * pDevice, const BufferDesc* pDesc);
  ~VulkanBuffer();
  Result GetDesc(BufferDesc * pDesc);
};

class VulkanTexture : public TResource<Texture>
{
public:
  VulkanTexture(VulkanDevice* pDevice, const TextureDesc* pDesc);
  ~VulkanTexture();
  Result GetDesc(TextureDesc * pDesc);
};

class VulkanPipelineLayout : public PipelineLayout
{
public:
  VulkanPipelineLayout(VulkanDevice* pDevice);
  ~VulkanPipelineLayout() override;
  Result CreateBindingTable(BindingTable ** ppBindingTable) override;
protected:
  VulkanDevice* OwningDevice = nullptr;

};

class VulkanDevice : public Device
{
public:
  VulkanDevice(class VulkanFactory* pFactory, VkPhysicalDevice PhysicalDevice);
  ~VulkanDevice() override;
  void GetDesc(DeviceDesc * pDesc) override;
  Result CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue) override;
  Result CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout) override;
  Result CreateBindingTable(PipelineLayout * pPipelineLayout, BindingTable ** ppBindingTable) override;
  Result CreateRenderPipeline(const RenderPipelineDesc * pPipelineDesc, PipelineLayout * pPipelineLayout, RenderPass * pRenderPass, Pipeline ** pPipelineState) override;
  Result CreateComputePipeline(Function * pComputeFunction, PipelineLayout * pPipelineLayout, Pipeline ** pPipeline) override;
  Result CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass) override;
  Result CreateRenderTarget(const RenderTargetDesc * desc, RenderTarget ** ppRenderTarget) override;
  Result CreateSampler(const SamplerDesc* desc, Sampler ** pSampler) override;
  Result CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer) override;
  Result CreateBufferView(const BufferViewDesc * desc, Buffer * pBuffer, BufferView ** pBufView) override;
  Result CreateTexture(const TextureDesc * desc, Texture ** pTexture) override;
  Result CreateTextureView(const TextureViewDesc * desc, Texture * pTexture, TextureView ** pTexView) override;
  void WaitIdle() override;

  VkDevice Handle = VK_NULL_HANDLE;

private:
  VkPhysicalDevice Device = VK_NULL_HANDLE;
  VmaAllocator MemoryAllocator = nullptr;
  std::vector<VkQueueFamilyProperties> QueueFamilyProps;
  int32_t GraphicsQueueFamilyId = 0;
  int32_t ComputeQueueFamilyId = 0;
  int32_t TransferQueueFamilyId = 0;
protected:
  VulkanFactory* OwningRoot;
  friend class VulkanFactory;
  friend class VulkanQueue;
  template<class T>
  friend class TResource;
};

VulkanQueue::VulkanQueue(VulkanDevice* pDevice)
  : OwningRoot(pDevice)
{
  OwningRoot->AddInternalRef();
  vkGetDeviceQueue(OwningRoot->Handle, 0, 0, &Handle);
}

VulkanQueue::~VulkanQueue()
{
  OwningRoot->ReleaseInternal();
}

CommandBuffer * VulkanQueue::CommandBuffer()
{
  
  return nullptr;
}

template<class TRHIResObj>
void * TResource<TRHIResObj>::Map(uint64_t offset, uint64_t size)
{
  MappedMemoryRange.offset = offset;
  MappedMemoryRange.size = size;
  void * pData = nullptr;
  vmaMapMemory(OwningDevice->MemoryAllocator, &MappedMemoryRange, &pData);
  return pData;
}

template<class TRHIResObj>
void TResource<TRHIResObj>::UnMap()
{
  vmaUnmapMemory(OwningDevice->MemoryAllocator, &MappedMemoryRange);
}

template<class TRHIResObj>
TResource<TRHIResObj>::~TResource()
{
  ResTrait<TRHIResObj>::Destroy(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
}

VulkanBuffer::VulkanBuffer(VulkanDevice * pDevice, const BufferDesc* pDesc)
  : Super(pDevice)
{

}

VulkanBuffer::~VulkanBuffer()
{

}

Result VulkanBuffer::GetDesc(BufferDesc * pDesc)
{
  return Result();
}

class VulkanCompiler : public Compiler
{
public:
  Result Compile(const ShaderOption * option, void * pData, uint32_t size, Function ** output) override;
  Result Reflect(void * pData, uint32_t size) override;
  VulkanCompiler();
  ~VulkanCompiler() override;
};

class VulkanSwapChain : public SwapChain
{
public:
  Result GetTexture(Texture ** ppTexture, uint32_t index) override;
  void Present() override;
  VulkanSwapChain(VulkanFactory* pFactory, void* pHandle, const SwapChainDesc* pDesc, VulkanQueue* pQueue);
  ~VulkanSwapChain() override;
private:
  VkSwapchainKHR Handle = VK_NULL_HANDLE;
  VkSurfaceKHR Surface = VK_NULL_HANDLE;
  VkSwapchainCreateInfoKHR CreateInfo;
protected:
  VulkanFactory* OwningRoot;
  VulkanDevice* OwningDevice;
};

class VulkanFactory : public Factory
{
public:
  Result EnumDevice(uint32_t * count, Device ** ppDevice);
  Result CreateSwapchain(const SwapChainDesc * desc, CommandQueue * pCommandQueue, void * pWindow, SwapChain ** pSwapchain);
  Result CreateCompiler(ShaderLang shaderLang, Compiler ** compiler);
  friend _declspec(dllexport) Factory* CreateFactory(const char * name)
  {
    return new VulkanFactory;
  }
  VulkanFactory()
  {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "test";
    appInfo.pEngineName = "test";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 1);
    appInfo.engineVersion = 1;
    appInfo.applicationVersion = 0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    std::vector<const char*> enabledExtensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    instanceCreateInfo.enabledExtensionCount = enabledExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    vkCreateInstance(&instanceCreateInfo, VULKAN_ALLOCATOR, &Handle);
  }
  ~VulkanFactory() override
  {
    vkDestroyInstance(Handle, VULKAN_ALLOCATOR);
  }
private:
  friend class VulkanSwapChain;
  VkInstance Handle = VK_NULL_HANDLE;
};

VulkanDevice::VulkanDevice(VulkanFactory* pFactory, VkPhysicalDevice PhysicalDevice)
: OwningRoot(pFactory)
, Device(PhysicalDevice)
{
  OwningRoot->AddInternalRef();

  uint32 queueCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueCount, NULL);
  QueueFamilyProps.resize(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueCount, QueueFamilyProps.data());

  uint32 qId = 0;
  for (qId = 0; qId < queueCount; qId++)
  {
    if (QueueFamilyProps[qId].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      GraphicsQueueFamilyId = qId;
      break;
    }
  }
  for (qId = 0; qId < queueCount; qId++)
  {
    if (QueueFamilyProps[qId].queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
      ComputeQueueFamilyId = qId;
      break;
    }
  }
  for (qId = 0; qId < queueCount; qId++)
  {
    if (QueueFamilyProps[qId].queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      TransferQueueFamilyId = qId;
      break;
    }
  }

  float QueuePriority = 0.0f;
  VkDeviceQueueCreateInfo QueueInfos[2];
  QueueInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[0].queueFamilyIndex = GraphicsQueueFamilyId;
  QueueInfos[0].queueCount = 1;
  QueueInfos[0].pQueuePriorities = &QueuePriority;
  QueueInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[1].queueFamilyIndex = ComputeQueueFamilyId;
  QueueInfos[1].queueCount = 1;
  QueueInfos[1].pQueuePriorities = &QueuePriority;

  std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = NULL;
  deviceCreateInfo.queueCreateInfoCount = 2;
  deviceCreateInfo.pQueueCreateInfos = QueueInfos;
  //deviceCreateInfo.pEnabledFeatures = &m_Features;

  //if (enableValidation)
  {
  //  deviceCreateInfo.enabledLayerCount = 1;
  //  deviceCreateInfo.ppEnabledLayerNames = g_ValidationLayerNames;
  }

  if (enabledExtensions.size() > 0)
  {
    deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
  }
  vkCreateDevice(Device, &deviceCreateInfo, VULKAN_ALLOCATOR, &Handle);

  VmaAllocatorCreateInfo AllocCreateInfo = {
    Device, Handle, 0, 0,
    VULKAN_ALLOCATOR
  };
  vmaCreateAllocator(&AllocCreateInfo, &MemoryAllocator);
}

VulkanDevice::~VulkanDevice()
{
  vmaDestroyAllocator(MemoryAllocator);
  vkDestroyDevice(Handle, VULKAN_ALLOCATOR);
  OwningRoot->ReleaseInternal();
}

void VulkanDevice::GetDesc(DeviceDesc * pDesc)
{
}

Result VulkanDevice::CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue)
{
  
  return Result();
}

Result VulkanDevice::CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout)
{
  return Result();
}

Result VulkanSwapChain::GetTexture(Texture ** ppTexture, uint32_t index)
{
  return Result();
}

void VulkanSwapChain::Present()
{
}

VulkanSwapChain::VulkanSwapChain(VulkanFactory* pFactory, void* pHandle, const SwapChainDesc* pDesc, VulkanQueue* pQueue)
  : OwningRoot(pFactory)
{
  OwningRoot->AddInternalRef();

  OwningDevice = pQueue->OwningRoot;
  OwningDevice->AddInternalRef();

  CreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  CreateInfo.imageArrayLayers = 1;
  CreateInfo.queueFamilyIndexCount = VK_SHARING_MODE_EXCLUSIVE;
  CreateInfo.oldSwapchain = VK_NULL_HANDLE;
  CreateInfo.clipped = VK_TRUE;
  CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

#if K3DPLATFORM_OS_WIN
  VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
  SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
  SurfaceCreateInfo.hwnd = (HWND)pHandle;
  vkCreateWin32SurfaceKHR(OwningRoot->Handle, &SurfaceCreateInfo, VULKAN_ALLOCATOR, &Surface);
#endif
  
  vkCreateSwapchainKHR(OwningDevice->Handle, &CreateInfo, VULKAN_ALLOCATOR, &Handle);
}

VulkanSwapChain::~VulkanSwapChain()
{
  vkDestroySwapchainKHR(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
  OwningDevice->ReleaseInternal();
  vkDestroySurfaceKHR(OwningRoot->Handle, Surface, VULKAN_ALLOCATOR);
  OwningRoot->ReleaseInternal();
}

Result VulkanFactory::CreateSwapchain(const SwapChainDesc* desc, CommandQueue* pCommandQueue, void* pWindow, SwapChain** pSwapchain)
{  
  *pSwapchain = new VulkanSwapChain(this, pWindow, desc, static_cast<VulkanQueue*>(pCommandQueue));
  return Result::Ok;
}

Result VulkanFactory::CreateCompiler(ShaderLang shaderLang, Compiler ** compiler)
{
  *compiler = nullptr;
  return Result::Ok;
}

Result VulkanFactory::EnumDevice(uint32_t * count, Device ** ppDevice)
{
  if(!ppDevice)
  {
    vkEnumeratePhysicalDevices(Handle, count, nullptr);
  }
  else
  {
    if(count && *count > 0)
    {
      VkPhysicalDevice * PhysicalDevices = new VkPhysicalDevice[*count];
      vkEnumeratePhysicalDevices(Handle, count, PhysicalDevices);
      for(uint32_t i=0; i<*count; i++)
      {
        ppDevice[i] = new VulkanDevice(this, PhysicalDevices[i]);
      }
      delete[] PhysicalDevices;
    }
    else
    {
      return Result::Failed;
    }
  //    vkEnumeratePhysicalDeviceGroupsKHX()
  }
  return Result::Ok;
}

VulkanTexture::VulkanTexture(VulkanDevice * pDevice, const TextureDesc * pDesc)
  : Super(pDevice)
{
}

VulkanTexture::~VulkanTexture()
{
}

Result VulkanTexture::GetDesc(TextureDesc * pDesc)
{
  return Result();
}

Result VulkanCompiler::Compile(const ShaderOption * option, void * pData, uint32_t size, Function ** output)
{
  return Result::Ok;
}

Result VulkanCompiler::Reflect(void * pData, uint32_t size)
{
  return Result::Ok;
}

VulkanCompiler::VulkanCompiler()
{
}

VulkanCompiler::~VulkanCompiler()
{
}

Result VulkanDevice::CreateBindingTable(PipelineLayout * pPipelineLayout, BindingTable ** ppBindingTable)
{
  return Result::Ok;
}

Result VulkanDevice::CreateRenderPipeline(const RenderPipelineDesc * pPipelineDesc, PipelineLayout * pPipelineLayout, RenderPass * pRenderPass, Pipeline ** pPipelineState)
{
  
  return Result::Ok;
}

Result VulkanDevice::CreateComputePipeline(Function * pComputeFunction, PipelineLayout * pPipelineLayout, Pipeline ** pPipeline)
{
  return Result::Ok;
}

Result VulkanDevice::CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass)
{
  return Result::Ok;
}

Result VulkanDevice::CreateRenderTarget(const RenderTargetDesc * desc, RenderTarget ** ppRenderTarget)
{
  return Result::Ok;
}

Result VulkanDevice::CreateSampler(const SamplerDesc * desc, Sampler ** pSampler)
{
  return Result();
}

Result VulkanDevice::CreateBuffer(const BufferDesc * desc, Buffer ** pBuffer)
{
  *pBuffer = new VulkanBuffer(this, desc);
  return Result();
}

Result VulkanDevice::CreateBufferView(const BufferViewDesc * desc, Buffer * pBuffer, BufferView ** pBufView)
{
  return Result();
}

Result VulkanDevice::CreateTexture(const TextureDesc * desc, Texture ** pTexture)
{
  *pTexture = new VulkanTexture(this, desc);
  return Result();
}

Result VulkanDevice::CreateTextureView(const TextureViewDesc * desc, Texture * pTexture, TextureView ** pTexView)
{
  return Result();
}

void VulkanDevice::WaitIdle()
{
  vkDeviceWaitIdle(Handle);
}

VulkanSampler::VulkanSampler(VulkanDevice* pDevice, const SamplerDesc* pDesc)
{
}

VulkanSampler::~VulkanSampler()
{
  vkDestroySampler(OwningRoot->Handle, Handle, VULKAN_ALLOCATOR);
}

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice * pDevice, Function * pComputeFunc, PipelineLayout * pLayout)
  : Super(pDevice)
{
}

VulkanComputePipeline::~VulkanComputePipeline()
{
}

VulkanRenderPipeline::VulkanRenderPipeline(VulkanDevice * pDevice, const RenderPipelineDesc * pDesc, RenderPass * pRenderPass, PipelineLayout * pLayout)
  : Super(pDevice)
{
}

VulkanRenderPipeline::~VulkanRenderPipeline()
{
}

Result VulkanRenderPipeline::GetDesc(RenderPipelineDesc * pDesc)
{
  return Result();
}

template<class T>
TPipeline<T>::~TPipeline()
{
  vkDestroyPipeline(OwningDevice->Handle, Handle, VULKAN_ALLOCATOR);
}

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice * pDevice)
{
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
}

Result VulkanPipelineLayout::CreateBindingTable(BindingTable ** ppBindingTable)
{
  return Result();
}
