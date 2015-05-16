#include "Kaleido3D.h"
#include "RootSignature.h"

#include <Core/LogUtil.h>

namespace k3d
{
  namespace d3d12
  {
	  void RootSignature::Create(ID3D12Device* device)
	  {
		  D3D12_ROOT_SIGNATURE_DESC descRootSignature = D3D12_ROOT_SIGNATURE_DESC();
		  descRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		  descRootSignature.NumParameters = 4;
		  //create an array that will describe each root parameter
		  D3D12_ROOT_PARAMETER rootParams[4];
		  descRootSignature.pParameters = rootParams; //set param array in the root sig
													  //added a CBV to API slot 0 of this root signature, uses 4 of the 16 dwords available. (https://msdn.microsoft.com/en-us/library/dn899209(v=vs.85).aspx)
		  rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		  rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		  rootParams[0].Descriptor.RegisterSpace = 0;
		  rootParams[0].Descriptor.ShaderRegister = 0;
		  //create an array of descriptor ranges, these range(s) form the entries in descriptor tables 
		  D3D12_DESCRIPTOR_RANGE descRange[3];
		  //CBV range
		  descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		  descRange[0].NumDescriptors = 2;
		  descRange[0].BaseShaderRegister = 1;
		  descRange[0].RegisterSpace = 0;
		  descRange[0].OffsetInDescriptorsFromTableStart = 0;
		  //SRV range
		  descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		  descRange[1].NumDescriptors = 1;
		  descRange[1].BaseShaderRegister = 0;
		  descRange[1].RegisterSpace = 0;
		  descRange[1].OffsetInDescriptorsFromTableStart = 0;
		  //sampler range
		  descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		  descRange[2].NumDescriptors = 1;
		  descRange[2].BaseShaderRegister = 0;
		  descRange[2].RegisterSpace = 0;
		  descRange[2].OffsetInDescriptorsFromTableStart = 0;

		  //added a descriptor table with 2 CBVs to slot 1 of root sig, uses 1 dword
		  rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		  rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		  rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		  rootParams[1].DescriptorTable.pDescriptorRanges = &descRange[0];
		  //table with 1 SRV to slot 2 of the root sig, uses 1 dword
		  rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		  rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		  rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		  rootParams[2].DescriptorTable.pDescriptorRanges = &descRange[1];
		  //added sampler descriptor table at slot 3 of root sig, costs 1 dword.
		  rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		  rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		  rootParams[3].DescriptorTable.NumDescriptorRanges = 1;
		  rootParams[3].DescriptorTable.pDescriptorRanges = &descRange[2];
		  //end of root sig, 7/16 dwords used

		  ThrowIfFailed(
			  D3D12SerializeRootSignature(
				  &descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1,
				  m_Blob.GetInitReference(), m_ErrorBlob.GetInitReference())
			  );
		  ThrowIfFailed(
			  device->CreateRootSignature(
				  1, m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(),
				  IID_PPV_ARGS(m_RootSignature.GetInitReference()))
			  );
	  }
  }

}