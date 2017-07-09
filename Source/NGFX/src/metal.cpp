#include <Kaleido3D.h>
#include <KTL/Allocator.hpp>
#include "ngfx.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

class MtlFactory;
class MtlDevice;
class MtlQueue;
class MtlRenderPipeline;
class MtlComputePipeline;
class MtlCommandBuffer;
class MtlRenderEncoder;
class MtlComputeEncoder;
class MtlBuffer;
class MtlTexture;
class MtlSampler;
class MtlRenderPass;
class MtlCompiler;
class MtlFunction;
class MtlReflection;

using namespace ngfx;

namespace ngfx
{
  NS_AVAILABLE(10_11, 8_0)
  class MtlFactory : public Factory
  {
  public:
    friend Result CreateFactory(Factory ** ppFactory, bool debugEnabled);
  
    MtlFactory();
    ~MtlFactory() override;
    Result EnumDevice(uint32_t * count, Device ** ppDevice) override;
    Result CreateSwapchain(const SwapChainDesc * desc,
                           CommandQueue * pCommandQueue,
                           void * pWindow,
                           SwapChain ** pSwapchain)override;
    Result CreateCompiler(ShaderLang shaderLang, Compiler ** compiler) override;
    
    NSArray <id<MTLDevice>>* Devices;
  };
  
  Result CreateFactory(Factory ** ppFactory, bool debugEnabled)
  {
    * ppFactory = new MtlFactory;
    return Result::Ok;
  }
}

NS_AVAILABLE(10_11, 8_0)
class MtlDevice : public Device
{
  friend class ngfx::MtlFactory;
public:
  MtlDevice(id<MTLDevice> Device);
  ~MtlDevice() override;
  void GetDesc(DeviceDesc * pDesc) override;
  Result CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue) override;
  Result CreateShaderLayout(const ShaderLayoutDesc * pShaderLayoutDesc, ShaderLayout ** ppShaderLayout) override;
  Result CreatePipelineLayout(const PipelineLayoutDesc * pPipelineLayoutDesc, PipelineLayout ** ppPipelineLayout) override;
  Result CreateBindingTable(PipelineLayout * pPipelineLayout, BindingTable ** ppBindingTable) override;
  Result CreateRenderPipeline(const RenderPipelineDesc * pPipelineDesc, PipelineLayout * pPipelineLayout, RenderPass * pRenderPass, Pipeline ** pPipelineState) override;
  Result CreateComputePipeline(Function * pComputeFunction, PipelineLayout * pPipelineLayout, Pipeline ** pPipeline) override;
  Result CreateRenderPass(const RenderPassDesc * desc, RenderPass ** ppRenderpass) override;
  Result CreateRenderTarget(const RenderTargetDesc * desc, RenderTarget ** ppRenderTarget) override;
  Result CreateSampler(const SamplerDesc* desc, Sampler ** pSampler) override;
  Result CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer) override;
  Result CreateTexture(const TextureDesc * desc, Texture ** pTexture) override;
  NS_AVAILABLE(10_13, 10_0)
  Result CreateFence(Fence ** ppFence) override;
  void WaitIdle() override;
  
  id<MTLDevice> Id;
};

class MtlSwapChain : public SwapChain
{
public:
  MtlSwapChain();
  ~MtlSwapChain() override;
  Drawable * CurrentDrawable() override;
  Drawable * NextDrawable() override;
  Result GetTexture(Texture ** ppTexture, uint32_t index) override;
  uint32_t BufferCount() override;
  void InitWithRenderPass(RenderPass * pRenderPass) override;
  
  CAMetalLayer* Layer;
};

template <class MTLClass>
struct MtlTraits;

/*
template <>
struct MtlTraits<Fence>
{
  typedef id<MTLFence> Obj;
};
 */

template <>
struct MtlTraits<RenderPipeline>
{
  typedef id<MTLRenderPipelineState> Obj;
};

template <>
struct MtlTraits<ComputePipeline>
{
  typedef id<MTLComputePipelineState> Obj;
};

template <>
struct MtlTraits<Texture>
{
  typedef id<MTLTexture> Obj;
};

template <>
struct MtlTraits<Buffer>
{
  typedef id<MTLBuffer> Obj;
};

