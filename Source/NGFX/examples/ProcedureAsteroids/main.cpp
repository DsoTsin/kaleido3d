/*
https://github.com/GameTechDev/asteroids_d3d12
Apache License
*/
#include <Kaleido3D.h>
#include <KTL/Allocator.hpp>
#include <Core/App.h>
#include <Core/Os.h>
#include <ngfx.h>
#include <ngfxu.h>

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

static char * HLSL = R"(
    #pragma pack_matrix( row_major )

    struct VS_IN
    {
      float3 inPos : POSITION;
      float3 inColor : COLOR;
    };

    struct VS_OUT
    {
      float4 outPos : SV_POSITION;
      float3 outColor : COLOR;
    };

    cbuffer UBO : register(b0)
    {
      row_major matrix projectMatrix;
      row_major matrix modelMatrix;
      row_major matrix viewMatrix;
    };

    [vertexshader] VS_OUT MainVS(VS_IN vsin)
    {
      VS_OUT vsout;
      vsout.outPos = projectMatrix * viewMatrix * modelMatrix * float4(vsin.inPos.xyz, 1.0);
      vsout.outColor = vsin.inColor;
      return vsout;
    }

    [pixelshader] float4 MainPS(VS_OUT psIn) : SV_TARGET {
        return float4(psIn.outColor, 1.0);
    }

    cbuffer AttactorMasses : register(b1)
    {
        float4 attractor[64];
    };
    uniform float dt;

    RWBuffer<float4> velocity_buffer : register(u2);
    RWBuffer<float4> position_buffer : register(u3);
    
    int tCost(float x)
    { return 0; }

    [numthreads(1024, 1, 1)]
    [computeshader] void MainCS(
        uint3 Gid : SV_GroupID, 
        uint3 DTid : SV_DispatchThreadID, 
        uint3 GTid : SV_GroupThreadID, 
        uint GI : SV_GroupIndex)
    {
        float4 vel = velocity_buffer[DTid.x];
        float4 pos = position_buffer[DTid.x];

        int i;

        pos.xyz += vel.xyz * dt;
        pos.w -= 0.0001 * dt;

        for (i = 0; i < 4; i++)
        {
            float3 dist = (attractor[i].xyz - pos.xyz);
            vel.xyz += dt * dt * attractor[i].w * normalize(dist) / (dot(dist, dist) + 10.0);
        }

        if (pos.w <= 0.0)
        {
            pos.xyz = -pos.xyz * 0.01;
            vel.xyz *= 0.01;
            pos.w += 1.0f;
        }
    
        position_buffer[DTid.x] = pos;
        velocity_buffer[DTid.x] = vel;
    }
    )";

using namespace ngfx;

class TestApp : public k3d::App
{
public:
  TestApp() : k3d::App("test", 1920, 1080) 
  {
  }

