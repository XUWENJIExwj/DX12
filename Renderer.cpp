#include "Manager.h"
#include "Renderer.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

DX12App* CRenderer::m_App = nullptr;

// DX12初期化
bool CRenderer::m_4xMsaaState = false;    // 4X MSAA enabled
UINT CRenderer::m_4xMsaaQuality = 0;      // quality level of 4X MSAA

ComPtr<IDXGIFactory4>  CRenderer::m_DxgiFactory = nullptr;
ComPtr<IDXGISwapChain> CRenderer::m_SwapChain = nullptr;
ComPtr<ID3D12Device>   CRenderer::m_D3DDevice = nullptr;

ComPtr<ID3D12Fence> CRenderer::m_Fence = nullptr;
UINT64              CRenderer::m_CurrentFence = 0;

ComPtr<ID3D12CommandQueue>        CRenderer::m_CommandQueue = nullptr;
ComPtr<ID3D12CommandAllocator>    CRenderer::m_DirectCmdListAlloc = nullptr;
ComPtr<ID3D12GraphicsCommandList> CRenderer::m_CommandList = nullptr;

const int CRenderer::m_SwapChainBufferCount = 2;
int       CRenderer::m_CurrentBackBuffer = 0;
ComPtr<ID3D12Resource> CRenderer::m_SwapChainBuffer[m_SwapChainBufferCount];
ComPtr<ID3D12Resource> CRenderer::m_DepthStencilBuffer = nullptr;

ComPtr<ID3D12DescriptorHeap> CRenderer::m_RtvHeap = nullptr;
ComPtr<ID3D12DescriptorHeap> CRenderer::m_DsvHeap = nullptr;

D3D12_VIEWPORT CRenderer::m_ScreenViewport;
D3D12_RECT     CRenderer::m_ScissorRect;

UINT CRenderer::m_RtvDescriptorSize = 0;
UINT CRenderer::m_DsvDescriptorSize = 0;
UINT CRenderer::m_CbvSrvUavDescriptorSize = 0;

D3D_DRIVER_TYPE CRenderer::m_D3DDriverType = D3D_DRIVER_TYPE_HARDWARE;
DXGI_FORMAT     CRenderer::m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
DXGI_FORMAT     CRenderer::m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

// CommonResources生成
ComPtr<ID3D12RootSignature>         CRenderer::m_RootSignature = nullptr;
ComPtr<ID3D12DescriptorHeap>        CRenderer::m_SrvDescriptorHeap = nullptr;
vector<ComPtr<ID3D12PipelineState>> CRenderer::m_PSOs((int)PSOTypeIndex::PSO_MAX);
int                                 CRenderer::m_CurrentPSO = (int)PSOTypeIndex::PSO_00_Solid_Opaque;

// CubeMap
CD3DX12_GPU_DESCRIPTOR_HANDLE CRenderer::m_SkyTextureDescriptorHandle;

// DynamicCubeMap
bool                          CRenderer::m_DynamicCubeMapOn = true;
unique_ptr<CCubeRenderTarget> CRenderer::m_DynamicCubeMap = nullptr;
CD3DX12_CPU_DESCRIPTOR_HANDLE CRenderer::m_DynamicCubeMapDsvHandle;
//DCM
//CD3DX12_GPU_DESCRIPTOR_HANDLE CRenderer::m_DynamicCubeMapDescHandle;
vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> CRenderer::m_DynamicCubeMapsDescHandle;
UINT                          CRenderer::m_DynamicCubeMapSize = 512;
ComPtr<ID3D12Resource>        CRenderer::m_DynamicCubeMapDepthStencilBuffer = nullptr;

// DX12初期化
bool CRenderer::Init()
{
	m_App = DX12App::GetApp();

#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_D3DDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_D3DDevice)));
	}

	ThrowIfFailed(m_D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_Fence)));

	m_RtvDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_D3DDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	return true;
}

void CRenderer::Uninit()
{
	if (m_D3DDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

void CRenderer::Set4xMsaaState(bool value)
{
	if (m_4xMsaaState != value)
	{
		m_4xMsaaState = value;

		// Recreate the swapchain and buffers with new multisample settings.
		CreateSwapChain();
		OnResize();
	}
}

void CRenderer::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

	ThrowIfFailed(m_D3DDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(m_D3DDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_DirectCmdListAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(m_CommandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	m_CommandList->Close();
}

void CRenderer::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_App->GetWindowWidth();
	sd.BufferDesc.Height = m_App->GetWindowHeight();
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = m_SwapChainBufferCount;
	sd.OutputWindow = m_App->GetMainWindowHandle();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(m_DxgiFactory->CreateSwapChain(
		m_CommandQueue.Get(),
		&sd,
		m_SwapChain.GetAddressOf()));
}

void CRenderer::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_DynamicCubeMapOn ? m_SwapChainBufferCount + 6 : m_SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_D3DDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = m_DynamicCubeMapOn ? 2 : 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_D3DDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));

	if (m_DynamicCubeMapOn)
	{
		m_DynamicCubeMap = make_unique<CCubeRenderTarget>(m_D3DDevice.Get(),
			m_DynamicCubeMapSize, m_DynamicCubeMapSize, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_DynamicCubeMapDsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_DsvHeap->GetCPUDescriptorHandleForHeapStart(),
			1,
			m_DsvDescriptorSize);
	}
}

