#include "DXCommon.h"
#include "RHI.h"
#include "Public/D3D12RHI.h"
#include "ModuleD3D12.h"
#include "Utils.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <Core/File.h>
#include <Core/AssetManager.h>



NS_K3D_D3D12_BEGIN

shared_ptr<Device> RHIRoot::s_PrimaryDevice;
DirectCommandListManager RHIRoot::s_PrimaryCmdListManager;

void RHIRoot::Init(shared_ptr<Device> pDevice)
{
	s_PrimaryDevice = pDevice;
	s_PrimaryCmdListManager.Create(pDevice->Get());
}

PtrDevice RHIRoot::GetPrimaryD3DDevice()
{
	K3D_ASSERT(s_PrimaryDevice != nullptr);
	return s_PrimaryDevice->Get();
}

shared_ptr<Device>& RHIRoot::GetPrimaryDevice() 
{
	return s_PrimaryDevice;
}

DirectCommandListManager & RHIRoot::GetPrimaryCommandListManager()
{
	return s_PrimaryCmdListManager;
}

using namespace rapidjson;

/**
* @see Data/Test/GfxRootParameter.json
*/
void GfxRootParameter::Load(const char * rpFile)
{
	File f(rpFile);
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
		Log::Out("GfxRootParameter", item["Table"].GetString());
		D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY(item["ShaderVisibility"].GetInt());
		UINT type = item["DescriptorRangeType"].GetInt();
		UINT count = item["Count"].GetInt();
		UINT baseReg = item["BaseShaderReg"].GetInt();
		m_DescriptorRanges[i].Init((D3D12_DESCRIPTOR_RANGE_TYPE)type, count, baseReg);
		m_TableSlots[i].InitAsDescriptorTable(1, &m_DescriptorRanges[i], vis);
	}
}


NS_K3D_D3D12_END
