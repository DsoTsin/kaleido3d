#include <Base/UTRHIAppBase.h>
#include <Core/App.h>
#include <Core/AssetManager.h>
#include <Core/LogUtil.h>
#include <Core/Message.h>
#include <Core/Kaleido3D.h>
#include <Core/Math/kMath.hpp>
#include <vector>

using namespace k3d;
using namespace kMath;

enum
{
  PARTICLE_GROUP_SIZE = 1024,
  PARTICLE_GROUP_COUNT = 8192,
  PARTICLE_COUNT = (PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT),
  MAX_ATTRACTORS = 64
};

static inline float random_float()
{
  float res;
  unsigned int tmp;
  static unsigned int seed = 0xFFFF0C59;
  seed *= 16807;
  tmp = seed ^ (seed >> 4) ^ (seed << 15);
  *((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;
  return (res - 1.0f);
}

static Vec3f random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
  Vec3f randomvec(random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f);
  randomvec = Normalize(randomvec);
  randomvec = randomvec * (random_float() * (maxmag - minmag) + minmag);
  return randomvec;
}

#pragma region AppDefine

class UTComputeParticles : public RHIAppBase
{
public:
  UTComputeParticles(String const& appName, uint32 width, uint32 height)
    : RHIAppBase(appName, width, height)
  {
  }

  explicit UTComputeParticles(String const& appName)
    : RHIAppBase(appName, 1920, 1080)
  {
  }

  ~UTComputeParticles() override
  {
  }

  bool OnInit() override;
  void OnDestroy() override;
  void OnProcess(Message& msg) override;
  void OnUpdate() override;

private:
  float m_AttractorMasses[MAX_ATTRACTORS];

private:
  NGFXPtr<NGFXPipeline> m_pGfxPso;
  NGFXPtr<NGFXPipelineLayout> m_pGfxPl;
  NGFXPtr<NGFXBindingGroup> m_GfxBindingGroup;

  NGFXPtr<NGFXPipeline> m_pCompPso;
  NGFXPtr<NGFXPipelineLayout> m_pCompPl;
  NGFXPtr<NGFXBindingGroup> m_CptBindingGroup;

  NGFXBufferRef m_PositionBuffer;
  NGFXUAVRef m_PosUAV;
  NGFXBufferRef m_VelocityBuffer;
  NGFXUAVRef m_VelUAV;
  NGFXVertexBufferView m_PosVBV;

  NGFXBufferRef m_AttractorMassBuffer;
  NGFXBufferRef m_DtBuffer;
  NGFXResourceRef m_MVPBuffer;

  uint64 CurrentTick;
private:
  NGFXVertexInputState m_IAState;
  NGFXRenderPassDesc m_RpDesc;
};

#pragma endregion

K3D_APP_MAIN(UTComputeParticles);

bool
UTComputeParticles::OnInit()
{
  bool inited = RHIAppBase::OnInit();
  if (!inited)
    return inited;
  CurrentTick = Os::GetTicks();

  // create buffers
  auto PositionBuffer = AllocateVertexBuffer<Vec4f>([](Vec4f* Pointer)
  {
    std::vector<SharedPtr<std::thread>> threadPool;
    // divide
    int ThreadCount = 8;
    int PerGroup = PARTICLE_COUNT / 8;
    for (auto i = 0; i < ThreadCount; i++)
    {
      auto t = MakeShared<std::thread>([=]()
      {
        for (auto g = 0; g < PerGroup; g++)
        {
          float w = random_float();
          Vec3f v = random_vector(-10.0f, 10.0f);
          Pointer[g + i * PerGroup][0] = v.x;
          Pointer[g + i * PerGroup][1] = v.y;
          Pointer[g + i * PerGroup][2] = v.z;
          Pointer[g + i * PerGroup][3] = w;
        }
      });
      threadPool.push_back(t);
    }
    for (auto t : threadPool)
    {
      t->join();
    }
    KLOG(Info, App, "Particle generate finished, %s", Os::Thread::GetCurrentThreadName().CStr());
  }, PARTICLE_COUNT);
  m_PositionBuffer = static_cast<NGFXBuffer*>(PositionBuffer.second);
  m_PositionBuffer->SetName("Position");

  auto VelocityBuffer = AllocateVertexBuffer<Vec4f>([](Vec4f* Pointer)
  {
    std::vector<SharedPtr<std::thread>> threadPool;
    // divide
    int ThreadCount = 8;
    int PerGroup = PARTICLE_COUNT / 8;
    for (auto i = 0; i < ThreadCount; i++)
    {
      auto t = MakeShared<std::thread>([=]()
      {
        for (auto g = 0; g < PerGroup; g++)
        {
          Pointer[g + i * PerGroup] = Vec4f(random_vector(-0.1f, 0.1f), 0.0f);
        }
      });
      threadPool.push_back(t);
    }
    for (auto t : threadPool)
    {
      t->join();
    }
    KLOG(Info, App, "velocity generate finished, %s", Os::Thread::GetCurrentThreadName().CStr());
  }, PARTICLE_COUNT);
  m_VelocityBuffer = static_cast<NGFXBuffer*>(VelocityBuffer.second);
  m_VelocityBuffer->SetName("Velocity");

  // create uavs
  NGFXUAVDesc uavDesc = {NGFX_VIEW_DIMENSION_BUFFER, NGFX_PIXEL_FORMAT_RGBA32_FLOAT};
  uavDesc.Buffer = {0, PARTICLE_COUNT, sizeof(Vec4f)};
  m_PosUAV = m_pDevice->CreateUnorderedAccessView(m_PositionBuffer, uavDesc);
  m_VelUAV = m_pDevice->CreateUnorderedAccessView(m_VelocityBuffer, uavDesc);

  // create constants
  auto AttractorMassBuffer = AllocateConstBuffer<Vec4f>([this](Vec4f* Ptr)
  {
    for (uint32 i = 0; i < MAX_ATTRACTORS; i++)
    {
      m_AttractorMasses[i] = 0.5f + random_float() * 0.5f;
    }
  }, MAX_ATTRACTORS);
  m_AttractorMassBuffer = static_cast<NGFXBuffer*>(AttractorMassBuffer);
  m_AttractorMassBuffer->SetName("Attractor");

  auto dt = AllocateConstBuffer<float>(1.0f);
  m_DtBuffer = static_cast<NGFXBuffer*>(dt);
  m_DtBuffer->SetName("Dt");

  // create input state
  m_IAState.Attribs[0] = {NGFX_VERTEX_FORMAT_FLOAT4X32, 0, 0};
  m_IAState.Layouts[0] = {NGFX_VERTEX_INPUT_RATE_PER_VERTEX, sizeof(float) * 4};
  m_PosVBV = {m_PositionBuffer->GetLocation(), 0, 0};

  // create MVP
  NGFXResourceDesc desc;
  desc.Flag = NGFX_ACCESS_HOST_VISIBLE;
  desc.ViewFlags = NGFX_RESOURCE_CONSTANT_BUFFER_VIEW;
  desc.Size = sizeof(MVPMatrix);
  m_MVPBuffer = m_pDevice->CreateResource(desc);
  m_MVPBuffer->SetName("MVPMatrix");

  // compile shaders
  NGFXShaderBundle vertSh, fragSh, compSh;
  Compile("asset://Test/particles.vert", NGFX_SHADER_TYPE_VERTEX, vertSh);
  Compile("asset://Test/particles.frag", NGFX_SHADER_TYPE_FRAGMENT, fragSh);

  Compile("asset://Test/particles.comp", NGFX_SHADER_TYPE_COMPUTE, compSh);

  auto vertBinding = vertSh.BindingTable;
  auto fragBinding = fragSh.BindingTable;
  auto mergedBindings = vertBinding | fragBinding;
  m_pGfxPl = NGFXPtr<NGFXPipelineLayout>(m_pDevice->CreatePipelineLayout(mergedBindings));
  m_GfxBindingGroup = NGFXPtr<NGFXBindingGroup>(m_pGfxPl->ObtainBindingGroup());
  m_GfxBindingGroup->Update(0, m_MVPBuffer);

  NGFXColorAttachmentDesc ColorAttach;
  ColorAttach.pTexture = m_pSwapChain->GetCurrentTexture();
  ColorAttach.LoadAction = NGFX_LOAD_ACTION_CLEAR;
  ColorAttach.StoreAction = NGFX_STORE_ACTION_STORE;
  ColorAttach.ClearColor = Vec4f(0, 0, 0, 1);

  m_RpDesc.ColorAttachments.Append(ColorAttach);
  auto pRenderPass = m_pDevice->CreateRenderPass(m_RpDesc);

  NGFXRenderPipelineDesc renderPipelineDesc;
  renderPipelineDesc.AttachmentsBlend.Append(NGFXAttachmentState());
  renderPipelineDesc.AttachmentsBlend[0].Blend.Enable = true;
  renderPipelineDesc.AttachmentsBlend[0].Blend.Src = NGFX_BLEND_FACTOR_ONE;
  renderPipelineDesc.AttachmentsBlend[0].Blend.Dest = NGFX_BLEND_FACTOR_ONE;
  renderPipelineDesc.AttachmentsBlend[0].Blend.DestBlendAlpha = NGFX_BLEND_FACTOR_ONE;
  renderPipelineDesc.PrimitiveTopology = NGFX_PRIMITIVE_POINTS;
  renderPipelineDesc.InputState = m_IAState;
  renderPipelineDesc.VertexShader = vertSh;
  renderPipelineDesc.PixelShader = fragSh;
  m_pGfxPso = NGFXPtr<NGFXPipeline>(m_pDevice->CreateRenderPipelineState(renderPipelineDesc, m_pGfxPl.Get(), pRenderPass));

  m_pCompPl = NGFXPtr<NGFXPipelineLayout>(m_pDevice->CreatePipelineLayout(compSh.BindingTable));
  m_CptBindingGroup = NGFXPtr<NGFXBindingGroup>(m_pCompPl->ObtainBindingGroup());
  m_CptBindingGroup->Update(0, (m_AttractorMassBuffer));
  m_CptBindingGroup->Update(1, (m_DtBuffer));
  m_CptBindingGroup->Update(2, m_VelUAV);
  m_CptBindingGroup->Update(3, m_PosUAV);
  NGFXComputePipelineDesc computePipelineDesc;
  computePipelineDesc.ComputeShader = compSh;
  m_pCompPso = NGFXPtr<NGFXPipeline>(m_pDevice->CreateComputePipelineState(computePipelineDesc, m_pCompPl.Get()));

  KLOG(Info, App, "pipeline creation finished, %s", Os::Thread::GetCurrentThreadName().CStr());
  // wait vertex buffer upload
  VelocityBuffer.first->Join();
  PositionBuffer.first->Join();

  return true;
}

void
UTComputeParticles::OnUpdate()
{
  auto currentImage = m_pSwapChain->GetCurrentTexture();
  auto ImageDesc = currentImage->GetDesc();
  m_RpDesc.ColorAttachments[0].pTexture = currentImage;

  auto renderCmdBuffer = NGFXPtr<NGFXCommandBuffer>(m_pQueue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT));
  auto renderCmd = NGFXPtr<NGFXRenderCommandEncoder>(renderCmdBuffer->RenderCommandEncoder(m_RpDesc));
  renderCmd->SetBindingGroup(m_GfxBindingGroup.Get());
  NGFXRect rect{0, 0, ImageDesc.TextureDesc.Width, ImageDesc.TextureDesc.Height};
  renderCmd->SetScissorRect(rect);
  renderCmd->SetViewport(NGFXViewportDesc(ImageDesc.TextureDesc.Width, ImageDesc.TextureDesc.Height));
  renderCmd->SetPipelineState(0, m_pGfxPso.Get());
  renderCmd->SetVertexBuffer(0, m_PosVBV);
  renderCmd->DrawInstanced(NGFXDrawInstancedParam(PARTICLE_COUNT, 1));
  renderCmd->EndEncode();
  renderCmdBuffer->Present(m_pSwapChain.Get(), nullptr);
  renderCmdBuffer->Commit(m_pFence.Get());

  static auto start_ticks = Os::GetTicks() - 100000;
  auto current_ticks = Os::GetTicks() - CurrentTick;
  static auto last_ticks = current_ticks;
  float time = ((start_ticks - current_ticks) & 0xFFFFF) / float(0xFFFFF);
  float delta_time = (float)(current_ticks - last_ticks) * 0.075f;

  if (delta_time < 0.01f)
  {
    return;
  }

  MVPMatrix* pMatrix = (MVPMatrix*)m_MVPBuffer->Map(0, sizeof(MVPMatrix));
  pMatrix->projectionMatrix =
    Perspective(45.0f, 1920.f / 1080.f, 0.1f, 1000.0f);
  pMatrix->viewMatrix =
    Translate(Vec3f(0.0f, 0.0f, -160.f), MakeIdentityMatrix<float>());
  pMatrix->modelMatrix = Rotate(Vec3f(0.0f, 1.0f, 0.0f), delta_time, MakeIdentityMatrix<float>());
  m_MVPBuffer->UnMap();

  Vec4f* attractors = (Vec4f*)m_AttractorMassBuffer->Map(0, sizeof(Vec4f) * 32);
  for (uint32 i = 0; i < 32; i++)
  {
    attractors[i] = Vec4f(sinf(time * (float)(i + 4) * 7.5f * 20.0f) * 50.0f,
                          cosf(time * (float)(i + 7) * 3.9f * 20.0f) * 50.0f,
                          sinf(time * (float)(i + 3) * 5.3f * 20.0f) * cosf(time * (float)(i + 5) * 9.1f) * 100.0f,
                          m_AttractorMasses[i]);
  }
  m_AttractorMassBuffer->UnMap();

  if (delta_time >= 2.0f)
  {
    delta_time = 2.0f;
  }

  float* pDt = (float*)m_DtBuffer->Map(0, sizeof(float));
  *pDt = delta_time;
  m_DtBuffer->UnMap();

  auto computeCmdBuffer = NGFXPtr<NGFXCommandBuffer>(m_pQueue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT));
  auto computeCmd = NGFXPtr<NGFXComputeCommandEncoder>(computeCmdBuffer->ComputeCommandEncoder());
  computeCmdBuffer->Transition(m_PositionBuffer, NGFX_RESOURCE_STATE_UNORDERED_ACCESS);
  computeCmd->SetPipelineState(0, m_pCompPso.Get());
  computeCmd->SetBindingGroup(m_CptBindingGroup.Get());
  computeCmd->Dispatch(PARTICLE_GROUP_COUNT, 1, 1);
  computeCmdBuffer->Transition(m_PositionBuffer, NGFX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  computeCmd->EndEncode();
  computeCmdBuffer->Commit(m_pFence.Get());

  last_ticks = current_ticks;

  //KLOG(Info, GPUPerf, "usage %.3f.", Os::GetGpuUsage(0));
}

void
UTComputeParticles::OnProcess(Message& msg)
{
  if (msg.type == Message::Resized)
  {
    KLOG(Info, App, "Resized.. %d, %d.", msg.size.width, msg.size.height);
    m_pSwapChain->Resize(msg.size.width, msg.size.height);
  }
}

void
UTComputeParticles::OnDestroy()
{
}
