#include "Kaleido3D.h"
#include <Core/App.h>
#include <Core/AssetManager.h>

@implementation AppDelegate
- (BOOL)application:(NSNotification*)notification didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    K3D_UNUSED(notification);
    m_App = __entry_ios_main__();
    [self performSelector:@selector(postFinishLaunch) withObject:nil afterDelay:0.0];
    return YES;
}

- (void)applicationWillResignActive:(NSNotification*)notification
{
    K3D_UNUSED(notification);
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    if(m_App)
    {
        m_App->OnInit();
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onFrame) name:@"RHIRender" object:nil];
    }
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    if(m_App)
    {
        m_App->OnDestroy();
        [[NSNotificationCenter defaultCenter] removeObserver:self];
    }
 }

- (void) onFrame
{
    if(m_App)
    {
        k3d::Message msg;
        m_App->OnProcess(msg);
    }
}

- (void)postFinishLaunch
{
}
@end
