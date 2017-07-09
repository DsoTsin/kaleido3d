#include <Kaleido3D.h>
#include <ngfx.h>
#include <ngfxu.h>
#include <Core/Os.h>
#include "gtest/gtest.h"

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

using namespace ngfx;

Ptr<Factory> GlobalTestFactory;
Ptr<Device> GlobalTestDevice;
Ptr<CommandQueue> gfxQueue;
Ptr<RenderPass> GlobalTestRenderPass;
Ptr<BindTableLayout> GlobalTableLayout;
Ptr<PipelineLayout> GlobalPipelineLayout;
Ptr<Function> ComputeFunction;
Ptr<Pipeline> GolbalPipeline;
Ptr<Pipeline> GlobalComputePipeline; 

TEST(CreateFactory, ngfxFactory)
{
  CreateFactory(GlobalTestFactory.GetAddressOf(), true);
  ASSERT_TRUE(GlobalTestFactory.Get());
}

TEST(CreateDevice, ngfxDevice)
{
  uint32_t Count = 0;
  GlobalTestFactory->EnumDevice(&Count, nullptr);
  ASSERT_TRUE(Count > 0);
  GlobalTestFactory->EnumDevice(&Count, GlobalTestDevice.GetAddressOf());
  ASSERT_TRUE(GlobalTestDevice.Get());
}

TEST(CreateLibrary, ngfxLibrary)
{
  Ptr<Library> library;
  Ptr<Function> function;
  Os::MemMapFile blobFile;
  ASSERT_TRUE(blobFile.Open("../../Data/Test/Test.blob", IORead));
  GlobalTestDevice->CreateLibrary(nullptr, blobFile.FileData(), blobFile.GetSize(), library.GetAddressOf());
  blobFile.Close();
  ASSERT_TRUE(library.Get());
  library->MakeFunction("MainVS", function.GetAddressOf());
  ASSERT_TRUE(function.Get());
  ASSERT_STREQ("MainVS", function->Name());
  ASSERT_EQ(ShaderType::Vertex, function->Type());
  library->MakeFunction("MainCS", ComputeFunction.GetAddressOf());
  ASSERT_STREQ("MainCS", ComputeFunction->Name());
  ASSERT_EQ(ShaderType::Compute, ComputeFunction->Type());
}

TEST(CreateQueue, ngfxQueue)
{
  Ptr<CommandQueue> compQueue, copyQueue;
  GlobalTestDevice->CreateCommandQueue(CommandQueueType::Graphics, gfxQueue.GetAddressOf());
  ASSERT_TRUE(gfxQueue.Get());
  GlobalTestDevice->CreateCommandQueue(CommandQueueType::Graphics, compQueue.GetAddressOf());
  ASSERT_TRUE(compQueue.Get());
  GlobalTestDevice->CreateCommandQueue(CommandQueueType::Graphics, copyQueue.GetAddressOf());
  ASSERT_TRUE(copyQueue.Get());

  ASSERT_TRUE(__is_enum(BufferViewBit));
}

/* Render Pass Creation */
TEST(CreateRenderPass, ngfxRenderPass)
{
  RenderPassDesc desc;
  GlobalTestDevice->CreateRenderPass(&desc, GlobalTestRenderPass.GetAddressOf());
  ASSERT_TRUE(GlobalTestRenderPass.Get());
}

TEST(CreateBindTableLayout, ngfxBindTableLayout)
{
  Ptr<BindTableLayoutInitializer> btlInitializer;
  GlobalTestDevice->CreateBindTableLayoutInitializer(btlInitializer.GetAddressOf());
  btlInitializer->AddBuffer(0, 1, ShaderStageBit::Compute);
  btlInitializer->Initialize(GlobalTableLayout.GetAddressOf());
  ASSERT_TRUE(GlobalTableLayout.Get());
}

TEST(CreateBindTable, ngfxBindTable)
{
  Ptr<BindTable> bindTable;
  ASSERT_TRUE(Result::Ok == GlobalTestDevice->CreateBindTable(GlobalTableLayout.Get(), bindTable.GetAddressOf()));
  ASSERT_TRUE(bindTable.Get() != nullptr);
}

TEST(CreatePipelineLayout, ngfxPipelineLayout)
{
  PipelineLayoutDesc desc = { GlobalTableLayout.Get(), 1 };
  GlobalTestDevice->CreatePipelineLayout(&desc, GlobalPipelineLayout.GetAddressOf());
  ASSERT_TRUE(GlobalPipelineLayout.Get() != nullptr);
}

Ptr<Texture> texture;
Ptr<TextureView> textureView;

