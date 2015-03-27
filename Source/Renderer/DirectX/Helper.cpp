#include "Kaleido3D.h"
#include "Helper.h"

namespace k3d 
{
  namespace d3d12
  {
    void Helper::SetResourceBarrier (ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, UINT StateBefore, UINT StateAfter)
    {
      D3D12_RESOURCE_BARRIER_DESC barrierDesc = {};
      barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrierDesc.Transition.pResource = resource;
      barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barrierDesc.Transition.StateBefore = StateBefore;
      barrierDesc.Transition.StateAfter = StateAfter;
      commandList->ResourceBarrier (1, &barrierDesc);
    }

    void Helper::ConvertVertexFormatToInputElementDesc (VtxFormat const & format, std::vector<D3D12_INPUT_ELEMENT_DESC>& inputDesc)
    {
      switch (format)
      {
      case VtxFormat::POS3_F32_NOR3_F32_UV2_F32:
        inputDesc.push_back ({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D12_INPUT_PER_VERTEX_DATA, 0 });
        inputDesc.push_back ({ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	12,	D3D12_INPUT_PER_VERTEX_DATA, 0 });
        inputDesc.push_back ({ "TEXCOOD0",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	24,	D3D12_INPUT_PER_VERTEX_DATA, 0 });
        break;
      case VtxFormat::POS3_F32_NOR3_F32:
        inputDesc.push_back ({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0, D3D12_INPUT_PER_VERTEX_DATA, 0 });
        inputDesc.push_back ({ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_PER_VERTEX_DATA, 0 });
        break;
      default:
        break;
      }
    }
  }
}