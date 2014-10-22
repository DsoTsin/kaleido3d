#include "k3dRenderManager.h"
#include <Engine/k3dEngineContext.h>
#include "k3dShader.h"
#include "k3dTexture.h"

k3dRenderManager::k3dRenderManager()
  : m_Context(nullptr)
{
}

k3dRenderManager::~k3dRenderManager()
{
}

void k3dRenderManager::Init()
{

}

void k3dRenderManager::Shutdown()
{

}

int k3dRenderManager::GetVideoMemoryUseage()
{
  return 0;
}

void k3dRenderManager::CreateContext(SharedWindowPtr &winptr)
{
  K3D_UNUSED(winptr);
  if( nullptr == m_Context )
  {
//    m_Context = new k3dEngineContext(winptr);
//    m_Context->InitContext();
    return;
  }
}

void k3dRenderManager::MakeCurrent()
{
  m_Context->MakeCurrent();
}
