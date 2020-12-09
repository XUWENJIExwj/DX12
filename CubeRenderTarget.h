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

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuRtvHandle[6];

	Microsoft::WRL::ComPtr<ID3D12Resource> m_CubeMapResource = nullptr;

public:
	CCubeRenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format);
	CCubeRenderTarget(const CCubeRenderTarget& rhs) = delete;
	CCubeRenderTarget& operator=(const CCubeRenderTarget& rhs) = delete;
	~CCubeRenderTarget() = default;

	ID3D12Resource*               GetResource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrv();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int FaceIndex);

	D3D12_VIEWPORT GetViewport()const;
	D3D12_RECT     GetScissorRect()const;

	void CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6]);

	void OnResize(UINT NewWidth, UINT NewHeight);

private:
	void CreateDescriptors();
	void CreateResource();
}; 