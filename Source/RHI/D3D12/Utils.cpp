#include "DXCommon.h"
#include "Utils.h"
#include "Public/D3D12RHI.h"
#include "Core/LogUtil.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

NS_K3D_D3D12_BEGIN

using namespace std;

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
		DX12LOG(Info, "Resource Binding Tier = %d, Conservative Rasterization Tier = %d, Tiled Resource Tier = %d, Cross Sharing Tier = %d.",
			options.ResourceBindingTier, options.ConservativeRasterizationTier, options.TiledResourcesTier, options.CrossNodeSharingTier);
	}
}


NS_K3D_D3D12_END