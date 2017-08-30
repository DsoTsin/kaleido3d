#include "VkCommon.h"
#include "Public/IVkRHI.h"
#include "VkRHI.h"
#include <RHI/RHIUtil.h>
#include <Core/Module.h>
#include <Core/Os.h>
#include <Core/Utils/farmhash.h>
#include <Core/WebSocket.h>
#include <Log/Public/ILogModule.h>
#include <cstdarg>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

using namespace std;

PFN_vklogCallBack g_logCallBack = nullptr;

static thread_local char logContent[2048] = { 0 };
#define LOG_SERVER 1

#if K3DPLATFORM_OS_ANDROID
#define OutputDebugStringA(str)                                                \
  __android_log_print(ANDROID_LOG_DEBUG, "VkRHI", "%s", str);
#endif

void
VkLog(k3d::ELogLevel const& Lv, const char* tag, const char* fmt, ...)
{
  if (g_logCallBack != nullptr) {
    va_list va;
    va_start(va, fmt);
    g_logCallBack(Lv, tag, fmt, va);
    va_end(va);
  } else {
#if LOG_SERVER
    va_list va;
    va_start(va, fmt);
    vsprintf(logContent, fmt, va);
    va_end(va);

    auto logModule = k3d::StaticPointerCast<k3d::ILogModule>(
      k3d::GlobalModuleManager.FindModule("KawaLog"));
    if (logModule) {
      k3d::ILogger* logger = logModule->GetLogger(k3d::ELoggerType::EWebsocket);
      if (logger) {
        logger->Log(Lv, tag, logContent);
      }

      if (Lv >= k3d::ELogLevel::Debug) {
        logger = logModule->GetLogger(k3d::ELoggerType::EConsole);
        if (logger) {
          logger->Log(Lv, tag, logContent);
        }
      }

      if (Lv >= k3d::ELogLevel::Warn) {
        logger = logModule->GetLogger(k3d::ELoggerType::EFile);
        if (logger) {
          logger->Log(Lv, tag, logContent);
        }
      }
    }
#else
    va_list va;
    va_start(va, fmt);
    vsprintf(logContent, fmt, va);
    va_end(va);
    OutputDebugStringA(logContent);
#endif
  }
}

void
SetVkLogCallback(PFN_vklogCallBack func)
{
  g_logCallBack = func;
}

K3D_VK_BEGIN

namespace
{
  int g_ValidationLayerCount = 1;

  const char* g_ValidationLayerNames[] = {
    "VK_LAYER_LUNARG_standard_validation",
  };

  VkShaderStageFlagBits g_ShaderType[] = {
    VK_SHADER_STAGE_FRAGMENT_BIT,
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_COMPUTE_BIT
  };

  VkPrimitiveTopology g_PrimitiveTopology[NGFXPrimitiveType::NGFX_PRIMITIVE_NUM] = {
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
  };

  VkVertexInputRate g_InputRates[] = {
    VK_VERTEX_INPUT_RATE_VERTEX,
    VK_VERTEX_INPUT_RATE_INSTANCE,
  };

  VkCullModeFlagBits g_CullMode[] =
  { VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT };

  VkPolygonMode g_FillMode[] = {
    VK_POLYGON_MODE_LINE,
    VK_POLYGON_MODE_FILL
  };

  VkStencilOp g_StencilOp[] = {
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INVERT,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP
  };

  VkBlendOp g_BlendOps[] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
  };

  VkBlendFactor g_Blend[] = {
    VK_BLEND_FACTOR_ZERO,      VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_DST_ALPHA
  };

  VkCompareOp g_ComparisonFunc[] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS
  };
  /*
  NGFX_PIXEL_FORMAT_RGBA16_UINT,
  NGFX_PIXEL_FORMAT_RGBA32_FLOAT,
  NGFX_PIXEL_FORMAT_RGBA8_UNORM,
  NGFX_PIXEL_FORMAT_RGBA8_UNORM_SRGB,
  NGFX_PIXEL_FORMAT_R11_G11_B10_FLOAT,
  NGFX_PIXEL_FORMAT_D32_FLOAT,
  NGFX_PIXEL_FORMAT_RGB32_FLOAT,
  NGFX_PIXEL_FORMAT_RGB8_UNORM,
  NGFX_PIXEL_FORMAT_BGRA8_UNORM, // Apple Metal Layer uses it as default pixel format
  NGFX_PIXEL_FORMAT_BGRA8_UNORM_SRGB,
  NGFX_PIXEL_FORMAT_RGBA16_FLOAT,
  */
  VkFormat g_FormatTable[NGFXPixelFormat::NGFX_PIXEL_FORMAT_NUM] = {
    VK_FORMAT_R16G16B16A16_UINT,       VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R8G8B8A8_UNORM,          VK_FORMAT_R8G8B8A8_SNORM,
    VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,        VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,          VK_FORMAT_B8G8R8A8_SNORM,
    VK_FORMAT_R16G16B16A16_SFLOAT,     VK_FORMAT_D24_UNORM_S8_UINT
  };

  VkFormat g_VertexFormatTable[] = { VK_FORMAT_R32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT };

  VkImageLayout g_ResourceState[] = {
    VK_IMAGE_LAYOUT_GENERAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_UNDEFINED
  };

  //			VK_BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001,
  //			VK_BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002,
  //			VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000004,
  //			VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT = 0x00000008,
  //			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
  //			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
  //			VK_BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
  //			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
  //			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100,
  /**
  * NGFX_RESOURCE_VERTEX_BUFFER_VIEW, // For VertexBuffer
  * NGFX_RESOURCE_INDEX_BUFFER_VIEW, // For IndexBuffer
  * NGFX_RESOURCE_CONSTANT_BUFFER_VIEW, // For ConstantBuffer
  * NGFX_RESOURCE_SHADER_RESOURCE_VIEW, // For Texture
  * NGFX_RESOURCE_UNORDERED_ACCESS_VIEW, // For Buffer
  * NGFX_RESOURCE_RENDER_TARGET_VIEW,
  * NGFX_RESOURCE_DEPTH_STENCIL_VIEW,
  * EGVT_Sampler,
  * EGVT_SOV,
  */
  VkFlags g_ResourceViewFlag[] = { 0,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
    VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
    0,
    0,
    0,
    0 };

  //@see  SamplerState::NGFXFilterMethod
  VkFilter g_Filters[] = { VK_FILTER_NEAREST, VK_FILTER_LINEAR };

  VkSamplerMipmapMode g_MipMapModes[] = {
    VK_SAMPLER_MIPMAP_MODE_NEAREST,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
  };

  // @see  SamplerState::NGFXAddressMode
  //	Wrap,
  //	Mirror, // Repeat
  //	Clamp,
  //	Border,
  //	MirrorOnce,
  VkSamplerAddressMode g_AddressModes[] = {
    VK_SAMPLER_ADDRESS_MODE_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
  };

  VkAttachmentLoadOp g_LoadAction[] = { VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE };

  VkAttachmentStoreOp g_StoreAction[] = {
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE,
  };

  NGFXPixelFormat
    ConvertToRHIFormat(VkFormat format)
  {
    switch (format) {
    case VK_FORMAT_R16G16B16A16_UINT:
      return NGFX_PIXEL_FORMAT_RGBA16_UINT;
    case VK_FORMAT_R8G8B8A8_UNORM:
      return NGFX_PIXEL_FORMAT_RGBA8_UNORM;
    case VK_FORMAT_R8G8B8A8_SNORM:
      return NGFX_PIXEL_FORMAT_RGBA8_UNORM_SRGB;
    case VK_FORMAT_B8G8R8A8_UNORM:
      return NGFX_PIXEL_FORMAT_BGRA8_UNORM;
    case VK_FORMAT_D32_SFLOAT:
      return NGFX_PIXEL_FORMAT_D32_FLOAT;
    case VK_FORMAT_B8G8R8A8_SNORM:
      return NGFX_PIXEL_FORMAT_BGRA8_UNORM_SRGB;
    }
    return NGFX_PIXEL_FORMAT_NUM;
  }

  VkImageViewType ConvertFromTextureType(NGFXResourceType const& Type)
  {
    switch (Type)
    {
    case NGFX_TEXTURE_1D:
      return VK_IMAGE_VIEW_TYPE_1D;
    case NGFX_TEXTURE_1D_ARRAY:
      return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case NGFX_TEXTURE_2DMS:
    case NGFX_TEXTURE_2D:
      return VK_IMAGE_VIEW_TYPE_2D;
    case NGFX_TEXTURE_2DMS_ARRAY:
    case NGFX_TEXTURE_2D_ARRAY:
      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case NGFX_TEXTURE_3D:
      return VK_IMAGE_VIEW_TYPE_3D;
    case NGFX_TEXTURE_CUBE:
      return VK_IMAGE_VIEW_TYPE_CUBE;
    }
    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
  }
}


LRUCache<uint64, FrameBufferRef, 4> DeviceObjectCache::s_CacheFrameBuffer;
MapFramebuffer DeviceObjectCache::s_Framebuffer;
MapRenderpass DeviceObjectCache::s_RenderPass;

VkDeviceSize
CalcAlignedOffset(VkDeviceSize offset, VkDeviceSize align)
{
  VkDeviceSize n = offset / align;
  VkDeviceSize r = offset % align;
  VkDeviceSize result = (n + (r > 0 ? 1 : 0)) * align;
  return result;
}
// Buffer functors
decltype(vkCreateBufferView)* ResTrait<NGFXBuffer>::CreateView =
&vkCreateBufferView;
decltype(vkDestroyBufferView)* ResTrait<NGFXBuffer>::DestroyView =
&vkDestroyBufferView;
decltype(vkCreateBuffer)* ResTrait<NGFXBuffer>::Create = &vkCreateBuffer;
decltype(vkDestroyBuffer)* ResTrait<NGFXBuffer>::Destroy = &vkDestroyBuffer;
decltype(vkGetBufferMemoryRequirements)* ResTrait<NGFXBuffer>::GetMemoryInfo =
&vkGetBufferMemoryRequirements;
decltype(vkBindBufferMemory)* ResTrait<NGFXBuffer>::BindMemory =
&vkBindBufferMemory;

decltype(vkCreateImageView)* ResTrait<NGFXTexture>::CreateView = &vkCreateImageView;
decltype(vkDestroyImageView)* ResTrait<NGFXTexture>::DestroyView =
&vkDestroyImageView;
decltype(vkCreateImage)* ResTrait<NGFXTexture>::Create = &vkCreateImage;
decltype(vkDestroyImage)* ResTrait<NGFXTexture>::Destroy = &vkDestroyImage;
decltype(vkGetImageMemoryRequirements)* ResTrait<NGFXTexture>::GetMemoryInfo =
&vkGetImageMemoryRequirements;
decltype(vkBindImageMemory)* ResTrait<NGFXTexture>::BindMemory = &vkBindImageMemory;

BufferImpl::BufferImpl(DeviceRef pDevice, NGFXResourceDesc const& desc)
  : Super(pDevice, desc)
{
  // TODO, 
  m_ResUsageFlags = g_ResourceViewFlag[desc.ViewFlags];

  if (desc.CreationFlag & NGFX_RESOURCE_TRANSFER_SRC) {
    m_ResUsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  if (desc.CreationFlag & NGFX_RESOURCE_TRANSFER_DST) {
    m_ResUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  m_ResUsageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

  if (desc.Flag & NGFX_ACCESS_HOST_VISIBLE) {
    m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }
  if (desc.Flag & NGFX_ACCESS_DEVICE_VISIBLE) {
    m_MemoryBits |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  }
  if (desc.Flag & NGFX_ACCESS_HOST_COHERENT) {
    m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if (desc.ViewFlags == NGFX_RESOURCE_VERTEX_BUFFER_VIEW)
  {
    m_UsageState = NGFX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  }
  Create(desc.Size);
}

BufferImpl::~BufferImpl()
{
  VKLOG(Info, ">>>>> BufferImpl Destroying.. 0x%0x.", m_NativeObj);
}

void
BufferImpl::Create(size_t size)
{
  VkBufferCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.size = size;
  createInfo.usage = m_ResUsageFlags;
  createInfo.flags = 0;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 0;
  createInfo.pQueueFamilyIndices = nullptr;

  Super::Allocate(createInfo);

  m_ResDescInfo.buffer = m_NativeObj;
  m_ResDescInfo.offset = 0;
  m_ResDescInfo.range = m_MemAllocInfo.allocationSize;

  K3D_VK_VERIFY(
    vkBindBufferMemory(NativeDevice(), m_NativeObj, m_DeviceMem, 0));
}

TextureImpl::TextureImpl(DeviceRef pDevice, NGFXResourceDesc const& desc)
  : TextureImpl::Super(pDevice, desc)
{
  InitCreateInfos();
}

TextureImpl::TextureImpl(NGFXResourceDesc const& Desc,
  VkImage image,
  // VkImageView imageView,
  DeviceRef pDevice,
  bool selfOwnShip)
  : TextureImpl::Super(pDevice, selfOwnShip)
{
  m_ResDesc = Desc;
  m_NativeObj = image;
  //  m_ResView = imageView;
  m_ImageViewInfo = {};
  m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
  K3D_ASSERT(m_ResDesc.Origin == NGFX_RESOURCE_ORIGIN_SWAPCHAIN);
  CreateViewForSwapchainImage();
#if 0
  auto Cmd = m_Device->AllocateImmediateCommand();

  VkCommandBufferBeginInfo BeginInfo = {};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.pNext = nullptr;
  BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  BeginInfo.pInheritanceInfo = nullptr;
  vkBeginCommandBuffer(Cmd, &BeginInfo);
  VkImageLayout SrcLayout = g_ResourceState[GetState()];
  VkImageLayout DestLayout = g_ResourceState[NGFX_RESOURCE_STATE_RENDER_TARGET];
  VkImageMemoryBarrier ImageBarrier = {
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    nullptr,
    0,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    SrcLayout,
    DestLayout,
    VK_QUEUE_FAMILY_IGNORED,
    VK_QUEUE_FAMILY_IGNORED,
    m_NativeObj,
    m_SubResRange
  };
  vkCmdPipelineBarrier(Cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    0, // DependencyFlag
    0,
    nullptr,
    0,
    nullptr,
    1,
    &ImageBarrier);
  vkEndCommandBuffer(Cmd);

  VkFenceCreateInfo fenceCreateInfo = {};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = 0;
  VkFence fence;
  K3D_VK_VERIFY(vkCreateFence(NativeDevice(), &fenceCreateInfo, nullptr, &fence));
  VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &Cmd;
  K3D_VK_VERIFY(vkQueueSubmit(m_Device->GetImmQueue(), 1, &SubmitInfo, fence));
  vkQueueWaitIdle(m_Device->GetImmQueue());
  K3D_VK_VERIFY(vkWaitForFences(NativeDevice(), 1, &fence, VK_TRUE, 1000));
  vkDestroyFence(NativeDevice(), fence, nullptr);
  vkFreeCommandBuffers(NativeDevice(), ImmCmdPool(), 1, &Cmd);
  VKLOG(Info, "Transition finished.");
  m_UsageState = NGFX_RESOURCE_STATE_RENDER_TARGET;
#endif
}

void
TextureImpl::BindSampler(NGFXSamplerRef sampler)
{
  m_ImageSampler = static_cast<SamplerImpl*>(sampler);
}

NGFXSamplerRef
TextureImpl::GetSampler() const
{
  return m_ImageSampler;
}

void
TextureImpl::CreateResourceView()
{
  // TODO
  m_ImageViewInfo = {
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    m_NativeObj,
    ConvertFromTextureType(m_ResDesc.Type),
    g_FormatTable[m_ResDesc.TextureDesc.Format],
    { VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A },
    m_SubResRange
  };
  K3D_VK_VERIFY(
    vkCreateImageView(NativeDevice(), &m_ImageViewInfo, nullptr, &m_ResView));
}

void
TextureImpl::CreateViewForSwapchainImage()
{
  m_ImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    m_NativeObj,
    VK_IMAGE_VIEW_TYPE_2D,
    g_FormatTable[m_ResDesc.TextureDesc.Format],
    { VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A },
    m_SubResRange };
  K3D_VK_VERIFY(
    vkCreateImageView(NativeDevice(), &m_ImageViewInfo, nullptr, &m_ResView));
}


template<typename PipelineSubType>
VkPipelineShaderStageCreateInfo TPipelineState<PipelineSubType>::ConvertStageInfoFromShaderBundle(
        NGFXShaderBundle const& Bundle)
{
  return { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           nullptr,
           0,
           g_ShaderType[Bundle.Desc.Stage],
           m_Device->CreateShaderModule(Bundle),
           Bundle.Desc.EntryFunction.CStr() };
}


        VkImageCreateInfo ConvertFromTextureDesc(NGFXResourceDesc const& ResDesc)
{
  auto Desc = ResDesc.TextureDesc;
  VkImageCreateInfo Info =
  {
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr,
  };
  switch (ResDesc.Type)
  {
  case NGFX_TEXTURE_1D:
  case NGFX_TEXTURE_1D_ARRAY:
    Info.imageType = VK_IMAGE_TYPE_1D;
    break;
  case NGFX_TEXTURE_2DMS_ARRAY:
  case NGFX_TEXTURE_CUBE:
  case NGFX_TEXTURE_2D_ARRAY:
  case NGFX_TEXTURE_2D:
    Info.imageType = VK_IMAGE_TYPE_2D;
    break;
  case NGFX_TEXTURE_3D:
    Info.imageType = VK_IMAGE_TYPE_3D;
    break;
  }
  Info.format = g_FormatTable[Desc.Format];
  Info.extent = { Desc.Width, Desc.Height, Desc.Depth };
  Info.mipLevels = Desc.MipLevels;
  Info.arrayLayers = Desc.Layers;
  // TODO
  Info.samples = VK_SAMPLE_COUNT_1_BIT;
  // Deferred Set
  Info.usage = 0;

  if (ResDesc.Flag == NGFX_ACCESS_DEVICE_VISIBLE
    || (ResDesc.CreationFlag & NGFX_RESOURCE_TRANSFER_DST)
    ) // texture upload use staging
  {
    Info.tiling = VK_IMAGE_TILING_OPTIMAL;
    Info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }
  else // directly upload
  {
    Info.tiling = VK_IMAGE_TILING_LINEAR;
    Info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  }

  Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  Info.queueFamilyIndexCount = 0;
  Info.pQueueFamilyIndices = nullptr;
  return Info;
}

void TextureImpl::InitCreateInfos()
{
  if (m_ResDesc.CreationFlag & NGFX_RESOURCE_TRANSFER_DST) {
    m_ResUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  if (m_ResDesc.Flag & NGFX_ACCESS_HOST_VISIBLE) {
    m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }
  if (m_ResDesc.Flag & NGFX_ACCESS_DEVICE_VISIBLE) {
    m_MemoryBits |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  }
  if (m_ResDesc.Flag & NGFX_ACCESS_HOST_COHERENT) {
    m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  // Initialize ImageCreateInfo
  m_ImageInfo = ConvertFromTextureDesc(m_ResDesc);
  switch (m_ResDesc.ViewFlags) {
  case NGFX_RESOURCE_SHADER_RESOURCE_VIEW:
    m_ImageInfo.usage = m_ResUsageFlags | VK_IMAGE_USAGE_SAMPLED_BIT;
    m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT,
      0,
      m_ResDesc.TextureDesc.MipLevels,
      0,
      m_ResDesc.TextureDesc.Layers };
    break;
  case NGFX_RESOURCE_RENDER_TARGET_VIEW:
    m_MemoryBits =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT,
      0,
      m_ResDesc.TextureDesc.MipLevels,
      0,
      m_ResDesc.TextureDesc.Layers };
    break;
  case NGFX_RESOURCE_DEPTH_STENCIL_VIEW:
    m_ImageInfo.usage = m_ResUsageFlags | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    m_MemoryBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    m_SubResRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
      0,
      m_ResDesc.TextureDesc.MipLevels,
      0,
      m_ResDesc.TextureDesc.Layers };
    break;
  }
  Super::Allocate(m_ImageInfo);
  K3D_VK_VERIFY(vkBindImageMemory(NativeDevice(), m_NativeObj, m_DeviceMem, 0));
  // TODO Allocate Resource View
  m_ImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    m_NativeObj,
    ConvertFromTextureType(m_ResDesc.Type),
    g_FormatTable[m_ResDesc.TextureDesc.Format],
    { VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A },
    m_SubResRange };
  K3D_VK_VERIFY(
    vkCreateImageView(NativeDevice(), &m_ImageViewInfo, nullptr, &m_ResView));
}

