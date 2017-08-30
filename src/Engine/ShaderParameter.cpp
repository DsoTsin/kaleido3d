#include "Kaleido3D.h"

#if K3DCOMPILER_MSVC
#pragma hdrstop
#endif

#include "ShaderParameter.h"

namespace k3d
{
	ShaderParameter::ShaderParameter()
	{
		for (auto & value : m_ValueID)
		{
			value = 0;
		}
	}

	ShaderParameter::~ShaderParameter()
	{
	}

	void ShaderParameter::SetName(const char * paramName)
	{
		m_ParameterName = { paramName };
	}

    String & ShaderParameter::GetName()
	{
		return m_ParameterName;
	}

	void ShaderParameter::InitParamData(void * pData)
	{
		for (int i = 0; i <= 4; i++)
			SetParamData(pData, i);
	}

	uint32 ShaderParameter::GetValueID(uint32 threadID)
	{
		assert(threadID >= 0);
		assert(threadID <= 4);
		return m_ValueID[threadID];
	}
}