#include "Kaleido3D.h"
#include <Core/Message.h>
#include <queue>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#import <QuartzCore/CAMetalLayer.h>

@interface MetalView : UIView
@property (nonatomic) CAMetalLayer *metalLayer;
- (void) onRender;
@end

namespace k3d
{
    namespace WindowImpl
    {
        class iOSWindow : public IWindow
        {
        public:
            iOSWindow(const kchar *windowName, int width, int height);
            ~iOSWindow();

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
            UIWindow*       m_Window;
        };

        iOSWindow::iOSWindow(const kchar *windowName, int width, int height)
        : m_Window(nil)
        {
            Init();
            SetWindowCaption(windowName);
            Resize(width, height);
        }
        
        iOSWindow::~iOSWindow()
        {
            if(nil!=m_Window)
            {
                [m_Window release];
                m_Window = nil;
            }
        }
        
        int iOSWindow::Init()
        {
            CGRect bounds = [[UIScreen mainScreen] bounds];
            UIViewController* view_controller = [[UIViewController alloc] initWithNibName:nil bundle:nil];
            // create window
            UIWindow* window = [[UIWindow alloc] initWithFrame:bounds];
            MetalView * mtlView = [[MetalView alloc] initWithFrame:bounds];            [window setRootViewController:view_controller];
            [window setBackgroundColor:[UIColor blackColor]];
            [window addSubview:mtlView];
            [window makeKeyAndVisible];
            m_Window = window;
            return 0;
        }
        
        void iOSWindow::Move(int x, int y)
        {
            
        }
        
        void iOSWindow::Resize(int width, int height)
        {
            
        }
        
        void iOSWindow::Show(k3d::WindowMode mode)
        {
            
        }
        
        void iOSWindow::SetWindowCaption(const kchar *name)
        {
            
        }
        
        void iOSWindow::PushMessage(const k3d::Message &message)
        {
            
        }
        
        void iOSWindow::ProcessMessage()
        {
            
        }
        
        bool iOSWindow::PopMessage(k3d::Message &message, bool block)
        {
            return true;
        }

    }

    IWindow::Ptr MakePlatformWindow(const kchar *windowName, int width, int height)
    {
        return std::make_shared<WindowImpl::iOSWindow>(windowName, width, height);
    }
}

@implementation MetalView
{
    CADisplayLink *dispLink;
}
@synthesize metalLayer;

- (BOOL) isOpaque {
    return YES;
}

- (void) onRender
{
    [[NSNotificationCenter defaultCenter] postNotificationName:@"RHIRender" object:nil];
}

-(instancetype) initWithFrame:(CGRect)frameRect {
    self = [super initWithFrame: frameRect];
    dispLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onRender)];
    [dispLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    if (self)
    {
        metalLayer = (CAMetalLayer*)self.layer;
        metalLayer.opaque = YES;
    }
    return self;
}

+ (Class)layerClass
{
    return [CAMetalLayer class];
}

- (BOOL) wantsLayer {
    return YES;
}

- (BOOL) wantsUpdateLayer {
    return YES;
}

- (void)dealloc {
    [dispLink invalidate];
}

@end
