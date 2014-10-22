#include "kGLRenderManager.h"
#include <GL/glew.h>
#include <Core/k3dDbg.h>
#include <Core/k3dLog.h>
#include <Engine/k3dEngine.h>
#include "kGLContext.h"
#include "kGLWrapper.h"
#include "kGLShader.h"
#include "kGLTexture.h"
#include "kGLRenderTexture.h"


kGLRenderManager::kGLRenderManager()
  : k3dRenderManager()
{
}

kGLRenderManager::~kGLRenderManager()
{
}

void kGLRenderManager::Shutdown()
{
  if(m_Context) {
    m_Context->DestroyContext();
    delete m_Context;
    m_Context = 0;
  }
}

void kGLRenderManager::Init()
{
  GLenum result = glewInit();
  if(result!=GLEW_OK)
  {
    kDebug("kGLRenderManager : GLEW failed to init.");
    k3dLog::Error("kGLRenderManager::Init failed. GLEW Init failed.");
    ::exit(-1) ;
//    return;
  }

  k3dString version((const char*)glGetString(GL_VERSION));
  k3dString vendor((const char*)glGetString(GL_VENDOR));
  k3dString renderer((const char*)glGetString(GL_RENDERER));
  k3dLog::Message("kaleido3d Renderer Init Succeed !.");
  k3dLog::Message("kGLRender OpenGL Version: %s.", version.c_str() );
  k3dLog::Message("kGLRender Vendor : %s.", vendor.c_str() );
  k3dLog::Message("kGLRender Renderer: %s.", renderer.c_str() );

  glViewport(0,0, engine.WindowWidth, engine.WindowHeight);

  K3D_CHECK_OGL_ERROR;
}

void kGLRenderManager::CreateContext(SharedWindowPtr &winptr)
{
  if( nullptr==m_Context )
  {
    m_Context = new kGLContext(winptr);
    m_Context->InitContext();
    return ;
  }
}

std::shared_ptr<k3dShader> kGLRenderManager::FindShader(const k3dString &shaderName)
{
  if(m_ShadersContainer.find(shaderName) == m_ShadersContainer.end())
    return nullptr;
  return m_ShadersContainer[shaderName];
}

std::shared_ptr<k3dTexture> kGLRenderManager::FindTexture(const k3dString &texName)
{
  if(m_TexturesContainer.find(texName) == m_TexturesContainer.end())
    return nullptr;
  return m_TexturesContainer[texName];
}

k3dRenderTexture *kGLRenderManager::MakeRenderTexture()
{
  return new kGLRenderTexture;
}

std::shared_ptr<k3dShader> kGLRenderManager::MakeShader()
{
  return std::shared_ptr<k3dShader>(new kGLShader);
}

std::shared_ptr<k3dTexture> kGLRenderManager::MakeTexture()
{
  return std::make_shared<kGLTexture>();
}
