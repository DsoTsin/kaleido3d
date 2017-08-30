#include "CoreMinimal.h"

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
  struct EnvironmentImpl
  {
    String DataDir;
    String ModuleDir;
    String AppName;
    String LogDir;
    
    EnvironmentImpl()
    {
      NSBundle* bundle = [NSBundle mainBundle];
      NSString* path = [[NSString alloc] initWithString:[bundle builtInPlugInsPath]];
      ModuleDir = [path UTF8String];
      [path release];
      path = [[NSString alloc] initWithString:[bundle bundlePath]];
      String BundleDir = [path UTF8String];
      [path release];
      LogDir = BundleDir + "/Logs";
      DataDir = BundleDir + "/Data";
      [path release];
      path = [[NSString alloc] initWithString:[bundle executablePath]];
      String exePath = [path UTF8String];
      auto sepPos = exePath.FindLastOf("/");
      AppName = exePath.SubStr(sepPos+1, exePath.Length()-sepPos);
      [path release];
    }
  };
  
  Environment::Environment()
  {
    d = new EnvironmentImpl;
  }
  
  Environment::~Environment()
  {
    delete d;
  }
  
  String Environment::GetInstanceName() const
  {
    return d->AppName;
  }
  
  String Environment::GetDataDir() const
  {
    return d->DataDir;
  }
  
  String Environment::GetModuleDir() const
  {
    return d->ModuleDir;
  }
  
  String Environment::GetLogDir() const
  {
    return d->LogDir;
  }
}
