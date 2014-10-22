#pragma once


namespace RState {
// RState Include Depth, Polygon, Blend states

  const uint32 BLEND_MASK     = 0x00ff;
  const uint32 POLYGON_MASK   = 0x000f << 8;
  const uint32 DEPTH_MASK     = 0x000f << 12;

  const uint32 BLEND_SRC_MASK = 0x00f0;
  const uint32 BLEND_DST_MASK = 0x000f;

  const uint32 BLEND_NONE           = 0x0000;
  const uint32 BLEND_ZERO           = 0x0001;
  const uint32 BLEND_ONE            = 0x0002;
  const uint32 BLEND_SRC_COLOR      = 0x0003;
  const uint32 BLEND_ONE_MINUS_SRC_COLOR = 0x0004;
  const uint32 BLEND_SRC_ALPHA      = 0x0005;
  const uint32 BLEND_ONE_MINUS_SRC_ALPHA = 0x0006;
  const uint32 BLEND_DST_COLOR      = 0x0007;
  const uint32 BLEND_ONE_MINUS_DST_COLOR = 0x0008;
  const uint32 BLEND_DST_ALPHA      = 0x0009;
  const uint32 BLEND_ONE_MINUS_DST_ALPHA = 0x000A;

  const uint32 POLYGON_CULL_MASK      = (0x1 << 12);
  const uint32 POLYGON_CULL_FACE_MASK = (0x1 << 11);
  const uint32 POLYGON_FILL_MASK      = (0x1 << 10);
  const uint32 POLYGON_FRONT_FACE_MASK= (0x1 << 9);

  const uint32 DEPTH_TEST_MASK      = 0x8000;
  const uint32 DEPTH_FUNCTION_MASK  = 0x7000;

  const uint32 DEPTH_FUNC_NEVER     = 0x0000;
  const uint32 DEPTH_FUNC_LESS      = 0x1000;
  const uint32 DEPTH_FUNC_EQUAL     = 0x2000;
  const uint32 DEPTH_FUNC_LEQUAL    = 0x3000;
  const uint32 DEPTH_FUNC_GREATER   = 0x4000;
  const uint32 DEPTH_FUNC_NOTEQUAL  = 0x5000;
  const uint32 DEPTH_FUNC_GEQUAL    = 0x6000;
  const uint32 DEPTH_FUNC_ALWAYS    = 0x7000;

  enum {
      BUFFER_NONE = 0,
      BUFFER_RED = 1 << 0,
      BUFFER_GREEN = 1 << 1,
      BUFFER_BLUE = 1 << 2,
      BUFFER_ALPHA = 1 << 3,
      BUFFER_COLOR = (BUFFER_RED | BUFFER_GREEN | BUFFER_BLUE | BUFFER_ALPHA),
      BUFFER_DEPTH = 1 << 4,
      BUFFER_STENCIL = 1 << 5,
      BUFFER_ALL = (BUFFER_DEPTH | BUFFER_STENCIL | BUFFER_COLOR),
  };

}


//struct DeferredComponent {
//  class k3dTexture *colorTex;
//  class k3dTexture *normalTex;
//  class k3dTexture *positionTex;
//  class k3dTexture *depthTex;
//  class k3dTexture *shadowTex;
//  class k3dRenderTexture* renderTex;

//  DeferredComponent()
//    : colorTex(nullptr)
//    , normalTex(nullptr)
//    , positionTex(nullptr)
//    , depthTex(nullptr)
//    , shadowTex(nullptr)
//    , renderTex(nullptr) {}
//};

class k3dRenderState {
public:
  k3dRenderState() {}

  virtual ~k3dRenderState() {}

  virtual void ClearBuffer(uint32 buffer, const float* color, float depth, uint32 stencil) = 0;
  virtual void EnableDepthTest(bool enable = true) = 0;
  virtual void DepthFunc(uint32 depthFunc) = 0;
  virtual void EnableCullFace(bool enable = false) = 0;
  virtual void CullFace(bool frontFace = false) = 0;
  virtual void PolygonFill(bool line = false) = 0;
  virtual void PolygonFrontFace(bool ccw = true) = 0;
  virtual void BlendFunc(uint32 src, uint32 dst) = 0;

  virtual void ClearState() = 0;

  virtual void FlushRender() = 0;

protected:

//  DeferredComponent m_DeferredComponent;
};
