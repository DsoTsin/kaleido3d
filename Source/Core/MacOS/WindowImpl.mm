#include "Kaleido3D.h"
#include "WindowImpl.h"

#import <Cocoa/Cocoa.h>



@interface Window : NSObject<NSWindowDelegate>
{
    uint32_t windowCount;
}

+ (Window*)sharedDelegate;
- (id)init;
- (void)windowCreated:(NSWindow*)window;
- (void)windowWillClose:(NSNotification*)notification;
- (BOOL)windowShouldClose:(NSWindow*)window;
- (void)windowDidResize:(NSNotification*)notification;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;

@end


namespace k3d
{
    constexpr uint32 DEFAULT_WINDOW_WIDTH = 1280;
    constexpr uint32 DEFAULT_WINDOW_HEIGHT = 720;
    
    namespace WindowImpl
    {
        WindowPrivate::WindowPrivate() {}
        
        WindowPrivate::~WindowPrivate() {}
        
        int WindowPrivate::Init()
        {
            uint32 windowStyle = 0
            | NSTitledWindowMask
            | NSClosableWindowMask
            | NSMiniaturizableWindowMask
            | NSResizableWindowMask;
            NSRect screenRect = [[NSScreen mainScreen] frame];
            const float centerX = (screenRect.size.width  - (float)DEFAULT_WINDOW_WIDTH )*0.5f;
            const float centerY = (screenRect.size.height - (float)DEFAULT_WINDOW_HEIGHT)*0.5f;
            NSRect rect = NSMakeRect(centerX, centerY, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
            NSWindow* window = [[NSWindow alloc]
                                initWithContentRect:rect
                                styleMask:windowStyle
                                backing:NSBackingStoreBuffered defer:NO
                                ];
            NSString* appName = [[NSProcessInfo processInfo] processName];
            [window setTitle:appName];
            [window makeKeyAndOrderFront:window];
            [window setAcceptsMouseMovedEvents:YES];
            [window setBackgroundColor:[NSColor blackColor]];
            //[[window sharedDelegate] windowCreated:window];
            //m_Window = window;
            return 0;
        }
        
        void WindowPrivate::Move(int x, int y)
        {
            
        }
        
        void WindowPrivate::Resize(int width, int height)
        {
            
        }
        
        void WindowPrivate::Show(k3d::WindowMode mode)
        {
            
        }
        
        void WindowPrivate::SetCaption(const kchar *name)
        {
            
        }
        
        void WindowPrivate::PushMessage(const k3d::Message &message)
        {
            
        }
        
        void WindowPrivate::ProcessMessage()
        {
            
        }
        
        bool WindowPrivate::PopMessage(k3d::Message &message, bool block)
        {
            return true;
        }
        
        class OSXWindowPrivate : public WindowPrivate
        {
        public:
            NSWindow    * m_Window;
            
            OSXWindowPrivate() : m_Window(nil) {}
        };
        
        WindowPrivate * makeNewWindowImpl()
        {
            return new OSXWindowPrivate;
        }
    }
}



@implementation Window

+ (Window*)sharedDelegate
{
    static id windowDelegate = [Window new];
    return windowDelegate;
}

- (id)init
{
    self = [super init];
    if (nil == self)
    {
        return nil;
    }
    
    self->windowCount = 0;
    return self;
}

- (void)windowCreated:(NSWindow*)window
{
    assert(window);
    
    [window setDelegate:self];
    
    assert(self->windowCount < ~0u);
    self->windowCount += 1;
}

- (void)windowWillClose:(NSNotification*)notification
{
    K3D_UNUSED(notification);
}

- (BOOL)windowShouldClose:(NSWindow*)window
{
    assert(window);
    
    [window setDelegate:nil];
    
    assert(self->windowCount);
    self->windowCount -= 1;
    
    if (self->windowCount == 0)
    {
        [NSApp terminate:self];
        return false;
    }
    
    return true;
}

- (void)windowDidResize:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    //s_ctx.windowDidResize();
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    //s_ctx.windowDidBecomeKey();
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    K3D_UNUSED(notification);
    //s_ctx.windowDidResignKey();
}

@end