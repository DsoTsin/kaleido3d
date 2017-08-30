#ifndef __VkRHI_h__
#define __VkRHI_h__
#pragma once
#include "VkCommon.h"
#include <Core/Os.h>
#include <list>
#include <tuple>

K3D_VK_BEGIN

typedef class DeviceImpl* DeviceRef;
typedef class CommandAllocator* CommandAllocatorRef;
typedef class SemaphoreImpl* SemaphoreRef;
typedef class FenceImpl* FenceRef;
typedef class SwapChainImpl* SwapChainRef;
typedef class CommandQueueImpl* CommandQueueRef;
typedef class CommandBufferImpl* CommandBufferRef;
typedef class PipelineLayoutImpl* PipelineLayoutRef;
typedef class FrameBufferImpl* FrameBufferRef;
typedef class RenderPassImpl* RenderPassRef;
typedef class TextureImpl* TextureRef;
typedef class SamplerImpl* SamplerRef;
typedef class BufferImpl* BufferRef;
typedef class FactoryImpl* InstanceRef;

typedef class ParallelCommandEncoderImpl* ParallelCommandEncoderRef;

class Gpu;
using GpuRef = SharedPtr<Gpu>;


class ResourceManager;
using PtrResManager = std::unique_ptr<ResourceManager>;

class CommandContextPool;

class DescriptorAllocator;
class DescriptorSetLayout;

class RenderTarget;
using SpRenderTarget = SharedPtr<RenderTarget>;


class ShaderResourceView;
using BindingArray = DynArray<VkDescriptorSetLayoutBinding>;

class DescriptorAllocator;
using DescriptorAllocRef = SharedPtr<DescriptorAllocator>;

class DescriptorSetLayout;
using DescriptorSetLayoutRef = SharedPtr<DescriptorSetLayout>;


typedef std::unordered_map<uint32, DescriptorSetLayoutRef>
  MapDescriptorSetLayout;
typedef std::unordered_map<uint32, DescriptorAllocRef> MapDescriptorAlloc;

template <class TKey, class TVal, int _MaxCacheCount>
class LRUCache
{ 
  typedef typename std::pair<TKey, TVal>                    TEntry;
  typedef typename std::list<TEntry>                        TEntryList;
  typedef typename TEntryList::iterator                     TEntryListIter;
  typedef typename std::unordered_map<TKey, TEntryListIter> TEntryMap;

public:
  LRUCache()
  {
  }

  ~LRUCache()
  {
  }

  bool Put(TKey Key, TVal Val)
  {
    bool CauseRetire = false;
    auto iter = m_CacheMap.find(Key);
    m_CacheList.push_front(TEntry(Key, Val));
    if (iter != m_CacheMap.end())
    {
      m_CacheMap.erase(iter);
      m_CacheList.erase(iter->second);
    }
    m_CacheMap[Key] = m_CacheList.begin();
    if (m_CacheMap.size() > _MaxCacheCount)
    {
      CauseRetire = true;
      auto Last = m_CacheList.end();
      Last--;
      m_CacheMap.erase(Last->first);
      m_CacheList.pop_back();
    }
    return CauseRetire;
  }

  TVal Get(TKey Key)
  {
    auto Iter = m_CacheMap.find(Key);
    if (Iter != std::end(m_CacheMap))
    {
      m_CacheList.splice(m_CacheList.begin(), m_CacheList, Iter->second); 
      return Iter->second->second;
    }
    return TVal();
  }

private:
  TEntryMap  m_CacheMap;
  TEntryList m_CacheList;
};

template<class RHIObj>
struct ResTrait
{
};

template<>
struct ResTrait<NGFXBuffer>
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
  static decltype(vkDestroyBuffer)* Destroy;
  static decltype(vkGetBufferMemoryRequirements)* GetMemoryInfo;
  static decltype(vkBindBufferMemory)* BindMemory;
};

template<>
struct ResTrait<NGFXTexture>
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
  static decltype(vkDestroyImage)* Destroy;
  static decltype(vkGetImageMemoryRequirements)* GetMemoryInfo;
  static decltype(vkBindImageMemory)* BindMemory;
};

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugReportCallback(VkDebugReportFlagsEXT flags,
  VkDebugReportObjectTypeEXT objectType,
  uint64_t object,
  size_t location,
  int32_t messageCode,
  const char* pLayerPrefix,
  const char* pMessage,
  void* pUserData);

#ifdef VK_NO_PROTOTYPES
#define __VK_DEVICE_PROC__(name) PFN_vk##name vk##name = NULL
#endif

class Gpu : public EnableSharedFromThis<Gpu>
{
public:
  ~Gpu();

  VkDevice CreateLogicDevice(bool enableValidation);
  VkBool32 GetSupportedDepthFormat(VkFormat* depthFormat);

