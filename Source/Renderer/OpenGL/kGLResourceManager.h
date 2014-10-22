#pragma once

class kGLResouceManager {
public:
  kGLResouceManager();
  ~kGLResouceManager();

  typedef std::shared_ptr<class k3dShader>      ShaderPtr;
  typedef std::shared_ptr<class k3dTexture>     TexturePtr;
  typedef std::shared_ptr<class k3dGPUBuffer>   BufferPtr;

  void AddShader    ( k3dString & shaderName,   ShaderPtr & shader );
  void AddTexture   ( k3dString & texName,      TexturePtr & texture );
  void AddGPUBuffer ( k3dString & bufferName,   BufferPtr & buffer );

  void ReleaseAllResource();
  void ReleaseAllShader();
  void ReleaseAllTexture();
  void ReleaseAllBuffer();

  typedef std::map<k3dString, ShaderPtr>    Shaders;
  typedef std::map<k3dString, TexturePtr>   Textures;
  typedef std::map<k3dString, BufferPtr>    Buffers;

protected:

  Shaders   m_ShadersContainer;
  Textures  m_TexturesContainer;
  Buffers   m_GPUBuffers;
};
