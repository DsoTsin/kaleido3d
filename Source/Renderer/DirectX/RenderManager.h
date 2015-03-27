#pragma once 
#include <KTL/Singleton.hpp>
#include "Infrastructure/IndexBuffer.h"
#include "Infrastructure/VertexBuffer.h"

namespace k3d
{
  using namespace d3d12;


  class RenderManager : public Singleton<RenderManager>
  {
  public:

    void Submit (VertexBuffer const & vb);
    void Submit (IndexBuffer const & ib);

  protected:
    RenderManager ();

  };
}