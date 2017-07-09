#include "Kaleido3D.h"
#include "ngfxu.h"

using namespace ngfx;

namespace ngfxu
{
  RenderPipelineDesc CreateDefaultRenderPipelineDesc()
  {
    RenderPipelineDesc d;

    d.rasterState.SetCullMode(CullMode::Back)
      .SetDepthBias(0)
      .SetFillMode(FillMode::Solid)
      .SetFrontCCW(true)
      .SetMultiSampleEnable(false);

    d.depthStencil.SetDepthEnable(true)
      .SetDepthFunc(ComparisonFunction::Greater)
      .SetDepthWriteMask(DepthWriteMask::All)
      .SetStencilEnable(true);
    
    d.SetPrimitiveTopology(PrimitiveType::Triangles);

    return d;
  }

  Ptr<Texture> CreateSampledTexture2D(Ptr<Device> device, PixelFormat const & format, uint32_t width, uint32_t height)
  {
    Ptr<Texture> texture;
    TextureDesc desc;
    desc.SetFormat(format)
      .SetAllowedViewBits(TextureViewBit::ShaderRead)
      .SetLayers(1)
      .SetMipLevels(1)
      .SetWidth(width).SetHeight(height).SetDepth(1)
      .SetOption(StorageOption::Shared);
    device->CreateTexture(&desc, texture.GetAddressOf());
    return texture;
  }

  Ptr<Texture> CreateRenderTexture2D( Ptr<Device> device,
                                      PixelFormat const& format,
                                      uint32_t width, uint32_t height)
  {
    Ptr<Texture> texture;
    TextureDesc desc;
    desc.SetFormat(format)
      .SetAllowedViewBits(TextureViewBit::RenderTarget)
      .SetLayers(1)
      .SetMipLevels(1)
      .SetWidth(width).SetHeight(height).SetDepth(1)
      .SetOption(StorageOption::Private);
    device->CreateTexture(&desc, texture.GetAddressOf());
    return texture;
  }

  Ptr<Texture> CreateDepthStencilTexture(Ptr<Device> device, 
                                               PixelFormat const & format, uint32_t width, uint32_t height)
  {
    Ptr<Texture> texture;
    TextureDesc desc;
    desc.SetFormat(format)
      .SetAllowedViewBits(TextureViewBit::DepthStencil)
      .SetLayers(1)
      .SetMipLevels(1)
      .SetWidth(width).SetHeight(height).SetDepth(1)
      .SetOption(StorageOption::Private);
    device->CreateTexture(&desc, texture.GetAddressOf());
    return texture;
  }
}