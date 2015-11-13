#include "Kaleido3D.h"
#include "../App.h"
#include "../ModuleCore.h"

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

namespace k3d {
    App::App(kString const & appName)
    : m_Window(nullptr)
    , m_AppName(appName){
        
    }
    
    App::App(kString const & appName, uint32 width, uint32 height)
    : m_Window(nullptr)
    , m_AppName(appName)
    {
    }
    
    App::~App() {
        
    }
    
    bool App::OnInit() {
        AssetManager & asset = Core::GetAssetManager();
        NSString* path = [[NSBundle mainBundle] resourcePath];
        asset.AddSearchPath([path UTF8String]);
        
        // TODO
        return true;
    }
    
    void App::OnUpdate() {
        
    }
    
    void App::OnDestroy() {
        
    }
}