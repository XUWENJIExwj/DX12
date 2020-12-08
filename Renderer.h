#pragma once

#include "DX12App.h"
#include "CubeRenderTarget.h"
#include "GameObject.h"
#include "FrameResource.h"

enum class PSOTypeIndex :int
{
	PSO_00_Opaque,
	PSO_01_Sky,
	PSO_MAX
};

class CRenderer
{
private:
	static DX12App* m_App;

	// DX12初期化
	static bool m4xMsaaState;    // 4X MSAA enabled
	static UINT m4xMsaaQuality;      // quality level of 4X MSAA

	static Microsoft::WRL::ComPtr<IDXGIFactory4>  mdxgiFactory;
	static Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	static Microsoft::WRL::ComPtr<ID3D12Device>   md3dDevice;

	static Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	static UINT64                              mCurrentFence;

	static Microsoft::WRL::ComPtr<ID3D12CommandQueue>        mCommandQueue;
	static Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    mDirectCmdListAlloc;
	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount;
	static int       mCurrBackBuffer;
	static Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[];
	static Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	static D3D12_VIEWPORT mScreenViewport;
	static D3D12_RECT     mScissorRect;

	static UINT mRtvDescriptorSize;
	static UINT mDsvDescriptorSize;
	static UINT mCbvSrvUavDescriptorSize;

	static D3D_DRIVER_TYPE md3dDriverType;
	static DXGI_FORMAT     mBackBufferFormat;
	static DXGI_FORMAT     mDepthStencilFormat;

	// CommonResources生成
	static Microsoft::WRL::ComPtr<ID3D12RootSignature>              mRootSignature;
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>             mSrvDescriptorHeap;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;
	static int                                                      m_CurrentPSO;

	// CubeMap
	static CD3DX12_GPU_DESCRIPTOR_HANDLE m_SkyTextureDescriptorHandle;

	// DynamicCubeMap
	static bool                                   m_DynamicCubeOn;
	static std::unique_ptr<CCubeRenderTarget>     m_DynamicCubeMap;
	static CD3DX12_CPU_DESCRIPTOR_HANDLE          m_DynamicCubeDsvHandle;
	static CD3DX12_GPU_DESCRIPTOR_HANDLE          m_DynamicCubeDescriptorHandle;
	static UINT                                   m_CubeMapSize;
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_CubeDepthStencilBuffer;

public:
	// DX12初期化
	static bool Init();
	static void Uninit();
	static void Set4xMsaaState(bool value);
	static void CreateCommandObjects();
	static void CreateSwapChain();
	static void CreateRtvAndDsvDescriptorHeaps();
	static void OnResize();
	static void ResetFence();
	static void FlushCommandQueue();

	// CommandObjects操作
	static void RestDirectCmdListAlloc();
	static void ExecuteCommandLists();

	// CommonResources生成
	static void CreateCommonResources();
	static void CreateRootSignature();
	static void CreateDescriptorHeaps();
	static void CreateCubeDepthStencil();
	static void CreataPSOs();

	// ゲッター
	static bool Get4xMsaaState() { return m4xMsaaState; }

	static ID3D12Device*               GetDevice() { return md3dDevice.Get(); }
	static ID3D12Fence*                GetFence() { return mFence.Get(); }
	static ID3D12Resource*             CurrentBackBuffer() { return mSwapChainBuffer[mCurrBackBuffer].Get(); }
	static D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();
	static D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() { return mDsvHeap->GetCPUDescriptorHandleForHeapStart(); }

	static CD3DX12_GPU_DESCRIPTOR_HANDLE CreateCubeMapDescriptorHandle(UINT Offset);
	static bool GetDynamicCubeOn() { return m_DynamicCubeOn; }

	// デバッガ―
	static void LogAdapters();
	static void LogAdapterOutputs(IDXGIAdapter* adapter);
	static void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	// StaticSamplers
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	// 描画用
	static void Begin();
	static void SetUpCommonResources();
	static void DrawDynamicCubeScene();
	static void SetUpBeforeDrawScene();
	static void SetPSO(int PSOType);
	static void DrawGameObjectsWithLayer(std::list<CGameObject*>& GameObjectsWithLayer);
	static void End();
};