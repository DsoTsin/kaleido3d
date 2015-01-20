#pragma once
#include <Config/OSHeaders.h>
#include <queue>
#include "../Message.h"

namespace k3d
{
	enum class WindowMode;
	
	namespace WindowImpl
	{
		class WindowPrivate {
		public:
			WindowPrivate() = default;

			union {
				HWND	handle;
			};
			LONG_PTR	m_callback;

			int			Init();
			void		SetCaption(const char * name);
			void		Show(WindowMode mode);
			void		Resize(int width, int height);
			void		Move(int x, int y);

			void		PushMessage(const Message & message);
			bool		PopMessage(Message & message, bool block);
			void		ProcessMessage();

			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT32 msg, WPARAM wParam, LPARAM lParam);

		protected:
			void		RegisterWindowClass();
			void		ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

			std::queue<Message> m_MessageQueue;
		};

		extern int InitApp();

		extern Keyboard::Key virtualKeyCodeToSF(WPARAM key, LPARAM flags);
	}

}