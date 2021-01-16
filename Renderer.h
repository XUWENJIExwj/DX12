#pragma once

#include "DX12App.h"
#include "CubeRenderTarget.h"
#include "ShadowMap.h"
#include "GameObject.h"
#include "FrameResource.h"

enum class PSOTypeIndex :int
{
	PSO_Solid_Opaque,
	PSO_Solid_Opaque_POM,
	PSO_Solid_Sky,
	PSO_WireFrame_Opaque,
	PSO_WireFrame_Sky,
	PSO_ShadowMapping,
	PSO_MAX
};

class CRenderer
{
private:
	static DX12App* m_App;

	// DX12初期化
	static bool m_4xMsaaState;   // 4X MSAA enabled
	static UINT m_4xMsaaQuality; // quality level of 4X MSAA

	static Microsoft::WRL::ComPtr<IDXGIFactory4>  m_DxgiFactory;
	static Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	static Microsoft::WRL::ComPtr<ID3D12Device>   m_D3DDevice;

	static Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	static UINT64                              m_CurrentFence;

	static Microsoft::WRL::ComPtr<ID3D12CommandQueue>        m_CommandQueue;
	static Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_DirectCmdListAlloc;
	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	static const int m_SwapChainBufferCount;
	static int       m_CurrentBackBuffer;
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[];
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	static D3D12_VIEWPORT m_ScreenViewport;
	static D3D12_RECT     m_ScissorRect;

	static UINT m_RtvDescSize;
	static UINT m_DsvDescSize;
	static UINT m_CbvSrvUavDescSize;

	static D3D_DRIVER_TYPE m_D3DDriverType;
	static DXGI_FORMAT     m_BackBufferFormat;
	static DXGI_FORMAT     m_DepthStencilFormat;

	// CommonResources生成
	static Microsoft::WRL::ComPtr<ID3D12RootSignature>              m_RootSignature;
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>             m_SrvHeap;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;
	static int                                                      m_CurrentPSO;

	// Null Textures
	static CD3DX12_GPU_DESCRIPTOR_HANDLE m_NullTextureSrv;
	static CD3DX12_GPU_DESCRIPTOR_HANDLE m_NullCubeMapSrv;

	// CubeMap
	static std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> m_SkyCubeMapDescHandles;
	static int                                        m_CurrentSkyCubeMapIndex;

	// DynamicCubeMap
	static bool                                   m_DynamicCubeMapOn;
	static std::unique_ptr<CCubeRenderTarget>     m_DynamicCubeMap;
	static CD3DX12_CPU_DESCRIPTOR_HANDLE          m_DynamicCubeMapDsvHandle;
	static UINT                                   m_DynamicCubeMapSize;
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_DynamicCubeMapDepthStencilBuffer;

	// ShadowMap
	static std::unique_ptr<CShadowMap> m_ShadowMap;

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
	static bool Get4xMsaaState() { return m_4xMsaaState; }

	static ID3D12Device*               GetDevice() { return m_D3DDevice.Get(); }
	static ID3D12Fence*                GetFence() { return m_Fence.Get(); }
	static ID3D12GraphicsCommandList*  GetCommandList() { return m_CommandList.Get(); }
	static ID3D12Resource*             GetCurrentBackBuffer() { return m_SwapChainBuffer[m_CurrentBackBuffer].Get(); }
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView();
	static D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() { return m_DsvHeap->GetCPUDescriptorHandleForHeapStart(); }
	static DXGI_FORMAT                 GetBackBufferFormat() { return m_BackBufferFormat; }
	static ID3D12DescriptorHeap*       GetSrvHeap() { return m_SrvHeap.Get(); }

	// CubeMap
	static int GetCurrentSkyCubeMapIndex() { return m_CurrentSkyCubeMapIndex; }

	// DynamicCubeMap
	static bool                          GetDynamicCubeOn() { return m_DynamicCubeMapOn; }
	static CD3DX12_GPU_DESCRIPTOR_HANDLE ComputeCubeMapDescHandle(UINT Offset);
	static UINT                          GetDynamicCubeMapSize() { return m_DynamicCubeMapSize; }

	// デバッガ―
	static void LogAdapters();
	static void LogAdapterOutputs(IDXGIAdapter* adapter);
	static void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	// StaticSamplers
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

	// 描画用
	static void Begin();
	static void SetUpCommonResources();
	static void SetCurrentSkyCubeMapIndex(int CubeMapIndex) { m_CurrentSkyCubeMapIndex = CubeMapIndex; }
	static void SetUpSkyCubeMapResources();
	static void SetUpDynamicCubeMapResources(int DCMResourcesIndex);
	static void SetUpBeforeCreateAllDynamicCubeMapResources(int DCMResourcesIndex);
	static void SetUpBeforeCreateEachDynamicCubeMapResource(int DCMResourcesIndex, int FaceIndex);
	static void CreateDynamicCubeMapResources(const GameTimer& GlobalTimer, std::list<CGameObject*>& RenderLayer);
	static void CompleteCreateDynamicCubeMapResources(int DCMResourcesIndex);
	static void SetUpBeforeDrawScene();
	static void SetPSO(int PSOType);
	static void DrawGameObjectsWithLayer(std::list<CGameObject*>& RenderLayer);
	static void DrawGameObjectsWithDynamicCubeMap(std::list<CGameObject*>& RenderLayer);
	static void DrawSingleGameObject(CGameObject* GameObject, ID3D12Resource* ObjectCB);
	static void End();
};