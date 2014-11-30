#include "DXCommon.h"
#include "Utils.h"
#include "Public/D3D12RHI.h"
#include "Core/File.h"
#include "Core/LogUtil.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

NS_K3D_D3D12_BEGIN

using namespace std;

namespace
{
	ID3D12QueryHeap* sm_QueryHeap = nullptr;
	ID3D12Resource* sm_ReadBackBuffer = nullptr;
	uint64_t* sm_TimeStampBuffer = nullptr;
	uint32_t sm_MaxNumTimers = 0ul;
	uint32_t sm_NumTimers = 0ul;
	uint64_t sm_SentryTimeStamp = 0ull;
	double sm_GpuTickDelta = 0.0;
}

namespace GpuTimeManager 
{
	void Initialize(uint32_t MaxNumTimers)
	{
		uint64_t GpuFrequency;
		RHIRoot::GetPrimaryCommandListManager().GetCommandQueue()->GetTimestampFrequency(&GpuFrequency);
		sm_GpuTickDelta = 1.0 / static_cast<double>(GpuFrequency);

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC BufferDesc;
		BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		BufferDesc.Alignment = 0;
		BufferDesc.Width = sizeof(uint64_t) * MaxNumTimers * 2 + 1;
		BufferDesc.Height = 1;
		BufferDesc.DepthOrArraySize = 1;
		BufferDesc.MipLevels = 1;
		BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		BufferDesc.SampleDesc.Count = 1;
		BufferDesc.SampleDesc.Quality = 0;
		BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFailed(RHIRoot::GetPrimaryD3DDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &BufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sm_ReadBackBuffer)));
		sm_ReadBackBuffer->SetName(L"GpuTimeStamp Buffer");

		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
		QueryHeapDesc.Count = MaxNumTimers * 2 + 1;
		QueryHeapDesc.NodeMask = 1;
		QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		ThrowIfFailed(RHIRoot::GetPrimaryD3DDevice()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&sm_QueryHeap)));
		sm_QueryHeap->SetName(L"GpuTimeStamp QueryHeap");

		sm_TimeStampBuffer = 0;
		sm_MaxNumTimers = (uint32_t)MaxNumTimers;
	}

	void Shutdown()
	{
		if (sm_TimeStampBuffer != nullptr)
		{
			D3D12_RANGE EmptyRange = {};
			sm_ReadBackBuffer->Unmap(0, &EmptyRange);
		}

		if (sm_ReadBackBuffer != nullptr)
			sm_ReadBackBuffer->Release();

		if (sm_QueryHeap != nullptr)
			sm_QueryHeap->Release();
	}

	void EndFrame(CommandContext& Context)
	{
		Context.ResolveTimeStamps(sm_ReadBackBuffer, sm_QueryHeap, 1 + sm_NumTimers * 2);
		Context.InsertTimeStamp(sm_QueryHeap, 0);
	}

	uint32_t NewTimer(void)
	{
		return sm_NumTimers++;
	}

	void StartTimer(CommandContext& Context, uint32_t TimerIdx)
	{
		Context.InsertTimeStamp(sm_QueryHeap, TimerIdx * 2 + 1);
	}

	void StopTimer(CommandContext& Context, uint32_t TimerIdx)
	{
		Context.InsertTimeStamp(sm_QueryHeap, TimerIdx * 2 + 2);
	}

	void BeginReadBack(void)
	{
		D3D12_RANGE Range;
		Range.Begin = 0;
		Range.End = (sm_NumTimers * 2 + 1) * sizeof(uint64_t);
		sm_ReadBackBuffer->Map(0, &Range, reinterpret_cast<void**>(&sm_TimeStampBuffer));
		sm_SentryTimeStamp = sm_TimeStampBuffer[0];
	}

	void EndReadBack(void)
	{
		D3D12_RANGE EmptyRange = {};
		sm_ReadBackBuffer->Unmap(0, &EmptyRange);
		sm_TimeStampBuffer = nullptr;
	}

	float GetTime(uint32_t TimerIdx)
	{
		K3D_ASSERT(sm_TimeStampBuffer != nullptr, "Time stamp readback buffer is not mapped");
		K3D_ASSERT(TimerIdx < sm_NumTimers, "Invalid GPU timer index");

		uint64_t TimeStamp1 = sm_TimeStampBuffer[TimerIdx * 2 + 1];
		uint64_t TimeStamp2 = sm_TimeStampBuffer[TimerIdx * 2 + 2];

		if (TimeStamp1 < sm_SentryTimeStamp || TimeStamp2 < sm_SentryTimeStamp)
			return 0.0f;

		return static_cast<float>(sm_GpuTickDelta * (TimeStamp2 - TimeStamp1));
	}
}

void Helper::ConvertVertexFormatToInputElementDesc(VtxFormat const & format, std::vector<D3D12_INPUT_ELEMENT_DESC>& inputDesc)
{
	switch (format)
	{
	case VtxFormat::POS3_F32_NOR3_F32_UV2_F32:
		inputDesc.push_back({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "TEXCOOD0",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		break;
	case VtxFormat::POS3_F32_NOR3_F32:
		inputDesc.push_back({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		break;
	default:
		break;
	}
}

void Helper::CheckHWFeatures(PtrDevice device)
{
	assert(device != nullptr);
	D3D12_FEATURE_DATA_D3D12_OPTIONS options;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options), sizeof(options))))
	{
	}
	else
	{
		Log::Out("d3d12", "Resource Binding Tier = %d, Conservative Rasterization Tier = %d, Tiled Resource Tier = %d, Cross Sharing Tier = %d.",
			options.ResourceBindingTier, options.ConservativeRasterizationTier, options.TiledResourcesTier, options.CrossNodeSharingTier);
	}
}


NS_K3D_D3D12_END