#include "DXCommon.h"
#include "Public/D3D12RHIResource.h"
#include "Public/D3D12RHI.h"
#include "LinearAllocator.h"
#include "../Color.h"
#include <Math/kMath.hpp>

NS_K3D_D3D12_BEGIN

void * GpuResource::Map()
{
	void * bufferPointer;
	assert(m_Resource != nullptr);
	ThrowIfFailed(m_Resource->Map(0, nullptr, reinterpret_cast<void**>(&bufferPointer)));
	return bufferPointer;
}

D3D12_GPU_VIRTUAL_ADDRESS GpuResource::GetGpuVirtualAddress() const
{
	return m_Resource->GetGPUVirtualAddress();
}

/////////////////////////////////// GpuBuffer Implementation ///////////////////////////////////////////
void GpuBuffer::Create(const kString& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData)
{
	m_ElementCount = NumElements;
	m_ElementSize = ElementSize;
	m_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	ThrowIfFailed(
		m_Device->Get()->CreateCommittedResource(
			&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, m_UsageState,
			nullptr, IID_PPV_ARGS(m_Resource.GetAddressOf()))
		);

	m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();

	if (initialData)
		CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);

#ifdef RELEASE
	(name);
#else
	m_Resource->SetName(name.c_str());
#endif

	CreateDerivedViews();
}

void GpuBuffer::Destroy(void)
{
	GpuResource::Destroy();
}

D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t Offset, uint32_t Size) const
{
	K3D_ASSERT(Offset + Size <= m_BufferSize);
	Size = kMath::AlignUp(Size, 16);
	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
	CBVDesc.BufferLocation = m_GpuVirtualAddress + (size_t)Offset;
	CBVDesc.SizeInBytes = Size;
	auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_CONSTANT_BUFFER_VIEW_DESC>();
	SIZE_T AllocatedIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE hCBV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	m_Device->Get()->CreateConstantBufferView(&CBVDesc, hCBV);
	return hCBV;
}

D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer(void)
{
	K3D_ASSERT(m_BufferSize != 0);

	D3D12_RESOURCE_DESC Desc = {};
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Flags = m_ResourceFlags;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Height = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.MipLevels = 1;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Width = (UINT64)m_BufferSize;
	return Desc;
}


class ByteAddressBuffer : public GpuBuffer
{
public:
	virtual void CreateDerivedViews(void) override;
};

class IndirectArgsBuffer : public ByteAddressBuffer
{
public:
	IndirectArgsBuffer(void)
	{
	}
};

class StructuredBuffer : public GpuBuffer
{
public:
	virtual void Destroy(void) override
	{
		m_CounterBuffer.Destroy();
		GpuBuffer::Destroy();
	}

	virtual void CreateDerivedViews(void) override;

	ByteAddressBuffer& GetCounterBuffer(void) { return m_CounterBuffer; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(CommandContext& Context);
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(CommandContext& Context);

private:
	ByteAddressBuffer m_CounterBuffer;
};

class TypedBuffer : public GpuBuffer
{
public:
	TypedBuffer(DXGI_FORMAT Format) : m_DataFormat(Format) {}
	virtual void CreateDerivedViews(void) override;

protected:
	DXGI_FORMAT m_DataFormat;
};

class PixelBuffer : public GpuResource
{
public:
	PixelBuffer() : m_Width(0), m_Height(0) {}

	uint32_t GetWidth(void) const { return m_Width; }
	uint32_t GetHeight(void) const { return m_Height; }
	uint32_t GetDepth(void) const { return m_ArraySize; }
	const DXGI_FORMAT& GetFormat(void) const { return m_Format; }

protected:

	D3D12_RESOURCE_DESC DescribeTex2D(uint32_t Width, uint32_t Height, uint32_t DepthOrArraySize, uint32_t NumMips, DXGI_FORMAT Format, UINT Flags);

	void AssociateWithResource(PtrDevice Device, const std::wstring& Name, ID3D12Resource* Resource, D3D12_RESOURCE_STATES CurrentState);