void CRenderer::OnResize()
{
	assert(m_D3DDevice);
	assert(m_SwapChain);
	assert(m_DirectCmdListAlloc);

	// Flush before changing any resources.
	FlushCommandQueue();

	RestDirectCmdListAlloc();

	// Release the previous resources we will be recreating.
	for (int i = 0; i < m_SwapChainBufferCount; ++i)
		m_SwapChainBuffer[i].Reset();
	m_DepthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(m_SwapChain->ResizeBuffers(
		m_SwapChainBufferCount,
		m_App->GetWindowWidth(), m_App->GetWindowHeight(),
		m_BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_CurrentBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < m_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_D3DDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_App->GetWindowWidth();
	depthStencilDesc.Height = m_App->GetWindowHeight();
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_D3DDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, GetDepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	ExecuteCommandLists();

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_App->GetWindowWidth());
	m_ScreenViewport.Height = static_cast<float>(m_App->GetWindowHeight());
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_App->GetWindowWidth(), m_App->GetWindowHeight() };
}

void CRenderer::ResetFence()
{
	
}

void CRenderer::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_CurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

// CommandObjects操作
void CRenderer::RestDirectCmdListAlloc()
{
	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));
}

void CRenderer::ExecuteCommandLists()
{
	// Execute the initialization commands.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

// CommonResources生成
void CRenderer::CreateCommonResources()
{
	RestDirectCmdListAlloc();

	CShaderManager::LoadShaders();
	CTextureManager::LoadTextures(m_D3DDevice.Get(), m_CommandList.Get());
	CMaterialManager::CreateMaterials();
	CGeoShapeManager::CreateGeoShapes(m_D3DDevice.Get(), m_CommandList.Get());
	CreateRootSignature();
	CreateDescriptorHeaps();
	CreataPSOs();

	m_SkyTextureDescriptorHandle = CreateCubeMapDescriptorHandle(CTextureManager::GetSkyTextureIndex());
	// DCM
	//m_DynamicCubeMapDescHandle = CreateCubeMapDescriptorHandle(CTextureManager::GetDynamicTextureIndex());
	m_DynamicCubeMapsDescHandle.resize(CTextureManager::GetDynamicTextureNum());
	for (int i = 0; i < m_DynamicCubeMapsDescHandle.size(); ++i)
	{
		m_DynamicCubeMapsDescHandle[i] = CreateCubeMapDescriptorHandle(CTextureManager::GetDynamicTextureIndex() + i);
	}

	ExecuteCommandLists();

	// Wait until initialization is complete.
	FlushCommandQueue();
}

void CRenderer::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, CTextureManager::GetTextureNum() + CTextureManager::GetDynamicTextureNum(), 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsShaderResourceView(0, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_D3DDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void CRenderer::CreateDescriptorHeaps()
{
	// Create the SRV heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = CTextureManager::GetTextureNum() + CTextureManager::GetDynamicTextureNum();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_D3DDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	// Fill out the heap with actual descriptors.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	UINT skyTextureIndex = CTextureManager::GetSkyTextureIndex();
	UINT dynamicTextureIndex = CTextureManager::GetDynamicTextureIndex();
	auto textures = CTextureManager::GetTextures().data();

	for (unsigned int i = 0; i < skyTextureIndex; ++i)
	{
		auto tex = textures[i]->Resource;
		srvDesc.Format = tex->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
		m_D3DDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);
	}

	// TextureCube
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	for (unsigned int i = skyTextureIndex; i < dynamicTextureIndex; ++i)
	{
		auto tex = textures[i]->Resource;
		srvDesc.Format = tex->GetDesc().Format;
		srvDesc.TextureCube.MipLevels = tex->GetDesc().MipLevels;
		m_D3DDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hDescriptor);
		// next descriptor
		hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);
	}

	// DynamicCube
	if (m_DynamicCubeMapOn)
	{
		auto srvCpuStart = m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		auto srvGpuStart = m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		auto rtvCpuStart = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();

		// Cubemap RTV goes after the swap chain descriptors.
		int rtvOffset = m_SwapChainBufferCount;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cubeRtvHandles[6];
		for (int i = 0; i < 6; ++i)
		{
			cubeRtvHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvCpuStart, rtvOffset + i, m_RtvDescriptorSize);
		}

		// Dynamic cubemap SRV is after the sky SRV.
		// DCM
		//m_DynamicCubeMap->CreateDescriptors(
		//	CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, dynamicTextureIndex, m_CbvSrvUavDescriptorSize),
		//	CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, dynamicTextureIndex, m_CbvSrvUavDescriptorSize),
		//	cubeRtvHandles);

		m_DynamicCubeMap->CreateDescriptors(srvCpuStart, srvGpuStart, cubeRtvHandles, dynamicTextureIndex, m_CbvSrvUavDescriptorSize);

		CreateCubeDepthStencil();
	}
}

