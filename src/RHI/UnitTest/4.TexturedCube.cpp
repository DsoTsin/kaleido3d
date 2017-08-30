#include <Base/TextureObject.h>
#include <Base/UTRHIAppBase.h>
#include <Core/App.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <Core/Kaleido3D.h>
#include <Core/Math/kMath.hpp>
//#include <steam/steam_api.h>

using namespace k3d;
using namespace kMath;

class CubeMesh;

struct ConstantBuffer
{
  Mat4f projectionMatrix;
  Mat4f modelMatrix;
  Mat4f viewMatrix;
};

class TCubeUnitTest : public RHIAppBase
{
public:
  TCubeUnitTest(String const& appName, uint32 width, uint32 height)
    : RHIAppBase(appName, width, height)
  {
  }

  explicit TCubeUnitTest(String const& appName)
    : RHIAppBase(appName, 1920, 1080)
  {
  }

  ~TCubeUnitTest() { KLOG(Info, CubeTest, "Destroying.."); }
  bool OnInit() override;
  void OnDestroy() override;
  void OnProcess(Message& msg) override;

  void OnUpdate() override;

protected:
  NGFXResourceRef CreateStageBuffer(uint64 size);
  void LoadTexture();
  void PrepareResource();
  void PreparePipeline();
  void PrepareCommandBuffer();

private:
  SharedPtr<CubeMesh> m_CubeMesh;
  NGFXResourceRef m_ConstBuffer;
  SharedPtr<TextureObject> m_Texture;
  ConstantBuffer m_HostBuffer;

  NGFXPipelineStateRef m_pPso;
  NGFXPipelineLayoutRef m_pl;
  NGFXBindingGroupRef m_BindingGroup;
};

K3D_APP_MAIN(TCubeUnitTest);

class CubeMesh
{
public:
  struct Vertex
  {
    float pos[3];
    float col[4];
    float uv[2];
  };
  typedef std::vector<Vertex> VertexList;
  typedef std::vector<uint32> IndiceList;

  explicit CubeMesh(NGFXDeviceRef device)
    : vbuf(nullptr)
    , m_pDevice(device)
  {
    m_szVBuf = sizeof(Vertex) * m_VertexBuffer.size();
    m_IAState.Attribs[0] = {NGFX_VERTEX_FORMAT_FLOAT3X32, 0, 0};
    m_IAState.Attribs[1] = {NGFX_VERTEX_FORMAT_FLOAT4X32, sizeof(float) * 3, 0};
    m_IAState.Attribs[2] = {NGFX_VERTEX_FORMAT_FLOAT2X32, sizeof(float) * 7, 0};

    m_IAState.Layouts[0] = {NGFX_VERTEX_INPUT_RATE_PER_VERTEX, sizeof(Vertex)};
  }

  ~CubeMesh()
  {
  }

  const NGFXVertexInputState& GetInputState() const { return m_IAState; }

  void Upload();

  void SetLoc(uint64 vbo) { vboLoc = vbo; }

  const NGFXVertexBufferView VBO() const
  {
    return NGFXVertexBufferView{ vboLoc, 0, 0 };
  }

private:
  NGFXVertexInputState m_IAState;

  uint64 m_szVBuf;

  VertexList m_VertexBuffer = {
    // left
    { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

    { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },

    // back
    { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },

    { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },

    // top
    { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },

    { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    // front
    { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },

    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },

    // right
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },

    { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },

    // bottom
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },

    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
  };

  NGFXResourceRef vbuf;
  uint64 vboLoc;
  NGFXDeviceRef m_pDevice;
};

void
CubeMesh::Upload()
{
  // create stage buffers
  NGFXResourceDesc stageDesc;
  stageDesc.ViewFlags = NGFX_RESOURCE_VIEW_UNDEFINED;
  stageDesc.CreationFlag = NGFX_RESOURCE_TRANSFER_SRC;
  stageDesc.Flag = NGFX_ACCESS_HOST_COHERENT | NGFX_ACCESS_HOST_VISIBLE;
  stageDesc.Size = m_szVBuf;
  auto vStageBuf = m_pDevice->CreateResource(stageDesc);
  void* ptr = vStageBuf->Map(0, m_szVBuf);
  memcpy(ptr, &m_VertexBuffer[0], m_szVBuf);
  vStageBuf->UnMap();

  NGFXResourceDesc bufferDesc;
  bufferDesc.ViewFlags = NGFX_RESOURCE_VERTEX_BUFFER_VIEW;
  bufferDesc.Size = m_szVBuf;
  bufferDesc.CreationFlag = NGFX_RESOURCE_TRANSFER_DST;
  bufferDesc.Flag = NGFX_ACCESS_DEVICE_VISIBLE;
  vbuf = m_pDevice->CreateResource(bufferDesc);

  auto pQueue = m_pDevice->CreateCommandQueue(NGFX_COMMAND_GRAPHICS);
  auto cmdBuf = pQueue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT);
  NGFXCopyBufferRegion region = {0, 0, m_szVBuf};
  cmdBuf->CopyBuffer(vbuf, vStageBuf, region);
  cmdBuf->Commit();
  m_pDevice->WaitIdle();
  SetLoc(vbuf->GetLocation());
  KLOG(Info, TCubeMesh, "finish buffer upload..");
}