TextureImpl::~TextureImpl()
{
}

ShaderResourceView::ShaderResourceView(DeviceRef pDevice,
  NGFXSRVDesc const& desc,
  NGFXResourceRef pGpuResource)
  : ShaderResourceView::ThisObj(pDevice)
  , m_Desc(desc)
  , m_WeakResource(pGpuResource)
  , m_TextureViewInfo{}
{
  auto resourceDesc = m_WeakResource->GetDesc();
  m_TextureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  m_TextureViewInfo.components = { VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A };

  m_TextureViewInfo.viewType = ConvertFromTextureType(resourceDesc.Type);
  m_TextureViewInfo.format = g_FormatTable[resourceDesc.TextureDesc.Format];
  m_TextureViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  m_TextureViewInfo.subresourceRange.baseMipLevel = 0;
  m_TextureViewInfo.subresourceRange.baseArrayLayer = 0;
  m_TextureViewInfo.subresourceRange.layerCount = 1;
  m_TextureViewInfo.subresourceRange.levelCount =
    resourceDesc.TextureDesc.MipLevels;
  m_TextureViewInfo.image = (VkImage)m_WeakResource->GetLocation();

  K3D_VK_VERIFY(vkCreateImageView(
    NativeDevice(), &m_TextureViewInfo, nullptr, &m_NativeObj));
}

ShaderResourceView::~ShaderResourceView()
{
  // destroy view
  VKLOG(Info, "ShaderResourceView destroying...");
  if (m_NativeObj) {
    vkDestroyImageView(NativeDevice(), m_NativeObj, nullptr);
    m_NativeObj = VK_NULL_HANDLE;
  }
}

UnorderedAceessView::UnorderedAceessView(DeviceRef pDevice, NGFXUAVDesc const& Desc, const NGFXResourceRef& pResource)
  : m_pDevice(pDevice)
  , m_Desc(Desc)
{
  switch (m_Desc.Dim)
  {
  case NGFX_VIEW_DIMENSION_BUFFER:
  {
    auto pBuffer = static_cast<const BufferImpl*>(pResource);
    VkBufferViewCreateInfo Info = {
      VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO
    };
    Info.buffer = pBuffer->NativeHandle();
    Info.offset = Desc.Buffer.FirstElement * Desc.Buffer.StructureByteStride;
    Info.range = Desc.Buffer.NumElements * Desc.Buffer.StructureByteStride;
    Info.format = g_FormatTable[Desc.Format];
    Info.flags = 0;
    K3D_VK_VERIFY(vkCreateBufferView(m_pDevice->GetRawDevice(), &Info, nullptr, &m_BufferView));
    break;
  }
  }
}

UnorderedAceessView::~UnorderedAceessView()
{
  switch (m_Desc.Dim)
  {
  case NGFX_VIEW_DIMENSION_BUFFER:
  {
    VKLOG(Info, "UAV (Buffer) Destroyed . 0x%0x.", m_BufferView);
    vkDestroyBufferView(m_pDevice->GetRawDevice(), m_BufferView, nullptr);
    break;
  }
  }
}

SamplerImpl::SamplerImpl(DeviceRef pDevice, NGFXSamplerState const& samplerDesc)
  : ThisObj(pDevice)
  , m_SamplerState(samplerDesc)
{
  if (pDevice) {
    m_SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    m_SamplerCreateInfo.magFilter = g_Filters[samplerDesc.Filter.MagFilter];
    m_SamplerCreateInfo.minFilter = g_Filters[samplerDesc.Filter.MinFilter];
    m_SamplerCreateInfo.mipmapMode =
      g_MipMapModes[samplerDesc.Filter.MipMapFilter];
    m_SamplerCreateInfo.addressModeU = g_AddressModes[samplerDesc.U];
    m_SamplerCreateInfo.addressModeV = g_AddressModes[samplerDesc.V];
    m_SamplerCreateInfo.addressModeW = g_AddressModes[samplerDesc.W];
    m_SamplerCreateInfo.mipLodBias = samplerDesc.MipLODBias;
    m_SamplerCreateInfo.compareOp =
      g_ComparisonFunc[samplerDesc.ComparisonFunc];
    m_SamplerCreateInfo.minLod = samplerDesc.MinLOD;
    // Max level-of-detail should match mip level count
    m_SamplerCreateInfo.maxLod = samplerDesc.MaxLOD;
    // Enable anisotropic filtering
    m_SamplerCreateInfo.maxAnisotropy = 1.0f * samplerDesc.MaxAnistropy;
    m_SamplerCreateInfo.anisotropyEnable = VK_TRUE;
    m_SamplerCreateInfo.borderColor =
      VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; // cannot convert...
    vkCreateSampler(
      NativeDevice(), &m_SamplerCreateInfo, nullptr, &m_NativeObj);
  }
}

SamplerImpl::~SamplerImpl()
{
  VKLOG(Info, "SamplerImpl Destroying %p...", m_NativeObj);
  vkDestroySampler(NativeDevice(), m_NativeObj, nullptr);
}

NGFXSamplerState
SamplerImpl::GetSamplerDesc() const
{
  return m_SamplerState;
}

template<typename VkObject>
ResourceManager::Allocation
ResourceManager::Pool<VkObject>::Allocate(
  const typename ResourceManager::ResDesc<VkObject>& objDesc)
{
  ResourceManager::Allocation result = {};
  const VkMemoryRequirements& memReqs = objDesc.MemoryRequirements;
  if (HasAvailable(memReqs)) {
    const VkDeviceSize initialOffset = m_Offset;
    const VkDeviceSize alignedOffset =
      CalcAlignedOffset(initialOffset, memReqs.alignment);
    const VkDeviceSize allocatedSize = memReqs.size;
    result.Memory = m_Memory;
    result.Offset = alignedOffset;
    result.Size = allocatedSize;
    m_Allocations.push_back(result);
    m_Offset += allocatedSize;
  }
  return result;
}

template<typename VkObjectT>
ResourceManager::Pool<VkObjectT>::Pool(uint32 memTypeIndex,
  VkDeviceMemory mem,
  VkDeviceSize sz)
  : m_MemoryTypeIndex(memTypeIndex)
  , m_Memory(mem)
  , m_Size(sz)
{
}

template<typename VkObject>
std::unique_ptr<ResourceManager::Pool<VkObject>>
ResourceManager::Pool<VkObject>::Create(
  VkDevice device,
  const VkDeviceSize poolSize,
  const typename ResourceManager::ResDesc<VkObject>& objDesc)
{
  std::unique_ptr<ResourceManager::Pool<VkObject>> result;
  const uint32_t memoryTypeIndex = objDesc.MemoryTypeIndex;
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.allocationSize = poolSize;
  allocInfo.memoryTypeIndex = memoryTypeIndex;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkResult res = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
  if (VK_SUCCESS == res) {
    result = std::unique_ptr<ResourceManager::Pool<VkObject>>(
      new ResourceManager::Pool<VkObject>(memoryTypeIndex, memory, poolSize));
  }
  return result;
}

template<typename VkObjectT>
bool
ResourceManager::Pool<VkObjectT>::HasAvailable(
  VkMemoryRequirements memReqs) const
{
  VkDeviceSize alignedOffst = CalcAlignedOffset(m_Offset, memReqs.alignment);
#ifdef min
#undef min
#endif
  VkDeviceSize remaining = m_Size - std::min(alignedOffst, m_Size);
  return memReqs.size <= remaining;
}

template<typename VkObjectT>
ResourceManager::PoolManager<VkObjectT>::PoolManager(DeviceRef pDevice,
  VkDeviceSize poolSize)
  : DeviceChild(pDevice)
  , m_PoolSize(poolSize)
{
}

template<typename VkObjectT>
ResourceManager::PoolManager<VkObjectT>::~PoolManager()
{
  Destroy();
}

template<typename VkObjectT>
void
ResourceManager::PoolManager<VkObjectT>::Destroy()
{
  if (!GetRawDevice())
    return;
  ::Os::Mutex::AutoLock lock(&m_Mutex);
  for (auto& pool : m_Pools) {
    vkFreeMemory(GetRawDevice(), pool->m_Memory, nullptr);
  }
  m_Pools.clear();
}

template<typename VkObjectT>
ResourceManager::Allocation
ResourceManager::PoolManager<VkObjectT>::Allocate(
  const typename ResourceManager::ResDesc<VkObjectT>& objDesc)
{
  ::Os::Mutex::AutoLock lock(&m_Mutex);
  ResourceManager::Allocation result;
  if (objDesc.MemoryRequirements.size > m_PoolSize) {
    auto pool = ResourceManager::Pool<VkObjectT>::Create(
      GetRawDevice(), objDesc.MemoryRequirements.size, objDesc);
    if (pool) {
      result = pool->Allocate(objDesc);
      m_Pools.push_back(std::move(pool));
    }
  }
  else {
    // Look to see if there's a pool that fits the requirements...
    auto it = std::find_if(
      std::begin(m_Pools),
      std::end(m_Pools),
      [objDesc](
        const ResourceManager::PoolManager<VkObjectT>::PoolRef& elem) -> bool {
      bool isMemoryType =
        (elem->GetMemoryTypeIndex() == objDesc.MemoryTypeIndex);
      bool hasSpace = elem->HasAvailable(objDesc.MemoryRequirements);
      return isMemoryType && hasSpace;
    });

    // ...if there is allocate from the available pool
    if (std::end(m_Pools) != it) {
      auto& pool = *it;
      result = pool->Allocate(objDesc);
    }
    // ...otherwise create a new pool and allocate from it
    else {
      auto pool = ResourceManager::Pool<VkObjectT>::Create(
        GetRawDevice(), m_PoolSize, objDesc);
      if (pool) {
        result = pool->Allocate(objDesc);
        m_Pools.push_back(std::move(pool));
      }
    }
  }
  return result;
}

ResourceManager::ResourceManager(DeviceRef pDevice,
  size_t bufferBlockSize,
  size_t imageBlockSize)
  : DeviceChild(pDevice)
  , m_BufferAllocations(pDevice, bufferBlockSize)
  , m_ImageAllocations(pDevice, imageBlockSize)
{
  Initialize();
}

ResourceManager::~ResourceManager()
{
  Destroy();
}

ResourceManager::Allocation
ResourceManager::AllocateBuffer(VkBuffer buffer,
  bool transient,
  VkMemoryPropertyFlags memoryProperty)
{
  VkMemoryRequirements memoryRequirements = {};
  vkGetBufferMemoryRequirements(GetRawDevice(), buffer, &memoryRequirements);
  uint32_t memoryTypeIndex = 0;
  bool foundMemory = GetDevice()->FindMemoryType(
    memoryRequirements.memoryTypeBits, memoryProperty, &memoryTypeIndex);
  K3D_ASSERT(foundMemory);
  ResourceManager::ResDesc<VkBuffer> objDesc = {};
  objDesc.Object = buffer;
  objDesc.IsTransient = transient;
  objDesc.MemoryTypeIndex = memoryTypeIndex;
  objDesc.MemoryProperty = memoryProperty;
  objDesc.MemoryRequirements = memoryRequirements;
  return m_BufferAllocations.Allocate(objDesc);
}

ResourceManager::Allocation
ResourceManager::AllocateImage(VkImage image,
  bool transient,
  VkMemoryPropertyFlags memoryProperty)
{
  VkMemoryRequirements memoryRequirements = {};
  vkGetImageMemoryRequirements(GetRawDevice(), image, &memoryRequirements);
  uint32_t memoryTypeIndex = 0;
  bool foundMemory = GetDevice()->FindMemoryType(
    memoryRequirements.memoryTypeBits, memoryProperty, &memoryTypeIndex);
  K3D_ASSERT(foundMemory);
  ResourceManager::ResDesc<VkImage> objDesc = {};
  objDesc.Object = image;
  objDesc.IsTransient = transient;
  objDesc.MemoryTypeIndex = memoryTypeIndex;
  objDesc.MemoryProperty = memoryProperty;
  objDesc.MemoryRequirements = memoryRequirements;
  return m_ImageAllocations.Allocate(objDesc);
}

void
ResourceManager::Initialize()
{
}

void
ResourceManager::Destroy()
{
  m_BufferAllocations.Destroy();
  m_ImageAllocations.Destroy();
}

CommandAllocatorRef
CommandAllocator::CreateAllocator(uint32 queueFamilyIndex,
                                  bool transient,
                                  DeviceRef device)
{
  CommandAllocatorRef result =
    CommandAllocatorRef(new CommandAllocator(queueFamilyIndex, transient, device));
  return result;
}

CommandAllocator::~CommandAllocator()
{
  Destroy();
}

void
CommandAllocator::Initialize()
{
  VkCommandPoolCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.queueFamilyIndex = m_FamilyIndex;
  createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (m_Transient) {
    createInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  }

  K3D_VK_VERIFY(
    vkCreateCommandPool(GetRawDevice(), &createInfo, nullptr, &m_Pool));
}

void
CommandAllocator::Destroy()
{
  if (VK_NULL_HANDLE == m_Pool || !GetRawDevice()) {
    return;
  }
  VKLOG(Info,
        "CommandAllocator destroy.. -- %d. (device:0x%0x)",
        m_Pool,
        GetRawDevice());
  vkDestroyCommandPool(GetRawDevice(), m_Pool, nullptr);
  m_Pool = VK_NULL_HANDLE;
}

CommandAllocator::CommandAllocator(uint32 queueFamilyIndex,
                                   bool transient,
                                   DeviceRef device)
  : m_FamilyIndex(queueFamilyIndex)
  , m_Transient(transient)
  , DeviceChild(device)
{
  Initialize();
}

CommandQueueImpl::CommandQueueImpl(DeviceRef pDevice,
                           VkQueueFlags queueTypes,
                           uint32 queueFamilyIndex,
                           uint32 queueIndex)
  : CommandQueueImpl::This(pDevice)
{
  Initialize(queueTypes, queueFamilyIndex, queueIndex);
}

CommandQueueImpl::~CommandQueueImpl()
{
  Destroy();
}

#pragma region ThreadLocalCommandBufferManager
thread_local CmdBufManagerRef TLS_CmdManager;
#pragma endregion

NGFXCommandBufferRef
CommandQueueImpl::ObtainCommandBuffer(NGFXCommandReuseType const& Usage)
{
  if (!TLS_CmdManager)
    TLS_CmdManager = MakeShared<CommandBufferManager>(
      m_Device->GetRawDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_QueueIndex);
  VkCommandBuffer Cmd = TLS_CmdManager->RequestCommandBuffer();
  TLS_CmdManager->BeginFrame();
  return new CommandBufferImpl(m_Device, this, Cmd);
}

