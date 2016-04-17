#include "DXCommon.h"
#include "D3D12RootSignature.h"
#include "../Utils.h"

NS_K3D_D3D12_BEGIN

using namespace std;

static std::map< size_t, ComPtr<ID3D12RootSignature> > s_RootSignatureHashMap;

void RootSignature::DestroyAll(void)
{
	s_RootSignatureHashMap.clear();
}

void RootSignature::InitStaticSampler(
	UINT Register,
	const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
	D3D12_SHADER_VISIBILITY Visibility)
{
	assert(m_NumInitializedStaticSamplers < m_NumSamplers);
	D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = m_SamplerArray[m_NumInitializedStaticSamplers++];

	StaticSamplerDesc.Filter = NonStaticSamplerDesc.Filter;
	StaticSamplerDesc.AddressU = NonStaticSamplerDesc.AddressU;
	StaticSamplerDesc.AddressV = NonStaticSamplerDesc.AddressV;
	StaticSamplerDesc.AddressW = NonStaticSamplerDesc.AddressW;
	StaticSamplerDesc.MipLODBias = NonStaticSamplerDesc.MipLODBias;
	StaticSamplerDesc.MaxAnisotropy = NonStaticSamplerDesc.MaxAnisotropy;
	StaticSamplerDesc.ComparisonFunc = NonStaticSamplerDesc.ComparisonFunc;
	StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	StaticSamplerDesc.MinLOD = NonStaticSamplerDesc.MinLOD;
	StaticSamplerDesc.MaxLOD = NonStaticSamplerDesc.MaxLOD;
	StaticSamplerDesc.ShaderRegister = Register;
	StaticSamplerDesc.RegisterSpace = 0;
	StaticSamplerDesc.ShaderVisibility = Visibility;

	if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
	{
		/*
		WARN_ONCE_IF_NOT(
		// Transparent Black
		NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
		NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
		NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
		NonStaticSamplerDesc.BorderColor[3] == 0.0f ||
		// Opaque Black
		NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
		NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
		NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
		NonStaticSamplerDesc.BorderColor[3] == 1.0f ||
		// Opaque White
		NonStaticSamplerDesc.BorderColor[0] == 1.0f &&
		NonStaticSamplerDesc.BorderColor[1] == 1.0f &&
		NonStaticSamplerDesc.BorderColor[2] == 1.0f &&
		NonStaticSamplerDesc.BorderColor[3] == 1.0f,
		"Sampler border color does not match static sampler limitations");
		*/
		if (NonStaticSamplerDesc.BorderColor[3] == 1.0f)
		{
			if (NonStaticSamplerDesc.BorderColor[0] == 1.0f)
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			else
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		}
	}
}

void RootSignature::Finalize(PtrDevice Device, D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	if (m_Finalized)
		return;

	K3D_ASSERT(m_NumInitializedStaticSamplers == m_NumSamplers);

	D3D12_ROOT_SIGNATURE_DESC RootDesc;
	RootDesc.NumParameters = m_NumParameters;
	RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)m_ParamArray.get();
	RootDesc.NumStaticSamplers = m_NumSamplers;
	RootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)m_SamplerArray.get();
	RootDesc.Flags = Flags;

	m_DescriptorTableBitMap = 0;
	m_MaxDescriptorCacheHandleCount = 0;

	size_t HashCode = HashStateArray(RootDesc.pStaticSamplers, m_NumSamplers);
	for (UINT Param = 0; Param < m_NumParameters; ++Param)
	{
		const D3D12_ROOT_PARAMETER& RootParam = RootDesc.pParameters[Param];
		m_DescriptorTableSize[Param] = 0;

		if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			K3D_ASSERT(RootParam.DescriptorTable.pDescriptorRanges != nullptr);

			HashCode = HashStateArray(RootParam.DescriptorTable.pDescriptorRanges,
				RootParam.DescriptorTable.NumDescriptorRanges, HashCode);

			// We don't care about sampler descriptor tables.  We don't manage them in DescriptorCache
			if (RootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
				continue;

			m_DescriptorTableBitMap |= (1 << Param);
			for (UINT TableRange = 0; TableRange < RootParam.DescriptorTable.NumDescriptorRanges; ++TableRange)
				m_DescriptorTableSize[Param] += RootParam.DescriptorTable.pDescriptorRanges[TableRange].NumDescriptors;

			m_MaxDescriptorCacheHandleCount += m_DescriptorTableSize[Param];
		}
		else
			HashCode = HashState(&RootParam, HashCode);
	}

	ID3D12RootSignature** RSRef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_HashMapMutex;
		lock_guard<mutex> CS(s_HashMapMutex);
		auto iter = s_RootSignatureHashMap.find(HashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_RootSignatureHashMap.end())
		{
			RSRef = s_RootSignatureHashMap[HashCode].GetAddressOf();
			firstCompile = true;
		}
		else
			RSRef = iter->second.GetAddressOf();
	}

	if (firstCompile)
	{
		ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
		ThrowIfFailed(D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf()));
		ThrowIfFailed(Device->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_Signature)));
		s_RootSignatureHashMap[HashCode].Attach(m_Signature);
		assert(*RSRef == m_Signature);
	}
	else
	{
		while (*RSRef == nullptr)
			this_thread::yield();
		m_Signature = *RSRef;
	}

	m_Finalized = TRUE;
}

NS_K3D_D3D12_END