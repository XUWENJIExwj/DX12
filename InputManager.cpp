#include "DX12App.h"
#include "InputManager.h"

using namespace std;
using namespace DirectX;
using namespace InputManager;

// Keyboard
unique_ptr<Keyboard>           CKeyboard::m_Keyboard = nullptr;
Keyboard::KeyboardStateTracker CKeyboard::m_KeyboardTracker;

void CKeyboard::Init()
{
	m_Keyboard = make_unique<Keyboard>();
}

void CKeyboard::Update()
{
	m_KeyboardTracker.Update(m_Keyboard->GetState());
}

bool CKeyboard::IsPressed(Keyboard::Keys key)
{
	return m_KeyboardTracker.IsKeyPressed(key);
}

bool CKeyboard::IsReleased(Keyboard::Keys key)
{
	return m_KeyboardTracker.IsKeyReleased(key);
}

bool CKeyboard::IsDown(Keyboard::Keys key)
{
	return m_Keyboard->GetState().IsKeyDown(key);
}

bool CKeyboard::IsUp(Keyboard::Keys key)
{
	return m_Keyboard->GetState().IsKeyUp(key);
}

// Mouse
unique_ptr<Mouse>         CMouse::m_Mouse = nullptr;
Mouse::ButtonStateTracker CMouse::m_MouseTracker;
Mouse::State              CMouse::m_MouseState;
Mouse::State              CMouse::m_MouseLastState;

void CMouse::Init()
{
	m_Mouse = make_unique<Mouse>();
	m_Mouse->SetWindow(DX12App::GetApp()->GetMainWindowHandle());
	m_Mouse->SetMode(Mouse::MODE_RELATIVE);
}

void CMouse::Update()
{
	m_MouseState = m_Mouse->GetState();
	m_MouseLastState = m_MouseTracker.GetLastState();
	m_MouseTracker.Update(m_MouseState);
}

int CMouse::GetMoveX()
{
	return m_MouseState.x;
}

int CMouse::GetMoveY()
{
	return m_MouseState.y;
}

bool CMouse::IsModeRelative()
{
	return m_Mouse->GetState().positionMode == Mouse::MODE_RELATIVE;
}

bool CMouse::IsLeftButtonPressed()
{
	return m_MouseState.leftButton && m_MouseTracker.leftButton == m_MouseTracker.PRESSED;
}

bool CMouse::IsLeftButtonReleased()
{
	return !m_MouseState.leftButton && m_MouseTracker.leftButton == m_MouseTracker.RELEASED;
}

bool CMouse::IsLeftButtonDown()
{
	return m_MouseState.leftButton && m_MouseTracker.leftButton == m_MouseTracker.HELD;
}

bool CMouse::IsLeftButtonUp()
{
	return !m_MouseState.leftButton && m_MouseTracker.leftButton == m_MouseTracker.UP;
}

bool CMouse::IsMiddleButtonPressed()
{
	return m_MouseState.middleButton && m_MouseTracker.middleButton == m_MouseTracker.PRESSED;
}

bool CMouse::IsMiddleButtonReleased()
{
	return !m_MouseState.middleButton && m_MouseTracker.middleButton == m_MouseTracker.RELEASED;
}

bool CMouse::IsMiddleButtonDown()
{
	return m_MouseState.middleButton && m_MouseTracker.middleButton == m_MouseTracker.HELD;
}

bool CMouse::IsMiddleButtonUp()
{
	return !m_MouseState.middleButton && m_MouseTracker.middleButton == m_MouseTracker.UP;
}

bool CMouse::IsRightButtonPressed()
{
	return m_MouseState.rightButton && m_MouseTracker.rightButton == m_MouseTracker.PRESSED;
}

bool CMouse::IsRightButtonReleased()
{
	return !m_MouseState.rightButton && m_MouseTracker.rightButton == m_MouseTracker.RELEASED;
}

bool CMouse::IsRightButtonDown()
{
	return m_MouseState.rightButton && m_MouseTracker.rightButton == m_MouseTracker.HELD;
}

bool CMouse::IsRightButtonUp()
{
	return !m_MouseState.rightButton && m_MouseTracker.rightButton == m_MouseTracker.UP;
}