  VkResult GetSurfaceSupportKHR(uint32_t queueFamilyIndex,
    VkSurfaceKHR surface,
    VkBool32* pSupported);
  VkResult GetSurfaceCapabilitiesKHR(
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
  VkResult GetSurfaceFormatsKHR(VkSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormatKHR* pSurfaceFormats);
  VkResult GetSurfacePresentModesKHR(VkSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes);

  void DestroyDevice();
  void FreeCommandBuffers(VkCommandPool, uint32, VkCommandBuffer*);
  VkResult CreateCommdPool(const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCommandPool* pCommandPool);
  VkResult AllocateCommandBuffers(
    const VkCommandBufferAllocateInfo* pAllocateInfo,
    VkCommandBuffer* pCommandBuffers);

  InstanceRef GetInstance() const { return m_Inst; }

#ifdef VK_NO_PROTOTYPES
  __VK_DEVICE_PROC__(DestroyDevice);
  __VK_DEVICE_PROC__(GetDeviceQueue);
  __VK_DEVICE_PROC__(QueueSubmit);
  __VK_DEVICE_PROC__(QueueWaitIdle);
  __VK_DEVICE_PROC__(QueuePresentKHR);
  __VK_DEVICE_PROC__(DeviceWaitIdle);
  __VK_DEVICE_PROC__(AllocateMemory);
  __VK_DEVICE_PROC__(FreeMemory);
  __VK_DEVICE_PROC__(MapMemory);
  __VK_DEVICE_PROC__(UnmapMemory);
  __VK_DEVICE_PROC__(FlushMappedMemoryRanges);
  __VK_DEVICE_PROC__(InvalidateMappedMemoryRanges);
  __VK_DEVICE_PROC__(GetDeviceMemoryCommitment);
  __VK_DEVICE_PROC__(BindBufferMemory);
  __VK_DEVICE_PROC__(BindImageMemory);
  __VK_DEVICE_PROC__(GetBufferMemoryRequirements);
  __VK_DEVICE_PROC__(GetImageMemoryRequirements);
  __VK_DEVICE_PROC__(GetImageSparseMemoryRequirements);
  __VK_DEVICE_PROC__(QueueBindSparse);
  __VK_DEVICE_PROC__(CreateFence);
  __VK_DEVICE_PROC__(DestroyFence);
  __VK_DEVICE_PROC__(ResetFences);
  __VK_DEVICE_PROC__(GetFenceStatus);
  __VK_DEVICE_PROC__(WaitForFences);
  __VK_DEVICE_PROC__(CreateSemaphore);
  __VK_DEVICE_PROC__(DestroySemaphore);
  __VK_DEVICE_PROC__(CreateEvent);
  __VK_DEVICE_PROC__(DestroyEvent);
  __VK_DEVICE_PROC__(GetEventStatus);
  __VK_DEVICE_PROC__(SetEvent);
  __VK_DEVICE_PROC__(ResetEvent);
  __VK_DEVICE_PROC__(CreateQueryPool);
  __VK_DEVICE_PROC__(DestroyQueryPool);
  __VK_DEVICE_PROC__(GetQueryPoolResults);
  __VK_DEVICE_PROC__(CreateBuffer);
  __VK_DEVICE_PROC__(DestroyBuffer);
  __VK_DEVICE_PROC__(CreateBufferView);
  __VK_DEVICE_PROC__(DestroyBufferView);
  __VK_DEVICE_PROC__(CreateImage);
  __VK_DEVICE_PROC__(DestroyImage);
  __VK_DEVICE_PROC__(GetImageSubresourceLayout);
  __VK_DEVICE_PROC__(CreateImageView);
  __VK_DEVICE_PROC__(DestroyImageView);
  __VK_DEVICE_PROC__(CreateShaderModule);
  __VK_DEVICE_PROC__(DestroyShaderModule);
  __VK_DEVICE_PROC__(CreatePipelineCache);
  __VK_DEVICE_PROC__(DestroyPipelineCache);
  __VK_DEVICE_PROC__(GetPipelineCacheData);
  __VK_DEVICE_PROC__(MergePipelineCaches);
  __VK_DEVICE_PROC__(CreateGraphicsPipelines);
  __VK_DEVICE_PROC__(CreateComputePipelines);
  __VK_DEVICE_PROC__(DestroyPipeline);
  __VK_DEVICE_PROC__(CreatePipelineLayout);
  __VK_DEVICE_PROC__(DestroyPipelineLayout);
  __VK_DEVICE_PROC__(CreateSampler);
  __VK_DEVICE_PROC__(DestroySampler);
  __VK_DEVICE_PROC__(CreateDescriptorSetLayout);
  __VK_DEVICE_PROC__(DestroyDescriptorSetLayout);
  __VK_DEVICE_PROC__(CreateDescriptorPool);
  __VK_DEVICE_PROC__(DestroyDescriptorPool);
  __VK_DEVICE_PROC__(ResetDescriptorPool);
  __VK_DEVICE_PROC__(AllocateDescriptorSets);
  __VK_DEVICE_PROC__(FreeDescriptorSets);
  __VK_DEVICE_PROC__(UpdateDescriptorSets);
  __VK_DEVICE_PROC__(CreateFramebuffer);
  __VK_DEVICE_PROC__(DestroyFramebuffer);
  __VK_DEVICE_PROC__(CreateRenderPass);
  __VK_DEVICE_PROC__(DestroyRenderPass);
  __VK_DEVICE_PROC__(GetRenderAreaGranularity);
  __VK_DEVICE_PROC__(CreateCommandPool);
  __VK_DEVICE_PROC__(DestroyCommandPool);
  __VK_DEVICE_PROC__(ResetCommandPool);
  __VK_DEVICE_PROC__(AllocateCommandBuffers);
  __VK_DEVICE_PROC__(FreeCommandBuffers);
  __VK_DEVICE_PROC__(BeginCommandBuffer);
  __VK_DEVICE_PROC__(EndCommandBuffer);
  __VK_DEVICE_PROC__(ResetCommandBuffer);
  __VK_DEVICE_PROC__(CmdBindPipeline);
  __VK_DEVICE_PROC__(CmdSetViewport);
  __VK_DEVICE_PROC__(CmdSetScissor);
  __VK_DEVICE_PROC__(CmdSetLineWidth);
  __VK_DEVICE_PROC__(CmdSetDepthBias);
  __VK_DEVICE_PROC__(CmdSetBlendConstants);
  __VK_DEVICE_PROC__(CmdSetDepthBounds);
  __VK_DEVICE_PROC__(CmdSetStencilCompareMask);
  __VK_DEVICE_PROC__(CmdSetStencilWriteMask);
  __VK_DEVICE_PROC__(CmdSetStencilReference);
  __VK_DEVICE_PROC__(CmdBindDescriptorSets);
  __VK_DEVICE_PROC__(CmdBindIndexBuffer);
  __VK_DEVICE_PROC__(CmdBindVertexBuffers);
  __VK_DEVICE_PROC__(CmdDraw);
  __VK_DEVICE_PROC__(CmdDrawIndexed);
  __VK_DEVICE_PROC__(CmdDrawIndirect);
  __VK_DEVICE_PROC__(CmdDrawIndexedIndirect);
  __VK_DEVICE_PROC__(CmdDispatch);
  __VK_DEVICE_PROC__(CmdDispatchIndirect);
  __VK_DEVICE_PROC__(CmdCopyBuffer);
  __VK_DEVICE_PROC__(CmdCopyImage);
  __VK_DEVICE_PROC__(CmdBlitImage);
  __VK_DEVICE_PROC__(CmdCopyBufferToImage);
  __VK_DEVICE_PROC__(CmdCopyImageToBuffer);
  __VK_DEVICE_PROC__(CmdUpdateBuffer);
  __VK_DEVICE_PROC__(CmdFillBuffer);
  __VK_DEVICE_PROC__(CmdClearColorImage);
  __VK_DEVICE_PROC__(CmdClearDepthStencilImage);
  __VK_DEVICE_PROC__(CmdClearAttachments);
  __VK_DEVICE_PROC__(CmdResolveImage);
  __VK_DEVICE_PROC__(CmdSetEvent);
  __VK_DEVICE_PROC__(CmdResetEvent);
  __VK_DEVICE_PROC__(CmdWaitEvents);
  __VK_DEVICE_PROC__(CmdPipelineBarrier);
  __VK_DEVICE_PROC__(CmdBeginQuery);
  __VK_DEVICE_PROC__(CmdEndQuery);
  __VK_DEVICE_PROC__(CmdResetQueryPool);
  __VK_DEVICE_PROC__(CmdWriteTimestamp);
  __VK_DEVICE_PROC__(CmdCopyQueryPoolResults);
  __VK_DEVICE_PROC__(CmdPushConstants);
  __VK_DEVICE_PROC__(CmdBeginRenderPass);
  __VK_DEVICE_PROC__(CmdNextSubpass);
  __VK_DEVICE_PROC__(CmdEndRenderPass);
  __VK_DEVICE_PROC__(CmdExecuteCommands);
  __VK_DEVICE_PROC__(AcquireNextImageKHR);
  __VK_DEVICE_PROC__(CreateSwapchainKHR);
  __VK_DEVICE_PROC__(DestroySwapchainKHR);
  __VK_DEVICE_PROC__(GetSwapchainImagesKHR);

  PFN_vkDestroyDevice fpDestroyDevice = NULL;
  PFN_vkFreeCommandBuffers fpFreeCommandBuffers = NULL;
  PFN_vkCreateCommandPool fpCreateCommandPool = NULL;
  PFN_vkAllocateCommandBuffers fpAllocateCommandBuffers = NULL;
#endif

  VkDevice m_LogicalDevice;

private:
  friend class FactoryImpl;
  friend class DeviceImpl;
  friend class DeviceAdapter;

  Gpu(VkPhysicalDevice const&, InstanceRef const& inst);

  void QuerySupportQueues();
  void LoadDeviceProcs();

  InstanceRef m_Inst;
  VkPhysicalDevice m_PhysicalGpu;
  VkPhysicalDeviceProperties m_Prop;
  VkPhysicalDeviceFeatures m_Features;
  VkPhysicalDeviceMemoryProperties m_MemProp;
  uint32 m_GraphicsQueueIndex = 0;
  uint32 m_ComputeQueueIndex = 0;
  uint32 m_CopyQueueIndex = 0;
  DynArray<VkQueueFamilyProperties> m_QueueProps;
};

class CommandBufferManager
{
public:
  CommandBufferManager(VkDevice pDevice,
    VkCommandBufferLevel bufferLevel,
    unsigned graphicsQueueIndex);
  ~CommandBufferManager();
  void Destroy();
  VkCommandBuffer RequestCommandBuffer();
  void BeginFrame();

private:

