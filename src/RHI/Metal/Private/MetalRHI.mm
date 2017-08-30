#include "MetalRHI.h"
#include "Public/IMetalRHI.h"
#include "MetalEnums.h"
#include <vector>

using namespace std;

NS_K3D_METAL_BEGIN

#if 0
    k3d::GpuResourceRef
    Device::NewGpuResource(k3d::ResourceDesc const& desc)
    {
        switch (desc.Type) {
            case k3d::EGT_Buffer:
                return k3d::MakeShared<Buffer>(this, desc);
            case k3d::EGT_Texture2D:
                return k3d::MakeShared<Texture>(this, desc);
            default:
                break;
        }
        return nullptr;
    }
#endif
    
#pragma mark Pipeline
    
    MTLPrimitiveTopologyClass ConvertPrimTopology(k3d::EPrimitiveType const& type)
    {
        switch(type)
        {
            case k3d::EPT_Triangles:
            case k3d::EPT_TriangleStrip:
                return MTLPrimitiveTopologyClassTriangle;
            case k3d::EPT_Points:
                return MTLPrimitiveTopologyClassPoint;
            case k3d::EPT_Lines:
                return MTLPrimitiveTopologyClassLine;
        }
        return MTLPrimitiveTopologyClassUnspecified;
    }
    
    /*
    RenderPipeline::RenderPipeline(id<MTLDevice> pDevice, k3d::RenderPipelineDesc const & desc, k3d::EPipelineType const& type)
    : m_DepthStencilDesc(nil)
    , m_RenderPipelineDesc(nil)
    , m_Type(type)
    , m_Device(pDevice)
    {
        InitPSO(desc);
        Finalize();
    }*/
    
    RenderPipeline::RenderPipeline(SpDevice pDevice, k3d::RenderPipelineStateDesc const & desc)
    : Super(pDevice)
    {
    }
    
    RenderPipeline::~RenderPipeline()
    {
        
    }
    
    void RenderPipeline::InitPSO(const k3d::RenderPipelineStateDesc &desc)
    {
        m_NativeDesc = [[MTLRenderPipelineDescriptor alloc] init];
            // depth stencil setup
            m_DepthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
            m_DepthStencilDesc.depthCompareFunction = g_ComparisonFunc[desc.DepthStencil.DepthFunc];
            m_DepthStencilDesc.depthWriteEnabled = desc.DepthStencil.DepthEnable;
            
            MTLStencilDescriptor * stencilFront = m_DepthStencilDesc.frontFaceStencil;
            stencilFront.depthStencilPassOperation = g_StencilOp[desc.DepthStencil.FrontFace.StencilPassOp];
            stencilFront.depthFailureOperation = g_StencilOp[desc.DepthStencil.FrontFace.DepthStencilFailOp];
            stencilFront.stencilFailureOperation = g_StencilOp[desc.DepthStencil.FrontFace.StencilFailOp];
            stencilFront.stencilCompareFunction = g_ComparisonFunc[desc.DepthStencil.FrontFace.StencilFunc];
            
            MTLStencilDescriptor * stencilBack = m_DepthStencilDesc.backFaceStencil;
            stencilBack.depthStencilPassOperation = g_StencilOp[desc.DepthStencil.BackFace.StencilPassOp];
            stencilBack.depthFailureOperation = g_StencilOp[desc.DepthStencil.BackFace.DepthStencilFailOp];
            stencilBack.stencilFailureOperation = g_StencilOp[desc.DepthStencil.BackFace.StencilFailOp];
            stencilBack.stencilCompareFunction = g_ComparisonFunc[desc.DepthStencil.BackFace.StencilFunc];
            
            m_DepthBias = desc.Rasterizer.DepthBias;
            m_DepthBiasClamp = desc.Rasterizer.DepthBiasClamp;
            m_CullMode = g_CullMode[desc.Rasterizer.CullMode];
            m_NativeDesc.inputPrimitiveTopology = ConvertPrimTopology(desc.PrimitiveTopology);
            m_PrimitiveType = g_PrimitiveType[desc.PrimitiveTopology];
            // blending setup
            MTLRenderPipelineColorAttachmentDescriptor *renderbufferAttachment = m_NativeDesc.colorAttachments[0];
            
            renderbufferAttachment.pixelFormat = MTLPixelFormatBGRA8Unorm;
#if 0
            renderbufferAttachment.blendingEnabled = desc.Blend.Enable?YES:NO;
            renderbufferAttachment.rgbBlendOperation = g_BlendOperation[desc.Blend.Op];
            renderbufferAttachment.alphaBlendOperation = g_BlendOperation[desc.Blend.BlendAlphaOp];
            renderbufferAttachment.sourceRGBBlendFactor = g_BlendFactor[desc.Blend.Src];
            renderbufferAttachment.sourceAlphaBlendFactor = g_BlendFactor[desc.Blend.SrcBlendAlpha];
            renderbufferAttachment.destinationRGBBlendFactor = g_BlendFactor[desc.Blend.Dest];
            renderbufferAttachment.destinationAlphaBlendFactor = g_BlendFactor[desc.Blend.DestBlendAlpha];
#endif
            m_NativeDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
            
            
            // vertex descriptor setup
            for(uint32 i = 0; i<k3d::VertexInputState::kMaxVertexBindings; i++)
            {
                auto attrib = desc.InputState.Attribs[i];
                if(attrib.Slot==k3d::VertexInputState::kInvalidValue)
                    break;
                m_NativeDesc.vertexDescriptor.attributes[i].format = g_VertexFormats[attrib.Format];
                m_NativeDesc.vertexDescriptor.attributes[i].offset = attrib.OffSet;
                m_NativeDesc.vertexDescriptor.attributes[i].bufferIndex = attrib.Slot;
            }
            
            for(uint32 i = 0; i<k3d::VertexInputState::kMaxVertexBindings; i++)
            {
                auto layout = desc.InputState.Layouts[i];
                if(layout.Stride == k3d::VertexInputState::kInvalidValue)
                    break;
                m_NativeDesc.vertexDescriptor.layouts[i].stride = layout.Stride;
                m_NativeDesc.vertexDescriptor.layouts[i].stepRate = 1;
                m_NativeDesc.vertexDescriptor.layouts[i].stepFunction = g_VertexInputRates[layout.Rate];
            }
            
            // shader setup
            AssignShader(desc.VertexShader);
            AssignShader(desc.PixelShader);
        }
        //    m_ComputePipelineDesc = [[MTLComputePipelineDescriptor alloc] init];
    
        //    m_RenderPipelineDesc.

    
    void RenderPipeline::AssignShader(const k3d::ShaderBundle &shaderBundle)
    {
            if(m_NativeDesc)
            {
                const auto & shaderData = shaderBundle.RawData;
                if (shaderData.Length() == 0)
                    return;
                NSError * error = nil;
                dispatch_data_t data = dispatch_data_create(shaderData.Data(), shaderData.Length(), nil, nil);
                id<MTLLibrary> lib = [NativeDevice() newLibraryWithData:data error:&error];
                if(error)
                {
                    MTLLOGE("Failed to AssignShader, error: %s, occured %s@%d.", [[error localizedFailureReason] UTF8String], __FILE__, __LINE__);
                }
                NSString *entryName = [NSString stringWithFormat:@"%@0",
                                       [NSString stringWithUTF8String:shaderBundle.Desc.EntryFunction.CStr()]];
                id<MTLFunction> function = [lib newFunctionWithName:entryName];
                switch(shaderBundle.Desc.Stage)
                {
                    case k3d::ES_Vertex:
                        m_NativeDesc.vertexFunction = function;
                        break;
                    case k3d::ES_Fragment:
                        m_NativeDesc.fragmentFunction = function;
                        break;
                }
            }
    }
    
    void RenderPipeline::SetRasterizerState(const k3d::RasterizerState&)
    {
        
    }
    
    void RenderPipeline::SetBlendState(const k3d::BlendState&)
    {
        
    }
    
    void RenderPipeline::SetDepthStencilState(const k3d::DepthStencilState&)
    {
        
    }
    
    void RenderPipeline::SetPrimitiveTopology(const k3d::EPrimitiveType)
    {
        
    }
    
    void RenderPipeline::SetRenderTargetFormat(const k3d::RenderTargetFormat &)
    {
        
    }
    
    void RenderPipeline::SetSampler(k3d::SamplerRef)
    {
        
    }
    
    void RenderPipeline::Rebuild()
    {
        NSError *error = NULL;
            m_DepthStencilState = [NativeDevice() newDepthStencilStateWithDescriptor:m_DepthStencilDesc];
            m_NativeObj = [NativeDevice() newRenderRenderPipelineWithDescriptor:m_NativeDesc error:&error];
            if (error)
            {
                MTLLOGE("Failed to created render pipeline state, error: %s", [error.localizedDescription cStringUsingEncoding:NSASCIIStringEncoding]);
            }
    }