template <>
struct MtlTraits<RenderPass>
{
  typedef MTLRenderPassDescriptor* Obj;
};

template <>
struct MtlTraits<CommandQueue>
{
  typedef id<MTLCommandQueue> Obj;
};

template <>
struct MtlTraits<CommandBuffer>
{
  typedef id<MTLCommandBuffer> Obj;
};

template <>
struct MtlTraits<RenderCommandEncoder>
{
  typedef id<MTLRenderCommandEncoder> Obj;
};

template <>
struct MtlTraits<ComputeCommandEncoder>
{
  typedef id<MTLComputeCommandEncoder> Obj;
};

template <>
struct MtlTraits<Function>
{
  typedef id<MTLFunction> Obj;
};

template <>
struct MtlTraits<Sampler>
{
  typedef id<MTLSamplerState> Obj;
};

template <>
NS_AVAILABLE(10_13, 10_0)
struct MtlTraits<Fence>
{
  NS_AVAILABLE(10_13, 10_0)
  typedef id<MTLFence> Obj;
};

template <class T>
class DeviceObject : public T
{
  friend class MtlDevice;
public:
  using Super = DeviceObject<T>;
  
  DeviceObject(MtlDevice* pDevice);
  virtual ~DeviceObject();
  
  MtlDevice* Device = nullptr;
  
  typename MtlTraits<T>::Obj Id;
};


template <class T>
DeviceObject<T>::DeviceObject(MtlDevice* pDevice)
: Device(pDevice)
{
}

template <class T>
DeviceObject<T>::~DeviceObject()
{
}

NS_AVAILABLE(10_11, 8_0)
class MtlRenderPipeline : public DeviceObject<RenderPipeline>
{
public:
  MtlRenderPipeline(MtlDevice*);
  ~MtlRenderPipeline() override;
  PipelineType Type() override { return PipelineType::Graphics; }
  Result GetDesc(RenderPipelineDesc * pDesc) override;
  MTLRenderPipelineDescriptor* Desc = nil;
};

NS_AVAILABLE(10_11, 8_0)
class MtlComputePipeline : public DeviceObject<ComputePipeline>
{
public:
  MtlComputePipeline(MtlDevice*);
  ~MtlComputePipeline() override;
  PipelineType Type() override { return PipelineType::Compute; }
  MTLComputePipelineDescriptor* Desc = nil;
};

NS_AVAILABLE(10_11, 8_0)
class MtlBuffer : public DeviceObject<Buffer>
{
public:
  MtlBuffer(MtlDevice* pDevice, BufferDesc const* pDesc);
  ~MtlBuffer() override;
  void * Map(uint64_t offset, uint64_t size) override;
  void UnMap() override;
  Result GetDesc(BufferDesc * pDesc) override;
  Result CreateView(const BufferViewDesc * pDesc, BufferView ** ppView) override;
};

NS_AVAILABLE(10_11, 8_0)
class MtlTexture : public DeviceObject<Texture>
{
public:
  MtlTexture();
  ~MtlTexture() override;
  MTLTextureDescriptor* Desc = nil;
};

NS_AVAILABLE(10_13, 10_0)
class MtlFence : public DeviceObject<Fence>
{
public:
  MtlFence(MtlDevice*);
  ~MtlFence();
  void Wait() override;
  void Reset() override;
};

class MtlRenderPass : public DeviceObject<RenderPass>
{
public:
  MtlRenderPass(RenderPassDesc* pDesc, MtlDevice* pDevice);
  ~MtlRenderPass() override;
};

class MtlQueue : public DeviceObject<CommandQueue>
{
public:
  MtlQueue(MtlDevice* pDevice);
  ~MtlQueue() override;
  struct CommandBuffer * CommandBuffer() override;
};

class MtlCommandBuffer : public CommandBuffer
{
public:
  MtlCommandBuffer(MtlQueue* pQueue);
  ~MtlCommandBuffer() override;
  
  MtlTraits<CommandBuffer>::Obj Id;
  MtlQueue* OwningQueue;
  
  void Commit(Fence * pFence) override;
  
