#pragma once

#include "Common\\Keyboard.h"
#include "Common\\Mouse.h"

namespace InputManager
{
	class CKeyboard
	{
	private:
		static std::unique_ptr<DirectX::Keyboard>      m_Keyboard;
		static DirectX::Keyboard::KeyboardStateTracker m_KeyboardTracker;

	public:
		static void Init();
		static void Update();

		static DirectX::Keyboard* Get() { return m_Keyboard.get(); }
		static bool IsPressed(DirectX::Keyboard::Keys key);
		static bool IsReleased(DirectX::Keyboard::Keys key);
		static bool IsDown(DirectX::Keyboard::Keys key);
		static bool IsUp(DirectX::Keyboard::Keys key);
	};

	class CMouse
	{
	private:
		static std::unique_ptr<DirectX::Mouse>    m_Mouse;
		static DirectX::Mouse::ButtonStateTracker m_MouseTracker;
		static DirectX::Mouse::State              m_MouseState;
		static DirectX::Mouse::State              m_MouseLastState;

	public:
		static void Init();
		static void Update();

		static DirectX::Mouse*       Get() { return m_Mouse.get(); }
		static DirectX::Mouse::State GetLastState() { return m_MouseLastState; }
		static void SwitchMode();
		static int  GetMoveX();
		static int  GetMoveY();
		static bool IsModeRelative();
		static bool IsLeftButtonPressed();
		static bool IsLeftButtonReleased();
		static bool IsLeftButtonDown();
		static bool IsLeftButtonUp();
		static bool IsMiddleButtonPressed();
		static bool IsMiddleButtonReleased();
		static bool IsMiddleButtonDown();
		static bool IsMiddleButtonUp();
		static bool IsRightButtonPressed();
		static bool IsRightButtonReleased();
		static bool IsRightButtonDown();
		static bool IsRightButtonUp();
	};
}