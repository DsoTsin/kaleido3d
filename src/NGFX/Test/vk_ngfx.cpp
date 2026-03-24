#include "CoreMinimal.h"
#include "ngfx_shell.h"

static void log_print(int level, const char* msg)
{
    printf("Log: %s\n", msg);
}

typedef ngfx::Factory* (*fnCreate)(bool debug, decltype(log_print) call);

#if K3DPLATFORM_OS_PROSPERO
static constexpr const char* kDefaultNgfxLibrary = "ngfx_agc";
#else
static constexpr const char* kDefaultNgfxLibrary = "ngfx_vk.dll";
#endif

class MyGfxApp : public k3d::App {
public:
    MyGfxApp()
        : k3d::App("NewApp", 800, 600)
    {
    }

    void OnProcess(k3d::Message& message) override
    {
        if (message.type == k3d::Message::Resized) {
            int x = message.size.height;
            ++x;
        }
    }

    virtual bool OnInit() override
    {
        App::OnInit();

        k3d::os::LibraryLoader loader(kDefaultNgfxLibrary);
        fnCreate create = (fnCreate)loader.ResolveSymbol("CreateFactory");
        factory = create(true, log_print);
        device = factory.getDevice(0);
        auto handle = HostWindow()->GetHandle();
        layer = factory.newPresentLayer(
            ngfx::PresentLayerDesc {
                ngfx::PixelFormat::BGRA8Unorm, /* format */
                HostWindow()->Width(), HostWindow()->Height(),
                ngfx::ColorSpace::SRGBNonLinear, /* colorSpace */
                true,
                3,
                handle, /* hWnd */
                nullptr /* hInstance */
            },
            device
        );

        fence = device.newFence();
        gfxQueue = device.newQueue();
        computeQueue = device.newQueue();

        return true;
    }

    
  void OnUpdate() override;

private:
    ngfxu::Factory factory;
    ngfxu::Device device;
    ngfxu::CommandQueue gfxQueue;
    ngfxu::CommandQueue computeQueue;
    ngfxu::PresentLayer layer;
    ngfxu::Fence fence;
};

/*
 * Metal memoryBarrierWithResources
 */
#if _WIN32
int WinMain(void*, void*, char*, int)
#else
int main(int argc, const char* argv[])
#endif
{
    MyGfxApp app;
    return k3d::RunApplication(app, "MyGfxApp");
}

void MyGfxApp::OnUpdate()
{
    auto drawable = layer.nextDrawable();
    ngfxu::CommandBuffer cmdBuf = gfxQueue.obtainCommandBuffer();
    ngfxu::CommandBuffer computeCmdBuf = computeQueue.obtainCommandBuffer();
    ngfxu::ComputeEncoder computeEncoder = computeCmdBuf.newComputeEncoder();
    // render pass describes the in out resource used in this render pass
    ngfxu::RenderCommandEncoder renderCmd = cmdBuf.newRenderEncoder(ngfx::RenderpassDesc());

    // renderCmd.setRenderPipelineState();
    // renderCmd.setBindGroup();
    // renderCmd.draw()
    ngfxu::ParallelRenderEncoder parallelRenderCmd = cmdBuf.newParallelRenderEncoder(ngfx::RenderpassDesc());

    ngfxu::RenderCommandEncoder subRenderCmd0 = parallelRenderCmd.subEncoder();
    ngfxu::RenderCommandEncoder subRenderCmd1 = parallelRenderCmd.subEncoder();
    ngfxu::RenderCommandEncoder subRenderCmd2 = parallelRenderCmd.subEncoder();
    ngfxu::RenderCommandEncoder subRenderCmd3 = parallelRenderCmd.subEncoder();

    subRenderCmd0.endEncode();
    subRenderCmd1.endEncode();
    subRenderCmd2.endEncode();
    subRenderCmd3.endEncode();

    parallelRenderCmd.endEncode();
    renderCmd.waitForFence(fence); // wait for fence
    // present drawable
    renderCmd.presentDrawable(drawable);
    renderCmd.endEncode();

    computeEncoder.dispatch(64, 64, 1);
    computeEncoder.updateFence(fence); // signal the fence
    computeEncoder.endEncode();
    computeCmdBuf.commit();
    device.wait();
    // execution order
    // renderCmd ->
    // parallelRenderCmd ->
    // subRenderCmd0 -> subRenderCmd1 -> subRenderCmd2 -> subRenderCmd3
    cmdBuf.commit();
    device.wait();
}
