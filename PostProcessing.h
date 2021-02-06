#pragma once

#include "Common\\d3dUtil.h"

struct RadialBlurCB
{
	bool RadialBlurOn = false;
	int CenterX = 0;
	int CenterY = 0;
	int SampleDistance = 100;
	int SampleStrength = 100;
};

class CPostProcessing
{
private:
	ID3D12Device* m_D3DDevice = nullptr;

	UINT        m_Width = 0;
	UINT        m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandleA;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuUavHandleA;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandleB;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuUavHandleB;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandleA;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuUavHandleA;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandleB;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuUavHandleB;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ResourceA;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ResourceB;

public:
	CPostProcessing(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);
	CPostProcessing(const CPostProcessing& rhs) = delete;
	CPostProcessing& operator=(const CPostProcessing& rhs) = delete;
	~CPostProcessing() = default;

	//ID3D12Resource* GetResource() { return m_ResourceB.Get(); }

	void CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
		UINT DescSize);

	void OnResize(UINT NewWidth, UINT NewHeight);
	void DoRadialBlur(
		ID3D12GraphicsCommandList* CommandList,
		ID3D12RootSignature* RootSignature,
		ID3D12PipelineState* RadialBlurPSO,
		ID3D12Resource* ResourceIn,
		RadialBlurCB& RadialBlurCBuffer);

private:
	void CreateResources();
	void CreateDescriptors();
};