	void CreateTextureResource(PtrDevice Device, const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc,
		D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT Format);

	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_ArraySize;
	DXGI_FORMAT m_Format;
};

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float ClearDepth = 0.0f, uint32_t ClearStencil = 0)
		: m_ClearDepth(ClearDepth), m_ClearStencil(ClearStencil)
	{
		m_hDSV[0].ptr = ~0ull;
		m_hDSV[1].ptr = ~0ull;
		m_hDSV[2].ptr = ~0ull;
		m_hDSV[3].ptr = ~0ull;
		m_hDepthSRV.ptr = ~0ull;
		m_hStencilSRV.ptr = ~0ull;
	}

	// Create a depth buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Get pre-created CPU-visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return m_hDSV[0]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_DepthReadOnly() const { return m_hDSV[1]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_StencilReadOnly() const { return m_hDSV[2]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_ReadOnly() const { return m_hDSV[3]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const { return m_hDepthSRV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const { return m_hStencilSRV; }

	float GetClearDepth() const { return m_ClearDepth; }
	uint32_t GetClearStencil() const { return m_ClearStencil; }

private:

	void CreateDerivedViews(PtrDevice Device, DXGI_FORMAT Format);

	float m_ClearDepth;
	uint32_t m_ClearStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_hDSV[4];
	D3D12_CPU_DESCRIPTOR_HANDLE m_hDepthSRV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_hStencilSRV;
};

class ShadowBuffer : public DepthBuffer
{
public:
	ShadowBuffer() {}

	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return GetDepthSRV(); }

	void BeginRendering(CommandContext& context);
	void EndRendering(CommandContext& context);

private:
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Scissor;
};

class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer(Color ClearColor = Color(0.0f, 0.0f, 0.0f, 0.0f))
		: m_ClearColor(ClearColor), m_NumMipMaps(0)
	{
		m_SRVHandle.ptr = ~0ull;
		m_RTVHandle.ptr = ~0ull;
		std::memset(m_UAVHandle, 0xFF, sizeof(m_UAVHandle));
	}

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* BaseResource);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
		DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
		DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);


	// Get pre-created CPU-visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return m_RTVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAVHandle[0]; }

	Color GetClearColor(void) const { return m_ClearColor; }

	// This will work for all texture sizes, but it's recommended for speed and quality
	// that you use dimensions with powers of two (but not necessarily square.)  Pass
	// 0 for ArrayCount to reserve space for mips at creation time.
	void GenerateMipMaps(CommandContext& Context);

protected:

	// Compute the number of texture levels needed to reduce to 1x1.  This uses
	// _BitScanReverse to find the highest set bit.  Each dimension reduces by
	// half and truncates bits.  The dimension 256 (0x100) has 9 mip levels, same
	// as the dimension 511 (0x1FF).
	static inline uint32_t ComputeNumMips(uint32_t Width, uint32_t Height)
	{
		uint32_t HighBit;
		_BitScanReverse((unsigned long*)&HighBit, Width | Height);
		return HighBit + 1;
	}

	void CreateDerivedViews(PtrDevice, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips = 1);

	Color m_ClearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SRVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_UAVHandle[12];
	uint32_t m_NumMipMaps; // number of texture sublevels
};

void ByteAddressBuffer::CreateDerivedViews(void)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>();
		SIZE_T AllocatedIndex;
		m_SRV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	}
	m_Device->Get()->CreateShaderResourceView(m_Resource.Get(), &SRVDesc, m_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	UAVDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_UNORDERED_ACCESS_VIEW_DESC>();
		SIZE_T AllocatedIndex;
		m_UAV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	}
	m_Device->Get()->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &UAVDesc, m_UAV);
}

void StructuredBuffer::CreateDerivedViews(void)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = m_ElementCount;
	SRVDesc.Buffer.StructureByteStride = m_ElementSize;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>();
		SIZE_T AllocatedIndex;
		m_SRV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	}
	m_Device->Get()->CreateShaderResourceView(m_Resource.Get(), &SRVDesc, m_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.Buffer.CounterOffsetInBytes = 0;
	UAVDesc.Buffer.NumElements = m_ElementCount;
	UAVDesc.Buffer.StructureByteStride = m_ElementSize;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	m_CounterBuffer.Create(L"StructuredBuffer::Counter", 1, 4);

	if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_UNORDERED_ACCESS_VIEW_DESC>();
		SIZE_T AllocatedIndex;
		m_UAV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	}
	m_Device->Get()->CreateUnorderedAccessView(m_Resource.Get(), m_CounterBuffer.GetResource(), &UAVDesc, m_UAV);
}

void TypedBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = m_DataFormat;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = m_ElementCount;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
		
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>();
		SIZE_T AllocatedIndex;
		m_SRV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	}
	m_Device->Get()->CreateShaderResourceView(m_Resource.Get(), &SRVDesc, m_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = m_DataFormat;
	UAVDesc.Buffer.NumElements = m_ElementCount;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_UNORDERED_ACCESS_VIEW_DESC>();
		SIZE_T AllocatedIndex;
		m_UAV = HeapAllocator.AllocateHeapSlot(AllocatedIndex);
	}
	m_Device->Get()->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &UAVDesc, m_UAV);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandContext& Context)
{
	Context.TransitionResource(m_CounterBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	return m_CounterBuffer.GetSRV();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandContext& Context)
{
	Context.TransitionResource(m_CounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	return m_CounterBuffer.GetUAV();
}

DXGI_FORMAT PixelBuffer::GetBaseFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_TYPELESS;

		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_R32G8X24_TYPELESS;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_TYPELESS;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_R24G8_TYPELESS;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_R16_TYPELESS;

	default:
		return defaultFormat;
	}
}

DXGI_FORMAT PixelBuffer::GetUAVFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM;

	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_UNORM;

	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

#ifdef _DEBUG
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_D16_UNORM:

		K3D_ASSERT(false, "Requested a UAV format for a depth stencil format.");
#endif

	default:
		return defaultFormat;
	}
}

DXGI_FORMAT PixelBuffer::GetDSVFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_D32_FLOAT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_D16_UNORM;

	default:
		return defaultFormat;
	}
}

DXGI_FORMAT PixelBuffer::GetDepthFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

DXGI_FORMAT PixelBuffer::GetStencilFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

void PixelBuffer::AssociateWithResource(PtrDevice Device, const std::wstring& Name, ID3D12Resource* Resource, D3D12_RESOURCE_STATES CurrentState)
{
	K3D_ASSERT(Resource != nullptr);
	D3D12_RESOURCE_DESC ResourceDesc = Resource->GetDesc();

	m_Resource.Attach(Resource);
	m_UsageState = CurrentState;

	m_Width = (uint32_t)ResourceDesc.Width;		// We don't care about large virtual textures yet
	m_Height = ResourceDesc.Height;
	m_ArraySize = ResourceDesc.DepthOrArraySize;
	m_Format = ResourceDesc.Format;

#ifndef RELEASE
	m_Resource->SetName(Name.c_str());
#else
	(Name);
#endif
}

D3D12_RESOURCE_DESC PixelBuffer::DescribeTex2D(uint32_t Width, uint32_t Height, uint32_t DepthOrArraySize,
	uint32_t NumMips, DXGI_FORMAT Format, UINT Flags)
{
	m_Width = Width;
	m_Height = Height;
	m_ArraySize = DepthOrArraySize;
	m_Format = Format;

	D3D12_RESOURCE_DESC Desc = {};
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = (UINT16)DepthOrArraySize;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	Desc.Flags = (D3D12_RESOURCE_FLAGS)Flags;
	Desc.Format = GetBaseFormat(Format);
	Desc.Height = (UINT)Height;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	Desc.MipLevels = (UINT16)NumMips;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Width = (UINT64)Width;
	return Desc;
}

void PixelBuffer::CreateTextureResource(PtrDevice Device, const std::wstring& Name,
	const D3D12_RESOURCE_DESC& ResourceDesc, D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS /*VidMemPtr*/)
{
	{
		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ThrowIfFailed(Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, D3D12_RESOURCE_STATE_COMMON, &ClearValue, IID_PPV_ARGS(&m_Resource)));
	}

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;
	m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;

#ifndef RELEASE
	m_Resource->SetName(Name.c_str());
#else
	(Name);
#endif
}


