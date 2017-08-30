#ifndef __RHIUtil_h__
#define __RHIUtil_h__

#include <Core/Interface/IRHI.h>
#include <map>

K3D_COMMON_NS
{
  struct PipelineLayoutKey
  {
    uint32 BindingKey = 0;
    uint32 SetKey = 0;
    uint32 UniformKey = 0;
    bool operator==(PipelineLayoutKey const& rhs)
    {
      return BindingKey == rhs.BindingKey && SetKey == rhs.SetKey &&
             UniformKey == rhs.UniformKey;
    }
    bool operator<(PipelineLayoutKey const& rhs) const
    {
      return BindingKey < rhs.BindingKey || SetKey < rhs.SetKey ||
             UniformKey < rhs.UniformKey;
    }
  };

  struct RenderPassKey
  {
    uint32 DepthAttachKey;
    uint32 StencilAttachKey;
    uint32 ColorAttachmentsKey;
  };

  uint64 K3D_CORE_API HashTextureDesc(NGFXResourceDesc const& Desc);
  // Hash renderpass for compat usage
  uint64 K3D_CORE_API HashRenderPassDesc(NGFXRenderPassDesc const& Desc);
  // For Vulkan Framebuffer Cache
  uint64 K3D_CORE_API HashAttachments(NGFXRenderPassDesc const& Desc);

  typedef std::map<k3d::PipelineLayoutKey, NGFXPipelineLayoutRef>
    MapPipelineLayout;
}

#endif