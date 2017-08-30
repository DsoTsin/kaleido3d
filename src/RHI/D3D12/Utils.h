#pragma once 
#include <Interface/IMesh.h>
#include "Public/D3D12RHI.h"

NS_K3D_D3D12_BEGIN

// Test create d3d12 device from feature level 12.1 - 11.0
extern D3D_FEATURE_LEVEL TestCreateDevice(IUnknown* comObj, Microsoft::WRL::ComPtr<ID3D12Device> & device);

class K3D_CORE_API Helper
{
public:
	static void	ConvertVertexFormatToInputElementDesc(
		VtxFormat const & format,
		std::vector<D3D12_INPUT_ELEMENT_DESC> & inputDesc);

	static void CheckHWFeatures(PtrDevice device);
};

// Hash Functions from MiniEngine Demo
inline size_t HashIterate(size_t Next, size_t CurrentHash = 2166136261U)
{
	return 16777619U * CurrentHash ^ Next;
}

template <typename T> inline size_t HashRange(const T* Begin, const T* End, size_t InitialVal = 2166136261U)
{
	size_t Val = InitialVal;

	while (Begin < End)
		Val = HashIterate((size_t)*Begin++, Val);

	return Val;
}

template <typename T> inline size_t HashStateArray(const T* StateDesc, size_t Count, size_t InitialVal = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
	return HashRange((UINT*)StateDesc, (UINT*)(StateDesc + Count), InitialVal);
}

template <typename T> inline size_t HashState(const T* StateDesc, size_t InitialVal = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
	return HashRange((UINT*)StateDesc, (UINT*)(StateDesc + 1), InitialVal);
}

NS_K3D_D3D12_END