void CRenderer::CreateCubeDepthStencil()
{
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_DynamicCubeMapSize;
	depthStencilDesc.Height = m_DynamicCubeMapSize;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = m_DepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_DynamicCubeMapDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	m_D3DDevice->CreateDepthStencilView(m_DynamicCubeMapDepthStencilBuffer.Get(), nullptr, m_DynamicCubeMapDsvHandle);

	// Transition the resource from its initial state to be used as a depth buffer.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DynamicCubeMapDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void CRenderer::CreataPSOs()
{
	auto shaderTypes = CShaderManager::GetShaderTypes();

	// PSO for opaque objects.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout =
	{
		shaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].inputLayout->data(),
		(UINT)shaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].inputLayout->size()
	};
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(shaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].vertexShader->GetBufferPointer()),
		shaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].vertexShader->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(shaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].pixelShader->GetBufferPointer()),
		shaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].pixelShader->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
	ThrowIfFailed(m_D3DDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs[(int)PSOTypeIndex::PSO_00_Solid_Opaque])));

	// PSO for wireframe opaque objects.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframeOpaquePsoDesc = opaquePsoDesc;
	wireframeOpaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_D3DDevice->CreateGraphicsPipelineState(&wireframeOpaquePsoDesc, IID_PPV_ARGS(&m_PSOs[(int)PSOTypeIndex::PSO_01_WireFrame_Opaque])));

	// PSO for sky.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;

	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = m_RootSignature.Get();
	skyPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(shaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].vertexShader->GetBufferPointer()),
		shaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].vertexShader->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(shaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].pixelShader->GetBufferPointer()),
		shaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].pixelShader->GetBufferSize()
	};
	ThrowIfFailed(m_D3DDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&m_PSOs[(int)PSOTypeIndex::PSO_02_Solid_Sky])));

	// PSO for wireframe sky.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframeSkyPsoDesc = skyPsoDesc;
	wireframeSkyPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_D3DDevice->CreateGraphicsPipelineState(&wireframeSkyPsoDesc, IID_PPV_ARGS(&m_PSOs[(int)PSOTypeIndex::PSO_03_WireFrame_Sky])));
}

// ゲッター
D3D12_CPU_DESCRIPTOR_HANDLE CRenderer::GetCurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrentBackBuffer,
		m_RtvDescriptorSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CRenderer::CreateCubeMapDescriptorHandle(UINT Offset)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE cubeMapDescriptor(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	cubeMapDescriptor.Offset(Offset, m_CbvSrvUavDescriptorSize);
	return cubeMapDescriptor;
}

// デバッガ―
void CRenderer::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_DxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void CRenderer::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_BackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void CRenderer::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

// StaticSamplers
array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CRenderer::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

// 描画用
void CRenderer::Begin()
{
	auto cmdListAlloc = CFrameResourceManager::GetCurrentFrameResource()->CmdListAlloc;
	//auto cmdListAlloc = mDirectCmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs[(int)PSOTypeIndex::PSO_00_Solid_Opaque].Get()));
	m_CurrentPSO = (int)PSOTypeIndex::PSO_00_Solid_Opaque;

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
}

