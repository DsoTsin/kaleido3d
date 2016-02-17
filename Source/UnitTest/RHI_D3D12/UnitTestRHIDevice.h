#pragma once
#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>

using namespace k3d;

class UnitTestRHIDevice : public App
{
public:
	explicit UnitTestRHIDevice(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

private:

	rhi::IDevice * m_TestDevice;
};