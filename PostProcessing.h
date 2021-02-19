#pragma once

#include "RadialBlur.h"
#include "GaussBlur.h"
#include "Bloom.h"

enum class PostProcessingType :int
{
	RadialBlur,
	GaussBlur,
	Bloom,
	Max
};

class CPostProcessing
{
private:
	ID3D12Device* m_D3DDevice = nullptr;

	UINT        m_Width = 0;
	UINT        m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	PostProcessingResource m_PPResource;
	std::vector<std::unique_ptr<CPostProcessingExecution>> m_PPExecutions;

public:
	CPostProcessing(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);
	CPostProcessing(const CPostProcessing& rhs) = delete;
	CPostProcessing& operator=(const CPostProcessing& rhs) = delete;
	~CPostProcessing() = default;

	void CreateDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
		UINT DescSize);

	void CreatePostProcessingExecution(int PPType, CPostProcessingExecution* PPExecution);

	void OnResize(UINT NewWidth, UINT NewHeight);

	void Execute(
		ID3D12GraphicsCommandList* CommandList,
		ID3D12RootSignature* RootSignature,
		ID3D12Resource* ResourceIn,
		int PPType, void* CB,
		const std::vector<ID3D12PipelineState*>& PSOs);

	void* GetSrvHandleAForImGui() { return (void*)m_PPResource.GpuSrvHandleA.ptr; }
	void* GetSrvHandleBForImGui() { return (void*)m_PPResource.GpuSrvHandleB.ptr; }
	void* GetSrvHandleCForImGui() { return (void*)m_PPResource.GpuSrvHandleC.ptr; }

private:
	void CreateResources();
	void CreateDescriptors();
};