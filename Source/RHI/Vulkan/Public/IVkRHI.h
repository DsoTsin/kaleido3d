#ifndef __IVkRHI_h__
#define __IVkRHI_h__
#include <Core/Module.h>
#include <Interface/IRHI.h>

namespace k3d
{
	/**
	 * Vulkan RHI interfaces
	 */
	class IVkRHI : public IModule
	{
	public:
		
		virtual void			Initialize(const char* appName, bool debug) = 0;
		virtual rhi::DeviceRef	GetPrimaryDevice() = 0;
		virtual uint32			GetDeviceCount() = 0;
		virtual rhi::DeviceRef	GetDeviceById(uint32 id) = 0;
		virtual void			Destroy() = 0;

	};
}

#if	BUILD_STATIC_PLUGIN
K3D_STATIC_MODULE_DECLARE(RHI_Vulkan);
#endif

#endif