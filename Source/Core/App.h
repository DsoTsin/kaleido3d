#ifndef __App_h__
#define __App_h__

#include <memory>
#include "Module.h"

#if K3DPLATFORM_OS_MAC
#import <Cocoa/Cocoa.h>
#elif K3DPLATFORM_OS_ANDROID
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <jni.h>
#include <android/asset_manager.h>
#include "Os.h"
namespace
{
	typedef void (*PFN_NativeActivity_OnCreate)(ANativeActivity*, void*, size_t);
}
namespace k3d
{
	class App;
}
namespace __android_internal
{
	const char *	getLibraryName(JNIEnv* env, jobject& objectActivityInfo);
	void* 			loadLibrary(const char* libraryName, JNIEnv* env, jobject& ObjectActivityInfo);
	k3d::App*		createApplication(ANativeActivity *, k3d::App *, void *savedState,
									  size_t savedStateSize);
	extern k3d::App*g_App;
}
#endif

#include "Message.h"

using std::shared_ptr;

namespace k3d
{
    class IWindow;
    
	enum class AppStatus : uint32
	{
		Destroyed,
		UnInitialized
	};

    class K3D_API App
	{
    public:
        explicit App(kString const & appName);
		App(kString const & appName, uint32 width, uint32 height);
        virtual ~App();

		virtual void InitWindow(void *param);
        virtual bool OnInit();
        virtual void OnUpdate();
        virtual void OnDestroy();
		virtual void OnProcess(Message & message) = 0;
        
		IWindow::Ptr  HostWindow() { return m_Window; }
		AppStatus Run();

    protected:
		IWindow::Ptr m_Window;
		const kString & m_AppName;
        
#if K3DPLATFORM_OS_MAC
        id m_AppDelegate;
#endif
    };

	class CORE_API Environment
	{
	private:
#if K3DPLATFORM_OS_ANDROID
		JNIEnv * 		env;
		jobject 		context;
		AAssetManager* 	assets;
#endif
	public:
		static const kString ENV_KEY_LOG_DIR;
		static const kString ENV_KEY_MODULE_DIR;
		static const kString ENV_KEY_APP_NAME;
#if K3DPLATFORM_OS_ANDROID
		AAssetManager* GetAssets() { return assets; }
		void __init__(JNIEnv* env, jobject instance, jstring appName, jstring path, AAssetManager* asset);
#endif
		Environment();
		~Environment();
		kString GetEnvValue(const kString & var);
	};

	extern CORE_API Environment* GetEnv();

	uint32 K3D_API RunApplication(App & app, kString const & appName);
}

#if K3DPLATFORM_OS_MAC
#import <AppKit/NSApplication.h>
@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    k3d::App * m_App;
}
@end
#define K3D_APP_MAIN(className) \
    int main(int argc, char *argv[]) \
    {\
        className app(#className); \
        return ::k3d::RunApplication(app, #className); \
    }
#elif K3DPLATFORM_OS_IOS
#import <UIKit/UIKit.h>
@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
    k3d::App * m_App;
}
@end

k3d::App * __entry_ios_main__();

#define K3D_APP_MAIN(className) \
int main(int argc, char *argv[]) \
{\
@autoreleasepool {\
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));\
}}\
k3d::App * __entry_ios_main__()\
{\
return new className(#className);\
}

#elif K3DPLATFORM_OS_WIN
#define K3D_APP_MAIN(className) \
	int WinMain(HINSTANCE,HINSTANCE,LPSTR CmdLine,int) \
	{ \
		className app(L## #className); \
		return ::k3d::RunApplication(app, L## #className); \
	}
#elif K3DPLATFORM_OS_ANDROID
extern k3d::App* ACreateApp(ANativeWindow*,int,int);
#define K3D_APP_MAIN(clasName) \
k3d::App* ACreateApp(ANativeWindow*window,int width,int height) {\
	clasName* test = new clasName(#clasName, width, height);\
	test->InitWindow(window);\
	return test;\
}
#endif

#endif