#pragma mark Command

#if 0
    CommandContext::CommandContext(k3d::ECommandType const & cmdType, id<MTLCommandBuffer> cmdBuf)
    : m_CommandType(cmdType)
    , m_CmdBuffer(cmdBuf)
    {
    }
    
    CommandContext::~CommandContext()
    {
        
    }
    
    void CommandContext::Detach(k3d::IDevice *)
    {
        
    }
    
    void CommandContext::Begin()
    {
        if(m_CommandType == k3d::ECMD_Compute)
        {
            m_ComputeEncoder = [m_CmdBuffer computeCommandEncoder];
        }
        else if(m_CommandType == k3d::ECMD_Bundle)
        {
            m_ParallelRenderEncoder = [m_CmdBuffer parallelRenderCommandEncoderWithDescriptor:m_RenderpassDesc];
        }
    }
    
    void CommandContext::CopyTexture(const k3d::TextureCopyLocation& Dest, const k3d::TextureCopyLocation& Src)
    {
        
    }
    
    void CommandContext::CopyBuffer(k3d::IGpuResource& Dest, k3d::IGpuResource& Src, k3d::CopyBufferRegion const & Region)
    {
        
    }
    
    void CommandContext::Execute(bool Wait)
    {
        [m_CmdBuffer commit];
    }
    
    void CommandContext::Reset()
    {
    }
    
    void CommandContext::BeginRendering()
    {
    }
    
    void CommandContext::SetRenderTarget(k3d::RenderTargetRef pRenderTarget)
    {
        auto pRt = k3d::StaticPointerCast<RenderTarget>(pRenderTarget);
        m_RenderpassDesc = pRt->m_RenderPassDescriptor;
        m_RenderEncoder = [m_CmdBuffer renderCommandEncoderWithDescriptor:m_RenderpassDesc];
    }
    
    void CommandContext::SetIndexBuffer(const k3d::IndexBufferView& IBView)
    {
        m_TmpIndexBuffer = (id<MTLBuffer>)IBView.BufferLocation;
    }
    
    void CommandContext::SetVertexBuffer(uint32 Slot, const k3d::VertexBufferView& VBView)
    {
        [m_RenderEncoder setVertexBuffer:(id<MTLBuffer>)VBView.BufferLocation offset:0 atIndex:Slot];
    }
    
    void CommandContext::SetRenderPipeline(uint32 HashCode, k3d::RenderPipelineObjectRef pRenderPipeline)
    {
        if(!pRenderPipeline)
        {
            MTLLOGE("CommandContext::setRenderPipeline pRenderPipeline==null!");
            return;
        }
        auto mtlPs = StaticPointerCast<RenderPipeline>(pRenderPipeline);
        if(pRenderPipeline->GetType() == k3d::EPSO_Graphics)
        {
            [m_RenderEncoder setRenderRenderPipeline:    mtlPs->m_RenderRenderPipeline];
            [m_RenderEncoder setDepthStencilState:      mtlPs->m_DepthStencilState];
            [m_RenderEncoder setCullMode:               mtlPs->m_CullMode];
            m_CurPrimType = mtlPs->m_PrimitiveType;
        }
        else
        {
            [m_ComputeEncoder setComputeRenderPipeline:  mtlPs->m_ComputeRenderPipeline];
        }
    }
    
    void CommandContext::SetViewport(const k3d::ViewportDesc & vpDesc)
    {
        MTLViewport viewport = { vpDesc.Left, vpDesc.Top, vpDesc.Width, vpDesc.Height, vpDesc.MinDepth, vpDesc.MaxDepth };
        [m_RenderEncoder setViewport:viewport];
    }
    
    void CommandContext::SetPrimitiveType(k3d::EPrimitiveType Type)
    {
        m_CurPrimType = g_PrimitiveType[Type];
    }
    
    void CommandContext::DrawInstanced(k3d::DrawInstancedParam param)
    {
        [m_RenderEncoder drawPrimitives:m_CurPrimType
                            vertexStart:param.StartVertexLocation
                            vertexCount:param.VertexCountPerInstance
                          instanceCount:param.InstanceCount
                           baseInstance:param.StartInstanceLocation];
    }
    
    void CommandContext::DrawIndexedInstanced(k3d::DrawIndexedInstancedParam param)
    {
        [m_RenderEncoder drawIndexedPrimitives:m_CurPrimType
                                    indexCount:param.IndexCountPerInstance
                                     indexType:MTLIndexTypeUInt32
                                   indexBuffer:m_TmpIndexBuffer
                             indexBufferOffset:param.StartIndexLocation];
    }
    
    void CommandContext::EndRendering()
    {
        [m_RenderEncoder endEncoding];
    }
    
    void CommandContext::Dispatch(uint32 x, uint32 y, uint32 z)
    {
        [m_ComputeEncoder dispatchThreadgroups:MTLSizeMake(x, y, z) threadsPerThreadgroup:MTLSizeMake(x, y, z)];
    }
    
    void CommandContext::End()
    {
        
    }
    
    void CommandContext::PresentInViewport(k3d::RenderViewportRef rvp)
    {
        [m_CmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            
        }];
        auto vp = k3d::StaticPointerCast<RenderViewport>(rvp);
        [m_CmdBuffer presentDrawable:vp->m_CurrentDrawable];
        [m_CmdBuffer commit];
    }
