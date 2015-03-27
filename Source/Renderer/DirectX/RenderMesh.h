#pragma once
#include <Interface/IRenderMesh.h>
#include <KTL/NonCopyable.hpp>
#include <Core/Mesh.h>

#include "DXCommon.h"
#include "Renderer.h"

namespace k3d {
  namespace d3d12
  {
	  class RenderMesh : public IRenderMesh /*, public NonCopyable*/ {
	  public:

		  explicit RenderMesh(PtrDevice device);
		  ~RenderMesh() override;

		  void Render() override;

      void SetMesh(Mesh const & mesh);
		
      void CreatePSO(PtrDevice device, PtrBlob VS, PtrBlob PS);

	  private:
			
		  bool				m_MeshInitialized;
      D3D12_INPUT_LAYOUT_DESC mMeshLayout;
      D3D12_VERTEX_BUFFER_VIEW mDescViewBufVert;
      std::vector<D3D12_INPUT_ELEMENT_DESC> mInputEleDesc;
      PtrResource mBufVerts;

    protected:
      PtrDevice mDevice;
      PtrGfxCmdList mGfxCmdList;
	  };
  }
}