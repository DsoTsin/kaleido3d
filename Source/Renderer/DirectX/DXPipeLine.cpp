#include "Kaleido3D.h"
#include "DXPipeLine.h"

#include <Core/LogUtil.h>

namespace k3d
{
	using namespace DxRef;

	DXPipeLine::DXPipeLine(DXDevice & device)
	{
		m_pDevice			= device.Device();
		m_pImmediateContext = device.ImmediateContext();
	}

	DXPipeLine::~DXPipeLine()
	{
	}
	
	DevContextPtr DXPipeLine::ObtainDeferredContext()
	{
		DevContextPtr pDeferredContext;
		if (m_pDevice)
		{
			if (FAILED(m_pDevice->CreateDeferredContext(0, pDeferredContext.GetInitReference())))
			{
				Debug::Out("DXPipeLine", "error, m_pDevice->CreateDeferredContext");
				Log::Error("DXPipeLine::ObtainDeferredContext failed to create deferred context for multithreading..");
			}
		}
		return pDeferredContext;
	}

	CmdListPtr DXPipeLine::ObtainCommandList(DevContextPtr const & deferredContext)
	{
		CmdListPtr commandList;
		if (deferredContext && deferredContext != m_pImmediateContext)
		{
			if (FAILED(deferredContext->FinishCommandList(FALSE, commandList.GetInitReference())))
			{
				Debug::Out("DXPipeLine", "error, deferredContext->FinishCommandList");
			}
		}
		return commandList;
	}

	void DXPipeLine::ExecuteCommandList(CmdListPtr const & commandList)
	{
		if (m_pImmediateContext && commandList)
		{
			m_pImmediateContext->ExecuteCommandList(commandList, TRUE);
		}
	}
}