  struct RenderCommandEncoder* RenderCommandEncoder(Drawable * pDrawable, RenderPass * pRenderPass) override;
  struct ComputeCommandEncoder * ComputeCommandEncoder() override;
  struct CopyCommandEncoder * CopyCommandEncoder() override;
  struct ParallelRenderCommandEncoder * ParallelCommandEncoder() override;
};

template <class T>
class CmdEncoder : public T
{
public:
  CmdEncoder(MtlCommandBuffer* CmdBuffer)
  : OwningCmd(CmdBuffer)
  {}
  
  virtual ~CmdEncoder()
  {}
  
  virtual void EndEncode() override
  {
    [Id endEncoding];
  }
  
  typename MtlTraits<T>::Obj  Id;
  MtlCommandBuffer*           OwningCmd;

protected:
  typedef CmdEncoder<T> Super;
};

class MtlRenderEncoder : public CmdEncoder<RenderCommandEncoder>
{
public:
  MtlRenderEncoder(MtlCommandBuffer* CmdBuffer, MtlRenderPass* RenderPass);
  ~MtlRenderEncoder() override;
};

class MtlComputeEncoder : public CmdEncoder<ComputeCommandEncoder>
{
public:
  MtlComputeEncoder(MtlCommandBuffer* CmdBuffer);
  ~MtlComputeEncoder() override;
};

MtlDevice::MtlDevice(id<MTLDevice> Device)
: Id(Device)
{
}

MtlDevice::~MtlDevice()
{
}

void MtlDevice::GetDesc(ngfx::DeviceDesc *pDesc)
{
  
}

MtlCommandBuffer::MtlCommandBuffer(MtlQueue* pQueue)
: OwningQueue(pQueue)
{
  Id = [OwningQueue->Id commandBuffer];
}

MtlCommandBuffer::~MtlCommandBuffer()
{
  
}

void
MtlCommandBuffer::Commit(Fence * pFence)
{
  
}

MtlRenderEncoder::MtlRenderEncoder(MtlCommandBuffer* CmdBuffer, MtlRenderPass* RenderPass)
: Super(CmdBuffer)
{
  Id = [OwningCmd->Id renderCommandEncoderWithDescriptor:RenderPass->Id];
}

MtlRenderEncoder::~MtlRenderEncoder()
{
  
}

RenderCommandEncoder*
MtlCommandBuffer::RenderCommandEncoder(ngfx::Drawable *pDrawable, ngfx::RenderPass *pRenderPass)
{
  return nullptr;
}

MtlComputeEncoder::MtlComputeEncoder(MtlCommandBuffer* CmdBuffer)
: Super(CmdBuffer)
{
  Id = [OwningCmd->Id computeCommandEncoder];
}

MtlComputeEncoder::~MtlComputeEncoder()
{
  
}

ComputeCommandEncoder*
MtlCommandBuffer::ComputeCommandEncoder()
{
  return nullptr;
}

CopyCommandEncoder*
MtlCommandBuffer::CopyCommandEncoder()
{
  return nullptr;
}

ParallelRenderCommandEncoder*
MtlCommandBuffer::ParallelCommandEncoder()
{
  return nullptr;
}

MtlQueue::MtlQueue(MtlDevice* pDevice)
: Super(pDevice)
{
  Id = [Device->Id newCommandQueue];
}

MtlQueue::~MtlQueue()
{
  
}

CommandBuffer*
MtlQueue::CommandBuffer()
{
  return new MtlCommandBuffer(this);
}

Result
MtlDevice::CreateCommandQueue(CommandQueueType queueType, CommandQueue ** pQueue)
{
  *pQueue = new MtlQueue(this);
  return Result::Ok;
}

MtlRenderPipeline::MtlRenderPipeline(MtlDevice * pDevice)
: Super(pDevice)
{
  Id = [Device->Id newRenderPipelineStateWithDescriptor:nullptr error:nullptr];
}

MtlRenderPipeline::~MtlRenderPipeline()
{
  
}

Result
MtlRenderPipeline::GetDesc(ngfx::RenderPipelineDesc *pDesc)
{
  return Result::Ok;
}

Result
MtlDevice::CreateRenderPipeline(const RenderPipelineDesc * pPipelineDesc,
                                PipelineLayout * pPipelineLayout,
                                RenderPass * pRenderPass,
                                Pipeline ** pPipelineState)
{
  *pPipelineState = new MtlRenderPipeline(this);
  return Result::Ok;
}

