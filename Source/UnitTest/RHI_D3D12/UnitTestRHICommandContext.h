#pragma once
#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
#include <RHI/D3D12/DXCommon.h>
#include <RHI/D3D12/Private/D3D12CommandListManager.h>

using namespace k3d;
using namespace k3d::d3d12;

constexpr int frame_count = 3;

class UnitTestRHICommandContext : public App
{
public:
	explicit UnitTestRHICommandContext(kString const & appName)
		: App(appName, 1920, 1080)
		, m_TestDevice(nullptr)
		, m_TestCommandContext(nullptr)
		, m_TestPipelineState(nullptr)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;
	void OnUpdate() override;
	void OnSizeChanged(int width, int height);

	bool InitDevice();
	void InitCommandContext();
	void InitSwapChain();
	void InitPipeLineState();
	void InitRenderResource();

	void RenderFrame();

private:

	rhi::IDevice *			m_TestDevice;
	rhi::ICommandContext *	m_TestCommandContext;
	rhi::IPipelineLayout *	m_TestPipelineLayout;
	rhi::IPipelineStateObject *	m_TestPipelineState;
	rhi::VertexBufferView  m_TestVertexBufferView;
	DirectCommandListManager		m_TestCommandListManager;
	PtrSwapChain			m_SwapChain;
	rhi::Viewport			m_Viewport;
	uint32					m_FrameIndex;
	PtrResource				m_RenderTargets[frame_count];


	float					m_AspectRatio;
};