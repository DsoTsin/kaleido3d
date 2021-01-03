/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __App_h__
#define __App_h__

#include <memory>

#if K3DPLATFORM_OS_MAC
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif
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

namespace k3d
{
	class IWindow;
    
	enum class AppStatus : U32
	{
		Destroyed,
		UnInitialized
	};

	class K3D_CORE_API App
	{
	public:
		explicit App(String const & appName);
		App(String const & appName, U32 width, U32 height);
		App(int argc, char** argv, String const & appName);
		App(int argc, char** argv, String const & appName, U32 width, U32 height);

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
		const String & m_AppName;
        
#if K3DPLATFORM_OS_MAC
#ifdef __OBJC__
        id m_AppDelegate;
#endif
#endif
    };

	class K3D_CORE_API Environment
	{
	private:
#if K3DPLATFORM_OS_ANDROID
		JNIEnv * 		env;
		jobject 		context;
		AAssetManager* 	assets;
#endif
	public:
#if K3DPLATFORM_OS_ANDROID
		AAssetManager* GetAssets() { return assets; }
		void __init__(JNIEnv* env, jobject instance, jstring appName, jstring path, AAssetManager* asset);
#endif
		Environment();
		~Environment();

        String GetLogDir() const;
        String GetModuleDir() const;
        String GetDataDir() const;
        String GetInstanceName() const;
    
    private:
        struct EnvironmentImpl* d;
	};

	extern K3D_CORE_API Environment& GetEnv();

	U32 K3D_CORE_API RunApplication(App & app, String const & appName);
}

#if K3DPLATFORM_OS_MAC
#ifdef __OBJC__
#import <AppKit/NSApplication.h>
@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    k3d::App * m_App;
}
@end
#endif
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
#ifdef __cplusplus
extern "C" {
#endif
	K3D_CORE_API char** CommandLineToArgvA(const char* lpCmdLine, int *pNumArgs);
#ifdef __cplusplus
}
#endif
#define K3D_APP_MAIN(className) \
	int WinMain(HINSTANCE,HINSTANCE,LPSTR CmdLine,int) \
	{	\
		className app(#className); \
		return ::k3d::RunApplication(app, #className); \
	}
#define K3D_APP_MAIN_ARG(className) \
	int WinMain(HINSTANCE,HINSTANCE,LPSTR CmdLine,int) \
	{	int argc = 0; char** argv = nullptr;\
		argv = CommandLineToArgvA(CmdLine, &argc); \
		className app(argc, argv, #className); \
		return ::k3d::RunApplication(app, #className); \
	}
#elif K3DPLATFORM_OS_ANDROID
extern k3d::App* ACreateApp(ANativeWindow*,int,int);
#define K3D_APP_MAIN(clasName) \
k3d::App* ACreateApp(ANativeWindow*window,int width,int height) {\
	clasName* test = new clasName(#clasName, width, height);\
	test->InitWindow(window);\
	return test;\
}
#elif K3DPLATFORM_OS_LINUX
#define K3D_APP_MAIN(className) \
	int main(int argc, char *argv[]) \
    {\
        className app(#className); \
        return ::k3d::RunApplication(app, #className); \
    }

#define K3D_APP_MAIN_ARG(className) 
#endif

#endif