MtlComputePipeline::MtlComputePipeline(MtlDevice* pDevice)
: Super(pDevice)
{
  Id = [Device->Id newComputePipelineStateWithFunction:nullptr error:nullptr];
}

MtlComputePipeline::~MtlComputePipeline()
{
  
}

Result
MtlDevice::CreateComputePipeline(Function * pComputeFunction,
                                 PipelineLayout * pPipelineLayout,
                                 Pipeline ** pPipeline)
{
  * pPipeline = new MtlComputePipeline(this);
  return Result::Ok;
}

Result
MtlDevice::CreateSampler(const SamplerDesc* desc, Sampler ** pSampler)
{
  return Result::Ok;
}

MtlBuffer::MtlBuffer(MtlDevice* pDevice, BufferDesc const* pDesc)
: Super(pDevice)
{
  
}

MtlBuffer::~MtlBuffer()
{
  
}

Result
MtlBuffer::GetDesc(BufferDesc * pDesc)
{
  return Result::Ok;
}

Result
MtlBuffer::CreateView(const ngfx::BufferViewDesc *pDesc, ngfx::BufferView **ppView)
{
  return Result::Ok;
}

void * MtlBuffer::Map(uint64_t offset, uint64_t size)
{
  return [Id contents];
}

void MtlBuffer::UnMap()
{
//  [Id didModifyRange:<#(NSRange)#>];
}

Result
MtlDevice::CreateBuffer(const BufferDesc* desc, Buffer ** pBuffer)
{
  * pBuffer = new MtlBuffer(this, desc);
  return Result::Ok;
}

Result
MtlDevice::CreateTexture(const TextureDesc * desc, Texture ** pTexture)
{
  return Result::Ok;
}

MtlFence::MtlFence(MtlDevice* pDevice)
: Super(pDevice)
{
  
}

MtlFence::~MtlFence()
{
  
}

void
MtlFence::Wait()
{
  [Id debugDescription];
}

void
MtlFence::Reset()
{
  
}

Result
MtlDevice::CreateFence(Fence **ppFence)
{
  *ppFence = new MtlFence(this);
  return Result::Ok;
}

Result
MtlDevice::CreateRenderPass(const RenderPassDesc *desc, RenderPass **ppRenderpass)
{
  return Result::Ok;
}

Result
MtlDevice::CreateBindingTable(PipelineLayout *pPipelineLayout, BindingTable **ppBindingTable)
{
  return Result::Ok;
}

Result
MtlDevice::CreateShaderLayout(const ShaderLayoutDesc *pShaderLayoutDesc, ShaderLayout **ppShaderLayout)
{
  return Result::Ok;
}

Result
MtlDevice::CreatePipelineLayout(const PipelineLayoutDesc *pPipelineLayoutDesc, PipelineLayout **ppPipelineLayout)
{
  return Result::Ok;
}

Result
MtlDevice::CreateRenderTarget(const RenderTargetDesc *desc, RenderTarget **ppRenderTarget)
{
  return Result::Ok;
}

void
MtlDevice::WaitIdle()
{
  
}

namespace ngfx
{
  MtlFactory::MtlFactory()
  {
    Devices = MTLCopyAllDevices();
  }
  
  MtlFactory::~MtlFactory()
  {
  }
  
  Result
  MtlFactory::EnumDevice(uint32_t * count, Device ** ppDevice)
  {
    *count = [Devices count];
    if (ppDevice)
    {
      for (int i = 0; i < [Devices count]; i++)
      {
        *(ppDevice + i) = new MtlDevice([Devices objectAtIndex:i]);
      }
    }
    return Result::Ok;
  }
  
  Result
  MtlFactory::CreateSwapchain(const SwapChainDesc * desc,
                                     CommandQueue * pCommandQueue,
                                     void * pWindow,
                                     SwapChain ** pSwapchain)
  {
    return Result::Ok;
  }
  
  Result
  MtlFactory::CreateCompiler(ShaderLang shaderLang, Compiler ** compiler)
  {
    return Result::Ok;
  }
}
