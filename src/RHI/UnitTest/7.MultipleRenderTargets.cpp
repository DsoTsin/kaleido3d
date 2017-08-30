#include <Kaleido3D.h>
#include <Base/UTRHIAppBase.h>
#include <Core/App.h>
#include <Core/Os.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <Interface/IRHI.h>
#include <Math/kMath.hpp>
#include <Renderer/Render.h>

using namespace k3d;
using namespace render;
using namespace kMath;

class TriangleMesh;

struct ConstantBuffer
{
  Mat4f projectionMatrix;
  Mat4f modelMatrix;
  Mat4f viewMatrix;
};

class MultipleRenderTargetsApp : public RHIAppBase
{
public:
  MultipleRenderTargetsApp(kString const& appName, uint32 width, uint32 height)
    : RHIAppBase(appName, width, height, true)
  {
  }
  explicit MultipleRenderTargetsApp(kString const& appName)
    : RHIAppBase(appName, 1920, 1080, true)
  {
  }

  bool OnInit() override;
  void OnDestroy() override;
  void OnProcess(Message& msg) override;

  void OnUpdate() override;

protected:
  void PrepareResource();
  void PreparePipeline();
  void PrepareCommandBuffer();

private:
  rhi::IShCompiler::Ptr m_Compiler;
  std::unique_ptr<TriangleMesh> m_TriMesh;

  rhi::GpuResourceRef m_ConstBuffer;
  ConstantBuffer m_HostBuffer;

  rhi::PipelineStateRef m_pPso;
  rhi::PipelineLayoutRef m_pl;
  std::vector<rhi::CommandContextRef> m_Cmds;
  rhi::SyncFenceRef m_pFence;
  rhi::CommandQueueRef m_pQueue;
};

K3D_APP_MAIN(MultipleRenderTargetsApp)

class TriangleMesh
{
public:
  struct Vertex
  {
    float pos[3];
    float col[3];
  };
  typedef std::vector<Vertex> VertexList;
  typedef std::vector<uint32> IndiceList;

  explicit TriangleMesh(rhi::DeviceRef device)
    : m_pDevice(device)
    , vbuf(nullptr)
    , ibuf(nullptr)
  {
    m_szVBuf = sizeof(TriangleMesh::Vertex) * m_VertexBuffer.size();
    m_szIBuf = sizeof(uint32) * m_IndexBuffer.size();

    m_IAState.Attribs[0] = { rhi::EVF_Float3x32, 0, 0 };
    m_IAState.Attribs[1] = { rhi::EVF_Float3x32, sizeof(float) * 3, 0 };

    m_IAState.Layouts[0] = { rhi::EVIR_PerVertex, sizeof(Vertex) };
  }

  ~TriangleMesh() {}

  const rhi::VertexInputState& GetInputState() const { return m_IAState; }

  void Upload();

  void SetLoc(uint64 ibo, uint64 vbo)
  {
    iboLoc = ibo;
    vboLoc = vbo;
  }

  const rhi::VertexBufferView VBO() const
  {
    return rhi::VertexBufferView{ vboLoc, 0, 0 };
  }

  const rhi::IndexBufferView IBO() const
  {
    return rhi::IndexBufferView{ iboLoc, 0 };
  }

private:
  rhi::VertexInputState m_IAState;

  uint64 m_szVBuf;
  uint64 m_szIBuf;

