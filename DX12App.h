#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Common\\d3dUtil.h"
#include "Common\\GameTimer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12App
{
private:
	static DX12App* m_App;

	// WindowèÓïÒ
	HINSTANCE m_AppInstanceHandle = nullptr; // application instance handle
	HWND      m_MainWindowHandle = nullptr;  // main window handle
	bool      m_AppPaused = false;           // is the application paused?
	bool      m_Minimized = false;           // is the application minimized?
	bool      m_Maximized = false;           // is the application maximized?
	bool      m_Resizing = false;            // are the resize bars being dragged?
	bool      m_FullscreenState = false;     // fullscreen enabled

	std::wstring m_MainWindowCaption = L"DX12App";
	std::wstring m_WindowClassName = L"MainWnd";
	int          m_WindowWidth = 1280;
	int          m_WindowHeight = 720;
	int          m_TargetFPS = 60;

	GameTimer m_Timer;

public:
	DX12App(HINSTANCE hInstance);
	DX12App(const DX12App& rhs) = delete;
	DX12App& operator=(const DX12App& rhs) = delete;
	~DX12App() = default;

	bool Init();
	bool InitMainWindow();
	int  Run();

	LRESULT MsgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	void CalculateFrameStats();

	static DX12App* GetApp() { return m_App; }

	HINSTANCE GetAppInstanceHandle()const { return m_AppInstanceHandle; }
	HWND      GetMainWindowHandle()const { return m_MainWindowHandle; }
	float     GetAspectRatio()const { return static_cast<float>(m_WindowWidth) / m_WindowHeight; }
	int       GetWindowWidth()const { return m_WindowWidth; }
	int       GetWindowHeight()const { return m_WindowHeight; }
	int       GetTargetFPS()const { return m_TargetFPS; }
};