bool
TCubeUnitTest::OnInit()
{
  bool inited = RHIAppBase::OnInit();
  if (!inited)
    return inited;
#if 0
  bool res = SteamAPI_RestartAppIfNecessary(0);
  bool init = SteamAPI_Init();
  auto controller = SteamController();
  controller->Init();
  ControllerHandle_t pHandles[STEAM_CONTROLLER_MAX_COUNT];
  int nNumActive = SteamController()->GetConnectedControllers(pHandles);
#endif
  KLOG(Info, Test, __K3D_FUNC__);
  PrepareResource();
  PreparePipeline();
  PrepareCommandBuffer();
  return true;
}

NGFXResourceRef
TCubeUnitTest::CreateStageBuffer(uint64 size)
{
  NGFXResourceDesc stageDesc;
  stageDesc.ViewFlags = NGFX_RESOURCE_VIEW_UNDEFINED;
  stageDesc.CreationFlag = NGFX_RESOURCE_TRANSFER_SRC;
  stageDesc.Flag = NGFX_ACCESS_HOST_COHERENT |
    NGFX_ACCESS_HOST_VISIBLE;
  stageDesc.Size = size;
  return m_pDevice->CreateResource(stageDesc);
}

void
TCubeUnitTest::LoadTexture()
{
  IAsset* textureFile = AssetManager::Open("asset://Test/bricks.tga");
  if (textureFile) {
    uint64 length = textureFile->GetLength();
    uint8* data = new uint8[length];
    textureFile->Read(data, length);
    m_Texture = MakeShared<TextureObject>(m_pDevice.Get(), data, false);
    auto texStageBuf = CreateStageBuffer(m_Texture->GetSize());
    m_Texture->MapIntoBuffer(texStageBuf);
    m_Texture->CopyAndInitTexture(texStageBuf);
    NGFXSRVDesc viewDesc;
    auto srv = m_pDevice->CreateShaderResourceView(m_Texture->GetResource(), viewDesc);
    auto texure = static_cast<NGFXTexture*>(m_Texture->GetResource()); //
    texure->SetResourceView(Move(srv)); // here
    NGFXSamplerState samplerDesc;
    auto sampler2D = m_pDevice->CreateSampler(samplerDesc);
    texure->BindSampler(Move(sampler2D));
  }
}

void
TCubeUnitTest::PrepareResource()
{
  KLOG(Info, Test, __K3D_FUNC__);
  m_CubeMesh = MakeShared<CubeMesh>(m_pDevice.Get());
  m_CubeMesh->Upload();
  LoadTexture();
  NGFXResourceDesc desc;
  desc.Flag = NGFX_ACCESS_HOST_VISIBLE;
  desc.ViewFlags = NGFX_RESOURCE_CONSTANT_BUFFER_VIEW;
  desc.Size = sizeof(ConstantBuffer);
  m_ConstBuffer = m_pDevice->CreateResource(desc);
  MVPMatrix* ptr = (MVPMatrix*)m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
  (*ptr).projectionMatrix = Perspective(60.0f, 1920.f / 1080.f, 0.1f, 256.0f);
  (*ptr).viewMatrix = Translate(Vec3f(0.0f, 0.0f, -4.5f), MakeIdentityMatrix<float>());
  (*ptr).modelMatrix = MakeIdentityMatrix<float>();
  static auto angle = 60.f;
#if K3DPLATFORM_OS_ANDROID
  angle += 0.1f;
#else
  angle += 0.001f;
#endif
  (*ptr).modelMatrix = Rotate(Vec3f(1.0f, 0.0f, 0.0f), angle, (*ptr).modelMatrix);
  (*ptr).modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), angle, (*ptr).modelMatrix);
  (*ptr).modelMatrix = Rotate(Vec3f(0.0f, 0.0f, 1.0f), angle, (*ptr).modelMatrix);
  m_ConstBuffer->UnMap();
}