  VkDevice m_Device = VK_NULL_HANDLE;
  VkCommandPool m_Pool = VK_NULL_HANDLE;
  DynArray<VkCommandBuffer> m_Buffers;
  VkCommandBufferLevel m_CommandBufferLevel;
  uint32 m_Count = 0;
};

using CmdBufManagerRef = SharedPtr<CommandBufferManager>;

using MapFramebuffer = std::unordered_map<uint64, FrameBufferRef>;
using MapRenderpass = std::unordered_map<uint64, RenderPassRef>;

class DeviceObjectCache
{
public:
  static LRUCache<uint64, FrameBufferRef, 4> s_CacheFrameBuffer;
  static MapFramebuffer s_Framebuffer;
  static MapRenderpass s_RenderPass;
};

class DeviceImpl
  : public NGFXDevice
{
public:
  DeviceImpl();
  ~DeviceImpl() override;
  void Release();

  NGFXResourceRef CreateResource(NGFXResourceDesc const&) override;

  NGFXSRVRef CreateShaderResourceView(
    NGFXResourceRef,
    NGFXSRVDesc const&) override;

  NGFXUAVRef CreateUnorderedAccessView(const NGFXResourceRef&,
                                       NGFXUAVDesc const&) override;

  NGFXSamplerRef CreateSampler(const NGFXSamplerState&) override;

  NGFXPipelineLayoutRef CreatePipelineLayout(
    NGFXPipelineLayoutDesc const& table) override;

  NGFXRenderpassRef CreateRenderPass(NGFXRenderPassDesc const&) override;

  NGFXPipelineStateRef CreateRenderPipelineState(
    NGFXRenderPipelineDesc const&,
    NGFXPipelineLayoutRef,
    NGFXRenderpassRef) override;

  NGFXPipelineStateRef CreateComputePipelineState(
    NGFXComputePipelineDesc const&,
    NGFXPipelineLayoutRef) override;

  NGFXFenceRef CreateFence() override;

  NGFXCommandQueueRef CreateCommandQueue(NGFXCommandType const&) override;

  void WaitIdle() override { vkDeviceWaitIdle(m_Device); }

  void QueryTextureSubResourceLayout(NGFXTextureRef,
                                     NGFXTextureSpec const& spec,
                                     NGFXSubResourceLayout*) override;

  VkDevice const& GetRawDevice() const { return m_Device; }

  CommandAllocatorRef NewCommandAllocator(bool transient);
  bool FindMemoryType(uint32 typeBits,
                      VkFlags requirementsMask,
                      uint32* typeIndex) const;
  SemaphoreRef NewSemaphore();

  uint32 GetQueueCount() const { return m_Gpu->m_QueueProps.Count(); }

  DescriptorAllocRef NewDescriptorAllocator(uint32 maxSets,
                                            BindingArray const& bindings);
  DescriptorSetLayoutRef NewDescriptorSetLayout(BindingArray const& bindings);

  uint64 GetMaxAllocationCount();
  VkShaderModule CreateShaderModule(NGFXShaderBundle const& Bundle);
  VkQueue GetImmQueue() const { return m_ImmediateQueue; }
  VkCommandBuffer AllocateImmediateCommand();

  RenderPassRef GetOrCreateRenderPass(const NGFXRenderPassDesc& desc);
  FrameBufferRef GetOrCreateFramebuffer(const NGFXRenderPassDesc& desc);

  friend class DeviceChild;

protected:
  CommandQueueRef InitCmdQueue(VkQueueFlags queueTypes,
                               uint32 queueFamilyIndex,
                               uint32 queueIndex);

  Result Create(GpuRef const& gpu, bool withDebug);

  friend class FactoryImpl;
  friend class SwapChainImpl;
  template <class T1, class T2>
  friend class TVkRHIObjectBase;

private:
  CmdBufManagerRef m_CmdBufManager;

private:
  VkPhysicalDeviceMemoryProperties m_MemoryProperties = {};
  VkDevice m_Device = nullptr;

  VkQueue m_ImmediateQueue = nullptr;
  VkCommandPool m_ImmediateCmdPool = VK_NULL_HANDLE;

  GpuRef m_Gpu;
};

#define __VK_GLOBAL_LEVEL_PROC__(name) PFN_vk##name gp##name = NULL;
#define __VK_INSTANCE_LEVEL_PROC__(name) PFN_vk##name fp##name = NULL

class FactoryImpl
  : public NGFXFactory
{
public:
  FactoryImpl(const String& engineName,
              const String& appName,
              bool enableValidation = true);
  ~FactoryImpl();

  void Release();
  // NGFXFactory::EnumDevices
  void EnumDevices(DynArray<NGFXDeviceRef>& Devices) override;
  // NGFXFactory::CreateSwapChain
  NGFXSwapChainRef CreateSwapchain(NGFXCommandQueueRef pCommandQueue,
                                   void* nPtr,
                                   NGFXSwapChainDesc&) override;

  void SetupDebugging(VkDebugReportFlagsEXT flags,
                      PFN_vkDebugReportCallbackEXT callBack);
  void FreeDebugCallback();

  bool WithValidation() const { return m_EnableValidation; }

  DynArray<GpuRef> EnumGpus();

#ifdef VK_NO_PROTOTYPES
  VkResult CreateSurfaceKHR(const
#if K3DPLATFORM_OS_WIN
                            VkWin32SurfaceCreateInfoKHR
#elif K3DPLATFORM_OS_ANDROID
                            VkAndroidSurfaceCreateInfoKHR
#endif
                              * pCreateInfo,
                            const VkAllocationCallbacks* pAllocator,
                            VkSurfaceKHR* pSurface)
  {
#if K3DPLATFORM_OS_WIN
    return fpCreateWin32SurfaceKHR(
      m_Instance, pCreateInfo, pAllocator, pSurface);
#elif K3DPLATFORM_OS_ANDROID
    return fpCreateAndroidSurfaceKHR(
      m_Instance, pCreateInfo, pAllocator, pSurface);
#endif
  }

  void DestroySurfaceKHR(VkSurfaceKHR surface,
                         VkAllocationCallbacks* pAllocator)
  {
    fpDestroySurfaceKHR(m_Instance, surface, pAllocator);
  }
#endif

  friend class Gpu;
  friend class DeviceImpl;
  friend class SwapChainImpl;
  friend class RHIImpl;

private:
  void LoadGlobalProcs();
  void EnumExtsAndLayers();
  void ExtractEnabledExtsAndLayers();
  void LoadInstanceProcs();

  bool m_EnableValidation;
  DynArray<String> m_EnabledExts;
  DynArray<char*> m_EnabledExtsRaw;
  DynArray<String> m_EnabledLayers;
  DynArray<const char*> m_EnabledLayersRaw;

  VkInstance m_Instance;
  VkDebugReportCallbackEXT m_DebugMsgCallback;

  PFN_vkCreateDebugReportCallbackEXT CreateDebugReport = NULL;
  PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReport = NULL;
#ifdef VK_NO_PROTOTYPES

  PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
    fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
    fpGetPhysicalDeviceSurfacePresentModesKHR;

  __VK_GLOBAL_LEVEL_PROC__(CreateInstance);
  __VK_GLOBAL_LEVEL_PROC__(EnumerateInstanceExtensionProperties);
  __VK_GLOBAL_LEVEL_PROC__(EnumerateInstanceLayerProperties);
  __VK_GLOBAL_LEVEL_PROC__(GetInstanceProcAddr);
  __VK_GLOBAL_LEVEL_PROC__(DestroyInstance);

  __VK_INSTANCE_LEVEL_PROC__(CreateDebugReportCallbackEXT);
  __VK_INSTANCE_LEVEL_PROC__(DestroyDebugReportCallbackEXT);

#if K3DPLATFORM_OS_WIN
  __VK_INSTANCE_LEVEL_PROC__(CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
  __VK_INSTANCE_LEVEL_PROC__(CreateAndroidSurfaceKHR);
#endif
  __VK_INSTANCE_LEVEL_PROC__(DestroySurfaceKHR);

  __VK_INSTANCE_LEVEL_PROC__(EnumeratePhysicalDevices);
  __VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceProperties);
  __VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceFeatures);
  __VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceMemoryProperties);
  __VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceQueueFamilyProperties);
  __VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceFormatProperties);
  __VK_INSTANCE_LEVEL_PROC__(CreateDevice);
  __VK_INSTANCE_LEVEL_PROC__(GetDeviceProcAddr);

  dynlib::LibRef m_VulkanLib;
