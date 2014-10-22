#ifndef __kOpenGLWrapper_h__
#define __kOpenGLWrapper_h__
#pragma once

#include <Math/kMath.hpp>

#define K3D_CHECK_OGL_ERROR \
  GLenum Error = glGetError(); \
  assert( Error == GL_NO_ERROR && "OpenGL Error at Line " && __LINE__ )


namespace kGL {

  const int TexRGB8 = 0x8051;
  const int TexRGBA8 = 0x8058;
  const int TexRGB10A2 = 0x8059;
  ///
  /// \brief CreateTexture2DArray
  /// \param width
  /// \param height
  /// \param layerCount
  /// \param fill_fmt
  /// \return
  ///
  int CreateTexture2DArray(int width, int height, int layerCount, int fillFmt);

  ///
  /// \brief BindlessTexture
  /// \param texIds
  /// \param texCount
  /// \return
  ///
  uint64 BindlessTexture(int *texIds, int texCount);

  ///
  /// \brief CreateBufferStorage
  /// \param bufferType
  /// \param bufferSize
  /// \return
  ///
  int CreateBufferStorage(int bufferType, size_t bufferSize);

  ///
  /// \brief CreateVertexArrayWithIndexBuffer
  /// \param vboSize
  /// \param vboPtr
  /// \param indexSize
  /// \param indexPtr
  /// \param vertLoc
  int CreateVertexArrayWithIndexBuffer(int vboSize, float * vboPtr, int indexSize, int *indexPtr, int vertLoc);

  ///
  /// \brief TransUniform
  bool TransUniform(uint32 program, const char *uniformName, float val);
  bool TransUniform(uint32 program, const char *uniformName, kMath::Mat4f & mat);
  bool TransUniform(uint32 program, const char *uniformName, kMath::Vec4f & val);


  ///
  /// \brief check the frame buffer status
  /// \param framebufferId
  /// \return
  ///
  bool CheckFrameBuffer(uint32 framebufferId);

  ///
  /// \brief GetUniformLocation
  /// \param uniformName
  /// \param shaderHandle
  /// \return
  ///
  uint32 GetUniformLocation(const char*uniformName, uint32 shaderHandle);
}

#endif