#endif

#pragma mark Resource

    Buffer::Buffer(Device* device, k3d::ResourceDesc const & desc)
    : m_Desc(desc)
    , m_Device(device)
    {
        m_Buf = [m_Device->GetDevice()
                 newBufferWithLength:desc.Size
                 options: MTLResourceStorageModeManaged];
    }
    
    Buffer::~Buffer()
    {
        if(m_Buf)
        {
            [m_Buf release];
            m_Buf = nil;
        }
    }
    
    void * Buffer::Map(uint64 start, uint64 size)
    {
        m_MapRange.location = start;
        m_MapRange.length = size;
        return [m_Buf contents];
    }
    
    void Buffer::UnMap()
    {
#if K3DPLATFORM_OS_MAC
        [m_Buf didModifyRange:m_MapRange];
#endif
    }
    
    uint64 Buffer::GetLocation() const
    {
        return (uint64)m_Buf;
    }
    
    k3d::ResourceDesc Buffer::GetDesc() const
    {
        return m_Desc;
    }
    
    k3d::EResourceState Buffer::GetState() const
    {
        return k3d::ERS_Unknown;
    }
    
    uint64 Buffer::GetSize() const
    {
        return m_Buf.length;
    }

MTLTextureType RHIMTLTextureType(k3d::EGpuResourceType const & type)
{
    switch(type)
    {
        case k3d::EGT_Texture1D:
            return MTLTextureType1D;
        case k3d::EGT_Texture1DArray:
            return MTLTextureType1DArray;
        case k3d::EGT_Texture2D:
            return MTLTextureType2D;
        case k3d::EGT_Texture3D:
            return MTLTextureType3D;
        case k3d::EGT_Texture2DArray:
            return MTLTextureType2DArray;
    }
    return MTLTextureType2D;
}

