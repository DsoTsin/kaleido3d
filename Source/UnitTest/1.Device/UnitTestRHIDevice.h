#pragma once
#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <Interface/IRHI.h>

using namespace k3d;

class UnitTestRHIDevice : public App
{
public:
	explicit UnitTestRHIDevice(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	UnitTestRHIDevice(kString const & appName, uint32 width, uint32 height)
		: App(appName, width, height)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

private:

	rhi::DeviceRef m_TestDevice;
};