#include "Kaleido3D.h"
#include "RenderMesh.h"
#include "Helper.h"

namespace k3d
{
  using namespace std;
  namespace d3d12
  {
    RenderMesh::RenderMesh (PtrDevice device)
      : mDevice (device)
    {
      m_MeshInitialized = false;
    }

    RenderMesh::~RenderMesh ()
    {
    }

    void RenderMesh::Render ()
    {
      mGfxCmdList->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      mGfxCmdList->IASetVertexBuffers (0, 1, &mDescViewBufVert); // Vertex Buffer
      mGfxCmdList->DrawInstanced (3, 1, 0, 0);
    }
    void RenderMesh::SetMesh (Mesh const & mesh)
    {
      Helper::ConvertVertexFormatToInputElementDesc (mesh.GetVertexFormat (), mInputEleDesc);
      mMeshLayout.NumElements = (UINT)mInputEleDesc.size ();
      mMeshLayout.pInputElementDescs = &mInputEleDesc[0];

      UINT8* dataBegin;
      mBufVerts->Map (0, nullptr, reinterpret_cast<void**>(&dataBegin));
      memcpy (dataBegin, mesh.GetVertexBuffer (), Mesh::GetVertexByteWidth (mesh.GetVertexFormat (), mesh.GetVertexNum ()));
      mBufVerts->Unmap (0, nullptr);
      mDescViewBufVert.BufferLocation = mBufVerts->GetGPUVirtualAddress ();
      mDescViewBufVert.StrideInBytes = Mesh::GetVertexStride (mesh.GetVertexFormat ());
      mDescViewBufVert.SizeInBytes = Mesh::GetVertexByteWidth (mesh.GetVertexFormat (), mesh.GetVertexNum ());
    }

    void RenderMesh::CreatePSO (PtrDevice Device, PtrBlob VS, PtrBlob PS)
    {
      assert (Device!=nullptr);
      PtrPipeLineState mPSO;
      Ref<ID3D12RootSignature> mRootSignature;
      Ref<ID3DBlob> pOutBlob, pErrorBlob;
    }
  }
}