#endif
};

class DeviceChild
{
public:
  explicit DeviceChild(DeviceRef pDevice)
    : m_pDevice(pDevice)
  {
    if (m_pDevice)
    {
      m_pDevice->AddInternalRef();
    }
  }
  virtual ~DeviceChild() 
  {
    if (m_pDevice)
    {
      m_pDevice->ReleaseInternal();
    }
  }

  VkDevice const& GetRawDevice() const { return m_pDevice->GetRawDevice(); }
  GpuRef GetGpuRef() const { return m_pDevice->m_Gpu; }

  DeviceRef const GetDevice() const { return m_pDevice; }

private:
  DeviceRef m_pDevice;
};

template<class TVkObj, class TRHIObj>
class TVkRHIObjectBase
  : public TRHIObj
{
public:
  using ThisObj = TVkRHIObjectBase<TVkObj, TRHIObj>;

  explicit TVkRHIObjectBase(DeviceRef const& refDevice)
    : m_Device(refDevice)
    , m_NativeObj(VK_NULL_HANDLE)
  {
    if (m_Device)
    {
      m_Device->AddInternalRef();
    }
  }

  virtual ~TVkRHIObjectBase() 
  {
    if (m_Device)
    {
      m_Device->ReleaseInternal();
    }
  }

  DeviceRef SharedDevice() { return m_Device; }

  TVkObj NativeHandle() const { return m_NativeObj; }
  VkDevice NativeDevice() const { return m_Device->GetRawDevice(); }
  VkCommandPool ImmCmdPool() const { return m_Device->m_ImmediateCmdPool; }

protected:
  DeviceRef m_Device;
  TVkObj m_NativeObj;
};

/**
* If VK_FENCE_CREATE_SIGNALED_BIT is set then the fence is created already
* signaled, otherwise, the fence is created in an unsignaled state.
*/
class FenceCreateInfo : public VkFenceCreateInfo
{
public:
  static VkFenceCreateInfo Create()
  {
    return { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };
  }
};
/**
 * Fences are signaled by the system when work invoked by vkQueueSubmit
 * completes.
 */
class FenceImpl : public TVkRHIObjectBase<VkFence, NGFXFence>
{
  friend class DeviceImpl;
public:
  FenceImpl(DeviceRef pDevice,
        VkFenceCreateInfo const& info = FenceCreateInfo::Create())
    : ThisObj(pDevice)
  {
    if (pDevice) {
      K3D_VK_VERIFY(
        vkCreateFence(NativeDevice(), &info, nullptr, &m_NativeObj));
    }
  }

  ~FenceImpl() override
  {
    if (m_NativeObj) {
      vkDestroyFence(NativeDevice(), m_NativeObj, nullptr);
      VKLOG(Info, "FenceImpl Destroyed. -- %0x.", m_NativeObj);
      m_NativeObj = VK_NULL_HANDLE;
    }
  }

  void Signal(int32 val) override {}

  bool IsSignaled()
  {
    return VK_SUCCESS == vkGetFenceStatus(NativeDevice(), m_NativeObj);
  }

  void Reset() override { vkResetFences(NativeDevice(), 1, &m_NativeObj); }

  void WaitFor(uint64 time) override
  {
    vkWaitForFences(NativeDevice(), 1, &m_NativeObj, VK_TRUE, time);
  }

private:
  friend class SwapChainImpl;
  friend class CommandContext;
};

class SamplerImpl : public TVkRHIObjectBase<VkSampler, NGFXSampler>
{
public:
  explicit SamplerImpl(DeviceRef pDevice, NGFXSamplerState const& sampleDesc);
  ~SamplerImpl() override;
  NGFXSamplerState GetSamplerDesc() const override;

protected:
  VkSamplerCreateInfo m_SamplerCreateInfo = {};
  NGFXSamplerState m_SamplerState;
};

/**
 * @param binding
 */
VkDescriptorSetLayoutBinding
RHIBinding2VkBinding(NGFXShaderBinding const& binding);
/**
 * Vulkan has DescriptorPool, DescriptorSet, DescriptorSetLayout and
 * PipelineLayout DescriptorSet is allocated from DescriptorPool with
 * DescriptorSetLayout, PipelineLayout depends on DescriptorSetLayout,
 */
class DescriptorAllocator : public DeviceChild
{
public:
  struct Options
  {
    VkDescriptorPoolCreateFlags CreateFlags = 0;
    VkDescriptorPoolResetFlags ResetFlags = 0;
  };

  /**
   * @param maxSets
   * @param bindings
   */
  explicit DescriptorAllocator(DeviceRef pDevice,
                               Options const& option,
                               uint32 maxSets,
                               BindingArray const& bindings);
  ~DescriptorAllocator();

protected:
  void Initialize(uint32 maxSets, BindingArray const& bindings);
  void Destroy();

private:
  friend class BindingGroupImpl;
  friend class PipelineLayoutImpl;

  Options m_Options;
  VkDescriptorPool m_Pool;
};

using SpDescriptorAllocator = SharedPtr<DescriptorAllocator>;

class DescriptorSetLayout : public DeviceChild
{
public:
  DescriptorSetLayout(DeviceRef pDevice, BindingArray const& bindings);
  ~DescriptorSetLayout();

  VkDescriptorSetLayout GetNativeHandle() const
  {
    return m_DescriptorSetLayout;
  }

protected:
  void Initialize(BindingArray const& bindings);
  void Destroy();

private:
  friend class PipelineLayoutImpl;
  VkDescriptorSetLayout m_DescriptorSetLayout;
};

class BindingGroupImpl
  : public TVkRHIObjectBase<VkDescriptorSet, NGFXBindingGroup>
{
  using Super = TVkRHIObjectBase<VkDescriptorSet, NGFXBindingGroup>;
  template<typename T>
  friend class CommandEncoder;
public:
  static BindingGroupImpl* CreateDescSet(PipelineLayoutRef m_RootLayout,
                                      VkDescriptorSetLayout layout,
                                      BindingArray const& bindings,
                                      DeviceRef pDevice);
  virtual ~BindingGroupImpl();
  void Update(uint32 bindSet, NGFXUAVRef) override;
  void Update(uint32 bindSet, NGFXSamplerRef) override;
  void Update(uint32 bindSet, NGFXResourceRef) override;
  uint32 GetSlotNum() const override;

private:
  PipelineLayoutRef m_RootLayout;
  BindingArray m_Bindings;
  std::vector<VkWriteDescriptorSet> m_BoundDescriptorSet;

private:
  BindingGroupImpl(PipelineLayoutRef m_RootLayout,
                VkDescriptorSetLayout layout,
                BindingArray const& bindings,
                DeviceRef pDevice);

  void Initialize(VkDescriptorSetLayout layout, BindingArray const& bindings);
  void Destroy();
};

