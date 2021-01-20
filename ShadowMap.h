#pragma once

#include "Common\\d3dUtil.h"

class CShadowMap
{
private:
	ID3D12Device* m_D3dDevice = nullptr;

	std::vector<D3D12_VIEWPORT> m_Viewports;
	std::vector<D3D12_RECT>     m_ScissorRects;

	UINT        m_Width = 0;
	UINT        m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R24G8_TYPELESS;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuDsvHandle;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ShadowMap = nullptr;

public:
	CShadowMap(ID3D12Device* Device, UINT Width, UINT Height, UINT CascadNum = 1);
	CShadowMap(const CShadowMap& rhs) = delete;
	CShadowMap& operator=(const CShadowMap& rhs) = delete;
	~CShadowMap() = default;

	void CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuDsvHandle);

	void OnResize(UINT NewWidth, UINT NewHeight);

	UINT GetWidth()const { return m_Width; }
	UINT GetHeight()const { return m_Height; }

	ID3D12Resource*               GetResource() { return m_ShadowMap.Get(); }
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrvHandle()const { return m_GpuSrvHandle; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsvHandle()const { return m_CpuDsvHandle; }

	D3D12_VIEWPORT GetViewport(int CascadIndex = 0)const { return m_Viewports[CascadIndex]; }
	D3D12_RECT     GetScissorRect(int CascadIndex = 0)const { return m_ScissorRects[CascadIndex]; }

private:
	void CreateResource();
	void CreateDescriptors();
};

 