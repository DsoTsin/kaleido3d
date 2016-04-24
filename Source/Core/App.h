#ifndef __App_h__
#define __App_h__

#include <memory>

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

#include <Core/Thread/Thread.h>
#include <Core/Thread/ConditionVariable.h>

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
}
#endif

#include "Message.h"

using std::shared_ptr;

namespace k3d {

    class IWindow;
    
	enum class AppStatus : uint32 {
		Destroyed,
		UnInitialized
	};

    class K3D_API App {
    public:
        explicit App(kString const & appName);
		App(kString const & appName, uint32 width, uint32 height);
        virtual ~App();

		virtual void OnWindowCreated(void * param);

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
#elif K3DPLATFORM_OS_ANDROID
	public:

		struct android_poll_source {
			// The identifier of this source.  May be LOOPER_ID_MAIN or
			// LOOPER_ID_INPUT.
			int32_t id;

			// The android_app this ident is associated with.
			App* app;

			// Function to call to perform the standard processing of data from
			// this source.
			void (*process)(App* app, android_poll_source* source);
		};

		void* userData;
		void (*onAppCmd)(App* app, int32_t cmd);

		int32_t (*onInputEvent)(App* app, AInputEvent* event);



		ANativeActivity* 	m_Activity;
		AConfiguration* 	m_Config;
		void*				m_SavedState;
		size_t 				m_SavedStateSize;
		ALooper* 			m_Looper;
		AInputQueue* 		m_InputQueue;

		//ANativeWindow* window;
		ARect contentRect;

		Concurrency::Thread	* 			m_Thread;
		Concurrency::Mutex				m_Mutex;
		Concurrency::ConditionVariable 	m_CondiVar;

		int 			m_MsgRead;
		int 			m_MsgWrite;

		android_poll_source cmdPollSource;
		android_poll_source inputPollSource;

		int 			m_IsRunning;
		int stateSaved;
		int destroyed;
		int redrawNeeded;
		AInputQueue*	m_PendingInputQueue;
		ANativeWindow*	m_PendingWindow;
		ARect pendingContentRect;

		void 			WriteCmd(int8_t cmd);
		int8_t 			ReadCmd();

		void 			ProcessCmd();
		void 			PreExecCmd(int8_t cmd);
		void 			PostExecCmd(int8_t cmd);
		void ProcessInput(Message &msg);

		static void 	OnStart(ANativeActivity* activity);
		static void 	OnDestroyed(ANativeActivity * nativeActivity);
		static void 	OnResume(ANativeActivity* activity);
		static void 	OnPause(ANativeActivity* activity);
		static void 	OnStop(ANativeActivity* activity);

		static void* 	OnSaveInstanceState(ANativeActivity* activity, size_t* outLen);
		static void 	OnNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window);
		static void 	OnNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window);
		static void 	OnWindowFocusChanged(ANativeActivity* activity, int focused);

		static void 	OnInputQueueCreated(ANativeActivity *activity, AInputQueue *queue);
		static void 	OnInputQueueDestroyed(ANativeActivity *activity, AInputQueue *queue);

#endif
        
    };

	namespace Core
    {
		uint32 K3D_API RunApplication(App & app, kString const & appName);
	}
}

#if K3DPLATFORM_OS_MAC
@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    k3d::App * m_App;
}
@end

#define K3D_APP_MAIN \
    int main(int argc, char *argv[]) \
    {\
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];\
        int retVal = UIApplicationMain(argc, argv, nil, @"AppDelegate");\
        [pool release]; \
        return retVal; \
    }
#endif

#if K3DPLATFORM_OS_WIN
#define K3D_APP_MAIN(className) \
	int WinMain(HINSTANCE,HINSTANCE,LPSTR CmdLine,int) \
	{ \
		className app(L## #className); \
		return ::k3d::Core::RunApplication(app, L## #className); \
	}
#endif

#if K3DPLATFORM_OS_ANDROID
#define K3D_APP_MAIN(className) \
extern "C" void k3d_Core_App_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)\
{\
	activity->callbacks->onDestroy = k3d::App::OnDestroyed;\
	activity->callbacks->onStart = k3d::App::OnStart;\
	activity->callbacks->onResume = k3d::App::OnResume;\
	activity->callbacks->onSaveInstanceState = k3d::App::OnSaveInstanceState;\
	activity->callbacks->onPause = k3d::App::OnPause;\
	activity->callbacks->onStop = k3d::App::OnStop;\
	activity->callbacks->onConfigurationChanged = NULL;\
	activity->callbacks->onLowMemory = NULL;\
	activity->callbacks->onWindowFocusChanged = k3d::App::OnWindowFocusChanged;\
	activity->callbacks->onNativeWindowCreated = k3d::App::OnNativeWindowCreated;\
	activity->callbacks->onNativeWindowDestroyed = k3d::App::OnNativeWindowDestroyed;\
	activity->callbacks->onInputQueueCreated = k3d::App::OnInputQueueCreated;\
	activity->callbacks->onInputQueueDestroyed = k3d::App::OnInputQueueDestroyed;\
	className *app = new className(#className);\
	activity->instance = __android_internal::createApplication(activity, app, savedState, savedStateSize);\
}
#endif

#endif