#include "Kaleido3D.h"
#include "../Message.h"
#include <queue>
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
        class OSXWindow : public IWindow
        {
        public:
            OSXWindow(const kchar *windowName, int width, int height);
            ~OSXWindow();

            int     Init();
            void    SetWindowCaption(const kchar * name) override;
            void	Show(WindowMode mode) override;
            void	Resize(int width, int height) override;
            void	Move(int x, int y) override;


            bool	IsOpen() override { return true;}
            void*	GetHandle() const override { return m_Window; };

            bool	PollMessage(Message & messge) override { return PopMessage(messge, true); };

            uint32 	Width() const override {return 0;}
            uint32 	Height() const override {return 0;}

            virtual void	PushMessage(const Message & message);
            virtual bool	PopMessage(Message & message, bool block);
            virtual void	ProcessMessage();
            void *          handle;

        private:
            NSWindow*       m_Window;
        };

        OSXWindow::OSXWindow(const kchar *windowName, int width, int height)
        : m_Window(nil)
        {
            Init();
            SetWindowCaption(windowName);
            Resize(width, height);
        }
        
        OSXWindow::~OSXWindow()
        {
            if(nil!=m_Window)
            {
                [m_Window release];
                m_Window = nil;
            }
        }
        
        int OSXWindow::Init()
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
            [[Window sharedDelegate] windowCreated:window];
            m_Window = window;
            return 0;
        }
        
        void OSXWindow::Move(int x, int y)
        {
            
        }
        
        void OSXWindow::Resize(int width, int height)
        {
            
        }
        
        void OSXWindow::Show(k3d::WindowMode mode)
        {
            
        }
        
        void OSXWindow::SetWindowCaption(const kchar *name)
        {
            
        }
        
        void OSXWindow::PushMessage(const k3d::Message &message)
        {
            
        }
        
        void OSXWindow::ProcessMessage()
        {
            
        }
        
        bool OSXWindow::PopMessage(k3d::Message &message, bool block)
        {
            NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];

            return true;
        }

    }

    IWindow::Ptr MakePlatformWindow(const kchar *windowName, int width, int height)
    {
        return std::make_shared<WindowImpl::OSXWindow>(windowName, width, height);
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