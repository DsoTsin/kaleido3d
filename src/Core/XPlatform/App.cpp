#include "CoreMinimal.h"
#include "App.h"
#include "Message.h"

namespace k3d
{
	static void RegisterApp();
    
	App::App(String const & appName)
		: m_Window(nullptr)
		, m_AppName(appName)
	{
        RegisterApp();
		m_Window = MakePlatformWindow(appName.CStr(), 1920, 1080);
	}

	App::App(String const & appName, U32 width, U32 height)
		: m_Window(nullptr)
		, m_AppName(appName)
    {
        RegisterApp();
		m_Window = MakePlatformWindow(appName.CStr(), width, height);
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
		KLOG(Info, App, "Super::OnDestroy..");
	}

	AppStatus App::Run() {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_LINUX
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
#else
        // TODO: add event handler
//        while(true) {
			Message msg;
			OnProcess(msg);
			OnUpdate();
//			usleep(16000);
//		}
#endif
		return AppStatus::Destroyed;
	}

#if K3DPLATFORM_OS_ANDROID
	void Environment::__init__(JNIEnv *_env, jobject _context, jstring appName, jstring path, AAssetManager *asset)
	{
		this->env 		= _env;
		this->context 	= _context;
		this->assets 	= asset;
		jboolean isCopy;
		//lEnvVars[Environment::ENV_KEY_MODULE_DIR] = env->GetStringUTFChars(path, &isCopy);
		//lEnvVars[Environment::ENV_KEY_APP_NAME] = env->GetStringUTFChars(appName, &isCopy);
	}
#endif

	Environment& GetEnv()
	{
        static Environment gEnv;
		return gEnv;
	}

		void RegisterApp()
		{
#if K3DPLATFORM_OS_MAC
			/// Init Cocoa Application
			[NSApplication sharedApplication];
			//        id m_AppDelegate = [AppDelegate sharedDelegate];
			//        [NSApp setDelegate:m_AppDelegate];
			[NSApp setActivationPolicy : NSApplicationActivationPolicyRegular];
			[NSApp activateIgnoringOtherApps : YES];
			[NSApp finishLaunching];

			[[NSNotificationCenter defaultCenter]
				postNotificationName:NSApplicationWillFinishLaunchingNotification
				object : NSApp];

			[[NSNotificationCenter defaultCenter]
				postNotificationName:NSApplicationDidFinishLaunchingNotification
				object : NSApp];

			/// Init App Menu
			id quitMenuItem = [NSMenuItem new];
			[quitMenuItem
				initWithTitle : @"Quit"
				action : @selector(terminate : )
						 keyEquivalent:@"q"];

			id appMenu = [NSMenu new];
			[appMenu addItem : quitMenuItem];

			id appMenuItem = [NSMenuItem new];
			[appMenuItem setSubmenu : appMenu];

			id menubar = [[NSMenu new] autorelease];
			[menubar addItem : appMenuItem];
			[NSApp setMainMenu : menubar];
            
#elif K3DPLATFORM_OS_WIN/*
            char szFileName[MAX_PATH];
            GetModuleFileNameA(NULL, szFileName, MAX_PATH);
            kString exeFilePath = szFileName;
            size_t pos = exeFilePath.find_last_of(KT("\\"));
            size_t nPos = exeFilePath.find_last_of(KT(".exe"));
            if (pos != kString::npos)
            {
                lEnvVars[Environment::ENV_KEY_MODULE_DIR] = exeFilePath.substr(0, pos);
                lEnvVars[Environment::ENV_KEY_APP_NAME] = exeFilePath.substr(pos+1, nPos - pos - 4);
            }
            else
            {
                lEnvVars[Environment::ENV_KEY_MODULE_DIR] = "./";
                lEnvVars[Environment::ENV_KEY_APP_NAME] = exeFilePath.substr(0, nPos - 4);
            }
            lEnvVars[Environment::ENV_KEY_LOG_DIR] = lEnvVars[Environment::ENV_KEY_MODULE_DIR];*/
#elif K3DPLATFORM_OS_IOS
            lEnvVars[Environment::ENV_KEY_MODULE_DIR] = KT("./");
            //lEnvVars[Environment::ENV_KEY_LOG_DIR] = KT("./");
#endif
		}

		U32 RunApplication(App & app, String const & appName)
		{
			app.OnInit();
			return (U32) app.Run();
		}


	void App::InitWindow(void *param) {
#if K3DPLATFORM_OS_ANDROID
		if(!m_Window)
		{
			m_Window = MakeAndroidWindow((ANativeWindow*)param);
		}
#endif
	}
}