void CRenderer::SetUpCommonResources()
{
	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	auto matBuffer = CFrameResourceManager::GetCurrentFrameResource()->MaterialBuffer->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

	// Bind all the textures used in this scene.  Observe
	// that we only have to specify the first descriptor in the table.  
	// The root signature knows how many descriptors are expected in the table.
	m_CommandList->SetGraphicsRootDescriptorTable(4, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

void CRenderer::SetUpCubeMapResources()
{
	// Bind the sky cube map.  For our demos, we just use one "world" cube map representing the environment
	// from far away, so all objects will use the same cube map and we only need to set it once per-frame.  
	// If we wanted to use "local" cube maps, we would have to change them per-object, or dynamically
	// index into an array of cube maps.
	m_CommandList->SetGraphicsRootDescriptorTable(3, m_SkyTextureDescriptorHandle);
}

void CRenderer::SetUpDynamicCubeMapResources(int DCMResourcesIndex)
{
	// Use the dynamic cube map for the dynamic reflectors layer.
	// DCM
	//m_CommandList->SetGraphicsRootDescriptorTable(3, m_DynamicCubeMapDescHandle);
	//m_CommandList->SetGraphicsRootDescriptorTable(3, m_DynamicCubeMapsDescHandle[DCMResourcesIndex]);
	m_CommandList->SetGraphicsRootDescriptorTable(3, m_DynamicCubeMap->GetSrvHandle(DCMResourcesIndex));
}

void CRenderer::SetUpBeforeCreateAllDynamicCubeMapResources(int DCMResourcesIndex)
{
	m_CommandList->RSSetViewports(1, &m_DynamicCubeMap->GetViewport());
	m_CommandList->RSSetScissorRects(1, &m_DynamicCubeMap->GetScissorRect());

	// Change to RENDER_TARGET.
	// DCM
	//m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DynamicCubeMap->GetResource(),
	//	D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DynamicCubeMap->GetResource(DCMResourcesIndex),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void CRenderer::SetUpRtvBeforeCreateEachDynamicCubeMapResource(int DCMResourcesIndex, int FaceIndex)
{
	m_DynamicCubeMap->CreateRtvToEachCubeFace(DCMResourcesIndex, FaceIndex);
}

void CRenderer::SetUpBeforeCreateEachDynamicCubeMapResource(int i)
{
	// Clear the back buffer and depth buffer.
	m_CommandList->ClearRenderTargetView(m_DynamicCubeMap->GetRtvHandle(i), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(m_DynamicCubeMapDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1, &m_DynamicCubeMap->GetRtvHandle(i), true, &m_DynamicCubeMapDsvHandle);

	// Bind the pass constant buffer for this cube map face so we use 
	// the right view/proj matrix for this cube face.
	auto passCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + (1 + i) * CFrameResourceManager::GetPassCBByteSize();
	m_CommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);
}

void CRenderer::CompleteCreateDynamicCubeMapResources(int DCMResourcesIndex)
{
	// Change back to GENERIC_READ so we can read the texture in a shader.
	// DCM
	//m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DynamicCubeMap->GetResource(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DynamicCubeMap->GetResource(DCMResourcesIndex),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void CRenderer::SetUpBeforeDrawScene()
{
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	m_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1, &GetCurrentBackBufferView(), true, &GetDepthStencilView());

	auto passCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());
}

void CRenderer::SetPSO(int PSOType)
{
	if (m_CurrentPSO != PSOType)
	{
		m_CommandList->SetPipelineState(m_PSOs[PSOType].Get());
		m_CurrentPSO = PSOType;
	}
}

void CRenderer::DrawGameObjectsWithLayer(std::list<CGameObject*>& GameObjectsWithLayer)
{
	auto objectCB = CFrameResourceManager::GetCurrentFrameResource()->ObjectCB->Resource();

	for (CGameObject* gameObject : GameObjectsWithLayer)
	{
		DrawSingleGameObject(gameObject, objectCB);
	}
}

void CRenderer::DrawGameObjectsWithDynamicCubeMap(std::list<CGameObject*>& GameObjectsWithLayer)
{
	auto objectCB = CFrameResourceManager::GetCurrentFrameResource()->ObjectCB->Resource();
	int dcmResourcesIndex = 0;

	for (CGameObject* gameObject : GameObjectsWithLayer)
	{
		CRenderer::SetUpDynamicCubeMapResources(dcmResourcesIndex);
		CRenderer::DrawSingleGameObject(gameObject, objectCB);
		++dcmResourcesIndex;
	}
}

void CRenderer::DrawSingleGameObject(CGameObject* GameObject, ID3D12Resource* ObjectCB)
{
	m_CommandList->IASetVertexBuffers(0, 1, &GameObject->GetMeshGeometry()->VertexBufferView());
	m_CommandList->IASetIndexBuffer(&GameObject->GetMeshGeometry()->IndexBufferView());
	m_CommandList->IASetPrimitiveTopology(GameObject->GetPrimitiveTopology());

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = ObjectCB->GetGPUVirtualAddress() + GameObject->GetObjCBIndex() * CFrameResourceManager::GetObjCBByteSize();

	m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);

	m_CommandList->DrawIndexedInstanced(GameObject->GetIndexCount(), 1, GameObject->GetStartIndexLocation(), GameObject->GetBaseVertexLocation(), 0);
}

void CRenderer::End()
{
	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ExecuteCommandLists();

	// Swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % m_SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	CFrameResourceManager::GetCurrentFrameResource()->Fence = ++m_CurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);

	//FlushCommandQueue();
}