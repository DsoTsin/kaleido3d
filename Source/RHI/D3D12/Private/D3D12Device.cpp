#include "DXCommon.h"
#include "Public/D3D12RHI.h"

namespace rhi
{
	using namespace ::k3d;
	using namespace ::k3d::d3d12;

	GpuVendor MapIdToVendor(uint32 id) {
		GpuVendor vendor = GpuVendor::UnKnown;
		switch (id)
		{
		case 0x8086:
			vendor = GpuVendor::Intel;
			break;
		case 0x10DE:
			vendor = GpuVendor::NVIDIA;
			break;
		case 0x1002:
			vendor = GpuVendor::AMD;
			break;
		default:
			break;
		}
		return vendor;
	}

	rhi::PFNEnumAllDevice EnumAllDeviceAdapter = [](rhi::IDeviceAdapter ** & adapterList, uint32 * num)->void
	{
		PtrGIFactory factory;
		HRESULT hr = ::CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()));
		K3D_ASSERT(hr==S_OK, "create dx factory failed.");
		vector<IDXGIAdapter1*> adapters;
		IDXGIAdapter1* pAdapter = nullptr;
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &pAdapter); ++adapterIndex) {
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
				adapters.push_back(pAdapter);
			}
		}

		*num = (uint32)adapters.size();
		if (adapters.empty())
		{
			return;
		}

		adapterList = new rhi::IDeviceAdapter*[adapters.size()];
		for (uint32 index = 0; index < adapters.size(); index++)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapters[index]->GetDesc1(&desc);
			GpuVendor vendor = MapIdToVendor(desc.VendorId);
			char VendorName[256] = { 0 };
			StringUtil::WCharToChar(desc.Description, VendorName, 256);
			Log::Out(LogLevel::Info, "Device", "Vendor is %s, Id is %d.", VendorName, desc.VendorId);
			adapterList[index] = new DeviceAdapter(adapters[index], vendor);
		}
	};

}


NS_K3D_D3D12_BEGIN

DirectCommandListManager g_DirectCommandListManager;


D3D_FEATURE_LEVEL TestFLs[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
};

D3D_FEATURE_LEVEL TestCreateDevice(IUnknown* comObj, PtrDevice & device) {
	HRESULT hr = -1;
	D3D_FEATURE_LEVEL level = (D3D_FEATURE_LEVEL)-1;
	for (int id = 0; id < sizeof(TestFLs); id++) {
		level = TestFLs[id];
		hr = ::D3D12CreateDevice(comObj, level, IID_PPV_ARGS(device.GetAddressOf()));
		if (SUCCEEDED(hr))
			return level;
	}
	return level;
}

Device::Device(bool useWarp)
	: m_Warp(useWarp)
	, m_RTVDHAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256)
	, m_UAVDHAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024)
	, m_SRVDHAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024)
	, m_CBVDHAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024)
	, m_DSVDHAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256)
	, m_SamplerDHAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 256)
{
}

Device::~Device()
{
}

rhi::IDevice::Result
Device::Create(rhi::IDeviceAdapter* pAdapter, bool withDbg)
{
	HRESULT hr = S_FALSE;
	hr = ::CreateDXGIFactory1(IID_PPV_ARGS(m_Factory.GetAddressOf()));
	if (withDbg) {
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
		debugController->EnableDebugLayer();
	}
	ComPtr<IDXGIAdapter> adapter;
	if (m_Warp) {
		m_Factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.GetAddressOf()));
	}
	else {
		K3D_ASSERT(pAdapter != nullptr);
		DeviceAdapter * realAdapter = static_cast<DeviceAdapter*>(pAdapter);
		adapter = realAdapter->m_Adapter;
	}
	D3D_FEATURE_LEVEL lev = TestCreateDevice(adapter.Get(), m_Inst);
	if (-1 != lev) {
		m_CBVDHAllocator.Init(m_Inst);
		m_RTVDHAllocator.Init(m_Inst);
		m_UAVDHAllocator.Init(m_Inst);
		m_SRVDHAllocator.Init(m_Inst);
		g_DirectCommandListManager.Create(m_Inst);
		return rhi::IDevice::DeviceFound;
	}
	return rhi::IDevice::DeviceNotFound;
}

rhi::ICommandContext*	Device::NewCommandContext(rhi::ECommandType Type)
{
	CommandContext * CmdContext = nullptr;
	switch(Type) 
	{
	case rhi::ECMD_Graphics:
		CmdContext = new GraphicsContext;
		break;
	case rhi::ECMD_Compute:
		CmdContext = new ComputeContext;
		break;
	default:
		CmdContext = nullptr;
		Log::Out("Device::NewCommandContext", "failed to create Unsupported CMDCTX.");
		break;
	}
	if (CmdContext) 
	{
		CmdContext->m_OwningManager = &g_DirectCommandListManager;
		CmdContext->Initialize();
	}
	return CmdContext;
}

