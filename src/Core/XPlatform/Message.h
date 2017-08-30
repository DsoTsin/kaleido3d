#pragma once
#ifndef __Message_h__
#define __Message_h__
#include "InputDevice.h"

namespace k3d
{
	class Message final 
	{
	public:
		Message() = default;
		
		struct SizeEvent
		{
			unsigned int width;  ///< New width, in pixels
			unsigned int height; ///< New height, in pixels
		};

		struct KeyEvent
		{
			Keyboard::Key code;    ///< Code of the key that has been pressed
			bool          alt;     ///< Is the Alt key pressed?
			bool          control; ///< Is the Control key pressed?
			bool          shift;   ///< Is the Shift key pressed?
			bool          system;  ///< Is the System key pressed?
		};

		struct MouseMoveEvent
		{
			int x; ///< X position of the mouse pointer, relative to the left of the owner window
			int y; ///< Y position of the mouse pointer, relative to the top of the owner window
		};

		struct MouseButtonEvent
		{
			Mouse::Button button; ///< Code of the button that has been pressed
			int           x;      ///< X position of the mouse pointer, relative to the left of the owner window
			int           y;      ///< Y position of the mouse pointer, relative to the top of the owner window
		};

		struct MouseWheelEvent
		{
			int delta; ///< Number of ticks the wheel has moved (positive is up, negative is down)
			int x;     ///< X position of the mouse pointer, relative to the left of the owner window
			int y;     ///< Y position of the mouse pointer, relative to the top of the owner window
		};

		enum EventType
		{
			Closed,                 ///< The window requested to be closed (no data)
			Resized,                ///< The window was resized (data in event.size)
			LostFocus,              ///< The window lost the focus (no data)
			GainedFocus,            ///< The window gained the focus (no data)
			TextEntered,            ///< A character was entered (data in event.text)
			KeyPressed,             ///< A key was pressed (data in event.key)
			KeyReleased,            ///< A key was released (data in event.key)
			MouseWheelMoved,        ///< The mouse wheel was scrolled (data in event.mouseWheel)
			MouseButtonPressed,     ///< A mouse button was pressed (data in event.mouseButton)
			MouseButtonReleased,    ///< A mouse button was released (data in event.mouseButton)
			MouseMoved,             ///< The mouse cursor moved (data in event.mouseMove)
			MouseEntered,           ///< The mouse cursor entered the area of the window (no data)
			MouseLeft,              ///< The mouse cursor left the area of the window (no data)
			JoystickButtonPressed,  ///< A joystick button was pressed (data in event.joystickButton)
			JoystickButtonReleased, ///< A joystick button was released (data in event.joystickButton)
			JoystickMoved,          ///< The joystick moved along an axis (data in event.joystickMove)
			JoystickConnected,      ///< A joystick was connected (data in event.joystickConnect)
			JoystickDisconnected,   ///< A joystick was disconnected (data in event.joystickConnect)

			Count                   ///< Keep last -- the total number of event types
		};

		EventType type = (EventType)-1; ///< Type of the event

		union
		{
			SizeEvent            size;            ///< Size event parameters (Event::Resized)
			KeyEvent             key;             ///< Key event parameters (Event::KeyPressed, Event::KeyReleased)
			MouseMoveEvent       mouseMove;       ///< Mouse move event parameters (Event::MouseMoved)
			MouseButtonEvent     mouseButton;     ///< Mouse button event parameters (Event::MouseButtonPressed, Event::MouseButtonReleased)
			MouseWheelEvent      mouseWheel;      ///< Mouse wheel event parameters (Event::MouseWheelMoved)
		};

	};

}

#endif
