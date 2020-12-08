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
	static DX12App* mApp;

	// WindowèÓïÒ
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

	std::wstring mMainWndCaption = L"DX12App";
	std::wstring m_WindowClassName = L"MainWnd";
	int mClientWidth = 800;
	int mClientHeight = 600;

	GameTimer mTimer;

public:
	DX12App(HINSTANCE hInstance);
	DX12App(const DX12App& rhs) = delete;
	DX12App& operator=(const DX12App& rhs) = delete;
	~DX12App() = default;

	bool Init();
	bool InitMainWindow();
	int  Run();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	void CalculateFrameStats();

	static DX12App* GetApp() { return mApp; }

	HINSTANCE AppInst()const { return mhAppInst; }
	HWND      MainWnd()const { return mhMainWnd; }
	float     AspectRatio()const { return static_cast<float>(mClientWidth) / mClientHeight; }
	int       GetWindowWidth()const { return mClientWidth; }
	int       GetWindowHeight()const { return mClientHeight; }
};