MTLTextureUsage RHIMTLTexUsage(k3d::EGpuMemViewType const & viewType, k3d::EGpuResourceAccessFlag const & accessType)
{
    MTLTextureUsage usage = MTLTextureUsageUnknown;
    switch (viewType) {
        case k3d::EGVT_SRV:
            usage |= MTLTextureUsagePixelFormatView;
            break;
        case k3d::EGVT_RTV:
            usage |= MTLTextureUsageRenderTarget;
            break;
        default:
            break;
    }
    switch (accessType) {
        case k3d::EGRAF_Read:
            usage |= MTLTextureUsageShaderRead;
            break;
        case k3d::EGRAF_Write:
            usage |= MTLTextureUsageShaderWrite;
            break;
        case k3d::EGRAF_ReadAndWrite:
            usage |= (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite);
            break;
        default:
            break;
    }
    return usage;
}

MTLTextureDescriptor * RHIMTLTexDesc(k3d::ResourceDesc const & desc)
{
    MTLTextureDescriptor* texDesc = [MTLTextureDescriptor new];
    texDesc.width = desc.TextureDesc.Width;
    texDesc.height = desc.TextureDesc.Height;
    texDesc.depth = desc.TextureDesc.Depth;
    texDesc.pixelFormat = g_PixelFormat[desc.TextureDesc.Format];
    texDesc.mipmapLevelCount = desc.TextureDesc.MipLevels;
    texDesc.arrayLength = desc.TextureDesc.Layers;
    texDesc.textureType = RHIMTLTextureType(desc.Type);
    texDesc.usage = RHIMTLTexUsage(desc.ViewType, desc.Flag);
    texDesc.resourceOptions;
    texDesc.cpuCacheMode;
    texDesc.storageMode;
    return texDesc;
}