class PipelineLayoutImpl
  : public TVkRHIObjectBase<VkPipelineLayout, NGFXPipelineLayout>
{
  friend class DeviceImpl;
public:
  PipelineLayoutImpl(DeviceRef pDevice, NGFXPipelineLayoutDesc const& desc);
  ~PipelineLayoutImpl() override;

  NGFXBindingGroupRef ObtainBindingGroup() override;

  VkDescriptorPool Pool() const
  {
    return m_DescAllocator->m_Pool;
  }

protected:
  void InitWithDesc(NGFXPipelineLayoutDesc const& desc);
  void Destroy();
  friend class RenderPipelineStateImpl;

private:
  BindingArray m_BindingArray;
  SpDescriptorAllocator m_DescAllocator;
  DescriptorSetLayoutRef m_DescSetLayout;
};

template<class TRHIResObj>
class TResource : public TVkRHIObjectBase<typename ResTrait<TRHIResObj>::Obj, TRHIResObj>
{
  friend class CommandBufferImpl;
public:
  using Super = TResource<TRHIResObj>;
  using TVkRHIObjectBase<typename ResTrait<TRHIResObj>::Obj, TRHIResObj>::m_NativeObj;
  using TVkRHIObjectBase<typename ResTrait<TRHIResObj>::Obj, TRHIResObj>::NativeDevice;
  using TVkRHIObjectBase<typename ResTrait<TRHIResObj>::Obj, TRHIResObj>::m_Device;

  explicit TResource(DeviceRef const& refDevice, bool SelfOwned = true)
    : TVkRHIObjectBase<typename ResTrait<TRHIResObj>::Obj, TRHIResObj>(refDevice)
    , m_MemAllocInfo{}
    , m_HostMemAddr(nullptr)
    , m_DeviceMem{ VK_NULL_HANDLE }
    , m_ResView(VK_NULL_HANDLE)
    , m_ResDesc{}
    , m_SelfOwned(SelfOwned)
  {
  }

  TResource(DeviceRef const& refDevice, NGFXResourceDesc const& desc)
    : TVkRHIObjectBase<typename ResTrait<TRHIResObj>::Obj, TRHIResObj>(refDevice)
      , m_MemAllocInfo{}
      , m_DeviceMem{}
      , m_HostMemAddr(nullptr)
      , m_ResView(VK_NULL_HANDLE)
      , m_ResDesc(desc)
  {
  }

  virtual ~TResource()
  {
    if (VK_NULL_HANDLE != m_ResView)
    {
      ResTrait<TRHIResObj>::DestroyView(NativeDevice(), m_ResView, nullptr);
      VKLOG(Info, "TResourceView Destroying.. -- 0x%0x.", m_ResView);
      m_ResView = VK_NULL_HANDLE;
    }
    if (m_SelfOwned && VK_NULL_HANDLE != m_NativeObj)
    {
      VKLOG(Info, "TResource Releasing.. 0x%0x. Name: %s.", m_NativeObj, m_Name.CStr());
      ResTrait<TRHIResObj>::Destroy(NativeDevice(), m_NativeObj, nullptr);
      m_NativeObj = VK_NULL_HANDLE;
    }
    if (VK_NULL_HANDLE != m_DeviceMem)
    {
      VKLOG(Info,
        "TResource Freeing Memory. -- 0x%0x, tid:%d",
        m_DeviceMem,
        Os::Thread::GetId());
      vkFreeMemory(NativeDevice(), m_DeviceMem, nullptr);
      m_DeviceMem = VK_NULL_HANDLE;
    }
  }

  void SetName(const char* Name) override
  {
    m_Name = Name;
  }

  void* Map(uint64 offset, uint64 size) override
  {
    K3D_VK_VERIFY(vkMapMemory(
      NativeDevice(), m_DeviceMem, offset, size, 0, &m_HostMemAddr));
    return m_HostMemAddr;
  }

  void UnMap() override { vkUnmapMemory(NativeDevice(), m_DeviceMem); }

  uint64 GetLocation() const override { return (uint64)m_NativeObj; }
  virtual uint64 GetSize() const override { return m_MemAllocInfo.allocationSize; }
  NGFXResourceDesc GetDesc() const override { return m_ResDesc; }

  typedef typename ResTrait<TRHIResObj>::CreateInfo CreateInfo;
  typedef typename ResTrait<TRHIResObj>::ViewCreateInfo ViewCreateInfo;
  typedef typename ResTrait<TRHIResObj>::View ResourceView;
  typedef typename ResTrait<TRHIResObj>::DescriptorInfo ResourceDescriptorInfo;
  typedef typename ResTrait<TRHIResObj>::UsageFlags ResourceUsageFlags;
  ResourceView NativeView() const { return m_ResView; }
  ResourceDescriptorInfo DescInfo() const { return m_ResDescInfo; }

protected:
  VkMemoryAllocateInfo m_MemAllocInfo;
  VkMemoryRequirements m_MemReq;
  VkDeviceMemory m_DeviceMem;
  VkMemoryPropertyFlags m_MemoryBits = 0;
  VkAccessFlagBits m_AccessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;
  void* m_HostMemAddr;
  ResourceView m_ResView;
  ResourceUsageFlags m_ResUsageFlags = 0;
  ResourceDescriptorInfo m_ResDescInfo{};
  NGFXResourceDesc m_ResDesc;
  NGFXResourceState m_UsageState = NGFX_RESOURCE_STATE_UNKNOWN;
  String m_Name;
  bool m_SelfOwned = true;

protected:
  void Allocate(CreateInfo const& info)
  {
    K3D_VK_VERIFY(ResTrait<TRHIResObj>::Create(
      NativeDevice(), &info, nullptr, &m_NativeObj));
    m_MemAllocInfo = {};
    m_MemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ResTrait<TRHIResObj>::GetMemoryInfo(NativeDevice(), m_NativeObj, &m_MemReq);

    m_MemAllocInfo.allocationSize = m_MemReq.size;
    m_Device->FindMemoryType(
      m_MemReq.memoryTypeBits, m_MemoryBits, &m_MemAllocInfo.memoryTypeIndex);
    K3D_VK_VERIFY(
      vkAllocateMemory(NativeDevice(), &m_MemAllocInfo, nullptr, &m_DeviceMem));
    m_MemAllocInfo.allocationSize;

    /*K3D_VK_VERIFY(vkBindBufferMemory(NativeDevice(), m_Buffer, m_DeviceMem,
     * m_AllocationOffset));*/
  }
};

class BufferImpl : public TResource<NGFXBuffer>
{
public:
  typedef BufferImpl* Ptr;

  explicit BufferImpl(DeviceRef pDevice)
    : Super(pDevice)
  {
  }

  BufferImpl(DeviceRef pDevice, NGFXResourceDesc const& desc);
  virtual ~BufferImpl();
  uint64 GetSize() const override { return m_ResDesc.Size; }
  void Create(size_t size);
};

class TextureImpl : public TResource<NGFXTexture>
{
public:
  typedef SharedPtr<TextureImpl> TextureRef;

  explicit TextureImpl(DeviceRef pDevice)
    : Super(pDevice)
  {
  }

  TextureImpl(DeviceRef pDevice, NGFXResourceDesc const&);
  // For Swapchain
  TextureImpl(NGFXResourceDesc const& Desc,
          VkImage image,
          DeviceRef pDevice,
          bool selfOwnShip = true);

  ~TextureImpl() override;

  void BindSampler(NGFXSamplerRef sampler) override;
  NGFXSamplerRef GetSampler() const override;
  NGFXSRVRef GetResourceView() const override { return m_SRV; }
  void SetResourceView(NGFXSRVRef srv) override { m_SRV = srv; }
  NGFXResourceState GetState() const override { return m_UsageState; }

  VkImageLayout GetImageLayout() const { return m_ImageLayout; }
  VkImageSubresourceRange GetSubResourceRange() const { return m_SubResRange; }

  void CreateResourceView();
  void CreateViewForSwapchainImage();

  friend class SwapChainImpl;
  friend class CommandBufferImpl;

private:

  void InitCreateInfos();

private:
  SamplerRef m_ImageSampler;
  VkImageViewCreateInfo m_ImageViewInfo = {};
  NGFXSRVRef m_SRV;
  VkImageCreateInfo m_ImageInfo = {};
  VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageMemoryBarrier m_Barrier;

  VkSubresourceLayout m_SubResourceLayout = {};
  VkImageSubresourceRange m_SubResRange = {};
};

class RenderTexture : public TextureImpl
{

};

class ResourceManager : public DeviceChild
{
public:
  struct Allocation
  {
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;
  };

  template<typename VkObjectT>
  struct ResDesc
  {
    VkObjectT Object;
    bool IsTransient = false;
    uint32_t MemoryTypeIndex;
    VkMemoryPropertyFlags MemoryProperty;
    VkMemoryRequirements MemoryRequirements;
  };

  template<typename VkObjectT>
  class PoolManager;

  template<typename VkObject>
  class Pool
  {
  public:
    typedef std::unique_ptr<Pool<VkObject>> PoolPtr;
    virtual ~Pool() {}

    static PoolPtr Create(
      VkDevice device,
      const VkDeviceSize poolSize,
      const typename ResourceManager::ResDesc<VkObject>& objDesc);

    uint32 GetMemoryTypeIndex() const { return m_MemoryTypeIndex; }
    VkDeviceSize GetSize() const { return m_Size; }
    bool HasAvailable(VkMemoryRequirements memReqs) const;

    Allocation Allocate(
      const typename ResourceManager::ResDesc<VkObject>& objDesc);
    friend class PoolManager<VkObject>;

  private:
    Pool(uint32 memTypeIndex, VkDeviceMemory mem, VkDeviceSize sz);

    std::vector<Allocation> m_Allocations;

    uint32 m_MemoryTypeIndex = UINT32_MAX;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    VkDeviceSize m_Size = 0;
    VkDeviceSize m_Offset = 0;
  };

  template<typename VkObjectT>
  class PoolManager : public DeviceChild
  {
  public:
    using PoolRef = std::unique_ptr<Pool<VkObjectT>>;

    PoolManager(DeviceRef pDevice, VkDeviceSize poolSize);
    virtual ~PoolManager();
    void Destroy();
    Allocation Allocate(
      const typename ResourceManager::ResDesc<VkObjectT>& objDesc);

    VkDeviceSize GetBlockSize() const { return m_PoolSize; }

  private:
    VkDevice m_Device = VK_NULL_HANDLE;
    size_t m_PoolSize = 0;
    ::Os::Mutex m_Mutex;
    std::vector<PoolRef> m_Pools;
  };

  explicit ResourceManager(DeviceRef pDevice,
                           size_t bufferBlockSize,
                           size_t imageBlockSize);

  ~ResourceManager();

  Allocation AllocateBuffer(VkBuffer buffer,
                            bool transient,
                            VkMemoryPropertyFlags memoryProperty);
  Allocation AllocateImage(VkImage image,
                           bool transient,
                           VkMemoryPropertyFlags memoryProperty);

private:
  void Initialize();
  void Destroy();

  PoolManager<VkBuffer> m_BufferAllocations;
  PoolManager<VkImage> m_ImageAllocations;
};

/**
 * TODO: Need a Renderpass manager to cache all renderpasses
 * [framebuffer, attachments, depth, stencil
 */
class RenderTarget
  : public DeviceChild
  , public NGFXRenderTarget
{
public:
  RenderTarget(DeviceRef pDevice,
               TextureImpl::TextureRef texture,
               FrameBufferRef framebuffer,
               VkRenderPass renderpass);
  ~RenderTarget() override;

//  static void CreateFromSwapchain();

  VkFramebuffer GetFramebuffer() const;
  VkRenderPass GetRenderpass() const;
  TextureImpl::TextureRef GetTexture() const;
  VkRect2D GetRenderArea() const;
  NGFXResourceRef GetBackBuffer() override;
  SemaphoreRef GetSemaphore() { return m_AcquireSemaphore; }

  void SetClearColor(kMath::Vec4f clrColor) override
  {
    m_ClearValues[0].color = {
      clrColor[0], clrColor[1], clrColor[2], clrColor[3]
    };
  }
  void SetClearDepthStencil(float depth, uint32 stencil) override
  {
    m_ClearValues[1].depthStencil = { depth, stencil };
  }

private:
  friend class CommandContext;

  VkClearValue m_ClearValues[2]/* = { {}, { 1.0f, 0 } }*/;
  
  VkFramebuffer m_Framebuffer;
  VkRenderPass m_Renderpass;

  SemaphoreRef m_AcquireSemaphore;
};

class CommandQueueImpl
  : public TVkRHIObjectBase<VkQueue, NGFXCommandQueue>
{
  friend class DeviceImpl;
public:
  using This = TVkRHIObjectBase<VkQueue, NGFXCommandQueue>;
  CommandQueueImpl(DeviceRef pDevice,
               VkQueueFlags queueTypes,
               uint32 queueFamilyIndex,
               uint32 queueIndex);
  virtual ~CommandQueueImpl();

  NGFXCommandBufferRef ObtainCommandBuffer(
    NGFXCommandReuseType const&) override;

  void Submit(const std::vector<VkCommandBuffer>& cmdBufs,
              const std::vector<VkSemaphore>& waitSemaphores,
              const std::vector<VkPipelineStageFlags>& waitStageMasks,
              VkFence fence,
              const std::vector<VkSemaphore>& signalSemaphores);

  CommandBufferRef ObtainSecondaryCommandBuffer();

  VkResult Submit(const std::vector<VkSubmitInfo>& submits, VkFence fence);

  void Present(SwapChainRef& pSwapChain);

  void WaitIdle();

protected:
  void Initialize(VkQueueFlags queueTypes,
                  uint32 queueFamilyIndex,
                  uint32 queueIndex);
  void Destroy();

private:
  VkQueueFlags m_QueueTypes = 0;
  uint32 m_QueueFamilyIndex = UINT32_MAX;
  uint32 m_QueueIndex = UINT32_MAX;

  CmdBufManagerRef m_TransientCmdBufferPool;
  CmdBufManagerRef m_ReUsableCmdBufferPool;
  CmdBufManagerRef m_SecondaryCmdBufferPool;
};

class CommandBufferImpl
  : public TVkRHIObjectBase<VkCommandBuffer, NGFXCommandBuffer>
{
public:
  using This = TVkRHIObjectBase<VkCommandBuffer, NGFXCommandBuffer>;

  void Release();

  void Commit(NGFXFenceRef pFence) override;
  void Present(NGFXSwapChainRef pSwapChain, NGFXFenceRef pFence) override;

  void Reset() override;
  NGFXRenderCommandEncoderRef RenderCommandEncoder(
    NGFXRenderPassDesc const&) override;
  NGFXComputeCommandEncoderRef ComputeCommandEncoder() override;
  NGFXParallelRenderCommandEncoderRef ParallelRenderCommandEncoder(
    NGFXRenderPassDesc const&) override;
  void CopyTexture(const NGFXTextureCopyLocation& Dest,
                   const NGFXTextureCopyLocation& Src) override;
  void CopyBuffer(NGFXResourceRef Dest,
                  NGFXResourceRef Src,
                  NGFXCopyBufferRegion const& Region) override;
  void Transition(NGFXResourceRef pResource,
                  NGFXResourceState const&
                    State /*, k3d::EPipelineStage const& Stage*/) override;

  CommandQueueRef OwningQueue() { return m_OwningQueue; }

  friend class CommandQueueImpl;
  template<typename T>
  friend class CommandEncoder;

protected:
  template<typename T>
  friend class k3d::TRefCountInstance;

  CommandBufferImpl(DeviceRef, CommandQueueRef, VkCommandBuffer);

  bool m_Ended;
  CommandQueueRef m_OwningQueue;
  NGFXSwapChainRef m_PendingSwapChain = nullptr;
};