void
CommandQueueImpl::Submit(const std::vector<VkCommandBuffer>& cmdBufs,
                     const std::vector<VkSemaphore>& waitSemaphores,
                     const std::vector<VkPipelineStageFlags>& waitStageMasks,
                     VkFence fence,
                     const std::vector<VkSemaphore>& signalSemaphores)
{
  const VkPipelineStageFlags* pWaitDstStageMask =
    (waitSemaphores.size() == waitStageMasks.size()) ? waitStageMasks.data()
                                                     : nullptr;
  VkSubmitInfo submits = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submits.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
  submits.pWaitSemaphores =
    waitSemaphores.empty() ? nullptr : waitSemaphores.data();
  submits.pWaitDstStageMask = pWaitDstStageMask;
  submits.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
  submits.pCommandBuffers = cmdBufs.empty() ? nullptr : cmdBufs.data();
  submits.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
  submits.pSignalSemaphores =
    signalSemaphores.empty() ? nullptr : signalSemaphores.data();
  this->Submit({ submits }, fence);
}



CommandBufferRef
CommandQueueImpl::ObtainSecondaryCommandBuffer()
{
  VkCommandBuffer Cmd = m_SecondaryCmdBufferPool->RequestCommandBuffer();
  //m_SecondaryCmdBufferPool->BeginFrame();
  return new CommandBufferImpl(m_Device, this, Cmd);
}

VkResult
CommandQueueImpl::Submit(const std::vector<VkSubmitInfo>& submits, VkFence fence)
{
  K3D_ASSERT(!submits.empty());
  uint32_t submitCount = static_cast<uint32_t>(submits.size());
  const VkSubmitInfo* pSubmits = submits.data();
  VkResult err = vkQueueSubmit(m_NativeObj, submitCount, pSubmits, fence);
  K3D_ASSERT((err == VK_SUCCESS) || (err == VK_ERROR_DEVICE_LOST));
  return err;
}

void
CommandQueueImpl::Present(SwapChainRef& pSwapChain)
{
  VkResult PresentResult = VK_SUCCESS;
  VkPresentInfoKHR PresentInfo = {
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    nullptr,
    1,                               // waitSemaphoreCount
    &pSwapChain->m_smpPresent,       // pWaitSemaphores
    1,                               // SwapChainCount
    &pSwapChain->m_NativeObj,
    &pSwapChain->m_CurrentBufferID, // SwapChain Image Index
    &PresentResult
  };
  VkResult Ret = vkQueuePresentKHR(NativeHandle(), &PresentInfo);
  VKLOG(Info, "CommandQueue :: Present, waiting on 0x%0x.", pSwapChain->m_smpPresent);
  K3D_VK_VERIFY(Ret);
  K3D_VK_VERIFY(PresentResult);
  //vkQueueWaitIdle(NativeHandle());
}

void
CommandQueueImpl::WaitIdle()
{
  K3D_VK_VERIFY(vkQueueWaitIdle(m_NativeObj));
}

void
CommandQueueImpl::Initialize(VkQueueFlags queueTypes,
                         uint32 queueFamilyIndex,
                         uint32 queueIndex)
{
  m_QueueFamilyIndex = queueFamilyIndex;
  m_QueueIndex = queueIndex;
  vkGetDeviceQueue(
    m_Device->GetRawDevice(), m_QueueFamilyIndex, m_QueueIndex, &m_NativeObj);
  m_ReUsableCmdBufferPool = MakeShared<CommandBufferManager>(
    m_Device->GetRawDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_QueueIndex);
  m_SecondaryCmdBufferPool = MakeShared<CommandBufferManager>(
    m_Device->GetRawDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY, m_QueueIndex);
}

void
CommandQueueImpl::Destroy()
{
  if (m_NativeObj == VK_NULL_HANDLE)
    return;
  m_NativeObj = VK_NULL_HANDLE;
}

void
CommandBufferImpl::Release()
{
}

void
CommandBufferImpl::Commit(NGFXFenceRef pFence)
{
  if (!m_Ended) {
    // when in presenting, check if present image is in `Present State`
    // if not, make state transition
    if (m_PendingSwapChain) {
      /*auto PresentImage = m_PendingSwapChain->GetCurrentTexture();
      auto ImageState = PresentImage->GetState();
      if (ImageState != NGFX_RESOURCE_STATE_PRESENT) {
        Transition(PresentImage, NGFX_RESOURCE_STATE_PRESENT);
      }*/
    }

    auto Ret = vkEndCommandBuffer(m_NativeObj);
    K3D_VK_VERIFY(Ret);
    m_Ended = true;
  }

  if (pFence)
  {
    pFence->Reset();
  }

  // Submit First
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  if (m_PendingSwapChain)
  {
    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    auto vSwapchain = static_cast<SwapChainImpl*>(m_PendingSwapChain);    
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vSwapchain->m_smpRenderFinish;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vSwapchain->m_smpPresent;
    submitInfo.pWaitDstStageMask = &stageFlags;
    VKLOG(Info, "Command Buffer :: Commit wait on 0x%0x, signal on 0x%0x.",
      vSwapchain->m_smpRenderFinish, vSwapchain->m_smpPresent);
  }
  else 
  {
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
  }
  
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_NativeObj;
  auto vFence = static_cast<FenceImpl*>(pFence);
  auto Ret = m_OwningQueue->Submit({ submitInfo }, vFence ?
    vFence->NativeHandle() : VK_NULL_HANDLE);
  K3D_VK_VERIFY(Ret);

  if (pFence)
  {
    pFence->WaitFor(10000);
  }

  m_OwningQueue->WaitIdle();

  // Do Present
  if (m_PendingSwapChain) {
    auto pSwapChain = static_cast<SwapChainImpl*>(m_PendingSwapChain);
    m_OwningQueue->Present(pSwapChain);
    // After Present, Prepare For Next Frame
    pSwapChain->AcquireNextImage();
    m_PendingSwapChain = nullptr;
  }
}

void
CommandBufferImpl::Present(NGFXSwapChainRef pSwapChain, NGFXFenceRef pFence)
{
  m_PendingSwapChain = pSwapChain;
}

void
CommandBufferImpl::Reset()
{
  vkResetCommandBuffer(m_NativeObj, 0);
}

NGFXRenderCommandEncoderRef
CommandBufferImpl::RenderCommandEncoder(NGFXRenderPassDesc const& Desc)
{
  auto pRenderpass = static_cast<RenderPassImpl*>(m_Device->CreateRenderPass(Desc));
  auto pFramebuffer = m_Device->GetOrCreateFramebuffer(Desc);
  pRenderpass->Begin(m_NativeObj, pFramebuffer, Desc);
  return new vk::RenderCommandEncoderImpl(this,
                                              ECmdLevel::Primary);
}

NGFXComputeCommandEncoderRef
CommandBufferImpl::ComputeCommandEncoder()
{
  return new vk::ComputeCommandEncoderImpl(this);
}

NGFXParallelRenderCommandEncoderRef
CommandBufferImpl::ParallelRenderCommandEncoder(NGFXRenderPassDesc const& Desc)
{
  auto pRenderpass = static_cast<RenderPassImpl*>(m_Device->CreateRenderPass(Desc));
  auto pFramebuffer = m_Device->GetOrCreateFramebuffer(Desc);
  pRenderpass->Begin(m_NativeObj, pFramebuffer, Desc, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); // next append secondary cmds
  return new vk::ParallelCommandEncoderImpl(this);
}

void
CommandBufferImpl::CopyTexture(const NGFXTextureCopyLocation& Dest,
                           const NGFXTextureCopyLocation& Src)
{
  K3D_ASSERT(Dest.pResource && Src.pResource);
  if (Src.pResource->GetDesc().Type == NGFX_BUFFER &&
      Dest.pResource->GetDesc().Type != NGFX_BUFFER) {
    DynArray<VkBufferImageCopy> Copies;
    for (auto footprint : Src.SubResourceFootPrints) {
      VkBufferImageCopy bImgCpy = {};
      bImgCpy.bufferOffset = footprint.BufferOffSet;
      bImgCpy.imageOffset = { footprint.TOffSetX,
                              footprint.TOffSetY,
                              footprint.TOffSetZ };
      bImgCpy.imageExtent = { footprint.Footprint.Dimension.Width,
                              footprint.Footprint.Dimension.Height,
                              footprint.Footprint.Dimension.Depth };
      bImgCpy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
      Copies.Append(bImgCpy);
    }
    auto SrcBuf = static_cast<BufferImpl*>(Src.pResource);
    auto DestImage = static_cast<TextureImpl*>(Dest.pResource);
    vkCmdCopyBufferToImage(m_NativeObj,
                           SrcBuf->NativeHandle(),
                           DestImage->NativeHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           Copies.Count(),
                           Copies.Data());
  }
}

void
CommandBufferImpl::CopyBuffer(NGFXResourceRef Dest,
                          NGFXResourceRef Src,
                          NGFXCopyBufferRegion const& Region)
{
  K3D_ASSERT(Dest && Src);
  K3D_ASSERT(Dest->GetDesc().Type == NGFX_BUFFER &&
             Src->GetDesc().Type == NGFX_BUFFER);
  auto DestBuf = static_cast<BufferImpl*>(Dest);
  auto SrcBuf = static_cast<BufferImpl*>(Src);
  vkCmdCopyBuffer(m_NativeObj,
                  SrcBuf->NativeHandle(),
                  DestBuf->NativeHandle(),
                  1,
                  (const VkBufferCopy*)&Region);
}

