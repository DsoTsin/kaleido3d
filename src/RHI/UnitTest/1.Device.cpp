#include <Core/Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <Core/Interface/IRHI.h>
#include <Core/Module.h>
#include <RHI/Vulkan/Public/IVkRHI.h>
#include <RHI/Metal/Public/IMetalRHI.h>

using namespace k3d;

class UnitTestRHIDevice : public App
{
public:
	explicit UnitTestRHIDevice(String const & appName)
		: App(appName, 1920, 1080)
	{}

	UnitTestRHIDevice(String const & appName, uint32 width, uint32 height)
		: App(appName, width, height)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

private:

#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
	SharedPtr<IVkRHI> m_pRHI;
#else
	SharedPtr<IMetalRHI> m_pRHI;
#endif
  NGFXPtr<NGFXFactory> m_Factory;
	NGFXPtr<NGFXDevice> m_TestDevice;
};

K3D_APP_MAIN(UnitTestRHIDevice);

using namespace k3d;

bool UnitTestRHIDevice::OnInit()
{
	App::OnInit();
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
	m_pRHI = StaticPointerCast<IVkRHI>(ACQUIRE_PLUGIN(RHI_Vulkan));
	if (m_pRHI)
	{
		m_pRHI->Initialize("UnitTestRHIDevice", false);
		m_pRHI->Start();
    m_Factory = NGFXPtr<NGFXFactory>(m_pRHI->GetFactory());
    DynArray<NGFXDeviceRef> Devices;
    m_Factory->EnumDevices(Devices);
		m_TestDevice = NGFXPtr<NGFXDevice>(Devices[0]);
	}
#else
    auto pMtlRHI = StaticPointerCast<IMetalRHI>(ACQUIRE_PLUGIN(RHI_Metal));
    if(pMtlRHI)
    {
        pMtlRHI->Start();
        m_TestDevice = pMtlRHI->GetPrimaryDevice();
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
