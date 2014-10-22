#ifndef __kGLRenderState_h__
#define __kGLRenderState_h__
#pragma once

#include <Renderer/k3dRenderState.h>


class kGLRenderState : public k3dRenderState {
public:

  kGLRenderState();
  ~kGLRenderState();

  void ClearBuffer(uint32 buffer, const float* color, float depth, uint32 stencil) override;
  void EnableDepthTest(bool enable = true);
  void DepthFunc(uint32 depthFunc);
  void EnableCullFace(bool enable = false);
  void CullFace(bool frontFace = false);
  void PolygonFill(bool line = false);
  void PolygonFrontFace(bool ccw = true);
  void BlendFunc(uint32 src, uint32 dst);

  void ClearState();

  void FlushRender();

private:

  // uint32 can denote 2^32 states
  uint32 m_StateFlag;
};

#endif
