#include "../Window.h"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>


@interface WindowPrivate : NSWindow
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
@end

@interface ViewPrivate : MTKView
- (void)render;
@end


@implementation WindowPrivate
- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}
@end

@implementation ViewPrivate

- (void) render {
    
}

@end


namespace k3d {
    
    
}