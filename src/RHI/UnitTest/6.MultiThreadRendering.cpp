#include <Core/Kaleido3D.h>
#include <Base/UTRHIAppBase.h>
#include <Core/App.h>
#include <Core/Os.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <Core/Interface/IRHI.h>
#include <Core/Math/kMath.hpp>

using namespace k3d;
using namespace kMath;

class TriangleMesh;

class MultiThreadRenderingApp : public RHIAppBase
{
public:
  MultiThreadRenderingApp(String const& appName, uint32 width, uint32 height)
    : RHIAppBase(appName, width, height, true)
  {
  }

  explicit MultiThreadRenderingApp(String const& appName)
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

private:
  std::unique_ptr<TriangleMesh> m_TriMesh;
  NGFXResourceRef m_ConstBuffer;

  NGFXPipelineStateRef m_pPso;
  NGFXPipelineLayoutRef m_pl;
};

K3D_APP_MAIN(MultiThreadRenderingApp)

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

  explicit TriangleMesh(NGFXDeviceRef device)
    : m_pDevice(device)
      , vbuf(nullptr)
      , ibuf(nullptr)
  {
    m_szVBuf = sizeof(Vertex) * m_VertexBuffer.size();
    m_szIBuf = sizeof(uint32) * m_IndexBuffer.size();

    m_IAState.Attribs[0] = {NGFX_VERTEX_FORMAT_FLOAT3X32, 0, 0};
    m_IAState.Attribs[1] = {NGFX_VERTEX_FORMAT_FLOAT3X32, sizeof(float) * 3, 0};

    m_IAState.Layouts[0] = {NGFX_VERTEX_INPUT_RATE_PER_VERTEX, sizeof(Vertex)};
  }

  ~TriangleMesh()
  {
  }

  const NGFXVertexInputState& GetInputState() const { return m_IAState; }

  void Upload();

  void SetLoc(uint64 ibo, uint64 vbo)
  {
    iboLoc = ibo;
    vboLoc = vbo;
  }

  const NGFXVertexBufferView VBO() const
  {
    return NGFXVertexBufferView{vboLoc, 0, 0};
  }

  const NGFXIndexBufferView IBO() const
  {
    return NGFXIndexBufferView{iboLoc, 0};
  }

private:
  NGFXVertexInputState m_IAState;

  uint64 m_szVBuf;
  uint64 m_szIBuf;

  VertexList m_VertexBuffer = {{{1.0f, 1.0f, 0.0f},{1.0f, 0.0f, 0.0f}},
    {{-1.0f, 1.0f, 0.0f},{0.0f, 1.0f, 0.0f}},
    {{0.0f, -1.0f, 0.0f},
      {0.0f, 0.0f, 1.0f}}};
  IndiceList m_IndexBuffer = {0, 1, 2};

  uint64 iboLoc;
  uint64 vboLoc;

  NGFXDeviceRef m_pDevice;
  NGFXResourceRef vbuf, ibuf;
};

void
TriangleMesh::Upload()
{
  // create stage buffers
  NGFXResourceDesc stageDesc;
  stageDesc.ViewFlags = NGFX_RESOURCE_VIEW_UNDEFINED;
  stageDesc.CreationFlag = NGFX_RESOURCE_TRANSFER_SRC;
  stageDesc.Flag = NGFX_ACCESS_HOST_COHERENT |
    NGFX_ACCESS_HOST_VISIBLE;
  stageDesc.Size = m_szVBuf;
  auto vStageBuf = m_pDevice->CreateResource(stageDesc);
  void* ptr = vStageBuf->Map(0, m_szVBuf);
  memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
  vStageBuf->UnMap();

  stageDesc.Size = m_szIBuf;
  auto iStageBuf = m_pDevice->CreateResource(stageDesc);
  ptr = iStageBuf->Map(0, m_szIBuf);
  memcpy(ptr, &m_IndexBuffer[0], m_szIBuf);
  iStageBuf->UnMap();

  NGFXResourceDesc bufferDesc;
  bufferDesc.ViewFlags = NGFX_RESOURCE_VERTEX_BUFFER_VIEW;
  bufferDesc.Size = m_szVBuf;
  bufferDesc.CreationFlag = NGFX_RESOURCE_TRANSFER_DST;
  bufferDesc.Flag = NGFX_ACCESS_DEVICE_VISIBLE;
  vbuf = m_pDevice->CreateResource(bufferDesc);
  bufferDesc.ViewFlags = NGFX_RESOURCE_INDEX_BUFFER_VIEW;
  bufferDesc.Size = m_szIBuf;
  ibuf = m_pDevice->CreateResource(bufferDesc);

  uint64 vboLoc = vbuf->GetLocation();
  uint64 iboLoc = ibuf->GetLocation();
  SetLoc(iboLoc, vboLoc);
  KLOG(Info, TriangleMesh, "finish buffer upload..");
}

bool
MultiThreadRenderingApp::OnInit()
{
  bool inited = RHIAppBase::OnInit();
  if (!inited)
    return inited;

  KLOG(Info, Test, __K3D_FUNC__);

  PrepareResource();
  PreparePipeline();

  return true;
}

