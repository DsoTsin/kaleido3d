#include "Kaleido3D.h"
#include <KTL/String.hpp>
#include "Core/Window.h"
#include "Core/Message.h"
#include "Core/LogUtil.h"
#include <queue>
#include <Config/OSHeaders.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

namespace k3d
{
	namespace WindowImpl
	{
        struct WinHandle
        {
            WinHandle() : Display(nullptr), Window(0) {}
            WinHandle(::Display* Disp, ::Window Win) : Display(Disp),Window (Win) {}
            ::Display*      Display;
            ::Window        Window;
        };

		class LinuxWindow : public IWindow
		{
		public:
			LinuxWindow() = default;
			LinuxWindow(const char *windowName, int width, int height)
            : m_WindowName(windowName)
            {
                XInitThreads();
                m_Display = ::XOpenDisplay(nullptr);
                m_Screen = DefaultScreen(m_Display);
                XSetWindowAttributes attr;
                attr.event_mask   = ExposureMask
                                    | VisibilityChangeMask
                                    | KeyPressMask
                                    | KeyReleaseMask
                                    | ButtonPressMask
                                    | ButtonReleaseMask
                                    | PointerMotionMask
                                    | StructureNotifyMask
                                    | SubstructureNotifyMask
                                    | FocusChangeMask
                                    | ResizeRedirectMask;
                m_Window = XCreateWindow(m_Display, RootWindow(m_Display, m_Screen),
                                         0, 0, width, height, 0, 0,
                                         InputOutput,
                                         DefaultVisual(m_Display, m_Screen),
                                         CWEventMask, &attr);
                m_Handle = new WinHandle(m_Display, m_Window);
                XStoreName(m_Display, m_Window, windowName);
                XMapWindow(m_Display, m_Window);
                XFlush(m_Display);

                XWindowAttributes win_attr;
                XGetWindowAttributes(m_Display, m_Window, &win_attr);
//                left_ = win_attr.x;
//                top_ = win_attr.y;
//                width_ = win_attr.width;
//                height_ = win_attr.height;

                m_DeleteWindow = XInternAtom(m_Display, "WM_DELETE_WINDOW", false);
                XSetWMProtocols(m_Display, m_Window, &m_DeleteWindow, 1);

                m_IsOpen = true;
            }

			/**
			* Interfaces
			*/
			void	SetWindowCaption(const char * name) override
			{
			}

			bool	IsOpen() override
			{
                m_Count = XPending(m_Display);
				return m_IsOpen || m_Count;
			}

			void*	GetHandle() const override
			{
				return m_Handle;
			}

			bool	PollMessage(Message & message) override
			{
                if(m_Count--)
                {
                    XEvent event;
                    int ret = XNextEvent(m_Display, &event);
                    GetXEvent(event, message);
                    return true;
                }
				return false;
			}


			uint32 Width() const override
			{
				return 0;
			}

			uint32 Height() const override
			{
				return 0;
			}

            ::Display*      m_Display;
            ::Window        m_Window;
            WinHandle*      m_Handle;
            int             m_Screen;
            Atom            m_DeleteWindow;

            bool        m_IsOpen = false;
            int         m_Count;


			void		Show(WindowMode mode) override {}
			void		Resize(int width, int height) override {}
			void		Move(int x, int y) override {}


            void GetXEvent(XEvent const &event, Message &message)
            {
                switch (event.type)
                {
                    case FocusIn:
                        message.type = Message::GainedFocus;
                        break;
                    case FocusOut:
                        message.type = Message::LostFocus;
                        break;
                    case Expose:
                        break;
                    case ResizeRequest:
                        message.type = Message::Resized;
                        break;
                    case KeyPress:
                        message.type = Message::KeyPressed;
                        break;
                    case KeyRelease:
                        message.type = Message::KeyReleased;
                        break;
                    case ClientMessage:
                        if(m_DeleteWindow == static_cast<Atom>(event.xclient.data.l[0]))
                        {
                            message.type = Message::Closed;
                            m_IsOpen = false;
                            XDestroyWindow(m_Display, m_Window);
                            XCloseDisplay(m_Display);
                        }
                        break;
                    default:
                        break;
                }
            }

		protected:
            String      m_WindowName;
			std::queue<Message> m_MessageQueue;
		};

	}

	IWindow::Ptr MakePlatformWindow(const char *windowName, int width, int height)
	{
		return MakeShared<WindowImpl::LinuxWindow>(windowName, width, height);
	}
}