void InferImageBarrierFromDesc(
  TextureRef pTexture,
  NGFXResourceState const& DestState,
  VkImageMemoryBarrier& Barrier)
{
  auto Desc = pTexture->GetDesc();
  VkImageLayout Src = pTexture->GetImageLayout();
  switch (Src)
  {
  case VK_IMAGE_LAYOUT_UNDEFINED:
    Barrier.srcAccessMask = 0;
    break;
  case VK_IMAGE_LAYOUT_PREINITIALIZED:
    Barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    Barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    Barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    Barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    Barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    break;
  }
    
  VkImageLayout Dest = g_ResourceState[DestState];
  switch(Dest)
  {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    Barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    Barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    Barrier.dstAccessMask = Barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    if (Barrier.srcAccessMask == 0)
    {
      Barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    Barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    break;
  default:
    break;
  }
}

// TODO:
void
CommandBufferImpl::Transition(NGFXResourceRef pResource,
                          NGFXResourceState const& State)
{
  auto Desc = pResource->GetDesc();
  if (Desc.Type == NGFX_BUFFER) // Buffer Barrier
  {
    // buffer offset, size
    VkBufferMemoryBarrier BufferBarrier = {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr,
    };
    auto pBuffer = static_cast<BufferImpl*>(pResource);
    BufferBarrier.buffer = pBuffer->NativeHandle();
    BufferBarrier.size = pBuffer->GetSize();

    VkPipelineStageFlagBits SrcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    VkPipelineStageFlagBits DestStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    if (pBuffer->m_UsageState == NGFX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
    {
      BufferBarrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (pBuffer->m_UsageState == NGFX_RESOURCE_STATE_UNORDERED_ACCESS)
    {
      BufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      SrcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if (State == NGFX_RESOURCE_STATE_UNORDERED_ACCESS)
    {
      BufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (State == NGFX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
    {
      BufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
      DestStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    BufferBarrier.srcQueueFamilyIndex = 0;
    BufferBarrier.dstQueueFamilyIndex = 0;
    vkCmdPipelineBarrier(
      m_NativeObj,
      SrcStage,
      DestStage,
      0,
      0, nullptr,
      1, &BufferBarrier,
      0, nullptr);
  } else if(State != NGFX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)// ImageLayout Transition
  {
    //if (pResource->GetState() == State)
    //  return;
    auto pTexture = static_cast<TextureImpl*>(pResource);
    VkImageLayout SrcLayout = g_ResourceState[pTexture->GetState()];
    VkImageLayout DestLayout = g_ResourceState[State];
    VkImageMemoryBarrier ImageBarrier = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      0, //srcAccessMask
      VK_ACCESS_MEMORY_READ_BIT, //dstAccessMask
      SrcLayout,
      DestLayout,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      pTexture->NativeHandle(),
      pTexture->GetSubResourceRange()
    };
    InferImageBarrierFromDesc(pTexture, State, ImageBarrier);
    VkPipelineStageFlagBits SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlagBits DestStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    if (DestLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
      SrcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (DestLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
      DestStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
    vkCmdPipelineBarrier(m_NativeObj,
                         SrcStage,
                         DestStage,
                         0, // DependencyFlag
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &ImageBarrier);
    pTexture->m_UsageState = State;
    pTexture->m_ImageLayout = DestLayout;
  }
}

CommandBufferImpl::CommandBufferImpl(DeviceRef pDevice,
                             CommandQueueRef pQueue,
                             VkCommandBuffer Buffer)
  : CommandBufferImpl::This(pDevice)
  , m_OwningQueue(pQueue)
  , m_Ended(false)
{
  m_NativeObj = Buffer;
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  beginInfo.pInheritanceInfo = nullptr;
  K3D_VK_VERIFY(vkBeginCommandBuffer(m_NativeObj, &beginInfo));
}

/**
 * @see #RHIRect2VkRect2D
 */
void
RHIRect2VkRect(const NGFXRect& rect, VkRect2D& r2d)
{
  r2d.extent.width = rect.Right - rect.Left;
  r2d.extent.height = rect.Bottom - rect.Top;
  r2d.offset.x = rect.Left;
  r2d.offset.y = rect.Top;
}

void
RenderCommandEncoderImpl::SetScissorRect(const NGFXRect& pRect)
{
  VkRect2D Rect2D;
  RHIRect2VkRect(pRect, Rect2D);
  vkCmdSetScissor(m_MasterCmd->NativeHandle(), 0, 1, &Rect2D);
}

void
RenderCommandEncoderImpl::SetViewport(const NGFXViewportDesc& viewport)
{
  vkCmdSetViewport(m_MasterCmd->NativeHandle(),
                   0,
                   1,
                   reinterpret_cast<const VkViewport*>(&viewport));
}

void
RenderCommandEncoderImpl::SetIndexBuffer(const NGFXIndexBufferView& IBView)
{
  VkBuffer buf = (VkBuffer)(IBView.BufferLocation);
  vkCmdBindIndexBuffer(
    m_MasterCmd->NativeHandle(), buf, 0, VK_INDEX_TYPE_UINT32);
}

void
RenderCommandEncoderImpl::SetVertexBuffer(uint32 Slot,
                                      const NGFXVertexBufferView& VBView)
{
  VkBuffer buf = (VkBuffer)(VBView.BufferLocation);
  VkDeviceSize offsets[1] = { 0 };
  vkCmdBindVertexBuffers(m_MasterCmd->NativeHandle(), Slot, 1, &buf, offsets);
}

void RenderCommandEncoderImpl::SetPrimitiveType(NGFXPrimitiveType)
{
}

void
RenderCommandEncoderImpl::DrawInstanced(NGFXDrawInstancedParam drawParam)
{
  vkCmdDraw(m_MasterCmd->NativeHandle(),
            drawParam.VertexCountPerInstance,
            drawParam.InstanceCount,
            drawParam.StartVertexLocation,
            drawParam.StartInstanceLocation);
}

void
RenderCommandEncoderImpl::DrawIndexedInstanced(
  NGFXDrawIndexedInstancedParam drawParam)
{
  vkCmdDrawIndexed(m_MasterCmd->NativeHandle(),
                   drawParam.IndexCountPerInstance,
                   drawParam.InstanceCount,
                   drawParam.StartIndexLocation,
                   drawParam.BaseVertexLocation,
                   drawParam.StartInstanceLocation);
}

void
RenderCommandEncoderImpl::EndEncode()
{
  if (m_Level != ECmdLevel::Secondary) {
    vkCmdEndRenderPass(m_MasterCmd->NativeHandle());
  }
  //This::EndEncode();
}

// from primary render command buffer
RenderCommandEncoderImpl::RenderCommandEncoderImpl(CommandBufferRef pCmd, ECmdLevel Level)
  : RenderCommandEncoderImpl::This(pCmd)
  , m_Level(Level)
{
}

RenderCommandEncoderImpl::RenderCommandEncoderImpl(ParallelCommandEncoderRef ParentEncoder,
                                           CommandBufferRef pCmd)
  : RenderCommandEncoderImpl::This(pCmd)
  , m_Level(ECmdLevel::Secondary)
{
  // render pass begun before allocate
}

ComputeCommandEncoderImpl::ComputeCommandEncoderImpl(CommandBufferRef pCmdBuffer)
  : Super(pCmdBuffer)
{
}

void
ComputeCommandEncoderImpl::Dispatch(uint32 GroupCountX,
                                uint32 GroupCountY,
                                uint32 GroupCountZ)
{
  vkCmdDispatch(
    m_MasterCmd->NativeHandle(), GroupCountX, GroupCountY, GroupCountZ);
}

NGFXRenderCommandEncoderRef
ParallelCommandEncoderImpl::SubRenderCommandEncoder()
{
  VkCommandBufferInheritanceInfo inheritanceInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
  VkCommandBufferBeginInfo commandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
  // allocate secondary cmd
  auto CmdBuffer = m_pQueue->ObtainSecondaryCommandBuffer();
  K3D_VK_VERIFY(vkBeginCommandBuffer(CmdBuffer->NativeHandle(), &commandBufferBeginInfo));
  return new RenderCommandEncoderImpl(this, CmdBuffer);
}

ParallelCommandEncoderImpl::ParallelCommandEncoderImpl(CommandBufferRef pCmd)
  : Super(pCmd)
{
}

void
ParallelCommandEncoderImpl::EndEncode()
{
  DynArray<VkCommandBuffer> Cmds;
  for (auto pCmd : m_RecordedCmds) {
    Cmds.Append(pCmd->OwningCmd());
  }
  vkCmdExecuteCommands(m_MasterCmd->NativeHandle(), Cmds.Count(), Cmds.Data());

  // end renderpass
  if (m_RenderpassBegun) {
    vkCmdEndRenderPass(m_MasterCmd->NativeHandle());
  }

  Super::EndEncode();
}

void ParallelCommandEncoderImpl::SubAllocateSecondaryCmd()
{
}

RenderTarget::RenderTarget(DeviceRef pDevice,
                           TextureImpl::TextureRef texture,
                           FrameBufferRef framebuffer,
                           VkRenderPass renderpass)
  : DeviceChild(pDevice)
  , m_Renderpass(renderpass)
{
  m_AcquireSemaphore = new SemaphoreImpl(pDevice);
}

RenderTarget::~RenderTarget()
{
}

VkFramebuffer
RenderTarget::GetFramebuffer() const
{
  return m_Framebuffer;
}

VkRenderPass
RenderTarget::GetRenderpass() const
{
  return m_Renderpass;
}

TextureImpl::TextureRef
RenderTarget::GetTexture() const
{
  return nullptr;
}

VkRect2D
RenderTarget::GetRenderArea() const
{
  VkRect2D renderArea = {};
  renderArea.offset = { 0, 0 };
  renderArea.extent = { /*m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight()*/ };
  return renderArea;
}

NGFXResourceRef
RenderTarget::GetBackBuffer()
{
  return nullptr;
}

RenderPipelineStateImpl::RenderPipelineStateImpl(
  DeviceRef pDevice,
  NGFXRenderPipelineDesc const& desc, 
  NGFXPipelineLayoutRef pPipelineLayout,
  NGFXRenderpassRef pRenderPass)
  : TPipelineState<NGFXRenderPipeline>(pDevice)
  , m_RenderPass(VK_NULL_HANDLE)
  , m_GfxCreateInfo{}
  , m_WeakRenderPassRef(pRenderPass)
  , m_weakPipelineLayoutRef(pPipelineLayout)
{
  if (m_WeakRenderPassRef)
    m_WeakRenderPassRef->AddInternalRef();
  if (m_weakPipelineLayoutRef)
    m_weakPipelineLayoutRef->AddInternalRef();
  memset(&m_GfxCreateInfo, 0, sizeof(m_GfxCreateInfo));
  InitWithDesc(desc);
}

RenderPipelineStateImpl::~RenderPipelineStateImpl()
{
  if (m_WeakRenderPassRef)
    m_WeakRenderPassRef->ReleaseInternal();
  if (m_weakPipelineLayoutRef)
    m_weakPipelineLayoutRef->ReleaseInternal();
  Destroy();
}

void
RenderPipelineStateImpl::BindRenderPass(VkRenderPass RenderPass)
{
  this->m_RenderPass = RenderPass;
}

void
RenderPipelineStateImpl::Rebuild()
{
  if (VK_NULL_HANDLE != m_Pipeline)
    return;
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  K3D_VK_VERIFY(vkCreatePipelineCache(m_Device->GetRawDevice(),
                                      &pipelineCacheCreateInfo,
                                      nullptr,
                                      &m_PipelineCache));
  K3D_VK_VERIFY(vkCreateGraphicsPipelines(m_Device->GetRawDevice(),
                                          m_PipelineCache,
                                          1,
                                          &m_GfxCreateInfo,
                                          nullptr,
                                          &m_Pipeline));
}

#if 0
void
RenderPipelineState::SavePSO(const char* path)
{
  size_t szPSO = 0;
  K3D_VK_VERIFY(
    vkGetPipelineCacheData(GetRawDevice(), m_PipelineCache, &szPSO, nullptr));
  if (!szPSO || !path)
    return;
  DynArray<char> dataBlob;
  dataBlob.Resize(szPSO);
  vkGetPipelineCacheData(
    GetRawDevice(), m_PipelineCache, &szPSO, dataBlob.Data());
  Os::File psoCacheFile(path);
  psoCacheFile.Open(IOWrite);
  psoCacheFile.Write(dataBlob.Data(), szPSO);
  psoCacheFile.Close();
}

void
RenderPipelineState::LoadPSO(const char* path)
{
  Os::MemMapFile psoFile;
  if (!path || !psoFile.Open(path, IORead))
    return;
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCacheCreateInfo.pInitialData = psoFile.FileData();
  pipelineCacheCreateInfo.initialDataSize = psoFile.GetSize();
  K3D_VK_VERIFY(vkCreatePipelineCache(
    GetRawDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
}
#endif
void
RenderPipelineStateImpl::SetRasterizerState(const NGFXRasterizerState& rasterState)
{
  m_RasterizationState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // Solid polygon mode
  m_RasterizationState.polygonMode = g_FillMode[rasterState.FillMode];
  // No culling
  m_RasterizationState.cullMode = g_CullMode[rasterState.CullMode];
  m_RasterizationState.frontFace = rasterState.FrontCCW
                                     ? VK_FRONT_FACE_COUNTER_CLOCKWISE
                                     : VK_FRONT_FACE_CLOCKWISE;
  m_RasterizationState.depthClampEnable =
    rasterState.DepthClipEnable ? VK_TRUE : VK_FALSE;
  m_RasterizationState.rasterizerDiscardEnable = VK_FALSE;
  m_RasterizationState.depthBiasEnable = VK_FALSE;
  this->m_GfxCreateInfo.pRasterizationState = &m_RasterizationState;
}

// One blend attachment state
// Blending is not used in this example
// TODO
void
RenderPipelineStateImpl::SetBlendState(const NGFXBlendState& blendState)
{
  m_ColorBlendState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
  blendAttachmentState[0].colorWriteMask = 0xf;
  blendAttachmentState[0].blendEnable = blendState.Enable ? VK_TRUE : VK_FALSE;
  m_ColorBlendState.attachmentCount = 1;
  m_ColorBlendState.pAttachments = blendAttachmentState;
  this->m_GfxCreateInfo.pColorBlendState = &m_ColorBlendState;
}

void
RenderPipelineStateImpl::SetDepthStencilState(
  const NGFXDepthStencilState& depthStencilState)
{
  m_DepthStencilState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_DepthStencilState.depthTestEnable =
    depthStencilState.DepthEnable ? VK_TRUE : VK_FALSE;
  m_DepthStencilState.depthWriteEnable = VK_TRUE;
  m_DepthStencilState.depthCompareOp =
    g_ComparisonFunc[depthStencilState.DepthFunc];
  m_DepthStencilState.depthBoundsTestEnable = VK_FALSE;
  m_DepthStencilState.back.failOp =
    g_StencilOp[depthStencilState.BackFace.StencilFailOp];
  m_DepthStencilState.back.passOp =
    g_StencilOp[depthStencilState.BackFace.StencilPassOp];
  m_DepthStencilState.back.compareOp =
    g_ComparisonFunc[depthStencilState.BackFace.StencilFunc];
  m_DepthStencilState.stencilTestEnable =
    depthStencilState.StencilEnable ? VK_TRUE : VK_FALSE;
  m_DepthStencilState.front = m_DepthStencilState.back;
  this->m_GfxCreateInfo.pDepthStencilState = &m_DepthStencilState;
}

void RenderPipelineStateImpl::SetSampler(NGFXSamplerRef)
{
}

void
RenderPipelineStateImpl::SetPrimitiveTopology(const NGFXPrimitiveType Type)
{
  m_InputAssemblyState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  m_InputAssemblyState.topology = g_PrimitiveTopology[Type];
  this->m_GfxCreateInfo.pInputAssemblyState = &m_InputAssemblyState;
}

DynArray<VkVertexInputAttributeDescription>
RHIInputAttribs(NGFXVertexInputState ia)
{
  DynArray<VkVertexInputAttributeDescription> iad;
  for (uint32 i = 0; i < NGFXVertexInputState::kMaxVertexBindings; i++) {
    auto attrib = ia.Attribs[i];
    if (attrib.Slot == NGFXVertexInputState::kInvalidValue)
      break;
    iad.Append(
      { i, attrib.Slot, g_VertexFormatTable[attrib.Format], attrib.OffSet });
  }
  return iad;
}

DynArray<VkVertexInputBindingDescription>
RHIInputLayouts(NGFXVertexInputState const& ia)
{
  DynArray<VkVertexInputBindingDescription> ibd;
  for (uint32 i = 0; i < NGFXVertexInputState::kMaxVertexLayouts; i++) {
    auto layout = ia.Layouts[i];
    if (layout.Stride == NGFXVertexInputState::kInvalidValue)
      break;
    ibd.Append({ i, layout.Stride, g_InputRates[layout.Rate] });
  }
  return ibd;
}

void
RenderPipelineStateImpl::InitWithDesc(NGFXRenderPipelineDesc const& desc)
{
  // setup shaders
  if (desc.VertexShader.RawData.Length() != 0) {
    auto StageInfo = ConvertStageInfoFromShaderBundle(desc.VertexShader);
    m_ShaderStageInfos.Append(StageInfo);
  }
  if (desc.PixelShader.RawData.Length() != 0) {
    auto StageInfo = ConvertStageInfoFromShaderBundle(desc.PixelShader);
    m_ShaderStageInfos.Append(StageInfo);
  }
  if (desc.GeometryShader.RawData.Length() != 0) {
    auto StageInfo = ConvertStageInfoFromShaderBundle(desc.GeometryShader);
    m_ShaderStageInfos.Append(StageInfo);
  }
  if (desc.DomainShader.RawData.Length() != 0) {
    auto StageInfo = ConvertStageInfoFromShaderBundle(desc.DomainShader);
    m_ShaderStageInfos.Append(StageInfo);
  }
  if (desc.HullShader.RawData.Length() != 0) {
    auto StageInfo = ConvertStageInfoFromShaderBundle(desc.HullShader);
    m_ShaderStageInfos.Append(StageInfo);
  }

  // Init PrimType
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    nullptr,
    0,
    g_PrimitiveTopology[desc.PrimitiveTopology],
    VK_FALSE
  };

  // Init RasterState
  VkPipelineRasterizationStateCreateInfo rasterizationState = {
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, NULL
  };
  rasterizationState.polygonMode = g_FillMode[desc.Rasterizer.FillMode];
  rasterizationState.cullMode = g_CullMode[desc.Rasterizer.CullMode];
  rasterizationState.frontFace = desc.Rasterizer.FrontCCW
                                   ? VK_FRONT_FACE_COUNTER_CLOCKWISE
                                   : VK_FRONT_FACE_CLOCKWISE;
  rasterizationState.depthClampEnable =
    desc.Rasterizer.DepthClipEnable ? VK_TRUE : VK_FALSE;
  rasterizationState.rasterizerDiscardEnable = VK_FALSE;
  rasterizationState.depthBiasEnable = VK_FALSE;
  rasterizationState.lineWidth = 1.0f;

  VkPipelineTessellationStateCreateInfo tessellation;

  // Init DepthStencilState
  VkPipelineDepthStencilStateCreateInfo depthStencilState = {
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, NULL
  };
  depthStencilState.depthTestEnable =
    desc.DepthStencil.DepthEnable ? VK_TRUE : VK_FALSE;
  depthStencilState.depthWriteEnable = VK_TRUE;
  depthStencilState.depthCompareOp =
    g_ComparisonFunc[desc.DepthStencil.DepthFunc];
  depthStencilState.depthBoundsTestEnable = VK_FALSE;
  depthStencilState.back.failOp =
    g_StencilOp[desc.DepthStencil.BackFace.StencilFailOp];
  depthStencilState.back.passOp =
    g_StencilOp[desc.DepthStencil.BackFace.StencilPassOp];
  depthStencilState.back.compareOp =
    g_ComparisonFunc[desc.DepthStencil.BackFace.StencilFunc];
  depthStencilState.stencilTestEnable =
    desc.DepthStencil.StencilEnable ? VK_TRUE : VK_FALSE;
  depthStencilState.front = depthStencilState.back;

  // Init BlendState
  VkPipelineColorBlendStateCreateInfo colorBlendState = {
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, NULL
  };
  DynArray<VkPipelineColorBlendAttachmentState> AttachmentStates;
  AttachmentStates.Resize(desc.AttachmentsBlend.Count());
  for (auto i = 0; i< desc.AttachmentsBlend.Count(); i++)
  {
    AttachmentStates[i].alphaBlendOp = g_BlendOps[desc.AttachmentsBlend[i].Blend.BlendAlphaOp];
    AttachmentStates[i].colorBlendOp = g_BlendOps[desc.AttachmentsBlend[i].Blend.Op];
    AttachmentStates[i].srcColorBlendFactor = g_Blend[desc.AttachmentsBlend[i].Blend.Src];
    AttachmentStates[i].dstColorBlendFactor = g_Blend[desc.AttachmentsBlend[i].Blend.Dest];
    AttachmentStates[i].srcAlphaBlendFactor = g_Blend[desc.AttachmentsBlend[i].Blend.SrcBlendAlpha];
    AttachmentStates[i].dstAlphaBlendFactor = g_Blend[desc.AttachmentsBlend[i].Blend.DestBlendAlpha];
    AttachmentStates[i].colorWriteMask = desc.AttachmentsBlend[i].Blend.ColorWriteMask;
    AttachmentStates[i].blendEnable = desc.AttachmentsBlend[i].Blend.Enable? VK_TRUE : VK_FALSE;
  }
  colorBlendState.attachmentCount = desc.AttachmentsBlend.Count();
  colorBlendState.pAttachments = AttachmentStates.Data();

  auto IAs = RHIInputAttribs(desc.InputState);
  auto IBs = RHIInputLayouts(desc.InputState);
  VkPipelineVertexInputStateCreateInfo vertexInputState = {
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, NULL
  };
  vertexInputState.vertexBindingDescriptionCount = IBs.Count();
  vertexInputState.pVertexBindingDescriptions = IBs.Data();
  vertexInputState.vertexAttributeDescriptionCount = IAs.Count();
  vertexInputState.pVertexAttributeDescriptions = IAs.Data();

  VkPipelineViewportStateCreateInfo vpInfo = {
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
  };
  vpInfo.viewportCount = 1;
  vpInfo.scissorCount = 1;

  VkPipelineDynamicStateCreateInfo dynamicState = {
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
  };
  std::vector<VkDynamicState> dynamicStateEnables;
  dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
  dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
  dynamicStateEnables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
  dynamicState.pDynamicStates = dynamicStateEnables.data();
  dynamicState.dynamicStateCount = (uint32)dynamicStateEnables.size();

  VkPipelineMultisampleStateCreateInfo msInfo = {
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
  };
  msInfo.pSampleMask = NULL;
  msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  this->m_GfxCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  this->m_GfxCreateInfo.stageCount = m_ShaderStageInfos.Count();
  this->m_GfxCreateInfo.pStages = m_ShaderStageInfos.Data();

  this->m_GfxCreateInfo.pInputAssemblyState = &inputAssemblyState;
  this->m_GfxCreateInfo.pRasterizationState = &rasterizationState;
  this->m_GfxCreateInfo.pDepthStencilState = &depthStencilState;
  this->m_GfxCreateInfo.pColorBlendState = &colorBlendState;
  this->m_GfxCreateInfo.pVertexInputState = &vertexInputState;
  this->m_GfxCreateInfo.pDynamicState = &dynamicState;
  this->m_GfxCreateInfo.pMultisampleState = &msInfo;
  this->m_GfxCreateInfo.pViewportState = &vpInfo;

  this->m_GfxCreateInfo.renderPass = static_cast<RenderPassImpl*>(m_WeakRenderPassRef)->NativeHandle();
  this->m_GfxCreateInfo.layout = static_cast<PipelineLayoutImpl*>(m_weakPipelineLayoutRef)->NativeHandle();

  // Finalize
  Rebuild();
}

void
RenderPipelineStateImpl::Destroy()
{ /*
   for (auto iter : m_ShaderStageInfos) {
     if (iter.module) {
       vkDestroyShaderModule(, iter.module, nullptr);
     }
   }*/
}

ComputePipelineStateImpl::ComputePipelineStateImpl(
  DeviceRef pDevice,
  NGFXComputePipelineDesc const& desc,
  PipelineLayoutImpl* ppl)
  : TPipelineState<NGFXComputePipeline>(pDevice)
  , m_ComputeCreateInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO }
  , m_PipelineLayout(ppl)
{
  auto stageInfo = ConvertStageInfoFromShaderBundle(desc.ComputeShader);
  m_ComputeCreateInfo.layout = ppl->NativeHandle();
  m_ComputeCreateInfo.stage = stageInfo;
  Rebuild();
}

void
ComputePipelineStateImpl::Rebuild()
{
  if (VK_NULL_HANDLE != m_Pipeline)
    return;
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  K3D_VK_VERIFY(vkCreatePipelineCache(m_Device->GetRawDevice(),
                                      &pipelineCacheCreateInfo,
                                      nullptr,
                                      &m_PipelineCache));
  K3D_VK_VERIFY(vkCreateComputePipelines(m_Device->GetRawDevice(),
                                         m_PipelineCache,
                                         1,
                                         &m_ComputeCreateInfo,
                                         nullptr,
                                         &m_Pipeline));
}

PipelineLayoutImpl::PipelineLayoutImpl(DeviceRef pDevice,
                               NGFXPipelineLayoutDesc const& desc)
  : PipelineLayoutImpl::ThisObj(pDevice)
  , m_DescSetLayout(nullptr)
{
  InitWithDesc(desc);
}

PipelineLayoutImpl::~PipelineLayoutImpl()
{
  Destroy();
}

NGFXBindingGroupRef 
PipelineLayoutImpl::ObtainBindingGroup()
{
    return NGFXBindingGroupRef(BindingGroupImpl::CreateDescSet(
      this,
      m_DescSetLayout->GetNativeHandle(), 
      m_BindingArray, 
      m_Device));
}

void
PipelineLayoutImpl::Destroy()
{
  if (m_NativeObj == VK_NULL_HANDLE)
    return;
  vkDestroyPipelineLayout(NativeDevice(), m_NativeObj, nullptr);
  VKLOG(Info, "PipelineLayoutImpl Destroyed . -- %0x.", m_NativeObj);
  m_NativeObj = VK_NULL_HANDLE;
}

uint64
BindingHash(NGFXShaderBinding const& binding)
{
  return (uint64)(1 << (3 + binding.VarNumber)) | binding.VarStage;
}

bool
operator<(NGFXShaderBinding const& lhs, NGFXShaderBinding const& rhs)
{
  return rhs.VarStage < lhs.VarStage && rhs.VarNumber < lhs.VarNumber;
}

BindingArray
ExtractBindingsFromTable(::k3d::DynArray<NGFXShaderBinding> const& bindings)
{
  //	merge image sampler
  std::map<uint64, NGFXShaderBinding> bindingMap;
  for (auto const& binding : bindings) {
    uint64 hash = BindingHash(binding);
    if (bindingMap.find(hash) == bindingMap.end()) {
      bindingMap.insert({ hash, binding });
    } else // binding slot override
    {
      auto& overrideBinding = bindingMap[hash];
      if (NGFXShaderBindType((uint32)overrideBinding.VarType |
                    (uint32)binding.VarType) ==
          NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLER_IMAGE_COMBINE) {
        overrideBinding.VarType = NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLER_IMAGE_COMBINE;
      }
    }
  }

  BindingArray array;
  for (auto& p : bindingMap) {
    array.Append(RHIBinding2VkBinding(p.second));
  }
  return array;
}

void
PipelineLayoutImpl::InitWithDesc(NGFXPipelineLayoutDesc const& desc)
{
  DescriptorAllocator::Options options;
  m_BindingArray = ExtractBindingsFromTable(desc.Bindings);
  // 
  m_DescAllocator = m_Device->NewDescriptorAllocator(16, m_BindingArray);
  // 
  m_DescSetLayout = m_Device->NewDescriptorSetLayout(m_BindingArray);

  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
  pPipelineLayoutCreateInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pPipelineLayoutCreateInfo.pNext = NULL;
  // TODO
  pPipelineLayoutCreateInfo.setLayoutCount = 1;
  pPipelineLayoutCreateInfo.pSetLayouts =
    &m_DescSetLayout->m_DescriptorSetLayout;

  K3D_VK_VERIFY(vkCreatePipelineLayout(
    NativeDevice(), &pPipelineLayoutCreateInfo, nullptr, &m_NativeObj));
}

/*
enum class NGFXShaderBindType
{
NGFX_SHADER_BIND_UNDEFINED		= 0,
NGFX_SHADER_BIND_BLOCK			= 0x1,
NGFX_SHADER_BIND_SAMPLER		= 0x1 << 1,
NGFX_SHADER_BIND_STORAGE_IMAGE	= 0x1 << 2,
NGFX_SHADER_BIND_STORAGE_BUFFER	= 0x1 << 3,
NGFX_SHADER_BIND_CONSTANTS		= 0x000000010
};
*/
VkDescriptorType
RHIDataType2VkType(NGFXShaderBindType const& type)
{
  switch (type) {
    case NGFXShaderBindType::NGFX_SHADER_BIND_BLOCK:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLER:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLED_IMAGE:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; // ReadOnly Texture
    case NGFXShaderBindType::NGFX_SHADER_BIND_SAMPLER_IMAGE_COMBINE:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case NGFXShaderBindType::NGFX_SHADER_BIND_STORAGE_IMAGE:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case NGFXShaderBindType::NGFX_SHADER_BIND_STORAGE_BUFFER:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case NGFXShaderBindType::NGFX_SHADER_BIND_RWTEXEL_BUFFER:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
      //VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ReadOnly ImageBuffer, texel fetch
      //VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER Read&Write ImageBuffer
  }
  return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkDescriptorSetLayoutBinding
RHIBinding2VkBinding(NGFXShaderBinding const& binding)
{
  VkDescriptorSetLayoutBinding vkbinding = {};
  vkbinding.descriptorType = RHIDataType2VkType(binding.VarType);
  vkbinding.binding = binding.VarNumber;
  vkbinding.stageFlags = g_ShaderType[binding.VarStage];
  vkbinding.descriptorCount = 1;
  return vkbinding;
}

DescriptorAllocator::DescriptorAllocator(
  DeviceRef pDevice,
  DescriptorAllocator::Options const& option,
  uint32 maxSets,
  BindingArray const& bindings)
  : DeviceChild(pDevice)
  , m_Options(option)
  , m_Pool(VK_NULL_HANDLE)
{
  Initialize(maxSets, bindings);
}

DescriptorAllocator::~DescriptorAllocator()
{
  Destroy();
}

void
DescriptorAllocator::Initialize(uint32 maxSets, BindingArray const& bindings)
{
  std::map<VkDescriptorType, uint32_t> mappedCounts;
  for (const auto& binding : bindings) {
    VkDescriptorType descType = binding.descriptorType;
    auto it = mappedCounts.find(descType);
    if (it != mappedCounts.end()) {
      ++mappedCounts[descType];
    } else {
      mappedCounts[descType] = 1;
    }
  }

  std::vector<VkDescriptorPoolSize> typeCounts;
  for (const auto& mc : mappedCounts) {
    typeCounts.push_back({ mc.first, mc.second });
  }

  VkDescriptorPoolCreateInfo createInfo = {
    VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
  };
  createInfo.flags = m_Options.CreateFlags | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.maxSets = 1;
  createInfo.poolSizeCount = typeCounts.size();
  createInfo.pPoolSizes = typeCounts.size() == 0 ? nullptr : typeCounts.data();
  K3D_VK_VERIFY(
    vkCreateDescriptorPool(GetRawDevice(), &createInfo, NULL, &m_Pool));
}

void
DescriptorAllocator::Destroy()
{
  if (VK_NULL_HANDLE == m_Pool || !GetRawDevice())
    return;
  vkDestroyDescriptorPool(GetRawDevice(), m_Pool, nullptr);
  VKLOG(Info, "DescriptorAllocator :: Destroy ... 0x%0x.", m_Pool);
  m_Pool = VK_NULL_HANDLE;
}

DescriptorSetLayout::DescriptorSetLayout(DeviceRef pDevice,
                                         BindingArray const& bindings)
  : DeviceChild(pDevice)
  , m_DescriptorSetLayout(VK_NULL_HANDLE)
{
  Initialize(bindings);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
  // Destroy();
}

void
DescriptorSetLayout::Initialize(BindingArray const& bindings)
{
  VkDescriptorSetLayoutCreateInfo createInfo = {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
  };
  createInfo.bindingCount = bindings.Count();
  createInfo.pBindings = bindings.Count() == 0 ? nullptr : bindings.Data();
  K3D_VK_VERIFY(vkCreateDescriptorSetLayout(
    GetRawDevice(), &createInfo, nullptr, &m_DescriptorSetLayout));
}

void
DescriptorSetLayout::Destroy()
{
  if (VK_NULL_HANDLE == m_DescriptorSetLayout || !GetRawDevice())
    return;
  vkDestroyDescriptorSetLayout(GetRawDevice(), m_DescriptorSetLayout, nullptr);
  VKLOG(
    Info, "DescriptorSetLayout  destroying... -- %0x.", m_DescriptorSetLayout);
  m_DescriptorSetLayout = VK_NULL_HANDLE;
}

BindingGroupImpl::BindingGroupImpl(PipelineLayoutRef pRootLayout,
                             VkDescriptorSetLayout layout,
                             BindingArray const& bindings,
                             DeviceRef pDevice)
  : Super(pDevice)
  , m_Bindings(bindings)
  , m_RootLayout(pRootLayout)
{
  Initialize(layout, bindings);
}

BindingGroupImpl*
BindingGroupImpl::CreateDescSet(PipelineLayoutRef rootLayout,
                             VkDescriptorSetLayout layout,
                             BindingArray const& bindings,
                             DeviceRef pDevice)
{
  return new BindingGroupImpl(rootLayout, layout, bindings, pDevice);
}

BindingGroupImpl::~BindingGroupImpl()
{
  Destroy();
}

void BindingGroupImpl::Update(uint32 bindSet, NGFXUAVRef pUAV)
{
  auto uav = static_cast<UnorderedAceessView*>(pUAV);
  m_BoundDescriptorSet[bindSet].pTexelBufferView = &uav->m_BufferView;
//  m_BoundDescriptorSet[bindSet]. = &imageInfo;
  vkUpdateDescriptorSets(
    NativeDevice(), 1, &m_BoundDescriptorSet[bindSet], 0, NULL);
  VKLOG(Info,
    "%s , Set (0x%0x) updated with uav(view location:0x%x).",
    __K3D_FUNC__,
    NativeHandle(),
    uav->m_BufferView);
}

void
BindingGroupImpl::Update(uint32 bindSet, NGFXSamplerRef pRHISampler)
{
  auto pSampler = static_cast<SamplerImpl*>(pRHISampler);
  VkDescriptorImageInfo imageInfo = {
    pSampler->NativeHandle(),
    VK_NULL_HANDLE,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  };
  m_BoundDescriptorSet[bindSet].pImageInfo = &imageInfo;
  vkUpdateDescriptorSets(
    NativeDevice(), 1, &m_BoundDescriptorSet[bindSet], 0, NULL);
  VKLOG(Info,
        "%s , Set (0x%0x) updated with sampler(location:0x%x).",
        __K3D_FUNC__,
        NativeHandle(),
        pSampler->NativeHandle());
}

void
BindingGroupImpl::Update(uint32 bindSet, NGFXResourceRef gpuResource)
{
  auto desc = gpuResource->GetDesc();
  switch (desc.Type) {
    case NGFX_BUFFER: {
      switch (desc.ViewFlags)
      {
      case NGFX_RESOURCE_SHADER_RESOURCE_VIEW:
        break;
      case NGFX_RESOURCE_UNORDERED_ACCESS_VIEW:
        break;
      case NGFX_RESOURCE_CONSTANT_BUFFER_VIEW:
      {
        auto buffer = static_cast<BufferImpl*>(gpuResource);
        VkDescriptorBufferInfo bufferInfo = {
          buffer->NativeHandle(), 0, gpuResource->GetSize()
        };
        m_BoundDescriptorSet[bindSet].pBufferInfo = &bufferInfo;
        vkUpdateDescriptorSets(
          NativeDevice(), 1, &m_BoundDescriptorSet[bindSet], 0, NULL);
        VKLOG(Info,
          "%s , Set (0x%0x) updated with buffer(location:0x%x, size:%d).",
          __K3D_FUNC__,
          NativeHandle(),
          buffer->NativeHandle(),
          gpuResource->GetSize());
        break;
      }
      }
      break;
    }
    case NGFX_TEXTURE_1D:
    case NGFX_TEXTURE_2D:
    case NGFX_TEXTURE_3D:
    case NGFX_TEXTURE_2D_ARRAY: // combined/seperated image sampler should be
                                  // considered
    {
      auto pTex = static_cast<TextureRef>(gpuResource);
      auto rSampler = static_cast<SamplerRef>(pTex->GetSampler());
      assert(rSampler);
      auto srv =
        static_cast<ShaderResourceView*>(pTex->GetResourceView());
      VkDescriptorImageInfo imageInfo = {
        rSampler->NativeHandle(), srv->NativeImageView(), pTex->GetImageLayout()
      }; //TODO : sampler shouldn't be null
      m_BoundDescriptorSet[bindSet].pImageInfo = &imageInfo;
      vkUpdateDescriptorSets(
        NativeDevice(), 1, &m_BoundDescriptorSet[bindSet], 0, NULL);
      VKLOG(Info,
            "%s , Set (0x%0x) updated with image(location:0x%x, size:%d).",
            __K3D_FUNC__,
            NativeHandle(),
            gpuResource->GetLocation(),
            gpuResource->GetSize());
      break;
    }
  }
}

uint32
BindingGroupImpl::GetSlotNum() const
{
  return (uint32)m_BoundDescriptorSet.size();
}

void
BindingGroupImpl::Initialize(VkDescriptorSetLayout layout,
                          BindingArray const& bindings)
{
  std::vector<VkDescriptorSetLayout> layouts = { layout };
  VkDescriptorSetAllocateInfo allocInfo = {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
  };
  allocInfo.descriptorPool = m_RootLayout->Pool();
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.empty() ? nullptr : layouts.data();
  K3D_VK_VERIFY(
    vkAllocateDescriptorSets(NativeDevice(), &allocInfo, &m_NativeObj));
  VKLOG(Info, "%s , Set (0x%0x) created.", __K3D_FUNC__, m_NativeObj);

  for (auto& binding : m_Bindings) {
    VkWriteDescriptorSet entry = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    entry.dstSet = m_NativeObj;
    entry.descriptorCount = 1;
    entry.dstArrayElement = 0;
    entry.dstBinding = binding.binding;
    entry.descriptorType = binding.descriptorType;
    m_BoundDescriptorSet.push_back(entry);
  }
}

void
BindingGroupImpl::Destroy()
{
  if (VK_NULL_HANDLE == m_NativeObj)
    return;
  VKLOG(Info, "BindingGroup :: Destroy 0x%0x.", m_NativeObj);
  // const auto& options = m_DescriptorAllocator->m_Options;
  // if( options.hasFreeDescriptorSetFlag() ) {
  VkDescriptorSet descSets[1] = { m_NativeObj };
  vkFreeDescriptorSets(
    NativeDevice(), m_RootLayout->Pool(), 1, descSets);
  m_NativeObj = VK_NULL_HANDLE;
  //}
}

VkAttachmentDescription
ConvertAttachDesc(NGFXAttachmentDesc const& Desc, bool IsDStarget)
{
  auto pTexture = static_cast<TextureImpl*>(Desc.pTexture);
  VkFormat Format = g_FormatTable[pTexture->GetDesc().TextureDesc.Format];
  VkAttachmentDescription AttachDesc = {
    0,                                // VkAttachmentDescriptionFlags    flags;
    Format,                           // VkFormat                        format;
    VK_SAMPLE_COUNT_1_BIT,            // VkSampleCountFlagBits           samples;
    g_LoadAction[Desc.LoadAction],    // VkAttachmentLoadOp              loadOp;
    g_StoreAction[Desc.StoreAction],  // VkAttachmentStoreOp             storeOp;
    g_LoadAction[Desc.LoadAction],    // VkAttachmentLoadOp              stencilLoadOp;
    g_StoreAction[Desc.StoreAction],  // VkAttachmentStoreOp             stencilStoreOp;
    VK_IMAGE_LAYOUT_UNDEFINED,        // VkImageLayout                   initialLayout;
    IsDStarget? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR         // VkImageLayout                   finalLayout;
  };
  return AttachDesc;
}

RenderPassImpl::RenderPassImpl(DeviceRef pDevice, NGFXRenderPassDesc const& desc)
  : Super(pDevice)
{
  memcpy(&m_ClearVal[0].color, desc.ColorAttachments[0].ClearColor, sizeof(m_ClearVal[0].color));
#if 0
  m_ClearVal[1].depthStencil.depth = desc.pDepthAttachment->ClearDepth;
  m_ClearVal[1].depthStencil.stencil = desc.pStencilAttachment->ClearStencil;
#endif
  DynArray<VkImageView> AttachViews;
  DynArray<VkAttachmentDescription> Attachs;
  DynArray<VkAttachmentReference> ColorRefers;
  uint32 ColorIndex = 0;
  // process color attachments
  for (auto colorAttach : desc.ColorAttachments) {
    auto pTexture = static_cast<TextureImpl*>(colorAttach.pTexture);
    AttachViews.Append(pTexture->NativeView());
    Attachs.Append(ConvertAttachDesc(colorAttach, false));
    ColorRefers.Append({ ColorIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    ColorIndex++;
  }

  VkSubpassDescription defaultSubpass = {
    0,                                //VkSubpassDescriptionFlags       flags;
    VK_PIPELINE_BIND_POINT_GRAPHICS,  //VkPipelineBindPoint             pipelineBindPoint;
    0,                                //uint32_t                        inputAttachmentCount;
    nullptr,                          //const VkAttachmentReference*    pInputAttachments;
    ColorRefers.Count(),              //uint32_t                        colorAttachmentCount;
    ColorRefers.Data(),               //const VkAttachmentReference*    pColorAttachments;
  //const VkAttachmentReference*    pResolveAttachments;
  //const VkAttachmentReference*    pDepthStencilAttachment;
  //uint32_t                        preserveAttachmentCount;
  //const uint32_t*                 pPreserveAttachments;
  };

  VkAttachmentReference DSRefer = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
  if (desc.pDepthAttachment)
  {
    auto DepthAttach = ConvertAttachDesc(*desc.pDepthAttachment, true);
    Attachs.Append(DepthAttach);
    defaultSubpass.pDepthStencilAttachment = &DSRefer;
  }

  VkSubpassDependency dependency = { 0 };
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = 0;
  
  VkRenderPassCreateInfo renderPassCreateInfo = {
    VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    nullptr,
    0,
    Attachs.Count(),
    Attachs.Data(),
    1,                  // uint32_t                          subpassCount;
    &defaultSubpass,    // const VkSubpassDescription*       pSubpasses;
    1,                  // uint32_t                          dependencyCount;
    &dependency  // const VkSubpassDependency*        pDependencies
  };
  // TODO: add at least one subpass
  auto Ret = vkCreateRenderPass(
    NativeDevice(), &renderPassCreateInfo, nullptr, &m_NativeObj);
}

void RenderPassImpl::Begin(VkCommandBuffer Cmd, FrameBufferRef pFramebuffer, NGFXRenderPassDesc const& Desc, VkSubpassContents contents)
{
  VkRect2D RenderArea;
  RenderArea.offset = { 0,0 };
  RenderArea.extent = { pFramebuffer->GetWidth(), pFramebuffer->GetHeight() };
  VkRenderPassBeginInfo RenderBeginInfo = {
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr,
    m_NativeObj, pFramebuffer->NativeHandle(),
    RenderArea,
    (pFramebuffer->m_HasDepthStencil ? 2 : 1), // uint32_t               clearValueCount;
    m_ClearVal               // const VkClearValue*    pClearValues;
  };
  if (pFramebuffer->m_HasDepthStencil)
  {
    if (Desc.pDepthAttachment)
    {
      m_ClearVal[1].depthStencil.depth = Desc.pDepthAttachment->ClearDepth;
    }
    if (Desc.pStencilAttachment)
    {
      m_ClearVal[1].depthStencil.stencil = Desc.pStencilAttachment->ClearStencil;
    }
  }
  vkCmdBeginRenderPass(Cmd, &RenderBeginInfo, contents);
  VKLOG(Info, "Begin renderpass, fbo: 0x%0x.", pFramebuffer->NativeHandle());
}

void RenderPassImpl::End(VkCommandBuffer Cmd)
{
  vkCmdEndRenderPass(Cmd);
}

void
RenderPassImpl::Release()
{
  if (m_NativeObj) {
    VKLOG(Info, "RenderPass Destroyed . 0x%0x.", m_NativeObj);
    vkDestroyRenderPass(NativeDevice(), m_NativeObj, nullptr);
    m_NativeObj = VK_NULL_HANDLE;
  }
}

RenderPassImpl::~RenderPassImpl()
{
  Release();
}

VkImageAspectFlags
DetermineAspectMask(VkFormat format)
{
  VkImageAspectFlags result = 0;
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT: {
      result = VK_IMAGE_ASPECT_DEPTH_BIT;
    } break;
    case VK_FORMAT_S8_UINT: {
      result = VK_IMAGE_ASPECT_STENCIL_BIT;
    } break;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: {
      result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } break;
    default: {
      result = VK_IMAGE_ASPECT_COLOR_BIT;
    } break;
  }
  return result;
}

FrameBufferImpl::FrameBufferImpl(DeviceRef pDevice, RenderPassRef pRenderPass, NGFXRenderPassDesc const& desc)
  : m_Device(pDevice)
  , m_OwningRenderPass(pRenderPass)
{
  if (m_OwningRenderPass)
    m_OwningRenderPass->AddInternalRef();
  DynArray<VkImageView> AttachViews;
  for (auto colorAttach : desc.ColorAttachments) {
    auto pTexture = static_cast<TextureImpl*>(colorAttach.pTexture);
    AttachViews.Append(pTexture->NativeView());
  }
  if (desc.pDepthAttachment)
  {
    auto pTexture = static_cast<TextureImpl*>(desc.pDepthAttachment->pTexture);
    AttachViews.Append(pTexture->NativeView());
    m_HasDepthStencil = true;
  }
  // create framebuffer
  VkFramebufferCreateInfo createInfo = {
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr,
  };
  // get default image size
  auto defaultImageDesc =
    static_cast<TextureImpl*>(desc.ColorAttachments[0].pTexture)
    ->GetDesc()
    .TextureDesc;
  createInfo.renderPass = m_OwningRenderPass->NativeHandle();
  createInfo.attachmentCount = AttachViews.Count();
  createInfo.pAttachments = AttachViews.Data();
  createInfo.width = defaultImageDesc.Width;
  createInfo.height = defaultImageDesc.Height;
  createInfo.layers = 1;
  createInfo.flags = 0;

  m_Width = createInfo.width;
  m_Height = createInfo.height;

  K3D_VK_VERIFY(
    vkCreateFramebuffer(m_Device->GetRawDevice(), &createInfo, nullptr, &m_FrameBuffer));
}

FrameBufferImpl::~FrameBufferImpl()
{
  if (VK_NULL_HANDLE == m_FrameBuffer) {
    return;
  }

  if (m_OwningRenderPass)
    m_OwningRenderPass->ReleaseInternal();
  VKLOG(Info, "FrameBuffer destroy... -- %0x.", m_FrameBuffer);
  vkDestroyFramebuffer(m_Device->GetRawDevice(), m_FrameBuffer, nullptr);
  m_FrameBuffer = VK_NULL_HANDLE;
}
#if 0
FrameBuffer::Attachment::Attachment(VkFormat format,
                                    VkSampleCountFlagBits samples)
{
  VkImageAspectFlags aspectMask = DetermineAspectMask(Format);
  if (VK_IMAGE_ASPECT_COLOR_BIT == aspectMask) {
    FormatFeatures = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
  } else {
    FormatFeatures = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
}
#endif

SwapChainImpl::SwapChainImpl(DeviceRef pDevice, void* pWindow, NGFXSwapChainDesc& Desc)
  : Super(pDevice)
{
  m_CachedCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  m_CachedCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  m_CachedCreateInfo.imageArrayLayers = 1;
  m_CachedCreateInfo.queueFamilyIndexCount = VK_SHARING_MODE_EXCLUSIVE;
  m_CachedCreateInfo.oldSwapchain = VK_NULL_HANDLE;
  m_CachedCreateInfo.clipped = VK_TRUE;
  m_CachedCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  Init(pWindow, Desc);
}

void
SwapChainImpl::Init(void* pWindow, NGFXSwapChainDesc& Desc)
{
  InitSurface(pWindow);

  m_CachedCreateInfo.surface = m_Surface;
  m_CachedCreateInfo.presentMode = ChoosePresentMode();

  uint32_t formatCount = 0;
  K3D_VK_VERIFY(
    m_Device->m_Gpu->GetSurfaceFormatsKHR(m_Surface, &formatCount, NULL));
  std::vector<VkSurfaceFormatKHR> surfFormats(formatCount);
  K3D_VK_VERIFY(m_Device->m_Gpu->GetSurfaceFormatsKHR(
    m_Surface, &formatCount, surfFormats.data()));
  VkFormat colorFormat;
  VkColorSpaceKHR colorSpace;
  if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
    colorFormat = g_FormatTable[Desc.Format];
  }
  else {
    K3D_ASSERT(formatCount >= 1);
    colorFormat = surfFormats[0].format;
  }
  colorSpace = surfFormats[0].colorSpace;
  m_CachedCreateInfo.imageColorSpace = colorSpace;
  m_CachedCreateInfo.imageFormat = colorFormat;
  m_CachedCreateInfo.imageExtent = { Desc.Width, Desc.Height };

  Desc.Format = ConvertToRHIFormat(colorFormat);
  
  m_SelectedPresentQueueFamilyIndex = ChooseQueueIndex();
  VkSurfaceCapabilitiesKHR surfProperties;
  K3D_VK_VERIFY(
    m_Device->m_Gpu->GetSurfaceCapabilitiesKHR(m_Surface, &surfProperties));
  m_CachedCreateInfo.imageExtent = surfProperties.currentExtent;
  m_CachedCreateInfo.preTransform = surfProperties.currentTransform;

  uint32 desiredNumBuffers = kMath::Clamp(Desc.NumBuffers,
    surfProperties.minImageCount,
    surfProperties.maxImageCount);

  m_CachedCreateInfo.minImageCount = desiredNumBuffers;

  K3D_VK_VERIFY(vkCreateSwapchainKHR(
    NativeDevice(), 
    &m_CachedCreateInfo, 
    nullptr, 
    &m_NativeObj)
  );

  K3D_VK_VERIFY(vkGetSwapchainImagesKHR(
    NativeDevice(), 
    NativeHandle(), 
    &m_ReserveBackBufferCount, 
    nullptr));

  m_ColorImages.Resize(m_ReserveBackBufferCount);

  K3D_VK_VERIFY(vkGetSwapchainImagesKHR(
    NativeDevice(),
    NativeHandle(),
    &m_ReserveBackBufferCount,
    m_ColorImages.Data()));

  for (auto ColorImage : m_ColorImages) {
    NGFXResourceDesc ImageDesc;
    ImageDesc.Type = NGFX_TEXTURE_2D;
    ImageDesc.ViewFlags = NGFX_RESOURCE_RENDER_TARGET_VIEW;
    ImageDesc.Flag = NGFX_ACCESS_DEVICE_VISIBLE;
    ImageDesc.Origin = NGFX_RESOURCE_ORIGIN_SWAPCHAIN;
    ImageDesc.TextureDesc.Format = Desc.Format;
    ImageDesc.TextureDesc.Width = m_CachedCreateInfo.imageExtent.width;
    ImageDesc.TextureDesc.Height = m_CachedCreateInfo.imageExtent.height;
    ImageDesc.TextureDesc.Depth = 1;
    ImageDesc.TextureDesc.Layers = 1;
    ImageDesc.TextureDesc.MipLevels = 1;
    m_Buffers.Append(new TextureImpl(ImageDesc,
      ColorImage, SharedDevice(), false));
  }
  Desc.NumBuffers = m_ReserveBackBufferCount;
  VKLOG(
    Info,
    "[SwapChain::Initialize] desired imageCount=%d, reserved imageCount = %d.",
    m_CachedCreateInfo.minImageCount,
    m_ReserveBackBufferCount);

  VkSemaphoreCreateInfo SemaphoreInfo = {
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0
  };
  vkCreateSemaphore(
    NativeDevice(), &SemaphoreInfo, nullptr, &m_smpPresent);
  vkCreateSemaphore(
    NativeDevice(), &SemaphoreInfo, nullptr, &m_smpRenderFinish);

  AcquireNextImage();
}

void
SwapChainImpl::AcquireNextImage()
{
  VkResult Ret = vkAcquireNextImageKHR(NativeDevice(),
    m_NativeObj,
    UINT64_MAX,
    m_smpRenderFinish,
    VK_NULL_HANDLE,
    &m_CurrentBufferID);

  VKLOG(Info, "SwapChain :: AcquireNextImage, acquiring on 0x%0x, id = %d.",
    m_smpRenderFinish, m_CurrentBufferID);
  switch (Ret) {
  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    VKLOG(Error, "VK_SUBOPTIMAL_KHR Swapchain need update");
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    // OnWindowSizeChanged();
    VKLOG(Error, "VK_ERROR_OUT_OF_DATE_KHR Swapchain need update");
  default:
    break;
  }
  K3D_VK_VERIFY(Ret);
}

void
SwapChainImpl::InitSurface(void* WindowHandle)
{
#if K3DPLATFORM_OS_WIN
  VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
  SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
  SurfaceCreateInfo.hwnd = (HWND)WindowHandle;
  K3D_VK_VERIFY(
    vkCreateWin32SurfaceKHR(m_Device->m_Gpu->GetInstance()->m_Instance,
      &SurfaceCreateInfo,
      nullptr,
      &m_Surface));
#elif K3DPLATFORM_OS_ANDROID
  VkAndroidSurfaceCreateInfoKHR SurfaceCreateInfo = {};
  SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
  SurfaceCreateInfo.window = (ANativeWindow*)WindowHandle;
  K3D_VK_VERIFY(
    vkCreateAndroidSurfaceKHR(m_Device->m_Gpu->GetInstance()->m_Instance,
      &SurfaceCreateInfo,
      nullptr,
      &m_Surface));
#elif K3DPLATFORM_OS_LINUX
  struct Handle
  {
      ::Display*      Display;
      ::Window        Window;
  };

  VkXlibSurfaceCreateInfoKHR SurfaceCreateInfo = {};
  SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  SurfaceCreateInfo.dpy = reinterpret_cast<Handle*>(WindowHandle)->Display;
  SurfaceCreateInfo.window = reinterpret_cast<Handle*>(WindowHandle)->Window;
  K3D_VK_VERIFY(
  vkCreateXlibSurfaceKHR(m_Device->m_Gpu->GetInstance()->m_Instance,
                         &SurfaceCreateInfo,
                         nullptr,
                         &m_Surface)
  );
#endif
}

VkPresentModeKHR
SwapChainImpl::ChoosePresentMode()
{
  uint32_t presentModeCount;
  K3D_VK_VERIFY(m_Device->m_Gpu->GetSurfacePresentModesKHR(
    m_Surface, &presentModeCount, NULL));
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  K3D_VK_VERIFY(m_Device->m_Gpu->GetSurfacePresentModesKHR(
    m_Surface, &presentModeCount, presentModes.data()));
  VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (size_t i = 0; i < presentModeCount; i++) {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    }
    if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
      (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
      swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }
  return swapchainPresentMode;
}

int
SwapChainImpl::ChooseQueueIndex()
{
  uint32 chosenIndex = 0;
  std::vector<VkBool32> queuePresentSupport(m_Device->GetQueueCount());

  for (uint32_t i = 0; i < m_Device->GetQueueCount(); ++i) {
    m_Device->m_Gpu->GetSurfaceSupportKHR(
      i, m_Surface, &queuePresentSupport[i]);
    if (queuePresentSupport[i]) {
      chosenIndex = i;
      break;
    }
  }

  return chosenIndex;
}

SwapChainImpl::~SwapChainImpl()
{
  Release();
}

void
SwapChainImpl::Release()
{
  VKLOG(Info, "SwapChain Destroying..");
  if (m_NativeObj) {
    vkDestroySwapchainKHR(NativeDevice(), m_NativeObj, nullptr);
    m_NativeObj = VK_NULL_HANDLE;
  }
  if (m_Surface) {
    vkDestroySurfaceKHR(
      m_Device->m_Gpu->GetInstance()->m_Instance, m_Surface, nullptr);
    m_Surface = VK_NULL_HANDLE;
  }
  if (m_smpPresent) {
    vkDestroySemaphore(NativeDevice(), m_smpPresent, nullptr);
    m_smpPresent = VK_NULL_HANDLE;
  }
  if (m_smpRenderFinish) {
    vkDestroySemaphore(NativeDevice(), m_smpRenderFinish, nullptr);
    m_smpRenderFinish = VK_NULL_HANDLE;
  }
}

void
SwapChainImpl::Resize(uint32 Width, uint32 Height)
{
  VkSurfaceCapabilitiesKHR SurfProps;
  K3D_VK_VERIFY(m_Device->m_Gpu->
    GetSurfaceCapabilitiesKHR(m_Surface, &SurfProps));
  
  if (SurfProps.currentExtent.width == m_CachedCreateInfo.imageExtent.width
    && SurfProps.currentExtent.height == m_CachedCreateInfo.imageExtent.height)
    return;
  // size changed
  m_CachedCreateInfo.imageExtent = SurfProps.currentExtent;

  vkDeviceWaitIdle(NativeDevice());
  VkSwapchainKHR oldSwapchain = NativeHandle();

  m_CachedCreateInfo.oldSwapchain = oldSwapchain;
  K3D_VK_VERIFY(vkCreateSwapchainKHR(
    NativeDevice(),
    &m_CachedCreateInfo,
    nullptr,
    &m_NativeObj)
  );

  if (oldSwapchain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(NativeDevice(), oldSwapchain, nullptr);
    // deconstruct textures
    m_Buffers.Clear();
    m_ColorImages.Clear();
  }

  K3D_VK_VERIFY(vkGetSwapchainImagesKHR(
    NativeDevice(), 
    NativeHandle(), 
    &m_ReserveBackBufferCount, 
    m_ColorImages.Data())
  );
  m_ColorImages.Resize(m_ReserveBackBufferCount);
  
  for (auto ColorImage : m_ColorImages) {
    NGFXResourceDesc ImageDesc;
    ImageDesc.Type = NGFX_TEXTURE_2D;
    ImageDesc.ViewFlags = NGFX_RESOURCE_RENDER_TARGET_VIEW;
    ImageDesc.Flag = NGFX_ACCESS_DEVICE_VISIBLE;
    ImageDesc.Origin = NGFX_RESOURCE_ORIGIN_SWAPCHAIN;
    ImageDesc.TextureDesc.Format = ConvertToRHIFormat(m_CachedCreateInfo.imageFormat);
    ImageDesc.TextureDesc.Width = m_CachedCreateInfo.imageExtent.width;
    ImageDesc.TextureDesc.Height = m_CachedCreateInfo.imageExtent.height;
    ImageDesc.TextureDesc.Depth = 1;
    ImageDesc.TextureDesc.Layers = 1;
    ImageDesc.TextureDesc.MipLevels = 1;
    m_Buffers.Append(new TextureImpl(ImageDesc,
      ColorImage, SharedDevice(), false));
  }
  VKLOG(
    Info,
    "[SwapChain::Resized] Width=%d, Height=%d.",
    m_CachedCreateInfo.imageExtent.width,
    m_CachedCreateInfo.imageExtent.height
   );
}

NGFXTextureRef
SwapChainImpl::GetCurrentTexture()
{
  return m_Buffers[m_CurrentBufferID];
}

void
SwapChainImpl::Present()
{
}

void
SwapChainImpl::QueuePresent(CommandQueueRef pQueue, NGFXFenceRef pFence)
{
  VkPresentInfoKHR PresentInfo = {};
}

::k3d::DynArray<GpuRef>
FactoryImpl::EnumGpus()
{
  DynArray<GpuRef> gpus;
  DynArray<VkPhysicalDevice> gpuDevices;
  uint32_t gpuCount = 0;
  K3D_VK_VERIFY(vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr));
  // gpus.Resize(gpuCount);
  gpuDevices.Resize(gpuCount);
  K3D_VK_VERIFY(
    vkEnumeratePhysicalDevices(m_Instance, &gpuCount, gpuDevices.Data()));
  for (auto gpu : gpuDevices) {
    auto gpuRef = GpuRef(new Gpu(gpu, this));
    gpus.Append(gpuRef);
  }
  return gpus;
}

DeviceImpl::DeviceImpl()
  : m_Gpu(nullptr)
  , m_Device(VK_NULL_HANDLE)
{
}

DeviceImpl::~DeviceImpl()
{
  Release();
}

void
DeviceImpl::Release()
{
  if (VK_NULL_HANDLE == m_Device)
    return;
  vkDeviceWaitIdle(m_Device);
  VKLOG(Info, "DeviceImpl Destroying .  -- %0x.", m_Device);
  if (m_CmdBufManager) {
    m_CmdBufManager->~CommandBufferManager();
  }
  vkDestroyCommandPool(m_Device, m_ImmediateCmdPool, nullptr);
  vkDestroyDevice(m_Device, nullptr);
  VKLOG(Info, "DeviceImpl Destroyed .  -- %0x.", m_Device);
  m_Device = VK_NULL_HANDLE;
}

NGFXDevice::Result
DeviceImpl::Create(GpuRef const& gpu, bool withDebug)
{
  m_Gpu = gpu;
  m_Device = m_Gpu->CreateLogicDevice(withDebug);
  if (m_Device) {
#ifdef VK_NO_PROTOTYPES
    LoadVulkan(m_Gpu->m_Inst->m_Instance, m_Device);
#endif
    // Create Immediate Objects
    vkGetDeviceQueue(m_Device, 0, 0, &m_ImmediateQueue);
    if (m_ImmediateQueue)
    {
      VKLOG(Info, "DeviceImpl::Create m_ImmediateQueue created.  -- %0x.", m_ImmediateQueue);
    }
    VkCommandPoolCreateInfo PoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    PoolInfo.queueFamilyIndex = m_Gpu->m_GraphicsQueueIndex;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    K3D_VK_VERIFY(vkCreateCommandPool(m_Device, &PoolInfo, nullptr, &m_ImmediateCmdPool));
    if (m_ImmediateCmdPool)
    {
      VKLOG(Info, "DeviceImpl::Create m_ImmediateCmdPool created.  -- %0x.", m_ImmediateCmdPool);
    }

    if (withDebug) {
      m_Gpu->m_Inst->SetupDebugging(VK_DEBUG_REPORT_ERROR_BIT_EXT |
                            VK_DEBUG_REPORT_WARNING_BIT_EXT |
                            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                          DebugReportCallback);
    }
    // m_ResourceManager = std::make_unique<ResourceManager>(this,
    // 1024, 1024);
    return NGFXDevice::DeviceFound;
  }
  return NGFXDevice::DeviceNotFound;
}

uint64
DeviceImpl::GetMaxAllocationCount()
{
  return m_Gpu->m_Prop.limits.maxMemoryAllocationCount;
}

CommandQueueRef
DeviceImpl::InitCmdQueue(VkQueueFlags queueTypes,
                     uint32 queueFamilyIndex,
                     uint32 queueIndex)
{
  return new CommandQueueImpl(
    this, queueTypes, queueFamilyIndex, queueIndex);
}

VkShaderModule
DeviceImpl::CreateShaderModule(NGFXShaderBundle const& Bundle)
{
  VkShaderModule shaderModule;
  VkShaderModuleCreateInfo moduleCreateInfo;

  moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  moduleCreateInfo.pNext = NULL;

  moduleCreateInfo.codeSize = Bundle.RawData.Length();
  moduleCreateInfo.pCode = (const uint32_t*)Bundle.RawData.Data();
  moduleCreateInfo.flags = 0;
  K3D_VK_VERIFY(
    vkCreateShaderModule(m_Device, &moduleCreateInfo, NULL, &shaderModule));
  return shaderModule;
}

VkCommandBuffer
DeviceImpl::AllocateImmediateCommand()
{
  VkCommandBuffer RetCmd = VK_NULL_HANDLE;
  VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  info.commandPool = m_ImmediateCmdPool;
  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  info.commandBufferCount = 1;
  K3D_VK_VERIFY(vkAllocateCommandBuffers(m_Device, &info, &RetCmd));
  return RetCmd;
}

RenderPassRef DeviceImpl::GetOrCreateRenderPass(const NGFXRenderPassDesc & desc)
{
  uint64 Hash = HashRenderPassDesc(desc);
  if (Hash != 0)
  {
    if (DeviceObjectCache::s_RenderPass.find(Hash) == std::end(DeviceObjectCache::s_RenderPass))
    {
      auto pRenderPass = new RenderPassImpl(this, desc);
      DeviceObjectCache::s_RenderPass[Hash] = pRenderPass;
    }
    else
    {
      VKLOG(Debug, "RenderPass cache Hit. 0x%0x.", DeviceObjectCache::s_RenderPass[Hash]->NativeHandle());
    }
    return RenderPassRef(DeviceObjectCache::s_RenderPass[Hash]);
  }
  else
  {
    return nullptr;
  }
}

FrameBufferRef
DeviceImpl::GetOrCreateFramebuffer(const NGFXRenderPassDesc& Info)
{
  uint64 Hash = HashAttachments(Info);
  auto Ret = DeviceObjectCache::s_CacheFrameBuffer.Get(Hash);
  if (!Ret)
  {
    auto pRenderPass = CreateRenderPass(Info);
    Ret = new FrameBufferImpl(this,
      static_cast<RenderPassImpl*>(pRenderPass), Info);
    bool CauseRetire = DeviceObjectCache::s_CacheFrameBuffer.Put(Hash, Ret);
    if (CauseRetire)
    {
      VKLOG(Debug, "Framebuffer Cache Cause Retire..");
    }
  }
  else
  {
    VKLOG(Debug, "Framebuffer Cache Hit! (0x%0x) HashCode:%ull. ",
      Ret->NativeHandle(), Hash);
  }
  return Ret;
}

bool
DeviceImpl::FindMemoryType(uint32 typeBits,
                       VkFlags requirementsMask,
                       uint32* typeIndex) const
{
#ifdef max
#undef max
  *typeIndex =
    std::numeric_limits<std::remove_pointer<decltype(typeIndex)>::type>::max();
#endif
  auto memProp = m_Gpu->m_MemProp;
  for (uint32_t i = 0; i < memProp.memoryTypeCount; ++i) {
    if (typeBits & 0x00000001) {
      if (requirementsMask ==
          (memProp.memoryTypes[i].propertyFlags & requirementsMask)) {
        *typeIndex = i;
        return true;
      }
    }
    typeBits >>= 1;
  }

  return false;
}

NGFXUAVRef
DeviceImpl::CreateUnorderedAccessView(const NGFXResourceRef& pResource, NGFXUAVDesc const& Desc)
{
  return new UnorderedAceessView(this, Desc, pResource);
}

NGFXSamplerRef
DeviceImpl::CreateSampler(const NGFXSamplerState& samplerDesc)
{
  return new SamplerImpl(this, samplerDesc);
}

k3d::PipelineLayoutKey
HashPipelineLayoutDesc(NGFXPipelineLayoutDesc const& desc)
{
  PipelineLayoutKey key;
  key.BindingKey =
    util::Hash32((const char*)desc.Bindings.Data(),
                 desc.Bindings.Count() * sizeof(NGFXShaderBinding));
  key.SetKey = util::Hash32((const char*)desc.Sets.Data(),
                            desc.Sets.Count() * sizeof(NGFXShaderSet));
  key.UniformKey =
    util::Hash32((const char*)desc.Uniforms.Data(),
                 desc.Uniforms.Count() * sizeof(NGFXShaderUniform));
  return key;
}

NGFXPipelineLayoutRef
DeviceImpl::CreatePipelineLayout(NGFXPipelineLayoutDesc const& table)
{
  // Hash the table parameter here,
  // Lookup the layout by hash code
  /*k3d::PipelineLayoutKey key = HashPipelineLayoutDesc(table);
  if (m_CachedPipelineLayout.find(key) == m_CachedPipelineLayout.end())
  {
  auto plRef = NGFXPipelineLayoutRef(new PipelineLayoutImpl(this, table));
  m_CachedPipelineLayout.insert({ key, plRef });
  }*/
  return NGFXPipelineLayoutRef(new PipelineLayoutImpl(this, table));
}

NGFXRenderpassRef
DeviceImpl::CreateRenderPass(NGFXRenderPassDesc const& RpDesc)
{
  return GetOrCreateRenderPass(RpDesc);
}

DescriptorAllocRef
DeviceImpl::NewDescriptorAllocator(uint32 maxSets, BindingArray const& bindings)
{
  uint32 key =
    util::Hash32((const char*)bindings.Data(),
                 bindings.Count() * sizeof(VkDescriptorSetLayoutBinding));
  /*if (m_CachedDescriptorPool.find(key) == m_CachedDescriptorPool.end())
  {
  DescriptorAllocator::Options options = {};
  auto descAllocRef = DescriptorAllocRef(new DescriptorAllocator(this, options,
  maxSets, bindings)); m_CachedDescriptorPool.insert({ key, descAllocRef });
  }*/
  DescriptorAllocator::Options options = {};
  return DescriptorAllocRef(
    new DescriptorAllocator(this, options, maxSets, bindings));
}

DescriptorSetLayoutRef
DeviceImpl::NewDescriptorSetLayout(BindingArray const& bindings)
{
  uint32 key =
    util::Hash32((const char*)bindings.Data(),
                 bindings.Count() * sizeof(VkDescriptorSetLayoutBinding));
  /*if (m_CachedDescriptorSetLayout.find(key) ==
  m_CachedDescriptorSetLayout.end())
  {
  auto descSetLayoutRef = DescriptorSetLayoutRef(new DescriptorSetLayout(this,
  bindings)); m_CachedDescriptorSetLayout.insert({ key, descSetLayoutRef });
  }*/
  return DescriptorSetLayoutRef(
    new DescriptorSetLayout(this, bindings));
}

NGFXPipelineStateRef
DeviceImpl::CreateRenderPipelineState(NGFXRenderPipelineDesc const& Desc,
                                  NGFXPipelineLayoutRef pPipelineLayout,
                                  NGFXRenderpassRef pRenderPass)
{
  return new RenderPipelineStateImpl(
    this,
    Desc, pPipelineLayout, pRenderPass);
}

NGFXPipelineStateRef
DeviceImpl::CreateComputePipelineState(NGFXComputePipelineDesc const& Desc,
                                   NGFXPipelineLayoutRef pPipelineLayout)
{
  return new ComputePipelineStateImpl(
    this,
    Desc,
    static_cast<PipelineLayoutImpl*>(pPipelineLayout));
}

NGFXFenceRef
DeviceImpl::CreateFence()
{
  return new FenceImpl(this);
}

NGFXCommandQueueRef
DeviceImpl::CreateCommandQueue(NGFXCommandType const& Type)
{
  if (Type == NGFX_COMMAND_GRAPHICS) {
    return new CommandQueueImpl(
      this, VK_QUEUE_GRAPHICS_BIT, m_Gpu->m_GraphicsQueueIndex, 0);
  } else if (Type == NGFX_COMMAND_COMPUTE) {
    return new CommandQueueImpl(
      this, VK_QUEUE_COMPUTE_BIT, m_Gpu->m_ComputeQueueIndex, 0);
  }
  return nullptr;
}

NGFXResourceRef
DeviceImpl::CreateResource(NGFXResourceDesc const& Desc)
{
  NGFXResource* resource = nullptr;
  switch (Desc.Type) {
    case NGFX_BUFFER:
      resource = new BufferImpl(this, Desc);
      break;
    case NGFX_TEXTURE_1D:
      break;
    case NGFX_TEXTURE_2D:
      resource = new TextureImpl(this, Desc);
      break;
    default:
      break;
  }
  return NGFXResourceRef(resource);
}

NGFXSRVRef
DeviceImpl::CreateShaderResourceView(NGFXResourceRef pRes,
                              NGFXSRVDesc const& Desc)
{
  return new ShaderResourceView(this, Desc, pRes);
}

CommandAllocatorRef
DeviceImpl::NewCommandAllocator(bool transient)
{
  return CommandAllocator::CreateAllocator(
    m_Gpu->m_GraphicsQueueIndex, transient, this);
}

SemaphoreRef
DeviceImpl::NewSemaphore()
{
  return new SemaphoreImpl(this);
}

void
DeviceImpl::QueryTextureSubResourceLayout(NGFXTextureRef resource,
                                      NGFXTextureSpec const& spec,
                                      NGFXSubResourceLayout* layout)
{
  K3D_ASSERT(resource);
  VkImageSubresource SubRes = { spec.Aspect, spec.MipLevel, spec.ArrayLayer };
  auto texture = static_cast<TextureImpl*>(resource);
  vkGetImageSubresourceLayout(m_Device, texture->NativeHandle(), 
    &SubRes, (VkSubresourceLayout*)layout);
}

#if K3DPLATFORM_OS_WINDOWS
#define PLATFORM_SURFACE_EXT VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(K3DPLATFORM_OS_LINUX) && !defined(K3DPLATFORM_OS_ANDROID)
#define PLATFORM_SURFACE_EXT VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#elif defined(K3DPLATFORM_OS_ANDROID)
#define PLATFORM_SURFACE_EXT VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif

::k3d::DynArray<VkLayerProperties> gVkLayerProps;
::k3d::DynArray<VkExtensionProperties> gVkExtProps;

FactoryImpl::FactoryImpl(const ::k3d::String& engineName,
                   const ::k3d::String& appName,
                   bool enableValidation)
  : m_EnableValidation(enableValidation)
  , m_Instance(VK_NULL_HANDLE)
  , m_DebugMsgCallback(VK_NULL_HANDLE)
{
  LoadGlobalProcs();
  EnumExtsAndLayers();
  ExtractEnabledExtsAndLayers();

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName.CStr();
  appInfo.pEngineName = engineName.CStr();
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 1);
  appInfo.engineVersion = 1;
  appInfo.applicationVersion = 0;

  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledExtensionCount = m_EnabledExtsRaw.Count();
  instanceCreateInfo.ppEnabledExtensionNames = m_EnabledExtsRaw.Data();
  instanceCreateInfo.enabledLayerCount = m_EnabledLayersRaw.Count();
  instanceCreateInfo.ppEnabledLayerNames = m_EnabledLayersRaw.Data();

  VkResult err = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
  if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
    VKLOG(Error,
          "Cannot find a compatible Vulkan installable client driver: "
          "vkCreateInstance Failure");
  } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
    VKLOG(
      Error,
      "Cannot find a specified extension library: vkCreateInstance Failure");
  } else {
    LoadInstanceProcs();
    EnumGpus();
  }
}

FactoryImpl::~FactoryImpl()
{
  Release();
}

void
FactoryImpl::Release()
{
  VKLOG(Info,
        "FactoryImpl Destroying...  -- %0x. (tid:%d)",
        m_Instance,
        Os::Thread::GetId());
  if (m_Instance) {
    FreeDebugCallback();
    vkDestroyInstance(m_Instance, nullptr);
    VKLOG(Info, "FactoryImpl Destroyed .  -- %0x.", m_Instance);
    m_Instance = VK_NULL_HANDLE;
  }
}

void
FactoryImpl::EnumDevices(DynArray<NGFXDeviceRef>& Devices)
{
  auto GpuRefs = EnumGpus();
  for (auto& gpu : GpuRefs) {
    auto pDevice = new DeviceImpl;
    pDevice->Create(gpu, WithValidation());
    Devices.Append(NGFXDeviceRef(pDevice));
  }
}

NGFXSwapChainRef
FactoryImpl::CreateSwapchain(NGFXCommandQueueRef pCommandQueue,
                          void* nPtr,
                          NGFXSwapChainDesc& Desc)
{
  CommandQueueRef pQueue = static_cast<CommandQueueImpl*>(pCommandQueue);
  return new SwapChainImpl(pQueue->SharedDevice(), nPtr, Desc);
}

#ifdef VK_NO_PROTOTYPES
#define __VK_GLOBAL_PROC_GET__(name, functor)                                  \
  gp##name = reinterpret_cast<PFN_vk##name>(functor("vk" K3D_STRINGIFY(name)))
#endif
void
FactoryImpl::LoadGlobalProcs()
{
#ifdef VK_NO_PROTOTYPES
#ifdef K3DPLATFORM_OS_WIN
  static const char* LIBVULKAN = "vulkan-1.dll";
#elif defined(K3DPLATFORM_OS_MAC)
  static const char* LIBVULKAN = "libvulkan.dylib";
#else
  static const char* LIBVULKAN = "libvulkan.so";
#endif

  m_VulkanLib = new dynlib::Lib(LIBVULKAN);
  // load global functions
  __VK_GLOBAL_PROC_GET__(CreateInstance, m_VulkanLib->ResolveEntry);
  __VK_GLOBAL_PROC_GET__(DestroyInstance, m_VulkanLib->ResolveEntry);
  __VK_GLOBAL_PROC_GET__(EnumerateInstanceExtensionProperties,
                         m_VulkanLib->ResolveEntry);
  __VK_GLOBAL_PROC_GET__(EnumerateInstanceLayerProperties,
                         m_VulkanLib->ResolveEntry);
  __VK_GLOBAL_PROC_GET__(GetInstanceProcAddr, m_VulkanLib->ResolveEntry);
#endif
}

void
FactoryImpl::EnumExtsAndLayers()
{
  uint32 layerCount = 0;
  K3D_VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
  if (layerCount > 0) {
    gVkLayerProps.Resize(layerCount);
    K3D_VK_VERIFY(
      vkEnumerateInstanceLayerProperties(&layerCount, gVkLayerProps.Data()));
  }
  uint32 extCount = 0;
  K3D_VK_VERIFY(
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr));
  if (extCount > 0) {
    gVkExtProps.Resize(extCount);
    K3D_VK_VERIFY(vkEnumerateInstanceExtensionProperties(
      nullptr, &extCount, gVkExtProps.Data()));
  }
  VKLOG(Info,
        ">> FactoryImpl::EnumLayersAndExts <<\n"
        "=================================>> layerCount = %d.\n"
        "=================================>> extensionCount = %d.\n",
        layerCount,
        extCount);
}

void
FactoryImpl::ExtractEnabledExtsAndLayers()
{
  VkBool32 surfaceExtFound = 0;
  VkBool32 platformSurfaceExtFound = 0;

  for (uint32_t i = 0; i < gVkExtProps.Count(); i++) {
    if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, gVkExtProps[i].extensionName)) {
      surfaceExtFound = 1;
      m_EnabledExts.Append(VK_KHR_SURFACE_EXTENSION_NAME);
      m_EnabledExtsRaw.Append(VK_KHR_SURFACE_EXTENSION_NAME);
    }
    if (!strcmp(PLATFORM_SURFACE_EXT, gVkExtProps[i].extensionName)) {
      platformSurfaceExtFound = 1;
      m_EnabledExts.Append(PLATFORM_SURFACE_EXT);
      m_EnabledExtsRaw.Append(PLATFORM_SURFACE_EXT);
    }
    if (m_EnableValidation) {
      m_EnabledExts.Append(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
      m_EnabledExtsRaw.Append(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    VKLOG(Info, "available extension : %s .", gVkExtProps[i].extensionName);
  }
  if (!surfaceExtFound) {
    VKLOG(Error,
          "vkEnumerateInstanceExtensionProperties failed to find "
          "the " VK_KHR_SURFACE_EXTENSION_NAME " extension.");
  }
  if (!platformSurfaceExtFound) {
    VKLOG(Error,
          "vkEnumerateInstanceExtensionProperties failed to find "
          "the " PLATFORM_SURFACE_EXT " extension.");
  }

  if (m_EnableValidation && !gVkLayerProps.empty()) {
    for (auto prop : gVkLayerProps) {
      if (strcmp(prop.layerName, g_ValidationLayerNames[0]) ==
          0 /*|| strcmp(prop.layerName, g_ValidationLayerNames[1])==0*/) {
        m_EnabledLayers.Append(prop.layerName);
        m_EnabledLayersRaw.Append(g_ValidationLayerNames[0]);
        VKLOG(Info, "enable validation layer [%s].", prop.layerName);
        break;
      }
    }
  }
}

void
FactoryImpl::LoadInstanceProcs()
{
  CreateDebugReport = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
#ifdef VK_NO_PROTOTYPES
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceSupportKHR);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceFormatsKHR);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfacePresentModesKHR);

  GET_INSTANCE_PROC_ADDR(m_Instance, EnumeratePhysicalDevices);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceFeatures);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceProperties);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceMemoryProperties);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceFormatProperties);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceQueueFamilyProperties);
  GET_INSTANCE_PROC_ADDR(m_Instance, CreateDevice);
  GET_INSTANCE_PROC_ADDR(m_Instance, GetDeviceProcAddr);

