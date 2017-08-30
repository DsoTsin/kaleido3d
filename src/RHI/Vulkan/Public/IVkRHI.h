#ifndef __IVkRHI_h__
#define __IVkRHI_h__

#include <Core/Module.h>
#include <Core/Interface/IRHI.h>

#define DEPRECATED(...)

namespace k3d {
/**
 * Vulkan RHI interfaces
 */
class IVkRHI : public IModule
{
public:
  // Create Factory
  virtual void Initialize(const char* appName, bool debug) = 0;
  virtual NGFXFactoryRef GetFactory() = 0;
};
}

#if BUILD_STATIC_PLUGIN
K3D_STATIC_MODULE_DECLARE(RHI_Vulkan);
#endif

#endif