Texture::Texture(Device * device, k3d::ResourceDesc const & desc)
: m_Device(device)
, m_Desc(desc)
, m_TexDesc(nil)
{
    m_TexDesc = RHIMTLTexDesc(desc);
    m_Tex = [m_Device->GetDevice() newTextureWithDescriptor:m_TexDesc];
}

Texture::~Texture()
{
}

void * Texture::Map(uint64 start, uint64 size)
{
    return nullptr;
}

void Texture::UnMap()
{
    
}

uint64 Texture::GetLocation() const
{
    return 0;
}

k3d::ResourceDesc Texture::GetDesc() const
{
    return m_Desc;
}

uint64 Texture::GetSize() const
{
    return 0;
}

k3d::SamplerCRef Texture::GetSampler() const
{
    return nullptr;
}

void Texture::BindSampler(k3d::SamplerRef)
{
    
}

void Texture::SetResourceView(k3d::ShaderResourceViewRef)
{
    
}

k3d::ShaderResourceViewRef
Texture::GetResourceView() const
{
    return nullptr;
}


#pragma mark Device

    Device::Device()
    {
    }

    Device::Device(id<MTLDevice> pDevice) : m_Device(pDevice)
    {
    }

    Device::~Device() {
    }

    void Device::Release() {
        
    }

    id<MTLDevice> Device::GetDevice() {
        return m_Device;
    }

k3d::GpuResourceRef
Device::CreateResource(const k3d::ResourceDesc &)
{
    return nullptr;
}

    k3d::ShaderResourceViewRef
    Device::CreateShaderResourceView(k3d::GpuResourceRef, const k3d::SRVDesc &)
    {
        return nullptr;
    }

k3d::UnorderedAccessViewRef
Device::CreateUnorderedAccessView(const k3d::GpuResourceRef &, const k3d::UAVDesc &)
{
    return nullptr;
}

    k3d::SamplerRef
    Device::CreateSampler(const k3d::SamplerState&)
    {
        return nullptr;
    }

    k3d::PipelineLayoutRef
    Device::CreatePipelineLayout(const k3d::PipelineLayoutDesc &table)
    {
        return nullptr;
    }
    
    k3d::SyncFenceRef
    Device::CreateFence()
    {
        return nullptr;
    }

