#pragma once 
#ifndef __Window_h__
#define __Window_h__

namespace k3d
{
	class Message;
	class App;

	enum class WindowMode {
		NORMAL,
		FULLSCREEN
	};

	class K3D_API IWindow
	{
	public:
		typedef std::shared_ptr<IWindow> Ptr;

		virtual ~IWindow() {}

		virtual void	SetWindowCaption(const kchar * name) = 0;
		virtual void	Show(WindowMode mode = WindowMode::NORMAL) = 0;
		virtual void	Resize(int width, int height) = 0;
		virtual void	Move(int x, int y) = 0;
		virtual bool	IsOpen() = 0;
		virtual void*	GetHandle() const = 0;

		virtual bool	PollMessage(Message & messge) = 0;

		virtual uint32 	Width() const = 0;
		virtual uint32 	Height() const = 0;

		IWindow & operator = (IWindow const &) = delete;
		IWindow(const IWindow&) = delete;
		IWindow(const IWindow&&) = delete;

	protected:
		IWindow() {} 
		IWindow(const kchar *windowName, int width, int height) {}

		friend Ptr MakePlatformWindow(const kchar *windowName, int width, int height);
	};

	extern IWindow::Ptr MakePlatformWindow(const kchar *windowName, int width, int height);

#if K3DPLATFORM_OS_ANDROID
	extern IWindow::Ptr MakeAndroidWindow(void * window);
#endif
}
#endif