void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
{
	D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, 1, 1, Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = Format;
	CreateTextureResource(m_Device->Get(), Name, ResourceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(m_Device->Get(), Format);
}

void DepthBuffer::CreateDerivedViews(PtrDevice Device, DXGI_FORMAT Format)
{
	ID3D12Resource* Resource = m_Resource.Get();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = GetDSVFormat(Format);
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (m_hDSV[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_DEPTH_STENCIL_VIEW_DESC>();
		SIZE_T OutIndex;
		m_hDSV[0] = HeapAllocator.AllocateHeapSlot(OutIndex);
		m_hDSV[1] = HeapAllocator.AllocateHeapSlot(OutIndex);
	}

	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[0]);

	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[1]);

	DXGI_FORMAT stencilReadFormat = GetStencilFormat(Format);
	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_hDSV[2].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_DEPTH_STENCIL_VIEW_DESC>();
			SIZE_T OutIndex;
			m_hDSV[2] = HeapAllocator.AllocateHeapSlot(OutIndex);
			m_hDSV[3] = HeapAllocator.AllocateHeapSlot(OutIndex);
		}

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[2]);

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		Device->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[3]);
	}
	else
	{
		m_hDSV[2] = m_hDSV[0];
		m_hDSV[3] = m_hDSV[1];
	}

	if (m_hDepthSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>();
		SIZE_T AllocID;
		m_hDepthSRV = HeapAllocator.AllocateHeapSlot(AllocID);
	}
	// Create the shader resource view
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = GetDepthFormat(Format);
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Texture2D.MipLevels = 1;
	Device->CreateShaderResourceView(Resource, &SRVDesc, m_hDepthSRV);

	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_hStencilSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
			auto & HeapAllocator = m_Device->GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>();
			SIZE_T AllocID;
			m_hStencilSRV = HeapAllocator.AllocateHeapSlot(AllocID);
		}
		SRVDesc.Format = stencilReadFormat;
		Device->CreateShaderResourceView(Resource, &SRVDesc, m_hStencilSRV);
	}
}

void ShadowBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
{
	DepthBuffer::Create(Name, Width, Height, DXGI_FORMAT_D16_UNORM, VidMemPtr);

	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = (float)Width;
	m_Viewport.Height = (float)Height;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	// Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
	m_Scissor.left = 1;
	m_Scissor.top = 1;
	m_Scissor.right = (LONG)Width - 2;
	m_Scissor.bottom = (LONG)Height - 2;
}

void ShadowBuffer::BeginRendering(CommandContext& Context)
{
	/*
	Context.ClearDepth(*this);
	Context.SetDepthStencilTarget(*this);
	Context.SetViewportAndScissor(m_Viewport, m_Scissor);
	*/
}

void ShadowBuffer::EndRendering(CommandContext& Context)
{
	Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void ColorBuffer::CreateDerivedViews(PtrDevice Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips)
{
	K3D_ASSERT(ArraySize == 1 || NumMips == 1, "We don't support auto-mips on texture arrays");

	m_NumMipMaps = NumMips - 1;

	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

	RTVDesc.Format = Format;
	UAVDesc.Format = GetUAVFormat(Format);
	SRVDesc.Format = Format;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (ArraySize > 1)
	{
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		RTVDesc.Texture2DArray.MipSlice = 0;
		RTVDesc.Texture2DArray.FirstArraySlice = 0;
		RTVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		UAVDesc.Texture2DArray.MipSlice = 0;
		UAVDesc.Texture2DArray.FirstArraySlice = 0;
		UAVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		SRVDesc.Texture2DArray.MipLevels = NumMips;
		SRVDesc.Texture2DArray.MostDetailedMip = 0;
		SRVDesc.Texture2DArray.FirstArraySlice = 0;
		SRVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
	}
	else
	{
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;

		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Texture2D.MipSlice = 0;

		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = NumMips;
		SRVDesc.Texture2D.MostDetailedMip = 0;
	}

	if (m_SRVHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		/*TODO:UNCOMMENT
		m_RTVHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_SRVHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		*/
	}

	ID3D12Resource* Resource = m_Resource.Get();

	// Create the render target view
	Device->CreateRenderTargetView(Resource, &RTVDesc, m_RTVHandle);

	// Create the shader resource view
	Device->CreateShaderResourceView(Resource, &SRVDesc, m_SRVHandle);

	// Create the UAVs for each mip level (RWTexture2D)
	for (uint32_t i = 0; i < NumMips; ++i)
	{
		if (m_UAVHandle[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
			// TODO : UnComment	m_UAVHandle[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		Device->CreateUnorderedAccessView(Resource, nullptr, &UAVDesc, m_UAVHandle[i]);

		UAVDesc.Texture2D.MipSlice++;
	}
}

void ColorBuffer::CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* BaseResource)
{
	AssociateWithResource(m_Device->Get(), Name, BaseResource, D3D12_RESOURCE_STATE_PRESENT);

	// BUG:  Currently, we are prohibited from creating UAVs of the swap chain.
	//CreateDerivedViews(GetD3DDevice(), BaseResource->GetDesc().Format, 1);

	// WORKAROUND:  Just create a typical RTV
	m_RTVHandle = ModuleD3D12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_Device->Get()->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RTVHandle);
}

void ColorBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
	DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMem)
{
	NumMips = (NumMips == 0 ? ComputeNumMips(Width, Height) : NumMips);
	D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, 1, NumMips, Format,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = Format;
	ClearValue.Color[0] = m_ClearColor.R();
	ClearValue.Color[1] = m_ClearColor.G();
	ClearValue.Color[2] = m_ClearColor.B();
	ClearValue.Color[3] = m_ClearColor.A();

	CreateTextureResource(m_Device->Get(), Name, ResourceDesc, ClearValue, VidMem);
	CreateDerivedViews(m_Device->Get(), Format, 1, NumMips);
}