k3d::RenderPassRef
Device::CreateRenderPass(const k3d::RenderPassDesc &)
{
    return nullptr;
}

k3d::PipelineStateRef
Device::CreateRenderPipelineState(k3d::RenderPipelineStateDesc const&,
                                  k3d::PipelineLayoutRef,
                                  k3d::RenderPassRef)
{
    return nullptr;
}

k3d::PipelineStateRef
Device::CreateComputePipelineState(k3d::ComputePipelineStateDesc const&,
                                   k3d::PipelineLayoutRef)
{
    return nullptr;
}

k3d::CommandQueueRef
Device::CreateCommandQueue(k3d::ECommandType const&)
{
    return nullptr;
}

void
Device::WaitIdle()
{
    
}

#if 0
    // The pixel format for a Metal layer must be bgra8Unorm, bgra8Unorm_srgb, or rgba16Float.
    // The default value is bgra8Unorm. https://developer.apple.com/reference/quartzcore/cametallayer/1478155-pixelformat
    MTLPixelFormat CorrectFormat(k3d::GfxSetting & setting)
    {
        auto pf = g_PixelFormat[setting.ColorFormat];
        switch (pf) {
            case MTLPixelFormatRGBA16Float:
            case MTLPixelFormatBGRA8Unorm_sRGB:
            case MTLPixelFormatBGRA8Unorm:
                return pf;
            default:
                setting.ColorFormat = k3d::EPF_BGRA8Unorm;
                break;
        }
        return g_PixelFormat[setting.ColorFormat];
    }
    
    k3d::RenderViewportRef
    Device::NewRenderViewport(void * winHandle, k3d::GfxSetting & setting)
    {
#if K3DPLATFORM_OS_MAC
        NSWindow* nWindow = (__bridge NSWindow*)winHandle;
        NSView* view = nWindow.contentView;
        NSRect rect = [view frame];
#else
        UIWindow* nWindow = (__bridge UIWindow*)winHandle;
        UIView* view = nWindow.subviews[0];
        CGRect rect = [view frame];
#endif
        setting.Width = rect.size.width;
        setting.Height = rect.size.height;
        CAMetalLayer* mtlayer = (CAMetalLayer*)view.layer;
        mtlayer.device          = GetDevice();
        mtlayer.pixelFormat     = CorrectFormat(setting);
        mtlayer.framebufferOnly = YES;
        return k3d::MakeShared<RenderViewport>(mtlayer);
    }
    
    RenderViewport::RenderViewport(CAMetalLayer * mtlLayer)
    : m_Layer(mtlLayer)
    , m_CurrentDrawable(nil)
    , m_RenderPassDescriptor(nil)
    {
        m_RenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    }
    
    RenderViewport::~RenderViewport()
    {
    }
    
    bool RenderViewport::InitViewport(void *windowHandle, k3d::IDevice *pDevice, k3d::GfxSetting & setting)
    {
        m_Width = setting.Width;
        m_Height = setting.Height;
        m_CurrentDrawable = [m_Layer nextDrawable];
        id<MTLTexture> texture = m_CurrentDrawable.texture;
        MTLRenderPassColorAttachmentDescriptor *colorAttachment = m_RenderPassDescriptor.colorAttachments[0];
        colorAttachment.texture = texture;
        
        // make sure to clear every frame for best performance
        colorAttachment.loadAction = MTLLoadActionClear;
        colorAttachment.clearColor = MTLClearColorMake(0.65f, 0.65f, 0.65f, 1.0f);
        
        // if sample count is greater than 1, render into using MSAA, then resolve into our color texture
        //    if(_sampleCount > 1)
        //    {
        //        BOOL doUpdate =     ( _msaaTex.width       != texture.width  )
        //        ||  ( _msaaTex.height      != texture.height )
        //        ||  ( _msaaTex.sampleCount != _sampleCount   );
        //
        //        if(!_msaaTex || (_msaaTex && doUpdate))
        //        {
        //            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: MTLPixelFormatBGRA8Unorm
        //                                                                                            width: texture.width
        //                                                                                           height: texture.height
        //                                                                                        mipmapped: NO];
        //            desc.textureType = MTLTextureType2DMultisample;
        //
        //            // sample count was specified to the view by the renderer.
        //            // this must match the sample count given to any pipeline state using this render pass descriptor
        //            desc.sampleCount = _sampleCount;
        //
        //            _msaaTex = [_device newTextureWithDescriptor: desc];
        //        }
        //
        //        // When multisampling, perform rendering to _msaaTex, then resolve
        //        // to 'texture' at the end of the scene
        //        colorAttachment.texture = _msaaTex;
        //        colorAttachment.resolveTexture = texture;
        //
        //        // set store action to resolve in this case
        //        colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
        //    }
        //    else
        {
            // store only attachments that will be presented to the screen, as in this case
            colorAttachment.storeAction = MTLStoreActionStore;
        } // color0
        
        // Now create the depth and stencil attachments
        
        if(setting.HasDepth)
        {
            //        BOOL doUpdate =     ( _depthTex.width       != texture.width  )
            //        ||  ( _depthTex.height      != texture.height )
            //        ||  ( _depthTex.sampleCount != _sampleCount   );
            //
            //        if(!_depthTex || doUpdate)
            //        {
            //  If we need a depth texture and don't have one, or if the depth texture we have is the wrong size
            //  Then allocate one of the proper size
            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: g_PixelFormat[setting.DepthStencilFormat]
                                                                                            width: texture.width
                                                                                           height: texture.height
                                                                                        mipmapped: NO];
            
            desc.textureType = /*(_sampleCount > 1) ? MTLTextureType2DMultisample :*/ MTLTextureType2D;
#if K3DPLATFORM_OS_MAC
            desc.resourceOptions = MTLResourceStorageModePrivate;
            desc.usage = MTLTextureUsageRenderTarget;
#endif
            //desc.sampleCount = _sampleCount;
            
            m_DepthTex = [m_Layer.device newTextureWithDescriptor: desc];
            
            MTLRenderPassDepthAttachmentDescriptor *depthAttachment = m_RenderPassDescriptor.depthAttachment;
            depthAttachment.texture = m_DepthTex;
            depthAttachment.loadAction = MTLLoadActionClear;
            depthAttachment.storeAction = MTLStoreActionDontCare;
            depthAttachment.clearDepth = 1.0;
            //        }
        } // depth
        
        //    if(_stencilPixelFormat != MTLPixelFormatInvalid)
        //    {
        //        BOOL doUpdate  =    ( _stencilTex.width       != texture.width  )
        //        ||  ( _stencilTex.height      != texture.height )
        //        ||  ( _stencilTex.sampleCount != _sampleCount   );
        //
        //        if(!_stencilTex || doUpdate)
        //        {
        //            //  If we need a stencil texture and don't have one, or if the depth texture we have is the wrong size
        //            //  Then allocate one of the proper size
        //            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: _stencilPixelFormat
        //                                                                                            width: texture.width
        //                                                                                           height: texture.height
        //                                                                                        mipmapped: NO];
        //
        //            desc.textureType = (_sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
        //            desc.sampleCount = _sampleCount;
        //
        //            _stencilTex = [_device newTextureWithDescriptor: desc];
        //
        //            MTLRenderPassStencilAttachmentDescriptor* stencilAttachment = m_RenderPassDescriptor.stencilAttachment;
        //            stencilAttachment.texture = _stencilTex;
        //            stencilAttachment.loadAction = MTLLoadActionClear;
        //            stencilAttachment.storeAction = MTLStoreActionDontCare;
        //            stencilAttachment.clearStencil = 0;
        //        }
        //    } //stencil
        
        return true;
    }
    
    void RenderViewport::PrepareNextFrame()
    {
        m_CurrentDrawable = [m_Layer nextDrawable];
    }
    
    k3d::RenderTargetRef
    RenderViewport::GetCurrentBackRenderTarget()
    {
        return k3d::MakeShared<RenderTarget>(m_RenderPassDescriptor, m_CurrentDrawable.texture);
    }
    
    k3d::RenderTargetRef
    Device::NewRenderTarget(k3d::RenderTargetLayout const&)
    {
        return nullptr;
    }