class ComputePipelineStateImpl;

template<typename CmdEncoderSubT>
class CommandEncoder : public CmdEncoderSubT
{
public:
  explicit CommandEncoder(CommandBufferRef pCmd)
    : m_MasterCmd(pCmd)
  {
    m_pQueue = m_MasterCmd->OwningQueue();
  }

  void SetPipelineState(uint32 HashCode,
                        NGFXPipelineStateRef const& pPipeline) override;

  void SetBindingGroup(NGFXBindingGroupRef const& pBindingGroup) override
  {
    K3D_ASSERT(pBindingGroup);
    auto pDescSet = static_cast<BindingGroupImpl*>(pBindingGroup);
    VkDescriptorSet sets[] = { pDescSet->NativeHandle() };
    vkCmdBindDescriptorSets(m_MasterCmd->NativeHandle(),
                            GetBindPoint(),
                            pDescSet->m_RootLayout->NativeHandle(),
                            0, // first set
                            1,
                            sets, // set count,
                            0,
                            NULL); // dynamic offset
  }

  virtual void EndEncode() override
  {
    vkEndCommandBuffer(m_MasterCmd->NativeHandle());
    m_MasterCmd->m_Ended = true;
  }

  virtual VkPipelineBindPoint GetBindPoint() const = 0;

protected:
  CommandBufferRef m_MasterCmd;
  CommandQueueRef m_pQueue;
};

enum class ECmdLevel : uint8
{
  Primary,
  Secondary
};

class RenderCommandEncoderImpl : public CommandEncoder<NGFXRenderCommandEncoder>
{
public:
  using This = CommandEncoder<NGFXRenderCommandEncoder>;
  void SetScissorRect(const NGFXRect&) override;
  void SetViewport(const NGFXViewportDesc&) override;
  void SetIndexBuffer(const NGFXIndexBufferView& IBView) override;
  void SetVertexBuffer(uint32 Slot,
                       const NGFXVertexBufferView& VBView) override;
  void SetPrimitiveType(NGFXPrimitiveType) override;
  void DrawInstanced(NGFXDrawInstancedParam) override;
  void DrawIndexedInstanced(NGFXDrawIndexedInstancedParam) override;
  void EndEncode() override;
  VkPipelineBindPoint GetBindPoint() const override
  {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
  }
  VkCommandBuffer OwningCmd() const { return m_MasterCmd->NativeHandle(); }
  friend class CommandBufferImpl;
  friend class ParallelCommandEncoderImpl;
  template<typename T>
  friend class k3d::TRefCountInstance;
protected:
  RenderCommandEncoderImpl(CommandBufferRef pCmd, ECmdLevel Level);
  RenderCommandEncoderImpl(ParallelCommandEncoderRef ParentEncoder, CommandBufferRef pCmd);

private:
  ECmdLevel m_Level;
};
using SpRenderCommandEncoder = SharedPtr<RenderCommandEncoderImpl>;
class ComputeCommandEncoderImpl : public CommandEncoder<NGFXComputeCommandEncoder>
{
  using Super = CommandEncoder<NGFXComputeCommandEncoder>;
public:
  void Dispatch(uint32 GroupCountX,
                uint32 GroupCountY,
                uint32 GroupCountZ) override;
  VkPipelineBindPoint GetBindPoint() const override
  {
    return VK_PIPELINE_BIND_POINT_COMPUTE;
  }
  friend class CommandBufferImpl;
  template<typename T>
  friend class k3d::TRefCountInstance;
private:
  ComputeCommandEncoderImpl(CommandBufferRef pCmd);
};

class ParallelCommandEncoderImpl
  : public CommandEncoder<NGFXParallelRenderCommandEncoder>
{
public:
  using Super = CommandEncoder<NGFXParallelRenderCommandEncoder>;
  NGFXRenderCommandEncoderRef SubRenderCommandEncoder() override;
  void EndEncode() override;
  VkPipelineBindPoint GetBindPoint() const override
  {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
  }
  friend class CommandBufferImpl;
  template<typename T>
  friend class k3d::TRefCountInstance;
private:
  ParallelCommandEncoderImpl(CommandBufferRef PrimaryCmdBuffer);
  void SubAllocateSecondaryCmd();
  bool m_RenderpassBegun = false;
  DynArray<SpRenderCommandEncoder> m_RecordedCmds;
};

class SwapChainImpl : public TVkRHIObjectBase<VkSwapchainKHR, NGFXSwapChain>
{
  friend class CommandQueueImpl;
  friend class CommandBufferImpl;
public:
  using Super = TVkRHIObjectBase<VkSwapchainKHR, NGFXSwapChain>;

  SwapChainImpl(DeviceRef pDevice, void* pWindow, NGFXSwapChainDesc& Desc);
  ~SwapChainImpl();

  // NGFXSwapChain::Release
  void Release();
  // NGFXSwapChain::Resize
  void Resize(uint32 Width, uint32 Height) override;
  // NGFXSwapChain::GetCurrentTexture
  NGFXTextureRef GetCurrentTexture() override;

  void Present() override;

  void QueuePresent(CommandQueueRef pQueue, NGFXFenceRef pFence);

  void AcquireNextImage();

  void Init(void* pWindow, NGFXSwapChainDesc& Desc);

  uint32 GetPresentQueueFamilyIndex() const
  {
    return m_SelectedPresentQueueFamilyIndex;
  }

  uint32 GetBackBufferCount() const { return m_ReserveBackBufferCount; }
  VkImage GetBackImage(uint32 i) const { return m_ColorImages[i]; }
  VkExtent2D GetCurrentExtent() const { return m_CachedCreateInfo.imageExtent; }
  VkFormat GetFormat() const { return m_CachedCreateInfo.imageFormat; }

private:
  void InitSurface(void* WindowHandle);
  VkPresentModeKHR ChoosePresentMode();
  int ChooseQueueIndex();

private:

  VkSemaphore m_smpRenderFinish = VK_NULL_HANDLE;
  VkSemaphore m_smpPresent = VK_NULL_HANDLE;
  VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

  uint32 m_CurrentBufferID = 0;
  uint32 m_SelectedPresentQueueFamilyIndex = 0;

  uint32 m_ReserveBackBufferCount = 0;

  VkSwapchainCreateInfoKHR m_CachedCreateInfo;

  DynArray<NGFXTextureRef> m_Buffers;
  DynArray<VkImage> m_ColorImages;
};

class RenderPassImpl;
class FrameBufferImpl
{
public:
  /**
    * create framebuffer with SwapChain ImageViews
    */
  FrameBufferImpl(DeviceRef pDevice, RenderPassRef pRenderPass, NGFXRenderPassDesc const&);
  ~FrameBufferImpl();

  uint32 GetWidth() const { return m_Width; }
  uint32 GetHeight() const { return m_Height; }
  VkFramebuffer NativeHandle() const { return m_FrameBuffer; }

private:
  friend class RenderPassImpl;
  RenderPassRef m_OwningRenderPass = nullptr;
  DeviceRef m_Device;
  VkFramebuffer m_FrameBuffer = VK_NULL_HANDLE;
  uint32 m_Width = 0;
  uint32 m_Height = 0;
  bool m_HasDepthStencil = false;
};

class RenderPassImpl : public TVkRHIObjectBase<VkRenderPass, NGFXRenderpass>
{
public:
  using Super = TVkRHIObjectBase<VkRenderPass, NGFXRenderpass>;

  RenderPassImpl(DeviceRef pDevice, NGFXRenderPassDesc const&);