void ColorBuffer::CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
	DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMem)
{
	D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, ArrayCount, 1, Format,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = Format;
	ClearValue.Color[0] = m_ClearColor.R();
	ClearValue.Color[1] = m_ClearColor.G();
	ClearValue.Color[2] = m_ClearColor.B();
	ClearValue.Color[3] = m_ClearColor.A();

	CreateTextureResource(m_Device->Get(), Name, ResourceDesc, ClearValue, VidMem);
	CreateDerivedViews(m_Device->Get(), Format, ArrayCount, 1);
}

/**

void ColorBuffer::GenerateMipMaps(CommandContext& BaseContext)
{
	if (m_NumMipMaps == 0)
		return;

	ComputeContext& Context = BaseContext.GetComputeContext();
	Context.SetRootSignature(Graphics::g_GenerateMipsRS);

	if (m_Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
	Context.SetPipelineState(Graphics::g_GenerateMipsGammaPSO);
	else
	Context.SetPipelineState(Graphics::g_GenerateMipsPSO);

	Context.TransitionResource(*this, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Context.SetDynamicDescriptor(1, 0, m_SRVHandle);
	
	for (uint32_t TopMip = 0; TopMip < m_NumMipMaps; )
	{
		uint32_t DstWidth = m_Width >> (TopMip + 1);
		uint32_t DstHeight = m_Height >> (TopMip + 1);

		// We can downsample up to four times, but if the ratio between levels is not
		// exactly 2:1, we have to shift our blend weights, which gets complicated or
		// expensive.  Maybe we can update the code later to compute sample weights for
		// each successive downsample.  We use _BitScanForward to count number of zeros
		// in the low bits.  Zeros indicate we can divide by two without truncating.
		uint32_t AdditionalMips;
		_BitScanForward((unsigned long*)&AdditionalMips, DstWidth | DstHeight);
		uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);
		if (TopMip + NumMips > m_NumMipMaps)
			NumMips = m_NumMipMaps - TopMip;

		// These are clamped to 1 after computing additional mips because clamped
		// dimensions should not limit us from downsampling multiple times.  (E.g.
		// 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
		if (DstWidth == 0)
			DstWidth = 1;
		if (DstHeight == 0)
			DstHeight = 1;

		Context.SetConstants(0, TopMip, NumMips, 1.0f / DstWidth, 1.0f / DstHeight);
		Context.SetDynamicDescriptors(2, 0, NumMips, m_UAVHandle + TopMip + 1);
		Context.Dispatch2D(DstWidth, DstHeight);

		Context.InsertUAVBarrier(*this);

		TopMip += NumMips;
	}

	Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}
**/



rhi::IGpuResource*		Device::NewGpuResource(rhi::EGpuResourceType type)
{
	switch (type)
	{
	case rhi::Buffer:
		ByteAddressBuffer * byteBuffer = new ByteAddressBuffer();
		byteBuffer->m_Device = this;
		return byteBuffer;
	}
	return nullptr;
}

NS_K3D_D3D12_END
