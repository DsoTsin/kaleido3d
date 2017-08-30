#include "RHIUtil.h"
#include <Core/Utils/farmhash.h>
#include <Core/Utils/SHA1.h>

using namespace kMath;

K3D_COMMON_NS
{
uint64 HashRenderPassDesc(NGFXRenderPassDesc const& Desc) 
{
  uint64 HashCode = 0x87654321L;
  auto ColorAttachments = Desc.ColorAttachments;
  struct RenderPassAttachDesc
  {
    NGFXPixelFormat Format;
    NGFXLoadAction LoadAction;
    NGFXStoreAction StoreAction;
    Vec4f ClearColor;
    RenderPassAttachDesc(const NGFXColorAttachmentDesc& Desc)
      : Format(Desc.pTexture->GetDesc().TextureDesc.Format)
      , LoadAction(Desc.LoadAction)
      , StoreAction(Desc.StoreAction)
 //     , ClearColor(Desc.ClearColor)
    {
    }
  };
  DynArray<RenderPassAttachDesc> RenderPassDescs;
  for (auto RenderPassDesc : ColorAttachments)
  {
    RenderPassDescs.Append(RenderPassDesc);
  }
  HashCode = util::Hash64(
    (const char*)RenderPassDescs.Data(), 
    sizeof(RenderPassAttachDesc) * RenderPassDescs.Count());
  
  if (Desc.pDepthAttachment)
  {

  }


  return HashCode;
}

uint64 HashTextureDesc(NGFXResourceDesc const& Desc)
{
  return util::Hash64((const char*)&Desc, sizeof(Desc));
}

uint64 HashAttachments(NGFXRenderPassDesc const& Desc)
{
  uint64 HashCode = 0x12345678L;
  auto Attachments = Desc.ColorAttachments;
  for (auto Attachment : Attachments)
  {
    auto TextureAddr = Attachment.pTexture->GetLocation();
    auto TextureDesc = Attachment.pTexture->GetDesc();
    HashCode = util::Hash64WithSeed(
      (const char*)&TextureAddr,
      8, HashCode);
    HashCode = util::Hash64WithSeed(
      (const char*)&TextureDesc,
      sizeof(TextureDesc), HashCode);
  }
  return HashCode;
}

}