#endif

#pragma mark RenderTarget
    
    RenderTarget::RenderTarget(MTLRenderPassDescriptor* rpd, id<MTLTexture> color)
    : m_RenderPassDescriptor(rpd)
    , m_ColorTexture(color)
    {
    }
    
    RenderTarget::~RenderTarget()
    {
    }
    
    void RenderTarget::SetClearColor(kMath::Vec4f clrColor)
    {
        
    }
    
    void RenderTarget::SetClearDepthStencil(float depth, uint32 stencil)
    {
        
    }
    
    k3d::GpuResourceRef RenderTarget::GetBackBuffer()
    {
        return nullptr;
    }

#if 0
    Library::Library(id<MTLDevice> device)
    : m_Device(device)
    {
    }
    
    void Library::LoadFromFile(const char *name) {
        NSString *fileName = [[NSString alloc] initWithUTF8String:name];
        m_ShaderLib = [m_Device newLibraryWithFile:fileName error:nullptr];
    }
    
    Shader* Library::GetShaderFunction(const char * functionName) {
        Shader * shader = new Shader;
        shader->m_Function = [m_ShaderLib newFunctionWithName:
                              [[NSString alloc] initWithUTF8String:functionName]];
        return shader;
    }
    
    
    Shader::Shader() {
        
    }
