#include "DXCommon.h"
#include "RHI.h"
#include "Public/D3D12RHI.h"
#include "Utils.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <Core/Os.h>
#include <Core/AssetManager.h>

NS_K3D_D3D12_BEGIN

using namespace rapidjson;

/**
* @see Data/Test/GfxRootParameter.json
*/
void GfxRootParameter::Load(const char * rpFile)
{
	Os::File f(rpFile);
	if (!f.Open(IORead)) return;
	int64 len = f.GetSize();
	/*scope*/ char* data = new char[len + 1];
	data[len] = 0;
	Document d;
	d.Parse(data);
	if (!d.IsArray()) return;
	int num = d.Capacity();
	m_Count = num;
	m_DescriptorRanges = new CD3DX12_DESCRIPTOR_RANGE[num];
	m_TableSlots = new CD3DX12_ROOT_PARAMETER[num];
	for (rapidjson::SizeType i = 0; i< d.Capacity(); ++i) {
		Value & item = d[i];
		//KLOG("GfxRootParameter", item["Table"].GetString());
		D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY(item["ShaderVisibility"].GetInt());
		UINT type = item["DescriptorRangeType"].GetInt();
		UINT count = item["Count"].GetInt();
		UINT baseReg = item["BaseShaderReg"].GetInt();
		m_DescriptorRanges[i].Init((D3D12_DESCRIPTOR_RANGE_TYPE)type, count, baseReg);
		m_TableSlots[i].InitAsDescriptorTable(1, &m_DescriptorRanges[i], vis);
	}
}


NS_K3D_D3D12_END