  void Begin(VkCommandBuffer Cmd, FrameBufferRef pFramebuffer,
              NGFXRenderPassDesc const& Desc,
              VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
  void End(VkCommandBuffer Cmd);

  NGFXRenderPassDesc GetDesc() const override
  {
    return NGFXRenderPassDesc();
  }

  ~RenderPassImpl();

  void Release();

private:
  VkClearValue m_ClearVal[2];
  VkRect2D  m_DefaultArea;
};

template<typename PipelineSubType>
class TPipelineState : public PipelineSubType
{
public:
  TPipelineState(DeviceRef pDevice)
    : m_Device(pDevice)
    , m_Pipeline(VK_NULL_HANDLE)
    , m_PipelineCache(VK_NULL_HANDLE)
  {
  }

  virtual ~TPipelineState() override
  {
    vkDestroyPipelineCache(m_Device->GetRawDevice(), m_PipelineCache, nullptr);
    vkDestroyPipeline(m_Device->GetRawDevice(), m_Pipeline, nullptr);
  }

  void SavePSO(String const& Path) override {}

  void LoadPSO(String const& Path) override {}

  VkPipeline NativeHandle() const { return m_Pipeline; }

protected:
  VkPipelineShaderStageCreateInfo ConvertStageInfoFromShaderBundle(
    NGFXShaderBundle const& Bundle);

  DeviceRef m_Device;

  VkPipeline m_Pipeline;
  VkPipelineCache m_PipelineCache;
};

class RenderPipelineStateImpl : public TPipelineState<NGFXRenderPipeline>
{
  friend class DeviceImpl;
public:
  RenderPipelineStateImpl(DeviceRef pDevice,
                      NGFXRenderPipelineDesc const& desc,
                      NGFXPipelineLayoutRef ppl,
                      NGFXRenderpassRef pRenderPass);
  virtual ~RenderPipelineStateImpl();

  void BindRenderPass(VkRenderPass RenderPass);

  void SetRasterizerState(const NGFXRasterizerState&) override;
  void SetBlendState(const NGFXBlendState&) override;
  void SetDepthStencilState(const NGFXDepthStencilState&) override;
  void SetSampler(NGFXSamplerRef) override;
  void SetPrimitiveTopology(const NGFXPrimitiveType) override;

  VkPipeline GetPipeline() const { return m_Pipeline; }
  void Rebuild() override;

  /**
   * TOFIX
   */
  NGFXPipelineType GetType() const override
  {
    return NGFXPipelineType::NGFX_PIPELINE_GRAPHICS;
  }

protected:
  void InitWithDesc(NGFXRenderPipelineDesc const& desc);
  void Destroy();

  friend class CommandContext;

  VkGraphicsPipelineCreateInfo m_GfxCreateInfo;
  VkRenderPass m_RenderPass;

private:
  DynArray<VkPipelineShaderStageCreateInfo> m_ShaderStageInfos;
  VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
  VkPipelineRasterizationStateCreateInfo m_RasterizationState;
  VkPipelineColorBlendStateCreateInfo m_ColorBlendState;
  VkPipelineDepthStencilStateCreateInfo m_DepthStencilState;
  VkPipelineViewportStateCreateInfo m_ViewportState;
  VkPipelineMultisampleStateCreateInfo m_MultisampleState;
  VkPipelineVertexInputStateCreateInfo m_VertexInputState;
  std::vector<VkVertexInputBindingDescription> m_BindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
  NGFXRenderpassRef m_WeakRenderPassRef = nullptr;
  NGFXPipelineLayoutRef m_weakPipelineLayoutRef = nullptr;
};

class ComputePipelineStateImpl : public TPipelineState<NGFXComputePipeline>
{
public:
  ComputePipelineStateImpl(DeviceRef pDevice,
                       NGFXComputePipelineDesc const& desc,
                       PipelineLayoutImpl* ppl);

  ~ComputePipelineStateImpl() override {}

  NGFXPipelineType GetType() const override
  {
    return NGFXPipelineType::NGFX_PIPELINE_Compute;
  }
  void Rebuild() override;

  friend class CommandContext;
  friend class DeviceImpl;
private:
  VkComputePipelineCreateInfo m_ComputeCreateInfo;
  PipelineLayoutImpl* m_PipelineLayout;
};

class ShaderResourceView
  : public TVkRHIObjectBase<VkImageView, NGFXShaderResourceView>
{
public:
  ShaderResourceView(DeviceRef pDevice,
                      NGFXSRVDesc const& desc,
                      NGFXResourceRef gpuResource);
  ~ShaderResourceView() override;

  NGFXResourceRef GetResource() const override
  {
    return NGFXResourceRef(m_WeakResource);
  }

  NGFXSRVDesc GetDesc() const override { return m_Desc; }
  VkImageView NativeImageView() const { return m_NativeObj; }

private:
  NGFXResourceRef m_WeakResource;
  NGFXSRVDesc m_Desc;
  VkImageViewCreateInfo m_TextureViewInfo;
};

class UnorderedAceessView : public NGFXUnorderedAccessView
{
public:
  UnorderedAceessView(DeviceRef pDevice, NGFXUAVDesc const& Desc, const NGFXResourceRef& pResource);
  ~UnorderedAceessView() override;
  friend class BindingGroupImpl;
private:
  DeviceRef m_pDevice;
  NGFXUAVDesc m_Desc;
  VkImageView m_ImageView;
  VkBufferView m_BufferView;
};

class CommandAllocator : public DeviceChild
{
public:
  static CommandAllocatorRef CreateAllocator(uint32 queueFamilyIndex,
                                     bool transient,
                                     DeviceRef device);

  ~CommandAllocator();

  VkCommandPool GetCommandPool() const { return m_Pool; }

  void Destroy();

protected:
  void Initialize();

private:
  CommandAllocator(uint32 queueFamilyIndex, bool transient, DeviceRef device);

  VkCommandPool m_Pool;
  bool m_Transient;
  uint32 m_FamilyIndex;
};

struct SemaphoreCreateInfo : public VkSemaphoreCreateInfo
{
public:
  static VkSemaphoreCreateInfo Create()
  {
    return { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
  }
};

class SemaphoreImpl : public DeviceChild
{
public:
  SemaphoreImpl(DeviceRef pDevice,
            VkSemaphoreCreateInfo const& info = SemaphoreCreateInfo::Create())
    : DeviceChild(pDevice)
  {
    K3D_VK_VERIFY(
      vkCreateSemaphore(GetRawDevice(), &info, nullptr, &m_Semaphore));
    VKLOG(Info, "Semaphore Created. (0x%0x).", m_Semaphore);
  }

  ~SemaphoreImpl()
  {
    VKLOG(Info, "Semaphore Destroyed. (0x%0x).", m_Semaphore);
    vkDestroySemaphore(GetRawDevice(), m_Semaphore, nullptr);
  }

  VkSemaphore GetNativeHandle() const { return m_Semaphore; }

private:
  friend class SwapChainImpl;
  friend class CommandContext;

  VkSemaphore m_Semaphore;
};


template<typename CmdEncoderSubT>
void CommandEncoder<CmdEncoderSubT>::SetPipelineState(uint32 HashCode, NGFXPipelineStateRef const &pPipeline)
{
  K3D_ASSERT(pPipeline);
  if (pPipeline->GetType() == NGFX_PIPELINE_Compute) {
    const ComputePipelineStateImpl* computePso =
            static_cast<const ComputePipelineStateImpl*>(pPipeline);
    vkCmdBindPipeline(m_MasterCmd->NativeHandle(),
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      computePso->NativeHandle());
  } else {
    const RenderPipelineStateImpl* gfxPso =
            static_cast<const RenderPipelineStateImpl*>(pPipeline);
    vkCmdBindPipeline(m_MasterCmd->NativeHandle(),
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      gfxPso->NativeHandle());
  }
}

K3D_VK_END

#endif