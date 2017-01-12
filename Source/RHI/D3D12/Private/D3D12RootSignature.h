#pragma once

NS_K3D_D3D12_BEGIN

// Maximum 64 DWORDS divied up amongst all root parameters.
// Root constants = 1 DWORD * NumConstants
// Root descriptor (CBV, SRV, or UAV) = 2 DWORDs each
// Descriptor table pointer = 1 DWORD
// Static samplers = 0 DWORDS (compiled into shader)
class RootSignature
{
	friend class DynamicDescriptorHeap;

public:

	RootSignature(UINT NumRootParams = 0, UINT NumStaticSamplers = 0) : m_Finalized(FALSE), m_NumParameters(NumRootParams)
	{
		Reset(NumRootParams, NumStaticSamplers);
	}

	~RootSignature()
	{
	}

	static void DestroyAll(void);

	void Reset(UINT NumRootParams, UINT NumStaticSamplers = 0)
	{
		if (NumRootParams > 0)
			m_ParamArray.reset(new CD3DX12_ROOT_PARAMETER[NumRootParams]);
		else
			m_ParamArray = nullptr;
		m_NumParameters = NumRootParams;

		if (NumStaticSamplers > 0)
			m_SamplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers]);
		else
			m_SamplerArray = nullptr;
		m_NumSamplers = NumStaticSamplers;
		m_NumInitializedStaticSamplers = 0;
	}

	CD3DX12_ROOT_PARAMETER& operator[] (size_t EntryIndex)
	{
		assert(EntryIndex < m_NumParameters);
		return m_ParamArray.get()[EntryIndex];
	}

	const CD3DX12_ROOT_PARAMETER& operator[] (size_t EntryIndex) const
	{
		assert(EntryIndex < m_NumParameters);
		return m_ParamArray.get()[EntryIndex];
	}

	void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
		D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

	void Finalize(PtrDevice Device, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ID3D12RootSignature* GetSignature() const { return m_Signature; }

protected:

	BOOL m_Finalized;
	UINT m_NumParameters;
	UINT m_NumSamplers;
	UINT m_NumInitializedStaticSamplers;
	uint32_t m_DescriptorTableBitMap;		// One bit is set for root parameters that are (non-sampler) descriptor tables
	uint32_t m_DescriptorTableSize[16];		// Non-sampler descriptor tables need to know their descriptor count
	UINT m_MaxDescriptorCacheHandleCount;	// The sum of all non-sampler descriptor table counts
	std::unique_ptr<CD3DX12_ROOT_PARAMETER[]> m_ParamArray;
	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> m_SamplerArray;
	ID3D12RootSignature* m_Signature;
};

NS_K3D_D3D12_END