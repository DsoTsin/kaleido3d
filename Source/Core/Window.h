#pragma once 
#ifndef __Window_h__
#define __Window_h__

namespace k3d {

	class Message;

	enum class WindowMode {
		NORMAL,
		FULLSCREEN
	};

	namespace WindowImpl {
		class WindowPrivate;
	}

	class Window {
	public:
		Window();
		explicit Window(const char *windowName, int width, int height);

		virtual ~Window();

		void	SetWindowCaption(const char * name);
		void	Show(WindowMode mode = WindowMode::NORMAL);
		void	Resize(int width, int height);
		void	Move(int x, int y);

		//implemented by platform
		void*	GetHandle() const;

		bool	PollMessage(Message & messge);

		Window & operator = (Window const &) = delete;
		Window(const Window&) = delete;
		Window(const Window&&) = delete;

	private:
		
		WindowImpl::WindowPrivate *pipl;
	};

};
#endif