TEST(CreateTexture, ngfxTexture)
{
  TextureDesc desc;
  desc.allowedViewBits |= TextureViewBit::ShaderRead;
  desc.SetWidth(1024).SetHeight(1024).SetDepth(1).SetLayers(1).SetMipLevels(1);
  GlobalTestDevice->CreateTexture(&desc, texture.GetAddressOf());
  ASSERT_TRUE(texture.Get() != nullptr);

  Ptr<Texture> DepthStencilTexture = ngfxu::CreateDepthStencilTexture(
    GlobalTestDevice, PixelFormat::D32Float, 1024, 1024);
  ASSERT_TRUE(DepthStencilTexture.Get());

  Ptr<Texture> RenderTexture2D = ngfxu::CreateRenderTexture2D(
    GlobalTestDevice, PixelFormat::RGBA8UNorm, 1024, 1024);
  ASSERT_TRUE(RenderTexture2D.Get());

  Ptr<Texture> SampledTexture2D = ngfxu::CreateSampledTexture2D(
    GlobalTestDevice, PixelFormat::RGBA8UNorm, 1024, 1024);
  ASSERT_TRUE(SampledTexture2D.Get());

  uint32_t * pData = (uint32_t *)SampledTexture2D->Map(0, 4 * 8);
  *pData = 2;
  *(pData + 7) = 9;
  ASSERT_TRUE(pData);
  SampledTexture2D->UnMap();
}

TEST(CreateTextureView, ngfxTextureView)
{
  TextureViewDesc desc{};
  desc.SetDimension(TextureDimension::Tex2D)
    .SetView(ResourceViewType::SampledTexture);
  texture->CreateView(&desc, textureView.GetAddressOf());
  ASSERT_TRUE(textureView.Get() != nullptr);
}

Ptr<Buffer> buffer;
Ptr<BufferView> bufferView;

TEST(CreateBuffer, ngfxBuffer)
{
  BufferDesc bufferDesc{ BufferViewBit::UnOrderedAccess, StorageOption::Managed, 10 };
  GlobalTestDevice->CreateBuffer(&bufferDesc, buffer.GetAddressOf());
  buffer->SetName("Buffer0");
  ASSERT_TRUE(buffer.Get() != nullptr);
}

TEST(CreateBufferView, ngfxBufferView)
{
  BufferViewDesc bufferViewDesc = {
    ResourceViewType::UnorderAccessBuffer,
    ResourceState::UnorderAccess,
    10, 0
  };
  buffer->CreateView(&bufferViewDesc, bufferView.GetAddressOf());
  ASSERT_TRUE(bufferView.Get() != nullptr);
}

TEST(CreateFrameBuffer, ngfxFrameBuffer)
{
  Ptr<FrameBuffer> frameBuffer;
  FrameBufferDesc fboDesc;
  GlobalTestDevice->CreateFrameBuffer(&fboDesc, frameBuffer.GetAddressOf());
  ASSERT_TRUE(frameBuffer.Get());
}

TEST(CreateSampler, ngfxSampler)
{
  Ptr<Sampler> sampler;
  SamplerDesc samplerDesc = {};
  GlobalTestDevice->CreateSampler(&samplerDesc, sampler.GetAddressOf());
  ASSERT_TRUE(sampler.Get());
}

TEST(CreateComputePipeline, ngfxComputePipeline)
{
  auto Ret = GlobalTestDevice->CreateComputePipeline(
    ComputeFunction.Get(),
    GlobalPipelineLayout.Get(),
    GlobalComputePipeline.GetAddressOf());
  ASSERT_TRUE(Ret == Result::Ok);
  ASSERT_TRUE(GlobalComputePipeline.Get() != nullptr);
}

TEST(CreateRenderPipeline, ngfxRenderPipeline)
{
  RenderPipelineDesc desc;
  GlobalTestDevice->CreateRenderPipeline(&desc, 
    GlobalPipelineLayout.Get(), 
    GlobalTestRenderPass.Get(),
    GolbalPipeline.GetAddressOf());
  ASSERT_TRUE(GolbalPipeline.Get() != nullptr);
}

TEST(CreateCommandBuffer, ngfxCommandBuffer)
{
  Ptr<CommandBuffer> commandBuffer;
  gfxQueue->CreateCommandBuffer(commandBuffer.GetAddressOf());
  ASSERT_TRUE(commandBuffer.Get());
  Ptr<RenderCommandEncoder> renderEncoder;
  commandBuffer->CreateRenderCommandEncoder(nullptr, nullptr, renderEncoder.GetAddressOf());
  ASSERT_TRUE(renderEncoder.Get());
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}