#if K3DPLATFORM_OS_WIN
  GET_INSTANCE_PROC_ADDR(m_Instance, CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
  GET_INSTANCE_PROC_ADDR(m_Instance, CreateAndroidSurfaceKHR);
#endif
  GET_INSTANCE_PROC_ADDR(m_Instance, DestroySurfaceKHR);
#endif
}

PFN_vkDebugReportMessageEXT dbgBreakCallback;

static VkDebugReportCallbackEXT msgCallback = NULL;

const char*
DebugLevelString(VkDebugReportFlagsEXT lev)
{
  switch (lev) {
#define STR(r)                                                                 \
  case VK_DEBUG_REPORT_##r##_BIT_EXT:                                          \
    return #r
    STR(INFORMATION);
    STR(WARNING);
    STR(PERFORMANCE_WARNING);
    STR(ERROR);
    STR(DEBUG);
#undef STR
    default:
      return "UNKNOWN_ERROR";
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugReportCallback(VkDebugReportFlagsEXT flags,
                    VkDebugReportObjectTypeEXT objectType,
                    uint64_t object,
                    size_t location,
                    int32_t messageCode,
                    const char* pLayerPrefix,
                    const char* pMessage,
                    void* pUserData)
{
  static char msg[4096] = { 0 };
  switch (flags) {
    case VK_DEBUG_REPORT_WARNING_BIT_EXT:
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
      VKLOG(Error, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
      break;
    case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
      VKLOG(Info, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
      break;
    case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
      VKLOG(Debug, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
      break;
    default:
      VKLOG(
        Default, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
      break;
  }
  return VK_TRUE;
}

#define __VK_GLOBAL_PROC_GET__(name, functor)                                  \
  fp##name = reinterpret_cast<PFN_vk##name>(functor("vk" K3D_STRINGIFY(name)))

void
FactoryImpl::SetupDebugging(VkDebugReportFlagsEXT flags,
                         PFN_vkDebugReportCallbackEXT callBack)
{
  if (!m_Instance) {
    VKLOG(Error, "SetupDebugging Failed. (m_Instance == null)");
    return;
  }
#ifdef VK_NO_PROTOTYPES
  __VK_GLOBAL_PROC_GET__(CreateDebugReportCallbackEXT,
                         m_VulkanLib->ResolveEntry);
  if (!vkCreateDebugReportCallbackEXT)
    GET_INSTANCE_PROC_ADDR(m_Instance, CreateDebugReportCallbackEXT);
  __VK_GLOBAL_PROC_GET__(DestroyDebugReportCallbackEXT,
                         m_VulkanLib->ResolveEntry);
  if (!fpDestroyDebugReportCallbackEXT)
    GET_INSTANCE_PROC_ADDR(m_Instance, DestroyDebugReportCallbackEXT);
#endif
  VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
  dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
  dbgCreateInfo.pNext = NULL;
  dbgCreateInfo.pfnCallback = callBack;
  dbgCreateInfo.pUserData = NULL;
  dbgCreateInfo.flags = flags;
  K3D_VK_VERIFY(CreateDebugReport(m_Instance, &dbgCreateInfo, NULL, &m_DebugMsgCallback));
}

void
FactoryImpl::FreeDebugCallback()
{
  if (m_Instance && m_DebugMsgCallback) {
    //vkDestroyDebugReportCallbackEXT(m_Instance, m_DebugMsgCallback, nullptr);
    m_DebugMsgCallback = VK_NULL_HANDLE;
  }
}


CommandBufferManager::CommandBufferManager(VkDevice gpu, VkCommandBufferLevel bufferLevel,
  unsigned graphicsQueueIndex)
  : m_Device(gpu)
  , m_CommandBufferLevel(bufferLevel)
  , m_Count(0)
{
  // RESET_COMMAND_BUFFER_BIT allows command buffers to be reset individually.
  VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
  info.queueFamilyIndex = graphicsQueueIndex;
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  K3D_VK_VERIFY(vkCreateCommandPool(m_Device, &info, nullptr, &m_Pool));
}

CommandBufferManager::~CommandBufferManager()
{
  Destroy();
}

void CommandBufferManager::Destroy()
{
  if (!m_Pool)
    return;
  vkDeviceWaitIdle(m_Device);
  VKLOG(Info, "CommandBufferManager destroy. -- 0x%0x. thread (%s).", m_Pool, Os::Thread::GetCurrentThreadName().CStr());
  vkFreeCommandBuffers(m_Device, m_Pool, m_Buffers.Count(), m_Buffers.Data());
  vkDestroyCommandPool(m_Device, m_Pool, nullptr);
  m_Pool = VK_NULL_HANDLE;
}

void CommandBufferManager::BeginFrame()
{
  Os::Thread::GetId();
  m_Count = 0;
}

VkCommandBuffer CommandBufferManager::RequestCommandBuffer()
{
  // Either we recycle a previously allocated command buffer, or create a new one.
  VkCommandBuffer ret = VK_NULL_HANDLE;
  if (m_Count < m_Buffers.Count())
  {
    ret = m_Buffers[m_Count++];
    K3D_VK_VERIFY(vkResetCommandBuffer(ret, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
  }
  else
  {
    VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool = m_Pool;
    info.level = m_CommandBufferLevel;
    info.commandBufferCount = 1;
    K3D_VK_VERIFY(vkAllocateCommandBuffers(m_Device, &info, &ret));
    m_Buffers.Append(ret);

    m_Count++;
  }

  return ret;
}

Gpu::Gpu(VkPhysicalDevice const& gpu, InstanceRef const& pInst)
  : m_Inst(pInst)
  , m_LogicalDevice(VK_NULL_HANDLE)
  , m_PhysicalGpu(gpu)
{
  m_Inst->AddInternalRef();
  vkGetPhysicalDeviceProperties(m_PhysicalGpu, &m_Prop);
  vkGetPhysicalDeviceMemoryProperties(m_PhysicalGpu, &m_MemProp);
  vkGetPhysicalDeviceFeatures(m_PhysicalGpu, &m_Features);
  VKLOG(Info, "Gpu: %s", m_Prop.deviceName);
  QuerySupportQueues();
}

void Gpu::QuerySupportQueues()
{
  uint32 queueCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalGpu, &queueCount, NULL);
  if (queueCount < 1)
    return;
  m_QueueProps.Resize(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalGpu, &queueCount, m_QueueProps.Data());
  uint32 qId = 0;
  for (qId = 0; qId < queueCount; qId++)
  {
    if (m_QueueProps[qId].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      m_GraphicsQueueIndex = qId;
      VKLOG(Info, "DeviceImpl::graphicsQueueIndex(%d) queueFlags(%d).", m_GraphicsQueueIndex, m_QueueProps[qId].queueFlags);
      break;
    }
  }
  for (qId = 0; qId < queueCount; qId++)
  {
    if (m_QueueProps[qId].queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
      m_ComputeQueueIndex = qId;
      VKLOG(Info, "DeviceImpl::ComputeQueueIndex(%d).", m_ComputeQueueIndex);
      break;
    }
  }
  for (qId = 0; qId < queueCount; qId++)
  {
    if (m_QueueProps[qId].queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      m_CopyQueueIndex = qId;
      VKLOG(Info, "DeviceImpl::CopyQueueIndex(%d).", m_CopyQueueIndex);
      break;
    }
  }
}

#define __VK_GET_DEVICE_PROC__(name, getDeviceProc, device) \
if(!vk##name) \
{\
vk##name = (PFN_vk##name)getDeviceProc(device, "vk" K3D_STRINGIFY(name)); \
if (!vk##name) \
{\
VKLOG(Fatal, "LoadDeviceProcs::" K3D_STRINGIFY(name) " not exist!" );\
exit(-1);\
}\
}

void Gpu::LoadDeviceProcs()
{
#ifdef VK_NO_PROTOTYPES
  if (!fpDestroyDevice)
  {
    fpDestroyDevice = (PFN_vkDestroyDevice)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkDestroyDevice");
  }
  if (!fpFreeCommandBuffers)
  {
    fpFreeCommandBuffers = (PFN_vkFreeCommandBuffers)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkFreeCommandBuffers");
  }
  if (!fpCreateCommandPool)
  {
    fpCreateCommandPool = (PFN_vkCreateCommandPool)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkCreateCommandPool");
  }
  if (!fpAllocateCommandBuffers)
  {
    fpAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkAllocateCommandBuffers");
  }
  __VK_GET_DEVICE_PROC__(DestroyDevice, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetDeviceQueue, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(QueueSubmit, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(QueueWaitIdle, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(QueuePresentKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DeviceWaitIdle, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(AllocateMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(FreeMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(MapMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(UnmapMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(FlushMappedMemoryRanges, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(InvalidateMappedMemoryRanges, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetDeviceMemoryCommitment, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(BindBufferMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(BindImageMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetBufferMemoryRequirements, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetImageMemoryRequirements, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetImageSparseMemoryRequirements, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(QueueBindSparse, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateFence, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyFence, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(ResetFences, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetFenceStatus, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(WaitForFences, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateSemaphore, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroySemaphore, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetEventStatus, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(SetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(ResetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateQueryPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyQueryPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetQueryPoolResults, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateBufferView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyBufferView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetImageSubresourceLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateImageView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyImageView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateShaderModule, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyShaderModule, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreatePipelineCache, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyPipelineCache, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetPipelineCacheData, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(MergePipelineCaches, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateGraphicsPipelines, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateComputePipelines, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyPipeline, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreatePipelineLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyPipelineLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateSampler, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroySampler, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateDescriptorSetLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyDescriptorSetLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateDescriptorPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyDescriptorPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(ResetDescriptorPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(AllocateDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(FreeDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(UpdateDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateFramebuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyFramebuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetRenderAreaGranularity, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateCommandPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroyCommandPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(ResetCommandPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(AllocateCommandBuffers, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(FreeCommandBuffers, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(BeginCommandBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(EndCommandBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(ResetCommandBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBindPipeline, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetViewport, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetScissor, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetLineWidth, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetDepthBias, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetBlendConstants, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetDepthBounds, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetStencilCompareMask, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetStencilWriteMask, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetStencilReference, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBindDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBindIndexBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBindVertexBuffers, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdDraw, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdDrawIndexed, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdDrawIndirect, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdDrawIndexedIndirect, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdDispatch, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdDispatchIndirect, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdCopyBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdCopyImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBlitImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdCopyBufferToImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdCopyImageToBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdUpdateBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdFillBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdClearColorImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdClearDepthStencilImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdClearAttachments, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdResolveImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdSetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdResetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdWaitEvents, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdPipelineBarrier, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBeginQuery, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdEndQuery, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdResetQueryPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdWriteTimestamp, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdCopyQueryPoolResults, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdPushConstants, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdBeginRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdNextSubpass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdEndRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CmdExecuteCommands, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(AcquireNextImageKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(CreateSwapchainKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(DestroySwapchainKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
  __VK_GET_DEVICE_PROC__(GetSwapchainImagesKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
#endif
}

Gpu::~Gpu()
{
  m_Inst->ReleaseInternal();
  VKLOG(Info, "Gpu Destroyed....");
  if (m_PhysicalGpu)
  {
    m_PhysicalGpu = VK_NULL_HANDLE;
  }
}

VkDevice Gpu::CreateLogicDevice(bool enableValidation)
{
  if (!m_LogicalDevice)
  {
    std::array<float, 1> queuePriorities = { 0.0f };
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_GraphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities.data();

    std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &m_Features;

    if (enableValidation)
    {
      deviceCreateInfo.enabledLayerCount = 1;
      deviceCreateInfo.ppEnabledLayerNames = g_ValidationLayerNames;
    }

    if (enabledExtensions.size() > 0)
    {
      deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
      deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    K3D_VK_VERIFY(vkCreateDevice(m_PhysicalGpu, &deviceCreateInfo, nullptr, &m_LogicalDevice));

    LoadDeviceProcs();
  }
  return m_LogicalDevice;
}

VkBool32 Gpu::GetSupportedDepthFormat(VkFormat * depthFormat)
{
  // Since all depth formats may be optional, we need to find a suitable depth format to use
  // Start with the highest precision packed format
  std::vector<VkFormat> depthFormats = {
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM
  };

  for (auto& format : depthFormats)
  {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(m_PhysicalGpu, format, &formatProps);
    // Format must support depth stencil attachment for optimal tiling
    if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      *depthFormat = format;
      return true;
    }
  }

  return false;
}

VkResult Gpu::GetSurfaceSupportKHR(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 * pSupported)
{
  return vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalGpu, queueFamilyIndex, surface, pSupported);
}

VkResult Gpu::GetSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR * pSurfaceCapabilities)
{
  return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalGpu, surface, pSurfaceCapabilities);
}

VkResult Gpu::GetSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t * pSurfaceFormatCount, VkSurfaceFormatKHR * pSurfaceFormats)
{
  return vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalGpu, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult Gpu::GetSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t * pPresentModeCount, VkPresentModeKHR * pPresentModes)
{
  return vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalGpu, surface, pPresentModeCount, pPresentModes);
}

void Gpu::DestroyDevice()
{
  vkDestroyDevice(m_LogicalDevice, nullptr);
}

void Gpu::FreeCommandBuffers(VkCommandPool pool, uint32 count, VkCommandBuffer * cmd)
{
  vkFreeCommandBuffers(m_LogicalDevice, pool, count, cmd);
}

VkResult Gpu::CreateCommdPool(const VkCommandPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkCommandPool * pCommandPool)
{
  return vkCreateCommandPool(m_LogicalDevice, pCreateInfo, pAllocator, pCommandPool);
}

VkResult Gpu::AllocateCommandBuffers(const VkCommandBufferAllocateInfo * pAllocateInfo, VkCommandBuffer * pCommandBuffers)
{
  return vkAllocateCommandBuffers(m_LogicalDevice, pAllocateInfo, pCommandBuffers);
}

class RHIImpl
  : public ::k3d::IVkRHI
{
public:
  RHIImpl() {}

  ~RHIImpl() override
  {
    VKLOG(Info, "Destroying..");
    Shutdown();
  }

  void Initialize(const char* appName, bool debug) override
  {
    if (!m_Instance) {
      m_Instance = new vk::FactoryImpl(appName, appName, debug);
    }
  }

  NGFXFactoryRef GetFactory() { return m_Instance; }

  void Start() override
  {
    if (!m_Instance) {
      VKLOG(Fatal,
            "RHI Module uninitialized!! You should call Initialize first!");
      return;
    }
  }

  void Shutdown() override { KLOG(Info, VKRHI, "Shuting down..."); }

  const char* Name() { return "RHI_Vulkan"; }

private:
  vk::InstanceRef m_Instance = nullptr;
};

K3D_VK_END

// RHI Exposed Interface
MODULE_IMPLEMENT(RHI_Vulkan, k3d::vk::RHIImpl)