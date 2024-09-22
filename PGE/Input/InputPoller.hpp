#pragma once

#include "Events/Signal.hpp"

struct GLFWwindow;

namespace pge
{
	enum class InputEventType : uint8_t
	{
		WindowMove,
		WindowResize,
		WindowClosed,
		Keyboard,
		MouseButton,
		MouseMove,
		MouseWheel,
		Text
	};

	struct InputEvent_Window
	{
		uint32_t Data1;
		uint32_t Data2;
	};

	struct InputEvent_Keyboard
	{
		uint32_t Key;
		uint32_t Mod;
		uint8_t State;
	};

	struct InputEvent_MouseButton
	{
		uint32_t Button;
		uint8_t State;
	};

	struct InputEvent_MouseMove
	{
		float X;
		float Y;
	};

	struct InputEvent_MouseWheel
	{
		float X;
		float Y;
	};

	struct InputEvent_Text
	{
		uint32_t CodePoint;
	};

	struct InputEvent
	{
		InputEventType Type;
		uint64_t Id;
		union
		{
			InputEvent_Window	   Window;
			InputEvent_Keyboard    Keyboard;
			InputEvent_MouseButton Button;
			InputEvent_MouseMove   Move;
			InputEvent_MouseWheel  Wheel;
			InputEvent_Text		   Text;
		};
	};

	class InputPoller
	{
	private:

	public:
		InputPoller();
		~InputPoller();

		void PollInput();
		void RegisterInput(GLFWwindow* window);

		Signal<InputEvent&> EventRecieved;
		Signal<int> LowMemory;
	};
}