rhi::ISampler*			Device::NewSampler(const rhi::SamplerState&)
{
	return nullptr;
}

rhi::IPipelineStateObject*	Device::NewPipelineState(rhi::EPipelineType type)
{
	PipelineState * pipeline = nullptr;
	switch (type)
	{
	case rhi::EPSO_Graphics:
		pipeline = new GraphicsPSO;
		break;
	case rhi::EPSO_Compute:
		pipeline = new ComputePSO;
		break;
	default:
		K3D_ASSERT(false, "unsupported pso type.");
	}
	pipeline->SetDevice(this);
	return pipeline;
}

rhi::ISyncPointFence * Device::NewFence()
{
	return nullptr;
}


DescriptorHeapAllocator::DescriptorHeapAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap)
	: m_Desc{ Type, NumDescriptorsPerHeap, D3D12_DESCRIPTOR_HEAP_FLAG_NONE }
	, m_DescriptorSize(0)
	, m_Device(nullptr)
{
}

void DescriptorHeapAllocator::Init(PtrDevice pDevice)
{
	m_Device = pDevice;
	m_DescriptorSize = pDevice->GetDescriptorHandleIncrementSize(m_Desc.Type);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::AllocateHeapSlot(SIZE_T & outIndex)
{
	std::unique_lock<std::mutex> lock(m_AllocMutex);
	if (m_FreeHeaps.empty()) 
	{
		AllocateHeap();
	}
	SIZE_T head = m_FreeHeaps.front();
	outIndex = head;
	Entry &heapEntry = m_Heaps[outIndex];
	assert(!heapEntry.m_FreeList.empty());
	Node range = heapEntry.m_FreeList.front();
	D3D12_CPU_DESCRIPTOR_HANDLE ret = { range.Start };
	range.Start += m_DescriptorSize;
	if (range.Start == range.End)
	{
		Node freeNode = heapEntry.m_FreeList.front();
		heapEntry.m_FreeList.remove(freeNode);
		if (heapEntry.m_FreeList.empty()) 
		{
			m_FreeHeaps.remove(head);
		}
	}
	return ret;
}

void DescriptorHeapAllocator::AllocateHeap()
{
	PtrDescHeap Heap;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&m_Desc, IID_PPV_ARGS(Heap.GetAddressOf())));
	D3D12_CPU_DESCRIPTOR_HANDLE HeapBase = Heap->GetCPUDescriptorHandleForHeapStart();
	m_Heaps.reserve(m_Heaps.size() + 1);
	NodeList freeList;
	freeList.push_back({ HeapBase.ptr, HeapBase.ptr + m_Desc.NumDescriptors * m_DescriptorSize });
	Entry entry = { Heap, freeList};
	m_Heaps.push_back(entry);
	m_FreeHeaps.push_back(m_Heaps.size() - 1);
}

void DescriptorHeapAllocator::FreeHeapSlot(D3D12_CPU_DESCRIPTOR_HANDLE Offset, SIZE_T index)
{
	std::unique_lock<std::mutex> lock(m_AllocMutex);
	Entry &heapEntry = m_Heaps[index];
	Node newRange = { Offset.ptr, Offset.ptr + m_DescriptorSize };
	bool bFound = false;
	if (!heapEntry.m_FreeList.empty()) {
		for (NodeList::iterator iter = heapEntry.m_FreeList.begin();
		iter != heapEntry.m_FreeList.end() && !bFound; iter++) {
			Node range = *iter;
			assert(range.Start < range.End);
			if (range.Start == Offset.ptr + m_DescriptorSize)
			{
				range.Start = Offset.ptr;
				bFound = true;
			}
			else if (range.End == Offset.ptr)
			{
				range.End += m_DescriptorSize;
				bFound = true;
			}
			else
			{
				assert(range.End < Offset.ptr || range.Start > Offset.ptr);
				if (range.Start > Offset.ptr)
				{
					heapEntry.m_FreeList.insert(iter, newRange);
					bFound = true;
				}
			}
		}
	}

	if (!bFound) {
		if (heapEntry.m_FreeList.empty()) {
			m_FreeHeaps.push_back(index);
		}
		heapEntry.m_FreeList.push_back(newRange);
	}
}


NS_K3D_D3D12_END
