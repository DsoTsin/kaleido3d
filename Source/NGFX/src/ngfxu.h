#pragma once
#ifndef __NGFXU__
#define __NGFXU__

#include "ngfx.h"

namespace ngfxu
{
  NGFX_API ngfx::RenderPipelineDesc CreateDefaultRenderPipelineDesc();

  NGFX_API ngfx::Ptr<ngfx::Texture> CreateSampledTexture2D(ngfx::Ptr<ngfx::Device> device,
                                                           ngfx::PixelFormat const& format,
                                                           uint32_t width, uint32_t height);

  NGFX_API ngfx::Ptr<ngfx::Texture> CreateRenderTexture2D(ngfx::Ptr<ngfx::Device> device,
                                                          ngfx::PixelFormat const& format,
                                                          uint32_t width, uint32_t height);

  NGFX_API ngfx::Ptr<ngfx::Texture> CreateDepthStencilTexture(ngfx::Ptr<ngfx::Device> device,
                                                              ngfx::PixelFormat const& format, 
                                                              uint32_t width, uint32_t height);
}

#endif