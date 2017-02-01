#include "Kaleido3D.h"
#include "App.h"
#include "Message.h"
#include "LogUtil.h"

namespace k3d
{
	static void RegisterApp();
    
	App::App(kString const & appName)
		: m_Window(nullptr)
		, m_AppName(appName)
	{
        RegisterApp();
		m_Window = MakePlatformWindow(appName.c_str(), 1920, 1080);
	}

	App::App(kString const & appName, uint32 width, uint32 height)
		: m_Window(nullptr)
		, m_AppName(appName)
    {
        RegisterApp();
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
		KLOG(Info, App, "Super::OnDestroy..");
	}

	AppStatus App::Run() {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_MAC
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

	const kString Environment::ENV_KEY_LOG_DIR		= KT("Log_Dir");
	const kString Environment::ENV_KEY_MODULE_DIR	= KT("Module_Dir");
	const kString Environment::ENV_KEY_APP_NAME 	= KT("App_Name");

	Environment::Environment() { }
	Environment::~Environment() { }

	std::map<kString, kString> lEnvVars;

#if K3DPLATFORM_OS_ANDROID
	void Environment::__init__(JNIEnv *_env, jobject _context, jstring appName, jstring path, AAssetManager *asset)
	{
		this->env 		= _env;
		this->context 	= _context;
		this->assets 	= asset;
		jboolean isCopy;
		lEnvVars[Environment::ENV_KEY_MODULE_DIR] = env->GetStringUTFChars(path, &isCopy);
		lEnvVars[Environment::ENV_KEY_APP_NAME] = env->GetStringUTFChars(appName, &isCopy);
	}
#endif

	Environment gEnv;

	Environment *GetEnv()
	{
		return &gEnv;
	}

	kString Environment::GetEnvValue(const kString &var)
	{
		if (lEnvVars.find(var) == lEnvVars.end())
		{
			return KT("");
		}
		return lEnvVars[var];
	}

#if K3DPLATFORM_OS_WIN
		std::wstring s_ModulePath = L"";
		void GetModulePath(IN HMODULE hModule, IN OUT LPWSTR lpPathBuffer, IN DWORD nSize);

		App* Init(kString const & appName)
		{
			if (s_ModulePath == KT("")) 
			{
				WCHAR path[2048] = { 0 };
				GetModulePath(NULL, path, 2048);
				s_ModulePath = path;
			}
			return nullptr;
		}
#endif

#if K3DPLATFORM_OS_WIN
		void GetModulePath(IN HMODULE hModule, IN OUT LPWSTR lpPathBuffer, IN DWORD nSize) {
			wchar_t *p, *q;
			p = (wchar_t*)calloc(nSize, sizeof(char));
			GetModuleFileNameW(hModule, p, nSize);
			q = p;
			while (wcschr(q, '\\'))
			{
				q = wcschr(q, '\\');
				q++;
			}
			*--q = '\0';
			wcscpy_s(lpPathBuffer, nSize, p);
		}

		kString GetExecutablePath() {
			if (s_ModulePath.empty()) {
				WCHAR path[2048] = { 0 };
				GetModulePath(NULL, path, 2048);
				s_ModulePath = path;
			}
			return s_ModulePath;
			//            NSString * pPath = [[NSBundle mainBundle] executablePath];
			//            return [pPath utf8String];
		}
#endif

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
            
            NSBundle* bundle = [NSBundle mainBundle];
            NSString* path = [[NSString alloc] initWithString:[bundle builtInPlugInsPath]];
            lEnvVars[Environment::ENV_KEY_MODULE_DIR] = [path UTF8String];
            [path release];
            path = [[NSString alloc] initWithString:[bundle bundlePath]];
            lEnvVars[Environment::ENV_KEY_LOG_DIR] = kString([path UTF8String]) + "/Logs";
            [path release];
            path = [[NSString alloc] initWithString:[bundle executablePath]];
            kString exePath = [path UTF8String];
            size_t sepPos = exePath.find_last_of("/");
            lEnvVars[Environment::ENV_KEY_APP_NAME] = exePath.substr(sepPos+1, exePath.length()-sepPos);
            [path release];
#elif K3DPLATFORM_OS_WIN
			wchar_t szFileName[MAX_PATH];
			GetModuleFileNameW(NULL, szFileName, MAX_PATH);
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
				lEnvVars[Environment::ENV_KEY_MODULE_DIR] = KT("./");
				lEnvVars[Environment::ENV_KEY_APP_NAME] = exeFilePath.substr(0, nPos - 4);
			}
			lEnvVars[Environment::ENV_KEY_LOG_DIR] = lEnvVars[Environment::ENV_KEY_MODULE_DIR];
#elif K3DPLATFORM_OS_IOS
            lEnvVars[Environment::ENV_KEY_MODULE_DIR] = KT("./");
            //lEnvVars[Environment::ENV_KEY_LOG_DIR] = KT("./");
#endif
		}

		uint32 RunApplication(App & app, kString const & appName)
		{
			app.OnInit();
			return (uint32) app.Run();
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
