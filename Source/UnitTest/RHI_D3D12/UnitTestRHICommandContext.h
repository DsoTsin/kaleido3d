#pragma once
#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
#include <RHI/D3D12/DXCommon.h>
#include <RHI/D3D12/Private/D3D12CommandListManager.h>

using namespace k3d;
using namespace k3d::d3d12;

class UnitTestRHICommandContext : public App
{
public:
	explicit UnitTestRHICommandContext(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;
	void OnUpdate() override;

	bool InitDevice();
	void InitCommandContext();
	void InitSwapChain();


private:

	rhi::IDevice *			m_TestDevice;
	rhi::ICommandContext *	m_TestCommandContext;
	CommandListManager		m_TestCommandListManager;
	PtrSwapChain			m_SwapChain;
};