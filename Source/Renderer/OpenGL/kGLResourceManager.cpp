#include "kGLResourceManager.h"
#include <Renderer/k3dShader.h>
#include <Renderer/k3dTexture.h>
#include <Renderer/k3dGPUBuffer.h>

kGLResouceManager::kGLResouceManager()
{

}

kGLResouceManager::~kGLResouceManager()
{

}

void kGLResouceManager::AddShader(k3dString &shaderName, kGLResouceManager::ShaderPtr &shader)
{
  m_ShadersContainer[shaderName] = shader;
}

void kGLResouceManager::AddTexture(k3dString &texName, kGLResouceManager::TexturePtr &texture)
{
  m_TexturesContainer[texName] = texture;
}

void kGLResouceManager::AddGPUBuffer(k3dString &bufferName, kGLResouceManager::BufferPtr &buffer)
{
  m_GPUBuffers[bufferName] = buffer;
}

void kGLResouceManager::ReleaseAllResource()
{
  ReleaseAllShader();
  ReleaseAllTexture();
  ReleaseAllBuffer();
}

void kGLResouceManager::ReleaseAllShader()
{
  for( auto & iShader : m_ShadersContainer ) {
    iShader.second->Release();
  }
}

void kGLResouceManager::ReleaseAllTexture()
{
  for( auto & iTexture : m_TexturesContainer ) {
    iTexture.second->Release();
  }
}

void kGLResouceManager::ReleaseAllBuffer()
{
  for( auto & iBuffer : m_GPUBuffers ) {
    iBuffer.second->Release();
  }
}
