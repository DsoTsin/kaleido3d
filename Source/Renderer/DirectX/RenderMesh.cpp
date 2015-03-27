#include "Kaleido3D.h"
#include "RenderMesh.h"
#include "Helper.h"

namespace k3d {
	using namespace std;
  namespace d3d12
  {
	  RenderMesh::RenderMesh(PtrDevice device)
      : mDevice (device)
	  {
		  m_MeshInitialized = false;
	  }

	  RenderMesh::~RenderMesh()
	  {
	  }

	  void RenderMesh::Render()
	  {
      mGfxCmdList->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      mGfxCmdList->SetVertexBuffers (0, &mDescViewBufVert, 1); // Vertex Buffer
      mGfxCmdList->DrawInstanced (3, 1, 0, 0);
	  }
    void RenderMesh::SetMesh (Mesh const & mesh)
    {
      Helper::ConvertVertexFormatToInputElementDesc(mesh.GetVertexFormat(), mInputEleDesc);
      mMeshLayout.NumElements = (UINT)mInputEleDesc.size();
      mMeshLayout.pInputElementDescs = &mInputEleDesc[0]; 

      RDX_ERR (mDevice->CreateCommittedResource (
        &CD3D12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_MISC_NONE,
        &CD3D12_RESOURCE_DESC::Buffer (Mesh::GetVertexByteWidth (mesh.GetVertexFormat (), mesh.GetVertexNum ())),
        D3D12_RESOURCE_USAGE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS (mBufVerts.GetInitReference ())), return);

      UINT8* dataBegin;
      mBufVerts->Map (0, nullptr, reinterpret_cast<void**>(&dataBegin));
      memcpy (dataBegin, mesh.GetVertexBuffer(), Mesh::GetVertexByteWidth (mesh.GetVertexFormat(), mesh.GetVertexNum ()));
      mBufVerts->Unmap (0, nullptr);
      mDescViewBufVert.BufferLocation = mBufVerts->GetGPUVirtualAddress ();
      mDescViewBufVert.StrideInBytes = Mesh::GetVertexStride(mesh.GetVertexFormat());
      mDescViewBufVert.SizeInBytes = Mesh::GetVertexByteWidth (mesh.GetVertexFormat (), mesh.GetVertexNum ());
    }

    void RenderMesh::CreatePSO (PtrDevice Device, PtrBlob VS, PtrBlob PS)
    {
      assert(Device!=nullptr);
      PtrPipeLineState mPSO;
      Ref<ID3D12RootSignature> mRootSignature;
      Ref<ID3DBlob> pOutBlob, pErrorBlob;
      D3D12_ROOT_SIGNATURE descRootSignature = D3D12_ROOT_SIGNATURE ();
      descRootSignature.Flags = D3D12_ROOT_SIGNATURE_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
      RDX_ERR (D3D12SerializeRootSignature (&descRootSignature, D3D_ROOT_SIGNATURE_V1, pOutBlob.GetInitReference(), pErrorBlob.GetInitReference()), return );
      RDX_ERR (Device->CreateRootSignature (0, pOutBlob->GetBufferPointer (), pOutBlob->GetBufferSize (), IID_PPV_ARGS (mRootSignature.GetInitReference())), return);

      D3D12_GRAPHICS_PIPELINE_STATE_DESC descPso = {};
      ZeroMemory (&descPso, sizeof (descPso));
      descPso.InputLayout = mMeshLayout;
      descPso.pRootSignature = mRootSignature;
      descPso.VS = { reinterpret_cast<BYTE*>(VS->GetBufferPointer ()), VS->GetBufferSize () };
      descPso.PS = { reinterpret_cast<BYTE*>(PS->GetBufferPointer ()), PS->GetBufferSize () };
      descPso.RasterizerState = CD3D12_RASTERIZER_DESC (D3D12_DEFAULT);
      descPso.BlendState = CD3D12_BLEND_DESC (D3D12_DEFAULT);
      descPso.DepthStencilState.DepthEnable = FALSE;
      descPso.DepthStencilState.StencilEnable = FALSE;
      descPso.SampleMask = UINT_MAX;
      descPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      descPso.NumRenderTargets = 1;
      descPso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
      descPso.SampleDesc.Count = 1;
      RDX_ERR (Device->CreateGraphicsPipelineState (&descPso, IID_PPV_ARGS (mPSO.GetInitReference())), return);
    }
  }
}