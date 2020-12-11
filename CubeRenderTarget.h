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

	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>              m_CpuSrvHandles;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE>              m_GpuSrvHandles;
	std::vector<std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>> m_CpuRtvHandles;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_CubeMapResources;

public:
	CCubeRenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format);
	CCubeRenderTarget(const CCubeRenderTarget& rhs) = delete;
	CCubeRenderTarget& operator=(const CCubeRenderTarget& rhs) = delete;
	~CCubeRenderTarget() = default;

	void CreateDescriptors(
		D3D12_CPU_DESCRIPTOR_HANDLE SrvCPUStartHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE SrvGPUStartHandle,
		std::vector<std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>>& RtvCpuHandles,
		UINT Offset, UINT CbvSrvUavDescriptorSize);

	void OnResize(UINT NewWidth, UINT NewHeight);

	ID3D12Resource*               GetResource(int DCMResourceIndex) { return m_CubeMapResources[DCMResourceIndex].Get(); }
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrvHandle(int DCMResourceIndex) { return m_GpuSrvHandles[DCMResourceIndex]; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int DCMResourceIndex, int FaceIndex) { return m_CpuRtvHandles[DCMResourceIndex][FaceIndex]; }

	D3D12_VIEWPORT GetViewport()const { return m_Viewport; }
	D3D12_RECT     GetScissorRect()const { return m_ScissorRect; }

private:
	void CreateResource();
	void CreateDescriptors();
}; 