#ifndef __App_h__
#define __App_h__

#include <memory>

#if K3DPLATFORM_OS_MAC
#import <Cocoa/Cocoa.h>
#endif
#include "Message.h"

using std::shared_ptr;

namespace k3d {

    class Window;
    
	enum class AppStatus : uint32 {
		Destroyed,
		UnInitialized
	};

    class App {
    public:
        explicit App(kString const & appName);
		App(kString const & appName, uint32 width, uint32 height);
        virtual ~App();
        
        virtual bool OnInit();
        virtual void OnUpdate();
        virtual void OnDestroy();
		virtual void OnProcess(Message & message) = 0;
        
		Window * HostWindow() { return m_Window; }
		AppStatus Run();

    protected:
        Window * m_Window;
		const kString & m_AppName;
    };

	namespace Core {
		uint32 RunApplication(App & app, kString const & appName);
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

#endif