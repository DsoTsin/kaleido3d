#pragma once
#include <Core/k3dWindow.h>
#include <Math/kMath.hpp>
#include <Engine/k3dSceneObject.h>

#include <queue>

typedef std::queue<kSObject::SObjPtr>  ObjQueue;

class k3dRenderManager;
class k3dRenderTexture;
class k3dTexture;
class k3dRenderState;
class k3dMaterial;
class k3dRenderNode;

///
/// \brief The k3dRender class's responsiblity is to render one frame in the game loop.
/// It obtains render queue from scene manager and perform deferred lighting and shading.
/// \sa{k3dSceneManager}
///
class k3dRender {
  typedef std::shared_ptr<k3dTexture>       TexPtr;
  typedef std::shared_ptr<k3dRenderTexture> RTPtr;
  typedef std::vector<RTPtr>                RTPtrArray;
public:
  k3dRender();
  virtual           ~k3dRender();

  ///
  /// \brief Init with a native windeow handle
  /// \param window
  ///
  virtual void      Init( SharedWindowPtr window = nullptr ) = 0;
  virtual void      Shutdown();

  ///
  /// \brief this is the first step of rendering, which performs visibility detection (frustum culling),
  /// and put the visible object surface into render queue, then sorted by material property (opaque or translucent)
  ///
  virtual void      PrepareFrame();

  ///
  /// \brief during this step, the render begins post processing in screen space.
  ///
  virtual void      FinishFrame();

  virtual void      Render();

  ///
  /// \brief RenderNode
  /// \param node
  /// \param pass
  /// \note   pass determined to call material or light callback
  virtual void      RenderNode( k3dRenderNode *node, int pass);

  ///
  /// \brief render material passes, this is forward render pass
  /// \param proj
  /// \param modelView
  /// \param node
  /// \param material
  /// \note  this step is rendered in view space
  /// \sa    {Render}
  virtual void      RenderNode( const kMath::Mat4f & proj, const kMath::Mat4f &modelView, k3dRenderNode *node, const k3dMaterial & material );

  ///
  /// \brief render light pass, this is deferred lighting pass
  /// \param proj
  /// \param modelView
  /// \param node
  /// \param shadows
  /// \note  this step is rendered in light space
  virtual void      RenderNode( const kMath::Mat4f &proj, const kMath::Mat4f &modelView, k3dRenderNode *node,int shadows );
  //  virtual void RenderPost

  k3dRenderManager* GetRenderManager();
  RTPtr             GetRenderTexture(int i) const;

protected:

  int createDeferredRender();

  struct DeferredRenderer {
    TexPtr	depthTexture;
    TexPtr	shadowTexture;
    TexPtr	positionTexture;
    TexPtr	normalTexture;
    TexPtr	albedoTexture;
    RTPtr		renderTexture;

    DeferredRenderer()
      : depthTexture(nullptr)
      , shadowTexture(nullptr)
      , positionTexture(nullptr)
      , normalTexture(nullptr)
      , albedoTexture(nullptr)
    {}

    // setup gbuffers
    int CreateDeferredRenderer( k3dRender * render );
  };

  DeferredRenderer  m_DeferredRenderer;
  

  k3dRenderManager* m_RenderManager;
  k3dRenderState*   m_RenderState;
  
  RTPtrArray        m_RenderTextures;

  kMath::Mat4f      m_CurrentModelMatrix;
  kMath::Mat4f      m_CurrentViewMatrix;
  kMath::Mat4f      m_CurrentProjectionMatrix;

private:

  ObjQueue          m_OpaqueObjectQueue;
  ObjQueue          m_TranslucentQueue;

};

typedef std::shared_ptr<k3dRender> SharedRenderPtr;