  bool OnInit() override
  {
    k3d::App::OnInit();

    CreateFactory(factory.GetAddressOf(), false);
    factory->SetName("VulkanFactory");

    uint32_t Count = 0;
    factory->EnumDevice(&Count, nullptr);
    factory->EnumDevice(&Count, device.GetAddressOf());

    device->CreateCommandQueue(CommandQueueType::Graphics, queue.GetAddressOf());
    queue->SetName("GraphicsQueue");

    SwapChainDesc swapChainDesc = { PixelFormat::RGBA8UNorm, 800, 600, 2, true, PixelFormat::D32Float };
    factory->CreateSwapchain(&swapChainDesc, queue.Get(), HostWindow()->GetHandle(), swapChain.GetAddressOf());
    swapChain->SetName("DefaultSwapchain");

    BufferDesc bufferDesc{ BufferViewBit::VertexBuffer, StorageOption::Managed, 10 };
    device->CreateBuffer(&bufferDesc, buffer.GetAddressOf());
    buffer->SetName("Buffer0");

    TextureDesc texDesc = { TextureViewBit::ShaderRead, StorageOption::Managed,
      MultiSampleFlag(0), PixelFormat::RGBA8UNorm,
      1024, 1024, 1, 1, 1 };
    device->CreateTexture(&texDesc, texture.GetAddressOf());
    texture->SetName("Texture0");

    device->CreateFence(fence.GetAddressOf());
    fence->SetName("DefaultFence");

    Ptr<Function> vertexFunction, fragFunction, computeFunction;
    uint64 BeginCompile = Os::GetTicks();
    device->CreateLibrary(nullptr, HLSL, strlen(HLSL), library.GetAddressOf());
    library->MakeFunction("MainCS", computeFunction.GetAddressOf());
    uint64 CompileCost = Os::GetTicks() - BeginCompile;
    
    uint64 BeginFile = Os::GetTicks();
    Os::MemMapFile blobFile;
    blobFile.Open("../../Data/Test/Test.blob", IORead);
    device->CreateLibrary(nullptr, blobFile.FileData(), blobFile.GetSize(), libraryBlob.GetAddressOf());
    blobFile.Close();
    libraryBlob->MakeFunction("MainVS", vertexFunction.GetAddressOf());
    uint64 FromFileCost = Os::GetTicks() - BeginFile;

    device->CreateComputePipeline(
      computeFunction.Get(), 
      pipelineLayout.Get(), 
      computePipeline.GetAddressOf());
    
    device->CreateComputePipeline(
      computeFunction.Get(), 
      computePipeline1.GetAddressOf(), 
      computePipelineReflection.GetAddressOf());

    ColorAttachmentDesc color0;
    DepthStencilAttachmentDesc depthStencil;
    RenderPassDesc renderPassDesc = { &color0, 1, &depthStencil };
    device->CreateRenderPass(&renderPassDesc, renderPass.GetAddressOf());
    swapChain->InitWithRenderPass(renderPass.Get());

    RenderPipelineDesc renderPipelineDesc = ngfxu::CreateDefaultRenderPipelineDesc();
    device->CreateRenderPipeline(
      &renderPipelineDesc, 
      pipelineLayout.Get(), 
      renderPass.Get(), 
      renderPipeline.GetAddressOf());

    device->CreateRenderPipeline(
      &renderPipelineDesc,
      renderPass.Get(),
      renderPipeline1.GetAddressOf(),
      renderPipelineReflection.GetAddressOf());

    device->CreatePipelineLibrary(nullptr, 0, pipelineLibrary.GetAddressOf());
    pipelineLibrary->StorePipeline("Render", renderPipeline.Get());
    pipelineLibrary->StorePipeline("Compute", computePipeline.Get());

    return true;
  }

  void OnProcess(k3d::Message &)override
  {
    auto drawable = swapChain->NextDrawable();
    Ptr<CommandBuffer> command;
    queue->CreateCommandBuffer(command.GetAddressOf());
    Ptr<RenderCommandEncoder> renderEncoder;
    command->CreateRenderCommandEncoder(drawable, renderPass.Get(), renderEncoder.GetAddressOf());
    renderEncoder->SetPipeline(renderPipeline.Get());
    renderEncoder->SetBindTable(nullptr);
    renderEncoder->SetIndexBuffer(nullptr);
    renderEncoder->SetVertexBuffer(0, 0, nullptr);
    renderEncoder->DrawIndexedInstanced(nullptr);
    renderEncoder->EndEncode();

    /*
    Ptr<Framebuffer> fbo;
    FramebufferDesc fbD;
    fbD.setSize(1024, 1024);
    fbD.attach(0, colorTexture0);
    fbD.attach(1, colorTexture1);
    fbD.attach(2, colorTexture2);
    fbD.attachDs(depthStencil);
    device->CreateFramebuffer(&fbD, fbo.GetAddressOf());
    command->CreateRenderCommandEncoder(fbo.Get(), renderPass.Get(), renderEncoder.GetAddressOf());
    */
    command->Commit(fence.Get());
    //swapChain->Present()
  }

  Ptr<Factory>      factory;
  Ptr<Device>       device;
  Ptr<CommandQueue> queue;
  Ptr<SwapChain>    swapChain;
  Ptr<Buffer>       buffer;
  Ptr<Texture>      texture;
  Ptr<Fence>        fence;
  //Ptr<Compiler>     compiler;

  Ptr<Library>            library, libraryBlob;
  Ptr<PipelineLibrary>    pipelineLibrary;
  Ptr<RenderPass>         renderPass;
  Ptr<PipelineLayout>     pipelineLayout;
  Ptr<Pipeline>           renderPipeline;
  Ptr<Pipeline>           renderPipeline1;
  Ptr<PipelineReflection> renderPipelineReflection;
  Ptr<Pipeline>           computePipeline;
  Ptr<Pipeline>           computePipeline1;
  Ptr<PipelineReflection> computePipelineReflection;
};

int main()
{
  TestApp app;
  app.OnInit();
  return (int)app.Run();
}
