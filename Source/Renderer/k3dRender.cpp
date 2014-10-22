#include "k3dRender.h"
#include "k3dRenderManager.h"
#include "k3dRenderState.h"
#include "k3dRenderTexture.h"
#include "k3dTexture.h"

#include <Engine/k3dEngine.h>
#include <Entity/k3dVehicle.h>


k3dRender::k3dRender()
{
  m_RenderManager   = nullptr;
  m_RenderState     = nullptr;
  //  m_RenderTexture   = nullptr;

}

k3dRender::~k3dRender() {}

void k3dRender::Shutdown()
{
  if(m_RenderManager) {
    delete m_RenderManager;
    m_RenderManager = nullptr;
  }
}

void k3dRender::PrepareFrame()
{

}

void k3dRender::FinishFrame()
{

}

void k3dRender::Render()
{
  const float color[] = {1.0f, 1.0f, 0.7f, 1.0f};
  m_RenderState->ClearBuffer( RState::BUFFER_COLOR | RState::BUFFER_DEPTH, color, 20.f, 1);
}

void k3dRender::RenderNode(k3dRenderNode *node, int pass)
{

}

void k3dRender::RenderNode(const kMath::Mat4f &proj, const kMath::Mat4f &modelView, k3dRenderNode *node, const k3dMaterial &material )
{
  assert(node!=nullptr && "render node cannot be null!");
  int type = node->GetType();

  // pass material params to unified shader
  if( type<k3dRenderNode::Node_RenderableMax ) {
    const k3dMaterialBatch & materialBatch = *(((kSObject*)node)->GetMaterialBatch());
    for(int i = 0; i<materialBatch.GetBatchCount(); i++) {
      const k3dMaterial & material = *(materialBatch.GetMaterialPtr(i));
//      RenderMaterial(proj, modelView, material, 0);
//      node->Draw();
    }
  }

  if(type == k3dRenderNode::Node_Vehicle) {
    kVehicle * vehicle = static_cast<kVehicle*>(node);
  }
}

void k3dRender::RenderNode(const kMath::Mat4f &proj, const kMath::Mat4f &modelView, k3dRenderNode *node, int shadows)
{
  assert(node!=nullptr && "render node cannot be null!");
  int type = node->GetType();
}

k3dRenderManager *k3dRender::GetRenderManager()
{
  return m_RenderManager;
}

k3dRender::RTPtr k3dRender::GetRenderTexture(int i) const
{
  assert(i<m_RenderTextures.size());
  return m_RenderTextures[i];
}

int k3dRender::createDeferredRender()
{
  return 0;
}


int k3dRender::DeferredRenderer::CreateDeferredRenderer(k3dRender *render)
{
  k3dRenderManager* renderManager = render->GetRenderManager();
  //  k3dRenderTexture* renderTex     = render->GetRenderTexture();
  //  depthTexture    = renderManager->MakeTexture();
  //  positionTexture = renderManager->MakeTexture();
  //  normalTexture   = renderManager->MakeTexture();
  //  shadowTexture   = renderManager->MakeTexture();
  //  albedoTexture   = renderManager->MakeTexture();
  //  renderTexture   = std::shared_ptr<k3dRenderTexture>(renderManager->MakeRenderTexture());

  //  depthTexture->Create2D(2048, 2048, Fmt_D24S8, Filter_Linear);
  //  positionTexture->Create2D(2048, 2048, Fmt_RGBA8, Filter_Linear);
  //  normalTexture->Create2D(2048, 2048, Fmt_RGBA8, Filter_Linear);
  //  albedoTexture->Create2D(2048, 2048, Fmt_RGBA8, Filter_Linear);
  //  shadowTexture->Create2D(2048, 2048, Fmt_RGBA8, Filter_Linear);

  //  renderTexture->Create2DRT(1024, 1024, RT_Filter_Linear|RT_Color_RGBA8|RT_Depth_24|RT_Tex_Depth|RT_Tex_Color);
  //  renderTexture->Enable();

  //	renderTexture->Flush();

  return 0;
}
