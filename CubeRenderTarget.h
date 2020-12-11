#pragma once

#include "Common\\d3dUtil.h"

enum class CubeMapFace :int
{
	PositiveX = 0,
	NegativeX = 1,
	PositiveY = 2,
	NegativeY = 3,
	PositiveZ = 4,
	NegativeZ = 5
};

class CCubeRenderTarget
{
private:
	ID3D12Device* m_D3DDevice = nullptr;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT     m_ScissorRect;

	UINT m_Width = 0;
	UINT m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// DCM
	//CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandle;
	//CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;

	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_CpuSrvHandles;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> m_GpuSrvHandles;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuRtvHandle[6];

	D3D12_RENDER_TARGET_VIEW_DESC m_RtvDesc;

	// DCM
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_CubeMapResource = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_CubeMapResources;

public:
	CCubeRenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format);
	CCubeRenderTarget(const CCubeRenderTarget& rhs) = delete;
	CCubeRenderTarget& operator=(const CCubeRenderTarget& rhs) = delete;
	~CCubeRenderTarget() = default;

	// DCM
	//void CreateDescriptors(
	//	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
	//	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
	//	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6]);

	void CreateDescriptors(
		D3D12_CPU_DESCRIPTOR_HANDLE SrvCPUStartHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE SrvGPUStartHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6],
		UINT Offset, UINT CbvSrvUavDescriptorSize);

	void OnResize(UINT NewWidth, UINT NewHeight);

	// DCM
	//ID3D12Resource*               GetResource() { return m_CubeMapResource.Get(); }
	ID3D12Resource*               GetResource(int Index) { return m_CubeMapResources[Index].Get(); }
	// DCM
	//CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrv() { return m_GpuSrvHandle; }
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrvHandle(int Index) { return m_GpuSrvHandles[Index]; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int FaceIndex) { return m_CpuRtvHandle[FaceIndex]; }

	D3D12_VIEWPORT GetViewport()const { return m_Viewport; }
	D3D12_RECT     GetScissorRect()const { return m_ScissorRect; }

	void CreateRtvToEachCubeFace(int DCMResourcesIndex, int FaceIndex);

private:
	void CreateResource();
	void CreateDescriptors();
	void CreateRtvDesc();
	void UpdateRtvDesc(int FaceIndex);
}; 