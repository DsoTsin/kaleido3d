#pragma once
#include "kGLTexture.h"

enum {
  RT_TEX_COLOR0 = 0,
  RT_TEX_COLOR1,
  RT_TEX_COLOR2,
  RT_TEX_COLOR3,
  RT_TEX_COLOR4,
  RT_TEX_COLOR5,
  NUM_RT_TEX
};

class kGLFrameBuffer {
public:
  kGLFrameBuffer(int width, int height, uint32 flags);
  ~kGLFrameBuffer();

  void Release();

  void SetDepthTexture( kGLTexture *texture );
  void SetColorTexture( kGLTexture *texture, uint32 colAttId );

  bool IsCompleted();
  void CheckFrameBuffer();
  void Enable();
  void Disable();
	void Flush();

	bool IsEnabled() const;

  int GetWidth() const;
  int GetHeight() const;

  int GetFlags() const;

private:

  int               m_Width;
  int               m_Height;
  int               m_Flags;
  uint32            m_Layers; // For 2D Array or CubeMap
  uint32            m_Enabled:  1;
  int               m_MemUsage;
  int               m_Samples;
  int               m_Viewport[4];

  int               m_NumTextures;
  kGLTexture*       m_ColorTextures[NUM_RT_TEX];
  kGLTexture*       m_DepthTexture;

  uint32      m_ColorBufferId;
  uint32      m_DepthBufferId;
  uint32      m_StencilBufferId;

  uint32      m_FrameBufferId;
  int32       m_FrameBufferPreviousId;
	uint32			m_BlitDrawBuffer;

};
