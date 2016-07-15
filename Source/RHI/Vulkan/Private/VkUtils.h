#ifndef __VkUtils_h__
#define __VkUtils_h__

#include "Core/Utils/farmhash.h"

K3D_VK_BEGIN

extern K3D_API VkDeviceSize				CalcAlignedOffset(VkDeviceSize offset, VkDeviceSize align);
extern K3D_API VkImageAspectFlags		DetermineAspectMask(VkFormat format);
extern K3D_API std::string				ErrorString(VkResult errorCode);
extern K3D_API VkBool32					GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat * depthFormat);
extern K3D_API void						SetupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack);
extern K3D_API void						FreeDebugCallback(VkInstance instance);
extern K3D_API rhi::PipelineLayoutKey	HashPipelineLayoutDesc(rhi::PipelineLayoutDesc const& desc);

class CommandAllocator;
using PtrCmdAlloc = std::shared_ptr<CommandAllocator>;

class CommandAllocator : public DeviceChild
{
public:
	static PtrCmdAlloc			CreateAllocator(
									uint32 queueFamilyIndex, 
									bool transient, 
									Device::Ptr device);

		~CommandAllocator();

	VkCommandPool				GetCommandPool() const { return m_Pool; }
protected:
	void						Initialize();
	void						Destroy();

private:
	CommandAllocator(uint32 queueFamilyIndex, bool transient, Device::Ptr device);

	VkCommandPool	m_Pool;
	bool			m_Transient;
	uint32			m_FamilyIndex;
};

class Semaphore : public DeviceChild
{
public:
	Semaphore(Device::Ptr pDevice, VkSemaphoreCreateInfo const & info = SemaphoreCreateInfo::Create())
		: DeviceChild(pDevice)
	{
		K3D_VK_VERIFY(vkCreateSemaphore(GetRawDevice(), &info, nullptr, &m_Semaphore));
		VKLOG(Info, "Semaphore Created. (0x%0x).", m_Semaphore);
	}
	~Semaphore() { vkDestroySemaphore(GetRawDevice(), m_Semaphore, nullptr); }

	VkSemaphore	GetNativeHandle() const { return m_Semaphore; }

private:
	friend class SwapChain;
	friend class CommandContext;

	VkSemaphore m_Semaphore;
};

K3D_VK_END

#endif // !__VkUtils_h__
