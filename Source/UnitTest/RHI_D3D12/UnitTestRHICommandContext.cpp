#include "UnitTestRHICommandContext.h"
#include "RHI/D3D12/Public/D3D12RHI.h"

using namespace rhi;

bool UnitTestRHICommandContext::OnInit()
{
	if(!App::OnInit())
		return false; 
	if (!InitDevice())
		return false;

	InitCommandContext();
	InitSwapChain();
	return true;
}

void UnitTestRHICommandContext::OnDestroy()
{
	if (m_TestDevice)
	{
		delete m_TestDevice;
		m_TestDevice = nullptr;
	}
	App::OnDestroy();
}

void UnitTestRHICommandContext::OnProcess(Message & msg)
{

}

void UnitTestRHICommandContext::OnUpdate()
{
	//m_TestCommandListManager.WaitForFence(1);
}

bool UnitTestRHICommandContext::InitDevice()
{
	IDeviceAdapter ** list = nullptr;
	uint32 adapterNum = 0;
	EnumAllDeviceAdapter(list, &adapterNum);
	if (list == nullptr)
	{
		return false;
	}

	m_TestDevice = new Device;
	IDevice::Result Result = m_TestDevice->Create(list[0], true);
	if (Result != IDevice::DeviceFound)
	{
		return false;
	}
	return true;
}

void UnitTestRHICommandContext::InitCommandContext()
{
	m_TestCommandListManager.Create(static_cast<Device*>(m_TestDevice)->Get());
	m_TestCommandContext = m_TestDevice->NewCommandContext(rhi::ECommandType::ECMD_Graphics);
}

void UnitTestRHICommandContext::InitSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 3;
	swapChainDesc.BufferDesc.Width = HostWindow()->Width();
	swapChainDesc.BufferDesc.Height = HostWindow()->Height();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = (HWND)HostWindow()->GetHandle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;

	ComPtr<IDXGISwapChain> swapChain;
	ThrowIfFailed(static_cast<Device*>(m_TestDevice)->GetDXGIFactory()->CreateSwapChain(
		m_TestCommandListManager.GetCommandQueue(),		// Swap chain needs the queue so that it can force a flush on it.
		&swapChainDesc,
		swapChain.GetAddressOf()
		));

	ThrowIfFailed(swapChain.As(&m_SwapChain));
}
