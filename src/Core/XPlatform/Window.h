#pragma once 
#ifndef __Window_h__
#define __Window_h__

namespace k3d
{
	class Message;
	class App;

	enum class WindowMode 
    {
		NORMAL,
		FULLSCREEN
	};

	class K3D_CORE_API IWindow
	{
	public:
		typedef SharedPtr<IWindow> Ptr;

		virtual ~IWindow() {}

		virtual void	SetWindowCaption(const char * name) = 0;
		virtual void	Show(WindowMode mode = WindowMode::NORMAL) = 0;
		virtual void	Resize(int width, int height) = 0;
		virtual void	Move(int x, int y) = 0;
		virtual bool	IsOpen() = 0;
		virtual void*	GetHandle() const = 0;

		virtual bool	PollMessage(Message & messge) = 0;

		virtual U32 	Width() const = 0;
		virtual U32 	Height() const = 0;

		IWindow & operator = (IWindow const &) = delete;
		IWindow(const IWindow&) = delete;
		IWindow(const IWindow&&) = delete;

	protected:
		IWindow() {} 
		IWindow(const char *windowName, int width, int height) {}

		friend Ptr MakePlatformWindow(const char *windowName, int width, int height);
	};

	extern IWindow::Ptr MakePlatformWindow(const char *windowName, int width, int height);

#if K3DPLATFORM_OS_ANDROID
	extern IWindow::Ptr MakeAndroidWindow(void * window);
#endif
}
#endif