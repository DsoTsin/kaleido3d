#include "kGLRenderState.h"
#include <GL/glew.h>

kGLRenderState::kGLRenderState()
{

}

kGLRenderState::~kGLRenderState()
{

}

void kGLRenderState::ClearBuffer(uint32 buffer, const float *color, float depth, uint32 stencil)
{
  GLuint mask = 0;
  if(buffer & RState::BUFFER_COLOR) {
    if(color != NULL) glClearColor(color[0],color[1],color[2],color[3]);
    else glClearColor(0.0f,0.0f,0.0f,0.0f);
    mask |= GL_COLOR_BUFFER_BIT;
  }
  if(buffer & RState::BUFFER_DEPTH) {
    glClearDepth(depth);
    mask |= GL_DEPTH_BUFFER_BIT;
  }
  if(buffer & RState::BUFFER_STENCIL) {
    glClearStencil(stencil);
    mask |= GL_STENCIL_BUFFER_BIT;
  }
  if(mask != 0) glClear(mask);
}

void kGLRenderState::EnableDepthTest(bool enable)
{
  if( (m_StateFlag & RState::DEPTH_TEST_MASK)
      != RState::DEPTH_TEST_MASK )
  {
    if( enable ) {
      m_StateFlag |= RState::DEPTH_TEST_MASK;
      glEnable(GL_DEPTH_TEST);
    }
    else {
      m_StateFlag &= ~RState::DEPTH_TEST_MASK;
      glDisable(GL_DEPTH_TEST);
    }
  }
}

void kGLRenderState::DepthFunc(uint32 depthFunc)
{

}

void kGLRenderState::EnableCullFace(bool enable)
{
  if( (m_StateFlag&RState::POLYGON_CULL_MASK)!=RState::POLYGON_CULL_MASK ) {
    if(enable)
      glEnable(GL_CULL_FACE);
    else {
      glDisable(GL_CULL_FACE);
      glCullFace(GL_BACK);
    }
  }
}

void kGLRenderState::CullFace(bool frontFace)
{
  if( (m_StateFlag & RState::POLYGON_CULL_FACE_MASK) != RState::POLYGON_CULL_FACE_MASK ) {
    m_StateFlag |= RState::POLYGON_CULL_MASK;
    glEnable(GL_CULL_FACE);
    if(frontFace)
      glCullFace(GL_FRONT);
    else
      glCullFace(GL_BACK);
  }
}

void kGLRenderState::PolygonFill(bool line)
{

}

void kGLRenderState::PolygonFrontFace(bool ccw)
{
  if( (m_StateFlag & RState::POLYGON_FRONT_FACE_MASK)
      != RState::POLYGON_FRONT_FACE_MASK )
  {
    if(ccw) {
      m_StateFlag |= RState::POLYGON_FRONT_FACE_MASK;
      glFrontFace(GL_CCW);
    }
    else {
      m_StateFlag &= ~RState::POLYGON_FRONT_FACE_MASK;
      glFrontFace(GL_CW);
    }
  }
}

void kGLRenderState::BlendFunc(uint32 src, uint32 dst)
{

  static const GLuint blendFunctions[] = {
      0,
      GL_ZERO,
      GL_ONE,
      GL_SRC_COLOR,
      GL_ONE_MINUS_SRC_COLOR,
      GL_SRC_ALPHA,
      GL_ONE_MINUS_SRC_ALPHA,
      GL_DST_COLOR,
      GL_ONE_MINUS_DST_COLOR,
      GL_DST_ALPHA,
      GL_ONE_MINUS_DST_ALPHA,
  };

  if( (m_StateFlag & (src<<4 | dst)) != m_StateFlag )
  {
    m_StateFlag &= (src<<4 | dst);
    if(src == RState::BLEND_NONE || dst == RState::BLEND_NONE ) {
      glDisable(GL_BLEND);
      glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    }
    else {
      glEnable(GL_BLEND);
      glBlendFuncSeparate(blendFunctions[src], blendFunctions[dst], GL_ONE, blendFunctions[dst]);
    }
  }

}

void kGLRenderState::ClearState()
{
  BlendFunc(RState::BLEND_NONE, RState::BLEND_NONE);
  EnableDepthTest(false);
  EnableCullFace(false);
  PolygonFill(false);
  PolygonFrontFace(true);
  m_StateFlag = 0;
}

void kGLRenderState::FlushRender()
{
  glFinish();
}