void
MultiThreadRenderingApp::PrepareResource()
{
  KLOG(Info, Test, __K3D_FUNC__);
  m_TriMesh = std::make_unique<TriangleMesh>(m_pDevice.Get());
  m_TriMesh->Upload();

  NGFXResourceDesc desc;
  desc.Flag = NGFX_ACCESS_HOST_VISIBLE;
  desc.ViewFlags = NGFX_RESOURCE_CONSTANT_BUFFER_VIEW;
  desc.Size = sizeof(MVPMatrix);
  m_ConstBuffer = m_pDevice->CreateResource(desc);
  OnUpdate();
}

void
MultiThreadRenderingApp::PreparePipeline()
{
  NGFXShaderBundle vertSh, fragSh;
  Compile("asset://Test/triangle.vert", NGFX_SHADER_TYPE_VERTEX, vertSh);
  Compile("asset://Test/triangle.frag", NGFX_SHADER_TYPE_FRAGMENT, fragSh);
  NGFXPipelineLayoutDesc ppldesc = vertSh.BindingTable;
  m_pl = m_pDevice->CreatePipelineLayout(ppldesc);
  if (m_pl)
  {
    auto descriptor = m_pl->ObtainBindingGroup();
    descriptor->Update(0, m_ConstBuffer);
  }
  auto attrib = vertSh.Attributes;

  NGFXRenderPipelineDesc desc;
  desc.AttachmentsBlend.Append(NGFXAttachmentState());
  desc.VertexShader = vertSh;
  desc.PixelShader = fragSh;
  desc.InputState = m_TriMesh->GetInputState();

  NGFXColorAttachmentDesc ColorAttach;
  ColorAttach.pTexture = m_pSwapChain->GetCurrentTexture();
  ColorAttach.LoadAction = NGFX_LOAD_ACTION_CLEAR;
  ColorAttach.StoreAction = NGFX_STORE_ACTION_STORE;
  ColorAttach.ClearColor = Vec4f(1, 1, 1, 1);

  NGFXRenderPassDesc Desc;
  Desc.ColorAttachments.Append(ColorAttach);
  auto pRenderPass = m_pDevice->CreateRenderPass(Desc);

  m_pPso = m_pDevice->CreateRenderPipelineState(desc, m_pl, pRenderPass);
}

void
MultiThreadRenderingApp::OnDestroy()
{
  App::OnDestroy();
  m_TriMesh->~TriangleMesh();
}

void
MultiThreadRenderingApp::OnProcess(Message& msg)
{
  auto currentImage = m_pSwapChain->GetCurrentTexture();
  auto commandBuffer = m_pQueue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT);
  // command encoder like Metal does
  NGFXColorAttachmentDesc ColorAttach;
  ColorAttach.pTexture = m_pSwapChain->GetCurrentTexture();
  ColorAttach.LoadAction = NGFX_LOAD_ACTION_CLEAR;
  ColorAttach.StoreAction = NGFX_STORE_ACTION_STORE;
  ColorAttach.ClearColor = Vec4f(1, 1, 1, 1);
  NGFXRenderPassDesc Desc;
  Desc.ColorAttachments.Append(ColorAttach);
  auto parallelRenderCmd = commandBuffer->ParallelRenderCommandEncoder(Desc);
  DynArray<SharedPtr<Os::Thread>> threadPool;
  threadPool.Append(MakeShared<Os::Thread>([=]()
                                         {
                                           auto subRenderCmd = parallelRenderCmd->SubRenderCommandEncoder();
                                           // TODO subRenderCmd->SetBindingGroup();
                                           NGFXRect rect{0,0,1920,1080};
                                           subRenderCmd->SetScissorRect(rect);
                                           subRenderCmd->SetViewport(NGFXViewportDesc(1920, 1080));
                                           subRenderCmd->SetPipelineState(0, m_pPso);
                                           subRenderCmd->SetIndexBuffer(m_TriMesh->IBO());
                                           subRenderCmd->SetVertexBuffer(0, m_TriMesh->VBO());
                                           subRenderCmd->DrawIndexedInstanced(NGFXDrawIndexedInstancedParam(3, 1));
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
  commandBuffer->Present(m_pSwapChain.Get(), m_pFence.Get());
  commandBuffer->Commit(); // submit
}

void
MultiThreadRenderingApp::OnUpdate()
{
  MVPMatrix* ptr = (MVPMatrix*)m_ConstBuffer->Map(0, sizeof(MVPMatrix));
  auto& HostBuffer = *ptr;
  HostBuffer.projectionMatrix = Perspective(60.0f,
                                            (float)1920 /
                                            (float)1080,
                                            0.1f,
                                            256.0f);
  HostBuffer.viewMatrix =
    Translate(Vec3f(0.0f, 0.0f, -2.5f), MakeIdentityMatrix<float>());
  HostBuffer.modelMatrix = MakeIdentityMatrix<float>();
  HostBuffer.modelMatrix =
    Rotate(Vec3f(1.0f, 0.0f, 0.0f), 0.f, HostBuffer.modelMatrix);
  HostBuffer.modelMatrix =
    Rotate(Vec3f(0.0f, 1.0f, 0.0f), 0.f, HostBuffer.modelMatrix);
  HostBuffer.modelMatrix =
    Rotate(Vec3f(0.0f, 0.0f, 1.0f), 0.f, HostBuffer.modelMatrix);

  m_ConstBuffer->UnMap();
}
