#include "UnitTestRHIDevice.h"
#include <Core/Module.h>
#include <RHI/Vulkan/Public/IVkRHI.h>
#include <RHI/Metal/Public/IMetalRHI.h>

K3D_APP_MAIN(UnitTestRHIDevice);

using namespace rhi;
using namespace k3d;

bool UnitTestRHIDevice::OnInit()
{
	App::OnInit();
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
	IVkRHI* pVkRHI = (IVkRHI*)ACQUIRE_PLUGIN(RHI_Vulkan);
	if (pVkRHI)
	{
		pVkRHI->Initialize("UnitTestRHIDevice", false);
		pVkRHI->Start();
		m_TestDevice = pVkRHI->GetPrimaryDevice();
	}
#else
    auto pMtlRHI = ACQUIRE_PLUGIN(RHI_Metal);
    if(pMtlRHI)
    {
        pMtlRHI->Start();
        m_TestDevice = ((IMetalRHI*)pMtlRHI)->GetPrimaryDevice();
    }
#endif
	return true;
}

void UnitTestRHIDevice::OnDestroy()
{
	App::OnDestroy();
}

void UnitTestRHIDevice::OnProcess(Message & msg)
{
}