#endif

NS_K3D_METAL_END

class MetalRHI : public k3d::IMetalRHI
    {
    public:
        MetalRHI()
        {
        }
        
        ~MetalRHI() override
        {
        }
        
        void Start()override
        {
            KLOG(Info, MetalRHI, "Starting...");
#if K3DPLATFORM_OS_MAC
            NSArray<id<MTLDevice>> * deviceList = MTLCopyAllDevices();
            
            MTLFeatureSet fSets[] = {
                MTLFeatureSet_OSX_ReadWriteTextureTier2,
                MTLFeatureSet_OSX_GPUFamily1_v2,
                MTLFeatureSet_OSX_GPUFamily1_v1
            };
            for (uint32 i = 0; i<deviceList.count; i++)
            {
                id<MTLDevice> device = [deviceList objectAtIndex:i];
                //m_Adapters[i] = new metal::DeviceAdapter(device);
                KLOG(Info, MetalRHI, "DeviceName: %s ReadWriteTextureTier2:%d GPUv2:%d",
                      [[device name] UTF8String],
                      [device supportsFeatureSet:fSets[0]],
                      [device supportsFeatureSet:fSets[1]]);
            }
#else
            MTLFeatureSet fSets[] = {
                MTLFeatureSet_iOS_GPUFamily2_v1,
                MTLFeatureSet_iOS_GPUFamily1_v2,
                MTLFeatureSet_iOS_GPUFamily2_v2,
                MTLFeatureSet_iOS_GPUFamily3_v1,
                MTLFeatureSet_iOS_GPUFamily1_v3,
                MTLFeatureSet_iOS_GPUFamily2_v3,
                MTLFeatureSet_iOS_GPUFamily3_v2
            };
            id<MTLDevice> device = MTLCreateSystemDefaultDevice();
            m_Adapters.resize(1);
            m_Adapters[0] = new metal::DeviceAdapter(device);
            KLOG(Info, MetalRHI, "DeviceName: %s ReadWriteTextureTier2:%d GPUv2:%d",
                 [[device name] UTF8String],
                 [device supportsFeatureSet:fSets[0]],
                 [device supportsFeatureSet:fSets[1]]);
#endif
        }
        
        void Shutdown()override
        {
            NSLog(@"Metalk3d::Shutdown....");
        }
        
        const char * Name() override { return "RHI_Metal"; }
        
        k3d::DeviceRef GetPrimaryDevice() override
        {
            return nullptr;
        }
        
    private:
    };

MODULE_IMPLEMENT(RHI_Metal, MetalRHI)
