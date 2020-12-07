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
	~DX12App();

	bool Initialize();
	bool InitMainWindow();
	int  Run();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse input.
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

	//bool Get4xMsaaState()const;
	//void Set4xMsaaState(bool value);

	//virtual void CreateRtvAndDsvDescriptorHeaps();
	//virtual void OnResize();
	//virtual void Update(const GameTimer& gt);
	//virtual void Draw(const GameTimer& gt) = 0;

	//bool InitDirect3D();
	//void CreateCommandObjects();
	//void CreateSwapChain();

	//void FlushCommandQueue();

	//ID3D12Resource* CurrentBackBuffer()const;
	//D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	//D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	//void LogAdapters();
	//void LogAdapterOutputs(IDXGIAdapter* adapter);
	//void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	//// Set true to use 4X MSAA (ß4.1.8).  The default is false.
	//bool      m4xMsaaState = false;    // 4X MSAA enabled
	//UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Used to keep track of the ìdelta-timeÅEand game time (ß4.4).

	//Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	//Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

	//Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	//UINT64 mCurrentFence = 0;

	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	//static const int SwapChainBufferCount = 2;
	//int mCurrBackBuffer = 0;
	//Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	//Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	//D3D12_VIEWPORT mScreenViewport;
	//D3D12_RECT mScissorRect;

	//UINT mRtvDescriptorSize = 0;
	//UINT mDsvDescriptorSize = 0;
	//UINT mCbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.

	//D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	//DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

};