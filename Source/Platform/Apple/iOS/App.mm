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
    }
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    if(m_App)
    {
        m_App->OnDestroy();
    }
 }

- (void)postFinishLaunch
{
}
@end
