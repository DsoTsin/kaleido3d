#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Base/vulkanswapchain.hpp"

K3D_VK_BEGIN

RenderViewport::RenderViewport()
	: m_pSwapChain(new VulkanSwapChain)
{}

RenderViewport::~RenderViewport()
{}

bool RenderViewport::InitViewport(	void *windowHandle, rhi::IDevice * pDevice,
									uint32 width, uint32 height,
									rhi::EPixelFormat rtFmt)
{
	Device * device = static_cast<Device*>(pDevice);
	m_pSwapChain->init(RHIRoot::GetInstance(), *device->GetRawPhysicDevice(), device->GetRawDevice());
	m_pSwapChain->initSwapChain(windowHandle, windowHandle);
	VkCommandBuffer cmdbuf;
	m_pSwapChain->setup(cmdbuf, &width, &height);
	return true;
}

bool RenderViewport::Present(bool vSync)
{
	return false;
}

K3D_VK_END