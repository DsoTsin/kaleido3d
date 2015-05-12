#pragma once 
#include <KTL/Singleton.hpp>
#include "DXCommon.h"
#include "Device.h"
#include "RootSignature.h"

namespace k3d
{
  namespace d3d12
  {
	  struct PipelineStateObjectDescription : D3D12_GRAPHICS_PIPELINE_STATE_DESC
	  {
		  static PipelineStateObjectDescription Simple(
			  const D3D12_INPUT_LAYOUT_DESC& inputLayout,
			  const RootSignature& rootSig,
			  const Shader& vs, const Shader& ps
			  );
	  };

	  class PipelineStateObject
	  {
	  public:
		  void Create(
			  ID3D12Device* device,
			  const PipelineStateObjectDescription& desc);

		  auto Get() const { return mPSO; }

	  private:
		  PtrPipeLineState mPSO;
	  };

	  class GraphicsPipeLineState {

	  };
  }
}