  VertexList m_VertexBuffer = { { { 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
  { { -1.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
  { { 0.0f, -1.0f, 0.0f },
  { 0.0f, 0.0f, 1.0f } } };
  IndiceList m_IndexBuffer = { 0, 1, 2 };

  uint64 iboLoc;
  uint64 vboLoc;

  rhi::DeviceRef m_pDevice;
  rhi::GpuResourceRef vbuf, ibuf;
};

void
TriangleMesh::Upload()
{
  // create stage buffers
  rhi::ResourceDesc stageDesc;
  stageDesc.ViewType = rhi::EGpuMemViewType::EGVT_Undefined;
  stageDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferSrc;
  stageDesc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_HostCoherent |
    rhi::EGpuResourceAccessFlag::EGRAF_HostVisible;
  stageDesc.Size = m_szVBuf;
  auto vStageBuf = m_pDevice->NewGpuResource(stageDesc);
  void* ptr = vStageBuf->Map(0, m_szVBuf);
  memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
  vStageBuf->UnMap();

  stageDesc.Size = m_szIBuf;
  auto iStageBuf = m_pDevice->NewGpuResource(stageDesc);
  ptr = iStageBuf->Map(0, m_szIBuf);
  memcpy(ptr, &m_IndexBuffer[0], m_szIBuf);
  iStageBuf->UnMap();

  rhi::ResourceDesc bufferDesc;
  bufferDesc.ViewType = rhi::EGpuMemViewType::EGVT_VBV;
  bufferDesc.Size = m_szVBuf;
  bufferDesc.CreationFlag = rhi::EGpuResourceCreationFlag::EGRCF_TransferDst;
  bufferDesc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_DeviceVisible;
  vbuf = m_pDevice->NewGpuResource(bufferDesc);
  bufferDesc.ViewType = rhi::EGpuMemViewType::EGVT_IBV;
  bufferDesc.Size = m_szIBuf;
  ibuf = m_pDevice->NewGpuResource(bufferDesc);
#if 0
  auto cmd = m_pQueue->ObtainCommandBuffer(rhi::ECMD_Graphics);
  rhi::CopyBufferRegion region = { 0, 0, m_szVBuf };
  cmd->Begin();
  cmd->CopyBuffer(*vbuf, *vStageBuf, region);
  region.CopySize = m_szIBuf;
  cmd->CopyBuffer(*ibuf, *iStageBuf, region);
  cmd->End();
  cmd->Execute(true);
#endif
  //	m_m_pDevice->WaitIdle();
  uint64 vboLoc = vbuf->GetLocation();
  uint64 iboLoc = ibuf->GetLocation();
  SetLoc(iboLoc, vboLoc);
  KLOG(Info, TriangleMesh, "finish buffer upload..");
}

bool
MultipleRenderTargetsApp::OnInit()
{
  bool inited = RHIAppBase::OnInit();
  if (!inited)
    return inited;

  KLOG(Info, Test, __K3D_FUNC__);

  PrepareResource();
  PreparePipeline();
  PrepareCommandBuffer();

  return true;
}

void
MultipleRenderTargetsApp::PrepareResource()
{
  KLOG(Info, Test, __K3D_FUNC__);
  m_TriMesh = std::make_unique<TriangleMesh>(m_pDevice);
  m_TriMesh->Upload();

  rhi::ResourceDesc desc;
  desc.Flag = rhi::EGpuResourceAccessFlag::EGRAF_HostVisible;
  desc.ViewType = rhi::EGpuMemViewType::EGVT_CBV;
  desc.Size = sizeof(ConstantBuffer);
  m_ConstBuffer = m_pDevice->NewGpuResource(desc);
  OnUpdate();
  //	m_pDevice->WaitIdle(); TOFIX: this may cause crash on Android N
}

void
MultipleRenderTargetsApp::PreparePipeline()
{
  rhi::ShaderBundle vertSh, fragSh;
  Compile("asset://Test/triangle.vert", rhi::ES_Vertex, vertSh);
  Compile("asset://Test/triangle.frag", rhi::ES_Fragment, fragSh);
  rhi::PipelineLayoutDesc ppldesc = vertSh.BindingTable;
  m_pl = m_pDevice->NewPipelineLayout(ppldesc);
  if (m_pl) {
    auto descriptor = m_pl->GetDescriptorSet();
    descriptor->Update(0, m_ConstBuffer);
  }
  auto attrib = vertSh.Attributes;
  rhi::RenderPipelineStateDesc desc;
  desc.VertexShader = vertSh;
  desc.PixelShader = fragSh;
  desc.InputState = m_TriMesh->GetInputState();
  m_pPso = m_pDevice->CreateRenderPipelineState(desc, m_pl);
  // m_pPso->LoadPSO("triagle.psocache");
}

void
MultipleRenderTargetsApp::PrepareCommandBuffer()
{
  m_pFence = m_pDevice->CreateFence();
}

void
MultipleRenderTargetsApp::OnDestroy()
{
  App::OnDestroy();
  m_TriMesh->~TriangleMesh();
}

void
MultipleRenderTargetsApp::OnProcess(Message& msg)
{
  auto currentImage = m_pSwapChain->GetCurrentTexture();
  auto commandBuffer = m_pQueue->ObtainCommandBuffer(rhi::ECMDUsage_OneShot);
  commandBuffer->Transition(currentImage, rhi::ERS_RenderTarget);
  // command encoder like Metal does
  auto parallelRenderCmd = commandBuffer->ParallelRenderCommandEncoder(nullptr, nullptr);
  DynArray<SharedPtr<Os::Thread>> threadPool;
  threadPool.Append(MakeShared<Os::Thread>([=]() {
    auto subRenderCmd = parallelRenderCmd->SubRenderCommandEncoder();
    subRenderCmd->SetPipelineLayout(m_pl);
    rhi::Rect rect{0,0,1920,1080};
    subRenderCmd->SetScissorRect(rect);
    subRenderCmd->SetViewport(rhi::ViewportDesc(1920,1080));
    subRenderCmd->SetPipelineState(0, m_pPso);
    subRenderCmd->SetIndexBuffer(m_TriMesh->IBO());
    subRenderCmd->SetVertexBuffer(0, m_TriMesh->VBO());
    subRenderCmd->DrawIndexedInstanced(rhi::DrawIndexedInstancedParam(3, 1));
    subRenderCmd->EndEncode(); // push into parallel command encoder
  }, "SubRenderThread"));
  for (auto subRenderThread : threadPool)
  {
    subRenderThread->Start();
  }
  for (auto subRenderThread : threadPool)
  {
    subRenderThread->Join();
  }
  // execute all sub commands
  parallelRenderCmd->EndEncode();

  commandBuffer->Transition(currentImage, rhi::ERS_Present);
  commandBuffer->Present(m_pSwapChain, m_pFence);
  commandBuffer->Commit(); // submit
}

void
MultipleRenderTargetsApp::OnUpdate()
{
  m_HostBuffer.projectionMatrix = Perspective(60.0f,
    (float)1920 /
    (float)1080,
    0.1f,
    256.0f);
  m_HostBuffer.viewMatrix =
    Translate(Vec3f(0.0f, 0.0f, -2.5f), MakeIdentityMatrix<float>());
  m_HostBuffer.modelMatrix = MakeIdentityMatrix<float>();
  m_HostBuffer.modelMatrix =
    Rotate(Vec3f(1.0f, 0.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
  m_HostBuffer.modelMatrix =
    Rotate(Vec3f(0.0f, 1.0f, 0.0f), 0.f, m_HostBuffer.modelMatrix);
  m_HostBuffer.modelMatrix =
    Rotate(Vec3f(0.0f, 0.0f, 1.0f), 0.f, m_HostBuffer.modelMatrix);
  void* ptr = m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
  memcpy(ptr, &m_HostBuffer, sizeof(ConstantBuffer));
  m_ConstBuffer->UnMap();
}
