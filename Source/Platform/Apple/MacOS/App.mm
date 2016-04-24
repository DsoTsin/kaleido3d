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
        /// Init Cocoa Application
        [NSApplication sharedApplication];
        
        id m_AppDelegate = [AppDelegate sharedDelegate];
        [NSApp setDelegate:m_AppDelegate];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp finishLaunching];
        
        [[NSNotificationCenter defaultCenter]
         postNotificationName:NSApplicationWillFinishLaunchingNotification
         object:NSApp];
        
        [[NSNotificationCenter defaultCenter]
         postNotificationName:NSApplicationDidFinishLaunchingNotification
         object:NSApp];
        
        /// Init App Menu
        id quitMenuItem = [NSMenuItem new];
        [quitMenuItem
         initWithTitle:@"Quit"
         action:@selector(terminate:)
         keyEquivalent:@"q"];
        
        id appMenu = [NSMenu new];
        [appMenu addItem:quitMenuItem];
        
        id appMenuItem = [NSMenuItem new];
        [appMenuItem setSubmenu:appMenu];
        
        id menubar = [[NSMenu new] autorelease];
        [menubar addItem:appMenuItem];
        [NSApp setMainMenu:menubar];
        
        /// Init Asset Manager
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
    
    AppStatus App::Run()
    {
        bool m_exit;
        while (!(m_exit = [m_AppDelegate applicationHasTerminated]) )
        {
            //while (dispatchEvent(peekEvent() ) )
            //{
            //}
        }
        return AppStatus::Destroyed;
    }
    
    namespace Core
    {
        uint32 RunApplication(App & app, kString const & appName)
        {
            app.OnInit();
            return (uint32)app.Run();
        }
    }
}