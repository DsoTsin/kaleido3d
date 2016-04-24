#include "Kaleido3D.h"
#include "App.h"
#include "ModuleCore.h"
#include "Message.h"

namespace k3d {

	App::App(kString const & appName)
		: m_Window(nullptr)
		, m_AppName(appName)
	{
		m_Window = MakePlatformWindow(appName.c_str(), 1920, 1080);
	}

	App::App(kString const & appName, uint32 width, uint32 height)
		: m_Window(nullptr)
		, m_AppName(appName)
	{
		m_Window = MakePlatformWindow(appName.c_str(), width, height);
	}

	App::~App()
	{
	}
	
	bool App::OnInit()
	{
		if (!m_Window)
			return false;

		m_Window->Show();
		return true;
	}

	void App::OnUpdate()
	{
	}

	void App::OnDestroy()
	{
	}

	AppStatus App::Run() {
		if (!m_Window) 
			return AppStatus::UnInitialized;

		Message msg;
		while ( m_Window->IsOpen() ) {
			bool isQuit = false;
			while (m_Window->PollMessage(msg)) {
				OnProcess(msg);
				if (msg.type == Message::Closed) {
					isQuit = true;
					break;
				}
			}
			
			if (isQuit)
				break;

			OnUpdate();
		}
		OnDestroy();
		Log::Get().Destroy();
		return AppStatus::Destroyed;
	}

	namespace Core
	{

		uint32 RunApplication(App & app, kString const & appName)
		{
			Log::Get();
			app.OnInit();
			return (uint32) app.Run();
		}

	}

	void App::OnWindowCreated(void * param) {
#if K3DPLATFORM_OS_ANDROID
		if(!m_Window)
		{
			m_Window = MakeAndroidWindow((ANativeWindow*)param);
		}
#endif
	}
}
