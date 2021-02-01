#pragma once

#include "Common\\d3dUtil.h"

class CRadialBlur
{
private:
	ID3D12Device* m_D3DDevice = nullptr;

	UINT        m_Width = 0;
	UINT        m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandleIn;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuUavHandleIn;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandleOut;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuUavHandleOut;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandleIn;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuUavHandleIn;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandleOut;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuUavHandleOut;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_RadialBlurIn;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_RadialBlurOut;

public:
	CRadialBlur(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);
	CRadialBlur(const CRadialBlur& rhs) = delete;
	CRadialBlur& operator=(const CRadialBlur& rhs) = delete;
	~CRadialBlur() = default;

	ID3D12Resource* GetResource() { return m_RadialBlurOut.Get(); }

	void CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
		UINT DescSize);

	void OnResize(UINT NewWidth, UINT NewHeight);
	void Execute(
		ID3D12GraphicsCommandList* CommandList,
		ID3D12RootSignature* RootSignature,
		ID3D12PipelineState* RadialBlurPSO,
		ID3D12Resource* ResourceIn);

private:
	void CreateResources();
	void CreateDescriptors();
};