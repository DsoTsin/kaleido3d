#include "Kaleido3D.h"
#include <Core/App.h>
#include <Core/AssetManager.h>

@implementation AppDelegate
- (BOOL)application:(NSNotification*)notification didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    K3D_UNUSED(notification);
    m_App = nullptr;
    
    [self performSelector:@selector(postFinishLaunch) withObject:nil afterDelay:0.0];
    return YES;
}

- (void)applicationWillResignActive:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    m_App->OnInit();
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    m_App->OnInit();
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    m_App->OnDestroy();
 }

- (void)postFinishLaunch
{

}
@end
