#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>

using namespace k3d;

class CommandBundleApp : public App {
public:
	explicit CommandBundleApp(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	bool OnInit() override;

	void OnProcess(Message & msg) override;

private:


};

bool CommandBundleApp::OnInit() 
{
	//ModuleD3D12::InitDevice();
	//ModuleD3D12::InitSurface(HostWindow());
	return App::OnInit();
}

void CommandBundleApp::OnProcess(Message & msg)
{
}

K3D_APP_MAIN(CommandBundleApp);