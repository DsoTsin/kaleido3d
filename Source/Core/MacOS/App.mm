#include "Kaleido3D.h"
#include "../App.h"
#include "../ModuleCore.h"

@implementation AppDelegate

- (BOOL)application:(NSNotification*)notification didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    K3D_UNUSED(notification);
    m_App = k3d::Core::Init();
    
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

namespace k3d {
    App::App()
    : m_Window(nullptr) {
        
    }
    
    App::~App() {
        
    }
    
    void App::OnInit() {
        Core::Init();
        AssetManager & asset = Core::GetAssetManager();
        NSString* path = [[NSBundle mainBundle] resourcePath];
        asset.AddSearchPath([path UTF8String]);
        
        // TODO
    }
    
    void App::OnUpdate() {
        
    }
    
    void App::OnDestroy() {
        
    }
}