#pragma once 
#ifndef __Window_h__
#define __Window_h__

#if K3DPLATFORM_OS_MAC
@class WindowPrivate;
@class ViewPrivate;
#endif

namespace k3d {

	class Message;

	enum class WindowMode {
		NORMAL,
		FULLSCREEN
	};

#if K3DPLATFORM_OS_WIN
	namespace WindowImpl {
		class WindowPrivate;
	}
#endif

	class K3D_API Window {
	public:
		Window();
		explicit Window(const kchar *windowName, int width, int height);

		virtual ~Window();

		void	SetWindowCaption(const kchar * name);
		void	Show(WindowMode mode = WindowMode::NORMAL);
		void	Resize(int width, int height);
		void	Move(int x, int y);
		bool	IsOpen();
		//implemented by platform
		void*	GetHandle() const;

		bool	PollMessage(Message & messge);

		Window & operator = (Window const &) = delete;
		Window(const Window&) = delete;
		Window(const Window&&) = delete;

		uint32 Width() const;
		uint32 Height() const;

	private:
#if K3DPLATFORM_OS_WIN
		WindowImpl::WindowPrivate *pipl;
#else
        WindowPrivate * pipl;
        ViewPrivate * piplView;
#endif
		uint32 m_Width, m_Height;
	};

};
#endif