void
TCubeUnitTest::PreparePipeline()
{
  NGFXShaderBundle vertSh, fragSh;
  Compile("asset://Test/cube.vert", NGFX_SHADER_TYPE_VERTEX, vertSh);
  Compile("asset://Test/cube.frag", NGFX_SHADER_TYPE_FRAGMENT, fragSh);
  auto vertBinding = vertSh.BindingTable;
  auto fragBinding = fragSh.BindingTable;
  auto mergedBindings = vertBinding | fragBinding;
  m_pl = m_pDevice->CreatePipelineLayout(mergedBindings);
  m_BindingGroup = m_pl->ObtainBindingGroup();
  m_BindingGroup->Update(0, m_ConstBuffer);
  m_BindingGroup->Update(1, m_Texture->GetResource());

  NGFXRenderPipelineDesc desc;
  desc.AttachmentsBlend.Append(NGFXAttachmentState());
  desc.VertexShader = vertSh;
  desc.PixelShader = fragSh;
  desc.InputState = m_CubeMesh->GetInputState();

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
TCubeUnitTest::PrepareCommandBuffer()
{
}

void
TCubeUnitTest::OnDestroy()
{
  m_pFence->WaitFor(1000);
  RHIAppBase::OnDestroy();
}

void
TCubeUnitTest::OnProcess(Message& msg)
{
  if (msg.type == Message::Resized)
  {
    KLOG(Info, App, "Resized.. %d, %d.", msg.size.width, msg.size.height);
    m_pSwapChain->Resize(msg.size.width, msg.size.height);
  }
}

void
TCubeUnitTest::OnUpdate()
{
  MVPMatrix* ptr = (MVPMatrix*)m_ConstBuffer->Map(0, sizeof(ConstantBuffer));
  (*ptr).projectionMatrix = Perspective(60.0f, 1920.f / 1080.f, 0.1f, 256.0f);
  (*ptr).viewMatrix = Translate(Vec3f(0.0f, 0.0f, -4.5f), MakeIdentityMatrix<float>());
  (*ptr).modelMatrix = MakeIdentityMatrix<float>();
  static auto angle = 60.f;
#if K3DPLATFORM_OS_ANDROID
  angle += 0.1f;
#else
  angle += 0.001f;
#endif
  (*ptr).modelMatrix = Rotate(Vec3f(1.0f, 0.0f, 0.0f), angle, (*ptr).modelMatrix);
  (*ptr).modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), angle, (*ptr).modelMatrix);
  (*ptr).modelMatrix = Rotate(Vec3f(0.0f, 0.0f, 1.0f), angle, (*ptr).modelMatrix);
  m_ConstBuffer->UnMap();

  auto currentImage = m_pSwapChain->GetCurrentTexture();
  auto ImageDesc = currentImage->GetDesc();
  NGFXColorAttachmentDesc ColorAttach;
  ColorAttach.pTexture = currentImage;
  ColorAttach.LoadAction = NGFX_LOAD_ACTION_CLEAR;
  ColorAttach.StoreAction = NGFX_STORE_ACTION_STORE;
  ColorAttach.ClearColor = Vec4f(1, 1, 1, 1);

  NGFXRenderPassDesc Desc;
  Desc.ColorAttachments.Append(ColorAttach);

  auto commandBuffer = NGFXPtr<NGFXCommandBuffer>(m_pQueue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT));
  // command encoder like Metal does, should use desc instead, look obj from cache
  auto renderCmd = NGFXPtr<NGFXRenderCommandEncoder>(commandBuffer->RenderCommandEncoder(Desc));
  renderCmd->SetBindingGroup(m_BindingGroup);
  NGFXRect rect{0, 0, ImageDesc.TextureDesc.Width, ImageDesc.TextureDesc.Height};
  renderCmd->SetScissorRect(rect);
  renderCmd->SetViewport(NGFXViewportDesc(ImageDesc.TextureDesc.Width, ImageDesc.TextureDesc.Height));
  renderCmd->SetPipelineState(0, m_pPso);
  renderCmd->SetVertexBuffer(0, m_CubeMesh->VBO());
  renderCmd->DrawInstanced(NGFXDrawInstancedParam(36, 1));
  renderCmd->EndEncode();

  commandBuffer->Present(m_pSwapChain.Get(), m_pFence.Get());
  commandBuffer->Commit(m_pFence.Get());
}
